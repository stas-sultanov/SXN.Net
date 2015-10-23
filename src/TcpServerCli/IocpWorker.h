#pragma once

#include "Stdafx.h"
#include "winsock.h"
#include "TcpServerException.h"
#include "RioBufferPool.h"
#include "Ovelapped.h"
#include "RioTask.h"
#include "ConnectionHandle.h"

using namespace System;
using namespace System::Threading;
using namespace System::Collections::Generic;

namespace SXN
{
	namespace Net
	{
		[System::Security::SuppressUnmanagedCodeSecurity]
		private ref class IocpWorker
		{
			private:

			const char* testMessage = "HTTP/1.1 200 OK\r\nServer:SXN.Ion\r\nContent-Length:0\r\nDate:Sat, 26 Sep 2015 17:45:57 GMT\r\n\r\n";

			#pragma region Fields

			/// <summary>
			/// The descriptor of the listening socket.
			/// </summary>
			SOCKET listenSocket;

			/// <summary>
			/// A reference to the object that provides work with the Winsock extensions.
			/// </summary>
			Winsock* winsock;

			/// <summary>
			/// The completion port of the Registered I/O operations.
			/// </summary>
			initonly HANDLE rioCompletionPort;

			/// <summary>
			/// The completion queue of the Registered I/O receive operations.
			/// </summary>
			initonly RIO_CQ rioCompletionQueue;

			/// <summary>
			/// The Registered I/O buffer pool.
			/// </summary>
			RioBufferPool* rioReceiveBufferPool;

			/// <summary>
			/// The Registered I/O buffer pool.
			/// </summary>
			RioBufferPool* rioSendBufferPool;

			/// <summary>
			/// The completion port of the disconnect operations.
			/// </summary>
			initonly HANDLE disconnectCompletionPort;

			/// <summary>
			/// The unique identifier of the worker.
			/// </summary>
			initonly Int32 Id;

			int connectionsCount;

			initonly Thread^ processRioOperationsThread;

			#pragma endregion

			internal:

			/// <summary>
			/// The collection of the connections.
			/// </summary>
			System::Collections::Concurrent::ConcurrentQueue<ConnectionHandle^>^ connectionsPool;

			/// <summary>
			/// The collection of the connections.
			/// </summary>
			array<ConnectionHandle^>^ connections;

			#pragma region Constructor & Destructor

			/// <summary>
			/// Initializes a new instance of the <see cref="IocpWorker" /> class.
			/// </summary>
			/// <param name="listenSocket">The descriptor of the listening socket.</param>
			/// <param name="winsock">A reference to the object that provides work with Winsock extensions.</param>
			/// <param name="id">The unique identifier of the worker.</param>
			/// <param name="segmentLength">The length of the segment.</param>
			/// <param name="connectionsCount">The count of the segments.</param>
			IocpWorker(SOCKET listenSocket, Winsock* winsock, Int32 id, UInt32 segmentLength, UInt32 connectionsCount)
			{
				this->Id = id;

				this->winsock = winsock;

				// set listen socket
				this->listenSocket = listenSocket;

				// set connections count
				this->connectionsCount = connectionsCount;

				{
					// create I/O completion port
					this->rioCompletionPort = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);

					// check if operation has failed
					if (rioCompletionPort == nullptr)
					{
						// get error code
						DWORD kernelErrorCode = ::GetLastError();

						// throw exception
						throw gcnew TcpServerException(kernelErrorCode);
					}

					// compose completion method structure
					RIO_NOTIFICATION_COMPLETION completionSettings;

					// set type to IOCP
					completionSettings.Type = RIO_IOCP_COMPLETION;

					// set IOCP handle to completion port
					completionSettings.Iocp.IocpHandle = rioCompletionPort;

					// set IOCP completion key to id of current worker
					// TODO: can be null
					completionSettings.Iocp.CompletionKey = (PVOID) id;

					// set IOCP overlapped to invalid
					completionSettings.Iocp.Overlapped = (LPOVERLAPPED)-1;

					// create the completion queue for the Registered I/O receive operations
					rioCompletionQueue = winsock->RIOCreateCompletionQueue(connectionsCount * 64, &completionSettings);

					// check if operation has failed
					if (rioCompletionQueue == RIO_INVALID_CQ)
					{
						// get error code
						WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

						// throw exception
						throw gcnew TcpServerException(winsockErrorCode);
					}
				}

				// create receive buffer pool
				{
					DWORD kernelErrorCode;

					int winsockErrorCode;

					rioReceiveBufferPool = RioBufferPool::Create(winsock, segmentLength, connectionsCount, kernelErrorCode, winsockErrorCode);

					// check if operation has failed
					if (rioReceiveBufferPool == nullptr)
					{
						// throw exception
						throw gcnew TcpServerException((WinsockErrorCode)winsockErrorCode, (int)kernelErrorCode);
					}
				}

				// create send buffer pool
				{
					DWORD kernelErrorCode;

					int winsockErrorCode;

					rioSendBufferPool = RioBufferPool::Create(winsock, segmentLength, connectionsCount, kernelErrorCode, winsockErrorCode);

					// check if operation has failed
					if (rioSendBufferPool == nullptr)
					{
						// throw exception
						throw gcnew TcpServerException((WinsockErrorCode)winsockErrorCode, (int)kernelErrorCode);
					}
				}

				connections = gcnew array<ConnectionHandle ^>(connectionsCount);

				// initialize connections
				for (auto connectionIndex = 0; connectionIndex < connectionsCount; connectionIndex++)
				{
					// get Registered IO receive buffer
					auto rioReceiveBuffer = rioReceiveBufferPool->GetBuffer(connectionIndex);

					// get Registered IO send buffer
					auto rioSendBuffer = rioSendBufferPool->GetBuffer(connectionIndex);

					// create connection handle
					auto connection = gcnew ConnectionHandle(connectionIndex, winsock, rioReceiveBuffer, rioSendBuffer);

					// add to collection
					connections[connectionIndex] = connection;

					// TODO: test data
					memcpy(rioSendBufferPool->GetBufferData(connectionIndex), testMessage, strlen(testMessage));
				}

				// initialize pool
				connectionsPool = gcnew System::Collections::Concurrent::ConcurrentQueue<ConnectionHandle^>(connections);

				{
					ThreadStart^ threadDelegate = gcnew ThreadStart(this, &IocpWorker::ProcessRioOperations);

					processRioOperationsThread = gcnew Thread(threadDelegate);

					processRioOperationsThread->Name = String::Format("RIO processing thread # {0}", id);

					processRioOperationsThread->IsBackground = true;

					processRioOperationsThread->Start();
				}
			}

			/// <summary>
			/// Releases all associated resources.
			/// </summary>
			~IocpWorker()
			{
				// release buffer pool
				delete rioReceiveBufferPool;

				// close completion queue
				winsock->RIOCloseCompletionQueue(rioCompletionQueue);

				// close completion port
				// ignore result
				::CloseHandle(rioCompletionPort);
			}

			#pragma endregion

			#pragma region Methods

				public:

			[System::Security::SuppressUnmanagedCodeSecurity]
			ConnectionHandle^ GetHandle(SOCKET connectionSocket)
			{
				// get connection handle
				ConnectionHandle^ connectionHandle;

				if (!connectionsPool->TryDequeue(connectionHandle))
				{
					::closesocket(connectionSocket);

					return nullptr;
				}

				// create request queue
				auto requestQueue = winsock->RIOCreateRequestQueue(connectionSocket, 32, 1, 32, 1, rioCompletionQueue, rioCompletionQueue, (PVOID)connectionHandle->Id);
				{
					// check if operation has failed
					if (requestQueue == RIO_INVALID_RQ)
					{
						// get error code
						auto winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

						// throw exception
						throw gcnew TcpServerException(winsockErrorCode);
					}
				}

				connectionHandle->Reuse(connectionSocket, requestQueue);

				/**

				// set output buffer to 0
				{
					int intValue = 0;

					auto setBufferResult = ::setsockopt(connectionSocket, SOL_SOCKET, SO_SNDBUF, (const char *)&intValue, sizeof(int));

					// check if operation has failed
					if (setBufferResult == SOCKET_ERROR)
					{
						// get error code
						auto winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

						// throw exception
						throw gcnew TcpServerException(winsockErrorCode);
					}
				}

				// disable nagle
				{
					BOOL boolValue = TRUE;

					int disableNagleResult = ::setsockopt(connectionSocket, IPPROTO_TCP, TCP_NODELAY, (const char *)&boolValue, sizeof(BOOL));

					// check if operation has failed
					if (disableNagleResult == SOCKET_ERROR)
					{
						// get error code
						WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

						// throw exception
						throw gcnew TcpServerException(winsockErrorCode);
					}
				}

				/**/


				return connectionHandle;
			}



			#pragma endregion

			[System::Security::SuppressUnmanagedCodeSecurity]
			inline void ProcessRioOperations()
			{
				// the number of bytes transferred during an I/O operation that has completed
				DWORD numberOfBytes = 0;

				// the completion key value associated with the file handle whose I/O operation has completed
				ULONG_PTR completionKey = 0;

				// the OVERLAPPED structure that was specified when the completed I/O operation was started.
				LPOVERLAPPED overlapped = nullptr;

				// array of the Registered IO results
				RIORESULT rioResults[1024];

				while (true)
				{
					// register the method to use for notification behavior with an I/O completion queue for use with the Winsock registered I/O extensions
					winsock->RIONotify(rioCompletionQueue);

					// dequeue completion status
					BOOL dequeueResult = ::GetQueuedCompletionStatus(rioCompletionPort, &numberOfBytes, &completionKey, &overlapped, WSA_INFINITE);

					// check if operation has failed
					if (dequeueResult == FALSE)
					{
						continue;
					}

					// dequeue Registered IO completion results
					ULONG receiveCompletionsCount;

					BOOL activatedCompletionPort = FALSE;

					while ((receiveCompletionsCount = winsock->RIODequeueCompletion(rioCompletionQueue, rioResults, 1024)) > 0)
					{
						for (int resultIndex = 0; resultIndex < receiveCompletionsCount; resultIndex++)
						{
							// get Registered IO result
							auto rioResult = rioResults[resultIndex];

							// get connection id
							auto connectionId = (ULONG) rioResult.RequestContext;

							// get connection
							auto connection = connections[connectionId];

							if (connection->State == Receiving)
							{
								// end receive
								connection->EndReceive(rioResult.BytesTransferred);
							}
							else if (connection->State == Sending)
							{
								// set connection state to sent
								//connection->state = SXN::Net::ConnectionState::Sent;
								connection->EndSend(rioResult.BytesTransferred);
							}
						}

						if (!activatedCompletionPort)
						{
							// register the method to use for notification behavior with an I/O completion queue for use with the Winsock registered I/O extensions
							winsock->RIONotify(rioCompletionQueue);

							activatedCompletionPort = TRUE;
						}
					}
				}
			}
		};
	}
}
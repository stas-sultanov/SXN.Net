#pragma once

#include "Stdafx.h"
#include "WinsockEx.h"
#include "TcpServerException.h"
#include "RioBufferPool.h"
#include "TcpConnection.h"
#include "Ovelapped.h"

extern void DoWork(SXN::Net::WinsockEx& winsockEx, HANDLE completionPort, RIO_CQ rioReciveCompletionQueue, RIO_CQ rioSendCompletionQueue, INT msgLen);

extern void DoOtherWork(SXN::Net::TcpConnection** connections, int connectionsCount, int msgLen);

using namespace System;
using namespace System::Threading;
using namespace System::Collections::Generic;

namespace SXN
{
	namespace Net
	{
		private ref class IocpWorker
		{
			private:

			#pragma region Fields

			/// <summary>
			/// The descriptor of the listening socket.
			/// <summary/>
			SOCKET listenSocket;

			/// <summary>
			/// A pointer to the object that provides work with Winsock extensions.
			/// </summary>
			WinsockEx* pWinsockEx;

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
			RioBufferPool* rioBufferPool;

			/// <summary>
			/// The completion port of the disconnect operations.
			/// </summary>
			initonly HANDLE disconnectCompletionPort;

			/// <summary>
			/// The unique identifier of the worker.
			/// </summary>
			initonly Int32 Id;

			/// <summary>
			/// The collection of the connections.
			/// </summary>
			TcpConnection** connections;

			int connectionsCount;

			initonly Thread^ processRioOperationsThread;

			initonly Thread^ processDisconnectOperationsThread;

			initonly Thread^ mainThread2;

			const char* testMessage = "HTTP/1.1 200 OK\r\nServer:SXN.Ion\r\nContent-Length:0\r\nDate:Sat, 26 Sep 2015 17:45:57 GMT\r\n\r\n";

			#pragma endregion

			internal:

			#pragma region Constructor & Destructor

			/// <summary>
			/// Initializes a new instance of the <see cref="IocpWorker" /> class.
			/// </summary>
			/// <param name="listenSocket">The descriptor of the listening socket.</param>
			/// <param name="pWinsockEx">A pointer to the object that provides work with Winsock extensions.</param>
			/// <param name="id">The unique identifier of the worker.</param>
			/// <param name="segmentLength">The length of the segment.</param>
			/// <param name="segmentsCount">The count of the segments.</param>
			/// <returns>
			/// An instance of <see cref="TryResult{T}" /> which encapsulates result of the operation.
			/// <see cref="TryResult{T}.Success" /> contains <c>true</c> if operation was successful, <c>false</c> otherwise.
			/// <see cref="TryResult{T}.Result" /> contains valid object if operation was successful, <c>null</c> otherwise.
			/// </returns>
			IocpWorker(SOCKET listenSocket, WinsockEx* pWinsockEx, Int32 id, UInt32 segmentLength, UInt32 connectionsCount)
			{
				// set listen socket
				this->listenSocket = listenSocket;

				// set winsock handle
				this->pWinsockEx = pWinsockEx;

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
					rioCompletionQueue = pWinsockEx->RIOCreateCompletionQueue(connectionsCount * 64, &completionSettings);

					// check if operation has failed
					if (rioCompletionQueue == RIO_INVALID_CQ)
					{
						// get error code
						WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

						// throw exception
						throw gcnew TcpServerException(winsockErrorCode);
					}
				}

				{
					// create I/O completion port
					this->disconnectCompletionPort = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);

					// check if operation has failed
					if (disconnectCompletionPort == nullptr)
					{
						// get error code
						DWORD kernelErrorCode = ::GetLastError();

						// throw exception
						throw gcnew TcpServerException(kernelErrorCode);
					}
				}

				// create buffer pool
				{
					DWORD kernelErrorCode;

					int winsockErrorCode;

					rioBufferPool = RioBufferPool::Create(*pWinsockEx, segmentLength, connectionsCount, kernelErrorCode, winsockErrorCode);

					// check if operation has failed
					if (rioBufferPool == nullptr)
					{
						// throw exception
						throw gcnew TcpServerException((WinsockErrorCode)winsockErrorCode, (int)kernelErrorCode);
					}
				}

				// initialize connections array
				connections = new TcpConnection*[connectionsCount];

				// initialize connections
				for (unsigned int index = 0; index < connectionsCount; index++)
				{
					// create connection
					TcpConnection* connection = CreateConnection(index, 24, 40);

					// add to collection
					connections[index] = connection;
				}

				{
					ThreadStart^ threadDelegate = gcnew ThreadStart(this, &IocpWorker::ProcessRioOperations);

					processRioOperationsThread = gcnew Thread(threadDelegate);

					processRioOperationsThread->Name = String::Format("RIO processing thread # {0}", id);

					processRioOperationsThread->IsBackground = true;

					processRioOperationsThread->Start();
				}

				{
					ThreadStart^ threadDelegate = gcnew ThreadStart(this, &IocpWorker::DoOtherWork);

					processDisconnectOperationsThread = gcnew Thread(threadDelegate);

					processDisconnectOperationsThread->Name = String::Format("disconnect processing thread # {0}", id);

					//processDisconnectOperationsThread->IsBackground = false;

					processDisconnectOperationsThread->Start();
				}
			}

			/// <summary>
			/// Releases all associated resources.
			/// </summary>
			~IocpWorker()
			{
				// release buffer pool
				delete rioBufferPool;

				// close completion queue
				pWinsockEx->RIOCloseCompletionQueue(rioCompletionQueue);

				// close completion port
				// ignore result
				::CloseHandle(rioCompletionPort);
			}

			#pragma endregion

			#pragma region Methods


			TcpConnection* CreateConnection(int connectionId, ULONG maxOutstandingReceive, ULONG maxOutstandingSend)
			{
				// create connection socket
				SOCKET connectionSocket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_REGISTERED_IO);

				// check if operation has failed
				if (connectionSocket == INVALID_SOCKET)
				{
					// get error code
					WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

					// throw exception
					throw gcnew TcpServerException(winsockErrorCode);
				}

				// associate the connection socket with the completion port
				HANDLE resultPort = ::CreateIoCompletionPort((HANDLE)connectionSocket, disconnectCompletionPort, 0, 0);

				// check if operation has succeed
				if ((resultPort == nullptr) || (resultPort != disconnectCompletionPort))
				{
					// get error code
					WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

					// throw exception
					throw gcnew TcpServerException(winsockErrorCode);
				}

				// create request queue
				RIO_RQ requestQueue = pWinsockEx->RIOCreateRequestQueue(connectionSocket, 24, 1, 40, 1, rioCompletionQueue, rioCompletionQueue, (PVOID)&connectionId);

				// check if operation has failed
				if (requestQueue == RIO_INVALID_RQ)
				{
					// get error code
					WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

					// throw exception
					throw gcnew TcpServerException(winsockErrorCode);
				}

				// set output buffer to 0
				{
					int intValue = 0;

					int setBufferResult = ::setsockopt(connectionSocket, SOL_SOCKET, SO_SNDBUF, (const char *)&intValue, sizeof(int));

					// check if operation has failed
					if (setBufferResult == SOCKET_ERROR)
					{
						// get error code
						WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

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

				// create connection handle
				TcpConnection* connection = new TcpConnection(*pWinsockEx, listenSocket, connectionSocket, requestQueue, rioCompletionPort);

				// set initial state
				connection->state = ConnectionState::Disconnected;

				connection->StartAccept();

				connection->receiveBuffer = rioBufferPool->GetReceiveBuffer(connectionId);

				connection->sendBuffer = rioBufferPool->GetSendBuffer(connectionId);

				memcpy(rioBufferPool->GetSendData(connectionId), testMessage, strlen(testMessage));

				return connection;
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

				while (true)
				{
					// register the method to use for notification behavior with an I/O completion queue for use with the Winsock registered I/O extensions
					pWinsockEx->RIONotify(rioCompletionQueue);

					// dequeue completion status
					BOOL dequeueResult = ::GetQueuedCompletionStatus(rioCompletionPort, &numberOfBytes, &completionKey, &overlapped, WSA_INFINITE);

					// check if operation has failed
					if (dequeueResult == FALSE)
					{
						continue;
					}

					// dequeue Registered IO completion results
					ULONG receiveCompletionsCount;

					// array of the Registered IO results
					RIORESULT rioResults[1024];

					BOOL activatedCompletionPort = FALSE;

					while ((receiveCompletionsCount = pWinsockEx->RIODequeueCompletion(rioCompletionQueue, rioResults, 1024)) > 0)
					{
						for (int resultIndex = 0; resultIndex < receiveCompletionsCount; resultIndex++)
						{
							// get result
							RIORESULT result = rioResults[resultIndex];

							if (result.RequestContext < 0)
							{
								continue;
							}

							// get connection
							SXN::Net::TcpConnection* connection = (SXN::Net::TcpConnection *)result.RequestContext;

							// set connection state to received
							connection->state = SXN::Net::ConnectionState::Received;

							//connection->StartSend(strlen(testMessage));

							System::Console::WriteLine("IOCP Thread: {0} - Connection: {1} - RECEIVED", Id, connection->connectionSocket);
						}

						if (!activatedCompletionPort)
						{
							// register the method to use for notification behavior with an I/O completion queue for use with the Winsock registered I/O extensions
							pWinsockEx->RIONotify(rioCompletionQueue);

							activatedCompletionPort = TRUE;
						}
					}
				}
			}

			//[System::Security::SuppressUnmanagedCodeSecurity]
			inline void DoOtherWork()
			{
				DWORD msgLen = strlen(testMessage);

				while (true)
				{
					Sleep(1);

					for (unsigned int connectionIndex = 0; connectionIndex < connectionsCount; connectionIndex++)
					{
						// get connection
						SXN::Net::TcpConnection* connection = connections[connectionIndex];

						switch (connection->state)
						{
							/**
							case SXN::Net::ConnectionState::Accepted:
							{
								// s accept
								int endAcceptResult = connection->EndAccepet();

								// check if operation has failed
								if (endAcceptResult == SOCKET_ERROR)
								{
									// get error code
									int winsockErrorCode = ::WSAGetLastError();

									return;
								}

								// start asynchronous receive operation
								connection->StartRecieve();

								//System::Console::WriteLine("IOCP Thread: {0} - Connection: {1} - ACCEPT", Id, (Int32)overlapped->connectionId);

								break;
							}
							/**/

							case SXN::Net::ConnectionState::Received:
							{
								System::Console::WriteLine("IOCP Thread: {0} - Connection: {1} - RIO SENDING", Id, connection->connectionSocket);

								// start asynchronous send operation
								connection->StartSend(msgLen);

								connection->state = SXN::Net::ConnectionState::Sent;

								System::Console::WriteLine("IOCP Thread: {0} - Connection: {1} - RIO SENT", Id, connection->connectionSocket);

								break;
							}

							case SXN::Net::ConnectionState::Sent:
							{
								System::Console::WriteLine("IOCP Thread: {0} - Connection: {1} - RIO DISCONNECTING", Id, connection->connectionSocket);

								// start asynchronous disconnect operation
								connection->StartDisconnect();

								connection->state = SXN::Net::ConnectionState::Disconnected;

								System::Console::WriteLine("IOCP Thread: {0} - Connection: {1} - RIO DISCONNECTED", Id, connection->connectionSocket);

								//System::Console::WriteLine("IOCP Thread: {0} - Connection: {1} - RIO SEND, BytesTransferred {2}, SocketContext {3}, Status {4}", Id, result.RequestContext, result.BytesTransferred, result.SocketContext, (WinsockErrorCode) result.Status);

								break;
							}

							case SXN::Net::ConnectionState::Disconnected:
							{
								System::Console::WriteLine("IOCP Thread: {0} - Connection: {1} - ACCEPTING", Id, connection->connectionSocket);

								// start asynchronous accept operation
								connection->StartAccept();

								break;
							}
						}
					}
				}
			}
		};
	}
}
#pragma once

#include "Stdafx.h"
#include "WinsockEx.h"
#include "TcpServerException.h"
#include "RioBufferPool.h"
#include "TcpConnection.h"
#include "Ovelapped.h"

extern void DoWork(SXN::Net::WinsockEx& winsockEx, HANDLE completionPort, RIO_CQ rioReciveCompletionQueue, RIO_CQ rioSendCompletionQueue, INT msgLen);

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
			/// The completion port of the of the Registered I/O operations.
			/// </summary>
			initonly HANDLE completionPort;

			/// <summary>
			/// The unique identifier of the worker.
			/// </summary>
			initonly Int32 Id;

			/// <summary>
			/// The completion queue of the Registered I/O receive operations.
			/// </summary>
			initonly RIO_CQ rioReciveCompletionQueue;

			/// <summary>
			/// The completion queue of the Registered I/O send operations.
			/// </summary>
			initonly RIO_CQ rioSendCompletionQueue;

			/// <summary>
			/// The collection of the connections.
			/// </summary>
			TcpConnection** connections;

			/// <summary>
			/// The Registered I/O buffer pool.
			/// </summary>
			RioBufferPool* receiveBufferPool;

			/// <summary>
			/// The Registered I/O buffer pool.
			/// </summary>
			RioBufferPool* sendBufferPool;

			initonly Thread^ mainThread;

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
			IocpWorker(SOCKET listenSocket, WinsockEx* pWinsockEx, Int32 id, UInt32 segmentLength, UInt32 segmentsCount)
			{
				// set listen socket
				this->listenSocket = listenSocket;

				// set winsock handle
				this->pWinsockEx = pWinsockEx;

				// create I/O completion port
				this->completionPort = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);

				// check if operation has failed
				if (completionPort == nullptr)
				{
					// get error code
					DWORD kernelErrorCode = ::GetLastError();

					// throw exception
					throw gcnew TcpServerException(kernelErrorCode);
				}

				#pragma region create Registered I/O completion queue

				{
					// compose completion method structure
					RIO_NOTIFICATION_COMPLETION completionSettings;

					// set type to IOCP
					completionSettings.Type = RIO_IOCP_COMPLETION;

					// set IOCP handle to completion port
					completionSettings.Iocp.IocpHandle = completionPort;

					// set IOCP completion key to id of current worker
					completionSettings.Iocp.CompletionKey = (PVOID)SOCK_ACTION_RECEIVE;

					// set IOCP overlapped to invalid
					completionSettings.Iocp.Overlapped = (LPOVERLAPPED)-1;

					// create the completion queue for the Registered I/O receive operations
					rioReciveCompletionQueue = pWinsockEx->RIOCreateCompletionQueue(segmentsCount * 4, &completionSettings);

					// check if operation has failed
					if (rioReciveCompletionQueue == RIO_INVALID_CQ)
					{
						// get error code
						WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

						// throw exception
						throw gcnew TcpServerException(winsockErrorCode);
					}
				}

				{
					// compose completion method structure
					RIO_NOTIFICATION_COMPLETION completionSettings;

					// set type to IOCP
					completionSettings.Type = RIO_IOCP_COMPLETION;

					// set IOCP handle to completion port
					completionSettings.Iocp.IocpHandle = completionPort;

					// set IOCP completion key to id of current worker
					completionSettings.Iocp.CompletionKey = (PVOID)SOCK_ACTION_SEND;

					// set IOCP overlapped to invalid
					completionSettings.Iocp.Overlapped = (LPOVERLAPPED) -1;

					// create the completion queue for the Registered I/O send operations
					rioSendCompletionQueue = pWinsockEx->RIOCreateCompletionQueue(segmentsCount * 4, &completionSettings);

					// check if operation has failed
					if (rioSendCompletionQueue == RIO_INVALID_CQ)
					{
						// get error code
						WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

						// throw exception
						throw gcnew TcpServerException(winsockErrorCode);
					}
				}
				/**/

				#pragma endregion

				// create buffer pool
				{
					DWORD kernelErrorCode;

					int winsockErrorCode;

					receiveBufferPool = RioBufferPool::Create(*pWinsockEx, segmentLength, segmentsCount, kernelErrorCode, winsockErrorCode);

					// check if operation has failed
					if (receiveBufferPool == nullptr)
					{
						// throw exception
						throw gcnew TcpServerException((WinsockErrorCode)winsockErrorCode, (int)kernelErrorCode);
					}

					sendBufferPool = RioBufferPool::Create(*pWinsockEx, segmentLength, segmentsCount, kernelErrorCode, winsockErrorCode);

					// check if operation has failed
					if (sendBufferPool == nullptr)
					{
						// throw exception
						throw gcnew TcpServerException((WinsockErrorCode)winsockErrorCode, (int)kernelErrorCode);
					}
				}

				// initialize connections array
				connections = new TcpConnection*[segmentsCount];

				// initialize connections
				for (unsigned int index = 0; index < segmentsCount; index++)
				{
					// create connection
					TcpConnection* connection = CreateConnection(index, 1, 1);

					// add to collection
					connections[index] = connection;
				}

				ThreadStart^ threadDelegate = gcnew ThreadStart(this, &IocpWorker::DoWork2);

				mainThread = gcnew Thread(threadDelegate);

				mainThread->Start();
			}

			/// <summary>
			/// Releases all associated resources.
			/// </summary>
			~IocpWorker()
			{
				// release buffer pool
				delete receiveBufferPool;

				// close completion queue
				pWinsockEx->RIOCloseCompletionQueue(rioReciveCompletionQueue);

				// close completion port
				// ignore result
				::CloseHandle(completionPort);
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
				HANDLE resultPort = ::CreateIoCompletionPort((HANDLE)connectionSocket, completionPort, SOCK_ACTION_DISCONNECT, 0);

				// check if operation has succeed
				if ((resultPort == nullptr) || (resultPort != completionPort))
				{
					// get error code
					WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

					// throw exception
					throw gcnew TcpServerException(winsockErrorCode);
				}

				// create request queue
				RIO_RQ requestQueue = pWinsockEx->RIOCreateRequestQueue(connectionSocket, maxOutstandingReceive, 1, maxOutstandingSend, 1, rioReciveCompletionQueue, rioSendCompletionQueue, (PVOID)&connectionId);

				// check if operation has failed
				if (requestQueue == RIO_INVALID_RQ)
				{
					// get error code
					WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

					// throw exception
					throw gcnew TcpServerException(winsockErrorCode);
				}

				// create connection handle
				TcpConnection* connection = new TcpConnection(*pWinsockEx, listenSocket, connectionSocket, requestQueue, completionPort);

				BOOL acceptResult = connection->StartAccept();

				// check if operation has succeed
				if (acceptResult == FALSE)
				{
					// get error code
					WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

					if (winsockErrorCode != WinsockErrorCode::IoPending)
					{
						// throw exception
						throw gcnew TcpServerException(winsockErrorCode);
					}
				}

				connection->receiveBuffer = receiveBufferPool->GetBuffer(connectionId);

				connection->sendBuffer = sendBufferPool->GetBuffer(connectionId);

				memcpy(sendBufferPool->GetData(connectionId), testMessage, strlen(testMessage));

				return connection;
			}

			#pragma endregion

			[System::Security::SuppressUnmanagedCodeSecurity]
			inline void DoWork2()
			{
				::DoWork(*pWinsockEx, completionPort, rioReciveCompletionQueue, rioSendCompletionQueue, strlen(testMessage));
			}

			[System::Security::SuppressUnmanagedCodeSecurity]
			void DoWork()
			{
				// define array of completion entries
				OVERLAPPED_ENTRY completionPortEntries[1024];

				// define array of the Registered IO results
				RIORESULT rioResults[1024];

				int rioRecieveNotify = this->pWinsockEx->RIONotify(rioReciveCompletionQueue);

				/*
				if (rioRecieveNotify != ERROR_SUCCESS)
				{
				System::Console::WriteLine("RIONotify:recive:{0}", (WinsockErrorCode) rioRecieveNotify);
				}*/

				int rioSendNotify = this->pWinsockEx->RIONotify(rioSendCompletionQueue);

				/**
				if (rioSendNotify != ERROR_SUCCESS)
				{
				System::Console::WriteLine("RIONotify:send:{0}", (WinsockErrorCode)rioSendNotify);
				}
				/**/

				// will contain number of entries removed from the completion queue
				ULONG numEntriesRemoved;

				while (true)
				{
					// dequeue completion status
					BOOL dequeueResult = ::GetQueuedCompletionStatusEx(completionPort, completionPortEntries, 1024, &numEntriesRemoved, WSA_INFINITE, FALSE);

					// check if operation has failed
					if (dequeueResult == FALSE)
					{
						continue;
						// TODO: ABORT
					}

					for (unsigned int entryIndex = 0; entryIndex < numEntriesRemoved; entryIndex++)
					{
						// get entry
						OVERLAPPED_ENTRY entry = completionPortEntries[entryIndex];

						switch (entry.lpCompletionKey)
						{
							case SOCK_ACTION_ACCEPT:
							{
								// get overlapped
								Ovelapped* overlapped = (Ovelapped *)entry.lpOverlapped;

								// end accept
								int endAcceptResult = overlapped->connection->EndAccepet();

								// check if operation has failed
								if (endAcceptResult == SOCKET_ERROR)
								{
									// get error code
									WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

									System::Console::WriteLine("set accept status error: {0}", winsockErrorCode);
								}

								// start asynchronous receive operation
								overlapped->connection->StartRecieve();

								//System::Console::WriteLine("IOCP Thread: {0} - Connection: {1} - ACCEPT", Id, (Int32)overlapped->connectionId);

								break;
							}

							case SOCK_ACTION_DISCONNECT:
							{
								// get overlapped
								Ovelapped* overlapped = (Ovelapped *)entry.lpOverlapped;

								// start asynchronous accept operation
								overlapped->connection->StartAccept();

								//System::Console::WriteLine("IOCP Thread: {0} - Connection: {1} - DISCONNECT", Id, over2->connectionId);

								break;
							}

							case SOCK_ACTION_RECEIVE:
							{
								// dequeue Registered IO completion results
								INT receiveCompletionsCount = pWinsockEx->RIODequeueCompletion(rioReciveCompletionQueue, rioResults, 1024);

								for (int resultIndex = 0; resultIndex < receiveCompletionsCount; resultIndex++)
								{
									// get result
									RIORESULT result = rioResults[resultIndex];

									// get connection
									TcpConnection* connection = (TcpConnection *)result.RequestContext;

									// check if client has requested the disconnect by sending 0 bytes
									if (result.BytesTransferred == 0)
									{
										// post disconnect operation
										connection->StartDisconnect();

										continue;
									}

									// start asynchronous send operation
									connection->StartSend(strlen(testMessage));

									//System::Console::WriteLine("IOCP Thread: {0} - Connection: {1} - RIO RECEIVE, BytesTransferred {2}, SocketContext {3}, Status {4}", Id, result.RequestContext, result.BytesTransferred, result.SocketContext, (WinsockErrorCode) result.Status);
								}

								this->pWinsockEx->RIONotify(rioReciveCompletionQueue);

								break;
							}

							case SOCK_ACTION_SEND:
							{
								// dequeue Registered IO completion results
								int sendCompletionsCount = pWinsockEx->RIODequeueCompletion(rioSendCompletionQueue, rioResults, 1024);

								for (int resultIndex = 0; resultIndex < sendCompletionsCount; resultIndex++)
								{
									// get result
									RIORESULT result = rioResults[resultIndex];

									// get connection
									TcpConnection* connection = (TcpConnection *)result.RequestContext;

									// start asynchronous disconnect operation
									connection->StartDisconnect();

									//System::Console::WriteLine("IOCP Thread: {0} - Connection: {1} - RIO SEND, BytesTransferred {2}, SocketContext {3}, Status {4}", Id, result.RequestContext, result.BytesTransferred, result.SocketContext, (WinsockErrorCode) result.Status);
								}

								this->pWinsockEx->RIONotify(rioSendCompletionQueue);

								break;
							}

							default:
							{
								//System::Console::WriteLine("IOCP Thread: {0} something other, iocp_port: {1} num_bytes: {2} key: {3}", Id, (Int32)completionPort, (Int32)numberOfBytes, (Int32)completionKey);

								break;
							}
						}
					}
				}
			}
		};
	}
}
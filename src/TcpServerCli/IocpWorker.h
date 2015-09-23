#pragma once

#include "Stdafx.h"
#include "WinsockEx.h"
#include "TcpServerException.h"
#include "RioBufferPool.h"
#include "TcpConnection.h"
#include "Overlap.h"

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
			/// The completion port of the listening socket.
			/// </summary>
			//initonly HANDLE completionPort;

			/// <summary>
			/// A pointer to the object that provides work with Winsock extensions.
			/// </summary>
			WinsockEx* pWinsockEx;

			/// <summary>
			/// The unique identifier of the worker.
			/// </summary>
			initonly Int32 Id;


			/// <summary>
			/// The completion port of the of the Registered I/O operations.
			/// </summary>
			initonly HANDLE rioCompletionPort;

			/// <summary>
			/// The completion queue of the Registered I/O receive operations.
			/// </summary>
			initonly RIO_CQ rioReciveCompletionQueue;

			/// <summary>
			/// The completion queue of the Registered I/O send operations.
			/// </summary>
			initonly RIO_CQ rioSendCompletionQueue;

			/// <summary>
			/// The dictionary of the connections.
			/// </summary>
			// ConcurrentDictionary<UInt64, TcpConnection> Connections;
			initonly array<TcpConnection ^>^ connections;

			/// <summary>
			/// The Registered I/O buffer pool.
			/// </summary>
			initonly RioBufferPool^ bufferPool;

			initonly Thread^ mainThread;

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
			IocpWorker(SOCKET listenSocket, HANDLE completionPort, WinsockEx* pWinsockEx, Int32 id, UInt32 segmentLength, UInt32 segmentsCount)
			{
				// set listen socket
				this->listenSocket = listenSocket;

				// set completion port
				//this->completionPort = completionPort;

				// set winsock handle
				this->pWinsockEx = pWinsockEx;

				// create I/O completion port
				rioCompletionPort = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
				//rioCompletionPort = completionPort;

				// check if operation has failed
				if (rioCompletionPort == NULL)
				{
					// get error code
					DWORD kernelErrorCode = ::GetLastError();

					// throw exception
					throw gcnew TcpServerException(kernelErrorCode);
				}

				#pragma region create Registered I/O completion queue

				// compose completion method structure
				RIO_NOTIFICATION_COMPLETION completionSettings;

				// set type to IOCP
				completionSettings.Type = RIO_IOCP_COMPLETION;

				// set IOCP handle to completion port
				completionSettings.Iocp.IocpHandle = rioCompletionPort;

				// set IOCP completion key to id of current worker
				completionSettings.Iocp.CompletionKey = (PVOID) id;

				// TODO: fix this
				// set IOCP overlapped to invalid
				completionSettings.Iocp.Overlapped = new OVERLAPPED();

				// create the completion queue for the Registered I/O receive operations
				rioReciveCompletionQueue = pWinsockEx->RIOCreateCompletionQueue(segmentsCount, &completionSettings);

				// check if operation has failed
				if (rioReciveCompletionQueue == RIO_INVALID_CQ)
				{
					// get error code
					WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

					// throw exception
					throw gcnew TcpServerException(winsockErrorCode);
				}

				// create the completion queue for the Registered I/O send operations
				rioSendCompletionQueue = pWinsockEx->RIOCreateCompletionQueue(segmentsCount, &completionSettings);

				// check if operation has failed
				if (rioReciveCompletionQueue == RIO_INVALID_CQ)
				{
					// get error code
					WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

					// throw exception
					throw gcnew TcpServerException(winsockErrorCode);
				}

				#pragma endregion

				// create buffer pool
				bufferPool = gcnew RioBufferPool(pWinsockEx, segmentLength, segmentsCount * 2);

				// initialize connections array
				connections = gcnew array<TcpConnection^>(segmentsCount);

				// initialize connections
				for (int index = 0; index < segmentsCount; index++)
				{
					// create connection
					TcpConnection^ connection = CreateConnection(id, index);

					// add to collection
					connections[index] = connection;
				}

				ThreadStart^ threadDelegate = gcnew ThreadStart(this, &IocpWorker::DoWork);

				mainThread = gcnew Thread(threadDelegate);

				mainThread->Start();
			}

			/// <summary>
			/// Releases all associated resources.
			/// </summary>
			~IocpWorker()
			{
				// release buffer pool
				delete bufferPool;

				// close completion queue
				pWinsockEx->RIOCloseCompletionQueue(rioReciveCompletionQueue);

				// close completion port
				// ignore result
				::CloseHandle(rioCompletionPort);
			}

			#pragma endregion

			#pragma region Methods

			void DoWork()
			{
				RIORESULT* results = new RIORESULT[128];

				while (true)
				{
					int rioRecieveNotify = this->pWinsockEx->RIONotify(rioReciveCompletionQueue);

					if (rioRecieveNotify != ERROR_SUCCESS)
					{
						System::Console::WriteLine("RIONotify:recive:{0}", (WinsockErrorCode) rioRecieveNotify);
					}

					int rioSendNotify = this->pWinsockEx->RIONotify(rioSendCompletionQueue);

					if (rioSendNotify != ERROR_SUCCESS)
					{
						System::Console::WriteLine("RIONotify:send:{0}", (WinsockErrorCode)rioSendNotify);
					}

					DWORD numberOfBytes;

					ULONG_PTR completionKey;

					LPOVERLAPPED over;

					BOOL res = ::GetQueuedCompletionStatus(rioCompletionPort, &numberOfBytes, &completionKey, &over, 1000);
				
					if (res)
					{
						System::Console::WriteLine("iocp_port: {0} num_bytes: {1} key: {2} commmand : {3} connectionid : {4}", (Int32)rioCompletionPort, (Int32)numberOfBytes, (Int32)completionKey);
					}
					else
					{
						//System::Console::WriteLine("GetQueuedCompletionStatus: fail");
					}

					// dequeue Registered IO completion results
					INT rioDequeue = pWinsockEx->RIODequeueCompletion(rioReciveCompletionQueue, results, 128);

					System::Console::WriteLine("RIODequeueCompletion count {0}", rioDequeue);

					for (int resultIndex = 0; resultIndex < rioDequeue; resultIndex++)
					{
						RIORESULT result = results[resultIndex];

						System::Console::WriteLine("BytesTransferred {0}, RequestContext {1}, SocketContext {2}, Status {3}", result.BytesTransferred, result.RequestContext, result.SocketContext, result.Status);
	
						char *data = bufferPool->GetData(result.RequestContext);

						String^ s = System::Runtime::InteropServices::Marshal::PtrToStringAnsi( IntPtr( data));

						System::Console::WriteLine("Data :: {0}", s);

					}
				}
			}

			TcpConnection^ CreateConnection(int workerId, int connectionId)
			{
				// create connection socket
				SOCKET connectionSocket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_REGISTERED_IO);

				// check if operation has failed
				if (connectionSocket == INVALID_SOCKET)
				{
					// get error code
					WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

					// throw exception
					throw gcnew TcpServerException(winsockErrorCode);
				}

				WSAOVERLAPPEDPLUS* ov = new WSAOVERLAPPEDPLUS();

				memset(ov, 0, sizeof(WSAOVERLAPPEDPLUS));

				ov->action = SOCK_ACTION_ACCEPT;

				ov->connectionId = connectionId;

				ov->connectionSocket = connectionSocket;

				DWORD dwBytes;

				PVOID lpoutbuf = new char[(sizeof(sockaddr_in) + 16) * 2];

				// accept connections
				BOOL acceptResult = pWinsockEx->AcceptEx(listenSocket, connectionSocket, lpoutbuf, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &dwBytes, ov);

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

				/**
				// associate the connection socket with the completion port
				HANDLE resultPort = ::CreateIoCompletionPort((HANDLE) connectionSocket, rioCompletionPort, 0, 0);

				// check if operation has succeed
				if ((resultPort == NULL) || (resultPort != rioCompletionPort))
				{
					// get error code
					WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

					// throw exception
					throw gcnew TcpServerException(winsockErrorCode);
				}

				SOCKET tListenSocket = listenSocket;

				int iResult = ::setsockopt(connectionSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,	(char *)&tListenSocket, sizeof(SOCKET));
				
				/**/
				ULONG maxOutstandingReceive = 1u;

				ULONG maxOutstandingSend = 1u;

				// create request queue
				RIO_RQ requestQueue = pWinsockEx->RIOCreateRequestQueue(connectionSocket, maxOutstandingReceive, (ULONG)1, maxOutstandingSend, (ULONG)1, rioReciveCompletionQueue, rioSendCompletionQueue, (PVOID)&connectionId);

				// check if operation has failed
				if (requestQueue == RIO_INVALID_RQ)
				{
					// get error code
					WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

					// throw exception
					throw gcnew TcpServerException(winsockErrorCode);
				}
				/**/

				PRIO_BUF rioRecivieBuffer = bufferPool->GetBuffer(connectionId);

				BOOL rioReceiveResult = pWinsockEx->RIOReceive(requestQueue, rioRecivieBuffer, 1, RIO_MSG_DONT_NOTIFY, (LPVOID)connectionId);

				// check if operation has failed
				if (rioReceiveResult == false)
				{
					// get error code
					WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

					// throw exception
					throw gcnew TcpServerException(winsockErrorCode);
				}

				// create connection handle
				TcpConnection^ connection = gcnew TcpConnection(connectionSocket, lpoutbuf, requestQueue, ov);

				// add connection into the collection of the connections
				//Connections.TryAdd(id, connection);

				return connection;
			}

			#pragma endregion

		};
	}
}
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
			/// The dictionary of the connections.
			/// </summary>
			// ConcurrentDictionary<UInt64, TcpConnection> Connections;
			initonly array<TcpConnection ^>^ connections;

			/// <summary>
			/// The Registered I/O buffer pool.
			/// </summary>
			initonly RioBufferPool^ receiveBufferPool;

			/// <summary>
			/// The Registered I/O buffer pool.
			/// </summary>
			initonly RioBufferPool^ sendBufferPool;

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
				this->completionPort = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

				// check if operation has failed
				if (completionPort == NULL)
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
				completionSettings.Iocp.IocpHandle = completionPort;

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

				/**/
				// create the completion queue for the Registered I/O send operations
				rioSendCompletionQueue = pWinsockEx->RIOCreateCompletionQueue(segmentsCount, &completionSettings);

				// check if operation has failed
				if (rioSendCompletionQueue == RIO_INVALID_CQ)
				{
					// get error code
					WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

					// throw exception
					throw gcnew TcpServerException(winsockErrorCode);
				}
				/**/

				#pragma endregion

				// create buffer pool

				receiveBufferPool = gcnew RioBufferPool(pWinsockEx, segmentLength, segmentsCount);

				sendBufferPool = gcnew RioBufferPool(pWinsockEx, segmentLength, segmentsCount);

				// initialize connections array
				connections = gcnew array<TcpConnection^>(segmentsCount);

				// initialize connections
				for (int index = 0; index < segmentsCount; index++)
				{
					// create connection
					TcpConnection^ connection = CreateConnection(id, index);

					connection->StartRecieve();

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
				delete receiveBufferPool;

				// close completion queue
				pWinsockEx->RIOCloseCompletionQueue(rioReciveCompletionQueue);

				// close completion port
				// ignore result
				::CloseHandle(completionPort);
			}

			#pragma endregion

			#pragma region Methods


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
				HANDLE resultPort = ::CreateIoCompletionPort((HANDLE) connectionSocket, completionPort, 0, 0);

				// check if operation has succeed
				if ((resultPort == NULL) || (resultPort != completionPort))
				{
					// get error code
					WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

					// throw exception
					throw gcnew TcpServerException(winsockErrorCode);
				}
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

				// create connection handle
				TcpConnection^ connection = gcnew TcpConnection(pWinsockEx, connectionId, connectionSocket, lpoutbuf, requestQueue, ov);

				connection->receiveBuffer = receiveBufferPool->GetBuffer(connectionId);

				connection->sendBuffer = sendBufferPool->GetBuffer(connectionId);

				memcpy(sendBufferPool->GetData(connectionId), testMessage, strlen(testMessage));

				return connection;
			}

			#pragma endregion

			void DoWork()
			{
				RIORESULT* results = new RIORESULT[128];

				while (true)
				{
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

					DWORD numberOfBytes;

					ULONG_PTR completionKey;

					LPOVERLAPPED over;

					BOOL res = ::GetQueuedCompletionStatus(completionPort, &numberOfBytes, &completionKey, &over, /*INFINITE*/ 2000);

					if (!res)
					{
						continue;
					}

					System::Console::WriteLine("Thread: {0} IOCP, iocp_port: {1} num_bytes: {2} key: {3}", Id, (Int32)completionPort, (Int32)numberOfBytes, (Int32)completionKey);

					// dequeue Registered IO completion results
					INT receiveCompletionsCount = pWinsockEx->RIODequeueCompletion(rioReciveCompletionQueue, results, 128);

					//System::Console::WriteLine("Thread: {0}, RIODequeueCompletion count {1}", this->Id, rioDequeue);
					for (int resultIndex = 0; resultIndex < receiveCompletionsCount; resultIndex++)
					{
						RIORESULT result = results[resultIndex];

						System::Console::WriteLine("Thread: {0} Receive, BytesTransferred {1}, RequestContext {2}, SocketContext {3}, Status {4}", Id, result.BytesTransferred, result.RequestContext, result.SocketContext, result.Status);

						int connectionId = result.RequestContext;

						//char *data = receiveBufferPool->GetData(connectionId);

						//String^ s = System::Runtime::InteropServices::Marshal::PtrToStringAnsi(IntPtr(data));

						//System::Console::WriteLine("Data :: {0}", s);

						TcpConnection^ x = connections[connectionId];

						x->StartSend(strlen(testMessage));


					}

					// dequeue Registered IO completion results
					int sendCompletionsCount = pWinsockEx->RIODequeueCompletion(rioSendCompletionQueue, results, 128);

					//System::Console::WriteLine("Thread: {0}, RIODequeueCompletion count {1}", this->Id, rioDequeue);

					for (int resultIndex = 0; resultIndex < sendCompletionsCount; resultIndex++)
					{
						RIORESULT result = results[resultIndex];

						System::Console::WriteLine("Thread: {0} Send, BytesTransferred {1}, RequestContext {2}, SocketContext {3}, Status {4}", Id, result.BytesTransferred, result.RequestContext, result.SocketContext, result.Status);
					
						int connectionId = result.RequestContext;

						TcpConnection^ x = connections[connectionId];

						BOOL disconnectResult = pWinsockEx->DisconnectEx(x->socket, nullptr, TF_REUSE_SOCKET, 0);

					}

					this->pWinsockEx->RIONotify(rioReciveCompletionQueue);

					this->pWinsockEx->RIONotify(rioSendCompletionQueue);


				}
			}
		};
	}
}
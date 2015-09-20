#pragma once

#include "Stdafx.h"
#include "WinsockEx.h"
#include "TcpServerException.h"
#include "RioBufferPool.h"
#include "TcpConnection.h"

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
			initonly SOCKET listenSocket;

			/// <summary>
			/// A pointer to the object that provides work with Winsock extensions.
			/// </summary>
			WinsockEx* pWinsockEx;

			/// <summary>
			/// The unique identifier of the worker.
			/// </summary>
			initonly Int32 Id;

			/// <summary>
			/// The I/O completion port.
			/// </summary>
			initonly HANDLE ioCompletionPort;

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
			IocpWorker(SOCKET listenSocket, WinsockEx* pWinsockEx, Int32 id, UInt32 segmentLength, UInt32 segmentsCount)
			{
				// set listen socket
				this->listenSocket = listenSocket;

				// set winsock handle
				this->pWinsockEx = pWinsockEx;

				// create I/O completion port
				ioCompletionPort = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

				// check if operation has failed
				if (ioCompletionPort == NULL)
				{
					// get error code
					DWORD kernelErrorCode = ::GetLastError();

					// throw exception
					throw gcnew TcpServerException(kernelErrorCode);
				}

				// associate the listening socket with the completion port
				HANDLE associateResult = ::CreateIoCompletionPort((HANDLE)listenSocket, ioCompletionPort, 0, 0);

				if ((associateResult == NULL) || (associateResult != ioCompletionPort))
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
				completionSettings.Iocp.IocpHandle = ioCompletionPort;

				// set IOCP completion key to id of current worker
				completionSettings.Iocp.CompletionKey = (PVOID) id;

				// set IOCP overlapped to invalid
				completionSettings.Iocp.Overlapped = (PVOID)-1;

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
				::CloseHandle(ioCompletionPort);
			}

			#pragma endregion

			#pragma region Methods

			void DoWork()
			{
				while (true)
				{
					DWORD numberOfBytes;

					ULONG_PTR completionKey;

					LPOVERLAPPED over;

					BOOL res = ::GetQueuedCompletionStatus(ioCompletionPort, &numberOfBytes, &completionKey, &over, WSA_INFINITE);
				
					System::Console::WriteLine("result: {0} port: {1} key: {2}", (Int64) res, (Int64) ioCompletionPort,(Int64) completionKey);
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

				LPOVERLAPPED ov = new OVERLAPPED();

				memset(ov, 0, sizeof(ov));

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

				// associate the connection socket with the completion port
				HANDLE resultPort = ::CreateIoCompletionPort((HANDLE) connectionSocket, ioCompletionPort, 0, 0);

				// check if operation has succeed
				if ((resultPort == NULL) || (resultPort != ioCompletionPort))
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

				// check if operation has succeed
				if (requestQueue == RIO_INVALID_RQ)
				{
					// get error code
					WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

					// throw exception
					throw gcnew TcpServerException(winsockErrorCode);
				}
				/**/

				// create connection handle
				TcpConnection^ connection = gcnew TcpConnection(connectionSocket, lpoutbuf, 0, ov);

				// add connection into the collection of the connections
				//Connections.TryAdd(id, connection);

				return connection;
			}

			#pragma endregion

		};
	}
}
#pragma once

#include "Stdafx.h"
#include "WinsockHandle.h"
#include "TcpServerException.h"
#include "RioBufferPool.h"
#include "TcpConnection.h"

using namespace System;
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
			/// A pointer to the object that provides work with Winsock extensions.
			/// </summary>
			initonly WinSocket^ serverSocket;

			/// <summary>
			/// The unique identifier of the worker within the <see cref = "TcpWorker"/>.
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

			#pragma endregion

			internal:

			#pragma region Constructor & Destructor

			/// <summary>
			/// Initializes a new instance of the <see cref="IocpWorker" /> class.
			/// </summary>
			/// <param name="serverSocket">A pointer to the object that provides work with Winsock extensions.</param>
			/// <param name="processorIndex">The index of the processor.</param>
			/// <param name="segmentLength">The length of the segment.</param>
			/// <param name="segmentsCount">The count of the segments.</param>
			/// <returns>
			/// An instance of <see cref="TryResult{T}" /> which encapsulates result of the operation.
			/// <see cref="TryResult{T}.Success" /> contains <c>true</c> if operation was successful, <c>false</c> otherwise.
			/// <see cref="TryResult{T}.Result" /> contains valid object if operation was successful, <c>null</c> otherwise.
			/// </returns>
			IocpWorker(WinSocket ^serverSocket, Int32 processorIndex, UInt32 segmentLength, UInt32 segmentsCount)
			{
				// set winsock handle
				this->serverSocket = serverSocket;

				// create I/O completion port
				ioCompletionPort = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);

				// check if operation has failed
				if (ioCompletionPort == NULL)
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
				completionSettings.Iocp.CompletionKey = (PVOID)processorIndex;

				// set IOCP overlapped to invalid
				completionSettings.Iocp.Overlapped = (PVOID)-1;

				// create the completion queue for the Registered I/O receive operations
				rioReciveCompletionQueue = serverSocket->RIOCreateCompletionQueue(segmentsCount, &completionSettings);

				// check if operation has failed
				if (rioReciveCompletionQueue == RIO_INVALID_CQ)
				{
					// get error code
					WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

					// throw exception
					throw gcnew TcpServerException(winsockErrorCode);
				}

				// create the completion queue for the Registered I/O send operations
				rioSendCompletionQueue = serverSocket->RIOCreateCompletionQueue(segmentsCount, &completionSettings);

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
				bufferPool = gcnew RioBufferPool(serverSocket, segmentLength, segmentsCount * 2);

				// initialize connections array
				connections = gcnew array<TcpConnection^>(segmentsCount);

				// initialize connections
				for (int index = 0; index < segmentsCount; index++)
				{
					// create connection
					TcpConnection^ connection = CreateConnection(index);

					// add to collection
					connections[index] = connection;
				}
			}

			/// <summary>
			/// Releases all associated resources.
			/// </summary>
			~IocpWorker()
			{
				// release buffer pool
				delete bufferPool;

				// close completion queue
				serverSocket->RIOCloseCompletionQueue(rioReciveCompletionQueue);

				// close completion port
				// ignore result
				::CloseHandle(ioCompletionPort);
			}

			#pragma endregion

			#pragma region Methods

			TcpConnection^ CreateConnection(int id)
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

				OVERLAPPED ov;
				memset(&ov, 0, sizeof(ov));

				DWORD dwBytes;

				char *lpoutbuf = new char[1024];

				// accept connections
				BOOL acceptResult = serverSocket->AcceptEx(connectionSocket, lpoutbuf, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &dwBytes, &ov);

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
					else
					{
						/**
						// Wait for AcceptEx events and also process WorkerRoutine() returns
						while (TRUE)
						{
							
							int Index = ::WSAWaitForMultipleEvents(1, &ov.hEvent, FALSE, WSA_INFINITE, TRUE);

							if (Index == WSA_WAIT_FAILED)
							{
								// get error code
								WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

								// throw exception
								throw gcnew TcpServerException(winsockErrorCode);
							}

							if (Index != WAIT_IO_COMPLETION)
							{
								// The AcceptEx() event is ready, break the wait loop
								break;
							}
						}/**/
					}
				}

				// associate the connection socket with the completion port
				HANDLE resultPort = ::CreateIoCompletionPort((HANDLE)connectionSocket, ioCompletionPort, NULL, 0);

				// check if operation has succeed
				if (resultPort == NULL)
				{
					// get error code
					WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

					// throw exception
					throw gcnew TcpServerException(winsockErrorCode);
				}

				ULONG maxOutstandingReceive = 1u;

				ULONG maxOutstandingSend = 1u;

				// create request queue
				RIO_RQ requestQueue = serverSocket->RIOCreateRequestQueue(maxOutstandingReceive, (ULONG)1, maxOutstandingSend, (ULONG)1, rioReciveCompletionQueue, rioSendCompletionQueue, (PVOID)&id);

				// check if operation has succeed
				if (requestQueue == RIO_INVALID_RQ)
				{
					// get error code
					WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

					// throw exception
					throw gcnew TcpServerException(winsockErrorCode);
				}

				TcpConnection^ connection = gcnew TcpConnection(connectionSocket, requestQueue);

				// add connection into the collection of the connections
				//Connections.TryAdd(id, connection);

				return connection;
			}

			#pragma endregion

		};
	}
}
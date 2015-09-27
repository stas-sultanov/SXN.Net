#pragma once

#include "Stdafx.h"

using namespace System;

namespace SXN
{
	namespace Net
	{
		/// <summary>
		/// Provides work with Winsock extensions.
		/// </summary>
		private class WinsockEx final
		{
			private:

			#pragma region Fields

			LPFN_ACCEPTEX pAcceptEx;

			LPFN_DISCONNECTEX pDisconnectEx;

			LPFN_GETACCEPTEXSOCKADDRS pGetAcceptExSockaddrs;

			LPFN_RIOCLOSECOMPLETIONQUEUE pRIOCloseCompletionQueue;

			LPFN_RIOCREATECOMPLETIONQUEUE pRIOCreateCompletionQueue;

			LPFN_RIOCREATEREQUESTQUEUE pRIOCreateRequestQueue;

			LPFN_RIODEQUEUECOMPLETION pRIODequeueCompletion;

			LPFN_RIODEREGISTERBUFFER pRIODeregisterBuffer;

			LPFN_RIONOTIFY pRIONotify;

			LPFN_RIORECEIVE pRIOReceive;

			LPFN_RIORECEIVEEX pRIOReceiveEx;

			LPFN_RIOREGISTERBUFFER pRIORegisterBuffer;

			LPFN_RIORESIZECOMPLETIONQUEUE pRIOResizeCompletionQueue;

			LPFN_RIORESIZEREQUESTQUEUE pRIOResizeRequestQueue;

			LPFN_RIOSEND pRIOSend;

			LPFN_RIOSENDEX pRIOSendEx;

			#pragma endregion

			#pragma region Constructor

			/// <summary>
			/// Initializes a new instance of the <see cref="WinSocket" /> class.
			/// </summary>
			inline WinsockEx(LPFN_ACCEPTEX pAcceptEx, LPFN_DISCONNECTEX pDisconnectEx, LPFN_GETACCEPTEXSOCKADDRS pGetAcceptExSockaddrs, RIO_EXTENSION_FUNCTION_TABLE& rioTable)
			{
				this->pAcceptEx = pAcceptEx;

				this->pDisconnectEx = pDisconnectEx;

				this->pGetAcceptExSockaddrs = pGetAcceptExSockaddrs;

				this->pRIOCloseCompletionQueue = rioTable.RIOCloseCompletionQueue;

				this->pRIOCreateCompletionQueue = rioTable.RIOCreateCompletionQueue;

				this->pRIOCreateRequestQueue = rioTable.RIOCreateRequestQueue;

				this->pRIODequeueCompletion = rioTable.RIODequeueCompletion;

				this->pRIODeregisterBuffer = rioTable.RIODeregisterBuffer;

				this->pRIONotify = rioTable.RIONotify;

				this->pRIOReceive = rioTable.RIOReceive;

				this->pRIOReceiveEx = rioTable.RIOReceiveEx;

				this->pRIORegisterBuffer = rioTable.RIORegisterBuffer;

				this->pRIOResizeCompletionQueue = rioTable.RIOResizeCompletionQueue;

				this->pRIOResizeRequestQueue = rioTable.RIOResizeRequestQueue;

				this->pRIOSend = rioTable.RIOSend;

				this->pRIOSendEx = rioTable.RIOSendEx;
			}

			#pragma endregion

			#pragma region Methods

			static inline int GetExtension(SOCKET socket, GUID extensionId, LPVOID ptr)
			{
				// will contain actual pointer size
				DWORD actualPtrSize;

				// get function pointer
				return ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &extensionId, sizeof(GUID), ptr, sizeof(LPFN_ACCEPTEX), &actualPtrSize, NULL, NULL);
			}

			#pragma endregion

			public:

			#pragma region Methods

			/// <summary>
			/// Initializes a new instance of the <see cref="WinsockEx" /> class.
			/// </summary>
			/// <param name="socket">A descriptor that identifies the socket.</param>
			/// <returns>
			/// If the function succeeds, the return value is the pointer to the instance of the class.
			/// If the function fails, the return value is <c>null</c>.
			/// To get extended error information, call <see cref="WSAGetLastError" />.
			/// </returns>
			static WinsockEx* Initialize(SOCKET socket)
			{
				// get pointer to AcceptEx function
				LPFN_ACCEPTEX pAcceptEx;
				{
					// get pointer
					int getResult = GetExtension(socket, WSAID_ACCEPTEX, &pAcceptEx);

					// check if operation has failed
					if (getResult == SOCKET_ERROR)
					{
						return NULL;
					}
				}

				// get pointer to DisconnectEx function
				LPFN_DISCONNECTEX pDisconnectEx;
				{
					// get pointer
					int getResult = GetExtension(socket, WSAID_DISCONNECTEX, &pDisconnectEx);

					// check if operation has failed
					if (getResult == SOCKET_ERROR)
					{
						return NULL;
					}
				}

				// get pointer to GetAcceptExSockaddrs function
				LPFN_GETACCEPTEXSOCKADDRS pGetAcceptExSockaddrs;
				{
					// get pointer
					int getResult = GetExtension(socket, WSAID_GETACCEPTEXSOCKADDRS, &pGetAcceptExSockaddrs);

					// check if operation has failed
					if (getResult == SOCKET_ERROR)
					{
						return NULL;
					}
				}

				// get Registered I/O functions table
				RIO_EXTENSION_FUNCTION_TABLE rioTable;
				{
					// get extension id
					GUID id = WSAID_MULTIPLE_RIO;

					// get table size
					DWORD tableSize = sizeof(RIO_EXTENSION_FUNCTION_TABLE);

					// will contain actual table size
					DWORD actualTableSize;

					// try get registered IO functions table
					int getResult = ::WSAIoctl(socket, SIO_GET_MULTIPLE_EXTENSION_FUNCTION_POINTER, &id, sizeof(GUID), &rioTable, tableSize, &actualTableSize, NULL, NULL);

					// check if operation was not successful
					if (getResult == SOCKET_ERROR)
					{
						return NULL;
					}
				}

				// compose and return result
				return new WinsockEx(pAcceptEx, pDisconnectEx, pGetAcceptExSockaddrs, rioTable);
			}

			#pragma endregion

			#pragma region Methods of the Extensions

			/// <summary>
			/// Accepts a new connection, returns the local and remote address, and receives the first block of data sent by the client application.
			/// </summary>
			/// <param name="sListenSocket">A descriptor identifying a socket that has already been called with the listen function.</param>
			/// <param name="sAcceptSocket">A descriptor identifying a socket on which to accept an incoming connection.</param>
			/// <returns>
			/// If no error occurs, the function completed successfully and a value of <see cref="TRUE"/> is returned.
			/// If the function fails, returns <see cref="FALSE"/>.
			/// The <see cref="WSAGetLastError"/> function can then be called to return extended error information.
			/// If <see cref ="WSAGetLastError"/> returns <see cref ="ERROR_IO_PENDING"/>, then the operation was successfully initiated and is still in progress.
			/// If the error is <see cref ="WSAECONNRESET"/>, an incoming connection was indicated, but was subsequently terminated by the remote peer prior to accepting the call.
			/// </returns>
			inline BOOL AcceptEx(SOCKET sListenSocket, SOCKET sAcceptSocket, PVOID lpOutputBuffer, DWORD dwReceiveDataLength, DWORD dwLocalAddressLength, DWORD dwRemoteAddressLength, LPDWORD lpdwBytesReceived, LPOVERLAPPED lpOverlapped)
			{
				return pAcceptEx(sListenSocket, sAcceptSocket, lpOutputBuffer, dwReceiveDataLength, dwLocalAddressLength, dwRemoteAddressLength, lpdwBytesReceived, lpOverlapped);
			}

			/// <summary>
			/// Closes a connection on a socket, and allows the socket handle to be reused.
			/// </summary>
			/// <param name="hSocket">A handle to a connected, connection-oriented socket.</param>
			/// <param name="lpOverlapped">A pointer to an <see href="OVERLAPPED" /> structure. If the socket handle has been opened as overlapped, specifying this parameter results in an overlapped (asynchronous) I/O operation.</param>
			/// <param name="dwFlags">A set of flags that customizes processing of the function call.</param>
			/// <param name="reserved">Reserved. Must be zero.</param>
			/// <returns>
			/// On success, returns TRUE.
			/// On failure, the function returns FALSE.
			/// Use the <see cref="WSAGetLastError"/> function to get extended error information.
			/// If a call to the <see cref="WSAGetLastError"/> function returns <see cref="ERROR_IO_PENDING"/>, the operation initiated successfully and is in progress.
			/// Under such circumstances, the call may still fail when the operation completes.
			/// </returns>
			inline BOOL DisconnectEx(SOCKET hSocket, LPOVERLAPPED lpOverlapped, DWORD dwFlags, DWORD reserved)
			{
				return pDisconnectEx(hSocket, lpOverlapped, dwFlags, reserved);
			}

			/// <summary>
			/// Parses the data obtained from a call to the <see href="AcceptEx"/> function and passes the local and remote addresses to a sockaddr structure.
			/// </summary>
			inline void GetAcceptExSockaddrs(PVOID lpOutputBuffer, DWORD dwReceiveDataLength, DWORD dwLocalAddressLength, DWORD dwRemoteAddressLength, LPSOCKADDR *LocalSockaddr, LPINT LocalSockaddrLength, LPSOCKADDR *RemoteSockaddr, LPINT RemoteSockaddrLength)
			{
				pGetAcceptExSockaddrs(lpOutputBuffer, dwReceiveDataLength, dwLocalAddressLength, dwRemoteAddressLength, LocalSockaddr, LocalSockaddrLength, RemoteSockaddr, RemoteSockaddrLength);
			}

			#pragma endregion

			#pragma region Methods of the Registered I/O Extensions

			/// <summary>
			/// Closes an existing completion queue used for I/O completion notification by send and receive requests with the Winsock registered I/O extensions.
			/// </summary>
			/// <param name="CQ">A descriptor identifying an existing completion queue.</param>
			inline void RIOCloseCompletionQueue(RIO_CQ CQ)
			{
				pRIOCloseCompletionQueue(CQ);
			}

			/// <summary>
			/// Creates an I/O completion queue of a specific size for use with the Winsock registered I/O extensions.
			/// </summary>
			/// <param name="QueueSize">The size, in number of entries, of the completion queue to create.</param>
			/// <param name="NotificationCompletion">The type of notification completion to use based on the Type member of the <see cref="RIO_NOTIFICATION_COMPLETION" /> structure (I/O completion or event notification).</param>
			/// <returns>
			/// If no error occurs, returns a descriptor referencing a new completion queue.
			/// Otherwise, a value of <see cref="WinsockInterop.RIO_CORRUPT_CQ" /> is returned, and a specific error code can be retrieved by calling the <see cref="WinsockInterop.WSAGetLastError" /> function.
			/// </returns>
			inline RIO_CQ RIOCreateCompletionQueue(DWORD QueueSize, PRIO_NOTIFICATION_COMPLETION NotificationCompletion)
			{
				return pRIOCreateCompletionQueue(QueueSize, NotificationCompletion);
			}

			/// <summary>
			/// Creates a registered I/O socket descriptor using a specified socket and I/O completion queues for use with the Winsock registered I/O extensions.
			/// </summary>
			/// <param name="Socket">A descriptor that identifies the socket.</param>
			/// <param name="MaxOutstandingReceive">The maximum number of outstanding receives allowed on the socket.</param>
			/// <param name="MaxReceiveDataBuffers">The maximum number of receive data buffers on the socket. For Windows 8 and Windows Server 2012, must be <c>1</c>.</param>
			/// <param name="MaxOutstandingSend">The maximum number of outstanding sends allowed on the socket.</param>
			/// <param name="MaxSendDataBuffers">The maximum number of send data buffers on the socket. For Windows 8 and Windows Server 2012, must be <c>1</c>.</param>
			/// <param name="ReceiveCQ">A descriptor that identifies the I/O completion queue to use for receive request completions.</param>
			/// <param name="SendCQ">A descriptor that identifies the I/O completion queue to use for send request completions.</param>
			/// <param name="SocketContext">The socket context to associate with this request queue.</param>
			/// <returns>
			/// If no error occurs, returns a descriptor referencing a new request queue.
			/// Otherwise, a value of <see cref="WinsockInterop.RIO_INVALID_RQ" /> is returned, and a specific error code can be retrieved by calling the <see cref="WinsockInterop.WSAGetLastError" /> function.
			/// </returns>
			inline RIO_RQ RIOCreateRequestQueue(SOCKET Socket, ULONG MaxOutstandingReceive, ULONG MaxReceiveDataBuffers, ULONG MaxOutstandingSend, ULONG MaxSendDataBuffers, RIO_CQ ReceiveCQ, RIO_CQ SendCQ, PVOID SocketContext)
			{
				return pRIOCreateRequestQueue(Socket, MaxOutstandingReceive, MaxReceiveDataBuffers, MaxOutstandingSend, MaxSendDataBuffers, ReceiveCQ, SendCQ, SocketContext);
			}

			/// <summary>
			/// Removes entries from an I/O completion queue for use with the Winsock registered I/O extensions.
			/// </summary>
			/// <param name="CQ">A descriptor that identifies an I/O completion queue.</param>
			/// <param name="Array">An array of <see cref="RIORESULT" /> structures to receive the description of the completions dequeued.</param>
			/// <param name="ArraySize">The maximum number of entries in the <paramref name="Array" /> to write.</param>
			/// <returns>
			/// If no error occurs, returns the number of completion entries removed from the specified completion queue.
			/// Otherwise, a value of <see cref="WinsockInterop.RIO_CORRUPT_CQ" /> is returned to indicate that the state of the <see cref="RIO_CQ" /> passed in the <paramref name="CQ" /> parameter has become corrupt due to memory corruption or misuse of the RIO functions.
			/// </returns>
			inline ULONG RIODequeueCompletion(RIO_CQ CQ, PRIORESULT Array, ULONG ArraySize)
			{
				return pRIODequeueCompletion(CQ, Array, ArraySize);
			}

			/// <summary>
			/// Deregisters a registered buffer used with the Winsock registered I/O extensions.
			/// </summary>
			/// <param name="BufferId">A descriptor identifying a registered buffer.</param>
			inline void RIODeregisterBuffer(RIO_BUFFERID BufferId)
			{
				pRIODeregisterBuffer(BufferId);
			}

			/// <summary>
			/// Registers the method to use for notification behavior with an I/O completion queue for use with the Winsock registered I/O extensions.
			/// </summary>
			/// <param name="CQ">A descriptor that identifies an I/O completion queue.</param>
			/// <returns>
			/// If no error occurs, returns <see cref="WinsockErrorCode.None" />.
			/// Otherwise, the function failed and a specific error code is returned.
			/// </returns>
			inline INT RIONotify(RIO_CQ CQ)
			{
				return pRIONotify(CQ);
			}

			/// <summary>
			/// Receives network data on a connected registered I/O TCP socket or a bound registered I/O UDP socket for use with the Winsock registered I/O extensions.
			/// </summary>
			/// <param name="SocketQueue">A descriptor that identifies a connected registered I/O TCP socket or a bound registered I/O UDP socket.</param>
			/// <param name="pData">A description of the portion of the registered buffer in which to receive data. This parameter may be <c>null</c> for a bound registered I/O UDP socket if the application does not need to receive the data payload in the UDP datagram.</param>
			/// <param name="DataBufferCount">A data buffer count parameter that indicates if data is to be received in the buffer pointed to by the pData parameter. This parameter should be set to zero if the <paramref name="pData" /> is <c>null</c>.Otherwise, this parameter should be set to 1.</param>
			/// <param name="Flags">A set of flags that modify the behavior of the function.</param>
			/// <param name="RequestContext">The request context to associate with this receive operation.</param>
			/// <returns>
			/// If no error occurs, returns <c>true</c>. In this case, the receive operation is successfully initiated and the completion will have already been queued or the operation has been successfully initiated and the completion will be queued at a later time.
			/// A value of <c>false</c> indicates the function failed, the operation was not successfully initiated and no completion indication will be queued.A specific error code can be retrieved by calling the <see cref="WinsockInterop.WSAGetLastError" /> function.
			/// </returns>
			inline BOOL RIOReceive(RIO_RQ SocketQueue, PRIO_BUF pData, ULONG DataBufferCount, DWORD Flags, PVOID RequestContext)
			{
				return pRIOReceive(SocketQueue, pData, DataBufferCount, Flags, RequestContext);
			}

			/// <summary>
			/// Registers a RIO_BUFFERID, a registered buffer descriptor, with a specified buffer for use with the Winsock registered I/O extensions.
			/// </summary>
			/// <param name="DataBuffer">A pointer to the beginning of the memory buffer to register.</param>
			/// <param name="DataLength">The length, in bytes, in the buffer to register.</param>
			/// <returns>If no error occurs, returns a registered buffer descriptor. Otherwise, a value of <see cref="WinsockInterop.RIO_INVALID_BUFFERID" /> is returned, and a specific error code can be retrieved by calling the <see cref="WinsockInterop.WSAGetLastError" /> function.</returns>
			inline RIO_BUFFERID RIORegisterBuffer(PCHAR DataBuffer, DWORD DataLength)
			{
				return pRIORegisterBuffer(DataBuffer, DataLength);
			}

			/// <summary>
			/// Resizes an I/O completion queue to be either larger or smaller for use with the Winsock registered I/O extensions.
			/// </summary>
			/// <param name="CQ">A descriptor that identifies an existing I/O completion queue to resize.</param>
			/// <param name="QueueSize">The new size, in number of entries, of the completion queue.</param>
			/// <returns>
			/// If no error occurs, returns <c>true</c>.
			/// Otherwise, a value of <c>false</c> is returned, and a specific error code can be retrieved by calling the <see cref="WinsockInterop.WSAGetLastError" /> function.
			/// </returns>
			inline BOOL RIOResizeCompletionQueue(RIO_CQ CQ, DWORD QueueSize)
			{
				return pRIOResizeCompletionQueue(CQ, QueueSize);
			}

			/// <summary>
			/// Resizes a request queue to be either larger or smaller for use with the Winsock registered I/O extensions.
			/// </summary>
			/// <param name="RQ">A descriptor that identifies an existing registered I/O socket descriptor (request queue) to resize.</param>
			/// <param name="MaxOutstandingReceive">The maximum number of outstanding sends allowed on the socket. This value can be larger or smaller than the original number.</param>
			/// <param name="MaxOutstandingSend">The maximum number of outstanding receives allowed on the socket. This value can be larger or smaller than the original number.</param>
			/// <returns>
			/// If no error occurs, returns <c>true</c>.
			/// Otherwise, a value of <c>false</c> is returned, and a specific error code can be retrieved by calling the <see cref="WinsockInterop.WSAGetLastError" /> function.
			/// </returns>
			inline BOOL RIOResizeRequestQueue(RIO_RQ RQ, DWORD MaxOutstandingReceive, DWORD MaxOutstandingSend)
			{
				return pRIOResizeRequestQueue(RQ, MaxOutstandingReceive, MaxOutstandingSend);
			}

			/// <summary>
			/// Sends a network data on a connected registered I/O TCP socket or a bound registered I/O UDP socket for use with the Winsock registered I/O extensions.
			/// </summary>
			/// <param name="SocketQueue">A descriptor that identifies a connected registered I/O TCP socket or a bound registered I/O UDP socket.</param>
			/// <param name="pData">A description of the portion of the registered buffer from which to send data. This parameter may be <c>null</c> for a bound registered I/O UDP socket if the application does not need to send a data payload in the UDP datagram.</param>
			/// <param name="DataBufferCount">A data buffer count parameter that indicates if data is to be sent in the buffer pointed to by the <see cref="pData" /> parameter.</param>
			/// <param name="Flags">A set of flags that modify the behavior of the function.</param>
			/// <param name="RequestContext">The request context to associate with this send operation.</param>
			/// <returns>
			/// If no error occurs, returns <c>true</c>. In this case, the send operation is successfully initiated and the completion will have already been queued or the operation has been successfully initiated and the completion will be queued at a later time.
			/// A value of <c>false</c> indicates the function failed, the operation was not successfully initiated and no completion indication will be queued.
			/// A specific error code can be retrieved by calling the <see cref="WinsockInterop.WSAGetLastError" /> function.
			/// </returns>
			inline BOOL RIOSend(RIO_RQ SocketQueue, PRIO_BUF pData, DWORD DataBufferCount, DWORD Flags, PVOID RequestContext)
			{
				return pRIOSend(SocketQueue, pData, DataBufferCount, Flags, RequestContext);
			}

			#pragma endregion
		};
	}
}

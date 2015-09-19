#pragma once

#include "Stdafx.h"
#include "TcpServerException.h"

using namespace System;

namespace SXN
{
	namespace Net
	{
		/// <summary>
		/// Encapsulates data and methods required to work with Winsock.
		/// </summary>
		private ref class WinSocket sealed
		{
			public:

			#pragma region Fields

			initonly SOCKET socket;

			initonly LPFN_ACCEPTEX pAcceptEx;

			initonly LPFN_RIORECEIVE pRIOReceive;

			initonly LPFN_RIORECEIVEEX pRIOReceiveEx;

			initonly LPFN_RIOSEND pRIOSend;

			initonly LPFN_RIOSENDEX pRIOSendEx;

			initonly LPFN_RIOCLOSECOMPLETIONQUEUE pRIOCloseCompletionQueue;

			initonly LPFN_RIOCREATECOMPLETIONQUEUE pRIOCreateCompletionQueue;

			initonly LPFN_RIOCREATEREQUESTQUEUE pRIOCreateRequestQueue;

			initonly LPFN_RIODEQUEUECOMPLETION pRIODequeueCompletion;

			initonly LPFN_RIODEREGISTERBUFFER pRIODeregisterBuffer;

			initonly LPFN_RIONOTIFY pRIONotify;

			initonly LPFN_RIOREGISTERBUFFER pRIORegisterBuffer;

			initonly LPFN_RIORESIZECOMPLETIONQUEUE pRIOResizeCompletionQueue;

			initonly LPFN_RIORESIZEREQUESTQUEUE pRIOResizeRequestQueue;

			#pragma endregion

			public:

			#pragma region Constructor

			/// <summary>
			/// Initializes a new instance of the <see cref="WinsockHandle" /> class.
			/// </summary>
			/// <exception cref="TcpServerException">If error occurs.</exception>
			inline WinSocket()
			{
				// 1 create server socket
				this->socket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_REGISTERED_IO);

				// check if socket is created
				if (INVALID_SOCKET == socket)
				{
					// get error code
					WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

					// throw exception
					throw gcnew TcpServerException(winsockErrorCode);
				}

				// get guid size
				DWORD idSize = sizeof(GUID);

				// declare rio functions table
				RIO_EXTENSION_FUNCTION_TABLE table;

				// Registered I/O
				{
					// get extension id
					GUID id = WSAID_MULTIPLE_RIO;

					// get table size
					DWORD tableSize = sizeof(RIO_EXTENSION_FUNCTION_TABLE);

					// will contain actual table size
					DWORD actualTableSize;

					// try get registered IO functions table
					int getResult = WSAIoctl(SIO_GET_MULTIPLE_EXTENSION_FUNCTION_POINTER, &id, idSize, &table, tableSize, &actualTableSize, NULL, NULL);

					// check if operation was not successful
					if (getResult == SOCKET_ERROR)
					{
						// get error code
						WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

						// throw exception
						throw gcnew TcpServerException(winsockErrorCode);
					}

					pRIOReceive = table.RIOReceive;

					pRIOReceiveEx = table.RIOReceiveEx;

					pRIOSend = table.RIOSend;

					pRIOSendEx = table.RIOSendEx;

					pRIOCloseCompletionQueue = table.RIOCloseCompletionQueue;

					pRIOCreateCompletionQueue = table.RIOCreateCompletionQueue;

					pRIOCreateRequestQueue = table.RIOCreateRequestQueue;

					pRIODequeueCompletion = table.RIODequeueCompletion;

					pRIODeregisterBuffer = table.RIODeregisterBuffer;

					pRIONotify = table.RIONotify;

					pRIORegisterBuffer = table.RIORegisterBuffer;

					pRIOResizeCompletionQueue = table.RIOResizeCompletionQueue;

					pRIOResizeRequestQueue = table.RIOResizeRequestQueue;
				}

				// AcceptEx
				{
					// get extension id
					GUID id = WSAID_ACCEPTEX;

					// will contain result
					LPFN_ACCEPTEX pAcceptExpTmp;

					// get pointer size
					DWORD ptrSize = sizeof(LPFN_ACCEPTEX);

					DWORD actualPtrSize;

					// get function pointer
					int getResult = WSAIoctl(SIO_GET_EXTENSION_FUNCTION_POINTER, &id, idSize, &pAcceptExpTmp, ptrSize, &actualPtrSize, NULL, NULL);

					// check if operation was not successful
					if (getResult == SOCKET_ERROR)
					{
						// get error code
						WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

						// throw exception
						throw gcnew TcpServerException(winsockErrorCode);
					}

					// set pointer
					pAcceptEx = pAcceptExpTmp;
				}
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
			inline RIO_RQ RIOCreateRequestQueue(ULONG MaxOutstandingReceive, ULONG MaxReceiveDataBuffers, ULONG MaxOutstandingSend, ULONG MaxSendDataBuffers, RIO_CQ ReceiveCQ, RIO_CQ SendCQ, PVOID SocketContext)
			{
				return pRIOCreateRequestQueue(socket, MaxOutstandingReceive, MaxReceiveDataBuffers, MaxOutstandingSend, MaxSendDataBuffers, ReceiveCQ, SendCQ, SocketContext);
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

			#pragma region WSA Methods

			/// <summary>
			/// Accepts a new connection, returns the local and remote address, and receives the first block of data sent by the client application.
			/// </summary>
			/// <param name="sListenSocket">A descriptor identifying a socket that has already been called with the <see cref="listen"/> function. A server application waits for attempts to connect on this socket.</param>
			/// <param name="sAcceptSocket">A descriptor identifying a socket on which to accept an incoming connection.</param>
			/// <returns>
			/// If no error occurs, the function completed successfully and a value of <see cref="TRUE"/> is returned.
			/// If the function fails, returns <see cref="FALSE"/>.
			/// The <see cref="WSAGetLastError"/> function can then be called to return extended error information.
			/// If <see cref ="WSAGetLastError"/> returns <see cref ="ERROR_IO_PENDING"/>, then the operation was successfully initiated and is still in progress.
			/// If the error is <see cref ="WSAECONNRESET"/>, an incoming connection was indicated, but was subsequently terminated by the remote peer prior to accepting the call.
			/// </returns>
			inline BOOL AcceptEx(SOCKET sAcceptSocket, PVOID lpOutputBuffer, DWORD dwReceiveDataLength, DWORD dwLocalAddressLength, DWORD dwRemoteAddressLength, LPDWORD lpdwBytesReceived, LPOVERLAPPED lpOverlapped)
			{
				return pAcceptEx(socket, sAcceptSocket, lpOutputBuffer, dwReceiveDataLength, dwLocalAddressLength, dwRemoteAddressLength, lpdwBytesReceived, lpOverlapped);
			}

			/// <summary>
			/// Controls the mode of a socket.
			/// </summary>
			/// <param name="dwIoControlCode">The control code of operation to perform.</param>
			/// <param name="lpvInBuffer">A pointer to the input buffer.</param>
			/// <param name="cbInBuffer">The size, in bytes, of the input buffer.</param>
			/// <param name="lpvOutBuffer">A pointer to the output buffer.</param>
			/// <param name="cbOutBuffer">The size, in bytes, of the output buffer.</param>
			/// <param name="lpcbBytesReturned">A pointer to actual number of bytes of output.</param>
			/// <param name="lpOverlapped">A pointer to a <see cref="WSAOVERLAPPED"/> structure (ignored for non-overlapped sockets).</param>
			/// <param name="lpCompletionRoutine">A pointer to the completion routine called when the operation has been completed.</param>
			/// <returns>
			/// Upon successful completion, returns zero. Otherwise, a value of <see cref="SOCKET_ERROR" /> is returned, and a specific error code can be retrieved by calling <see cref="WSAGetLastError" />.
			/// </returns>
			/// <remarks>
			/// The function is used to set or retrieve operating parameters associated with the socket, the transport protocol, or the communications subsystem.
			/// </remarks>
			inline int WSAIoctl(DWORD dwIoControlCode, LPVOID lpvInBuffer, DWORD cbInBuffer, LPVOID lpvOutBuffer, DWORD cbOutBuffer, LPDWORD lpcbBytesReturned, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
			{
				return ::WSAIoctl(socket, dwIoControlCode, lpvInBuffer, cbInBuffer, lpvOutBuffer, cbOutBuffer, lpcbBytesReturned, lpOverlapped, lpCompletionRoutine);
			}

			/// <summary>
			/// Sets a socket option.
			/// </summary>
			/// <param name="level">The level at which the option is defined.</param>
			/// <param name="optname">The socket option for which the value is to be set.</param>
			/// <param name="optval">A pointer to the buffer in which the value for the requested option is specified.</param>
			/// <param name="optlen">The size, in bytes, of the buffer pointed to by the <paramref name="optval" /> parameter.</param>
			/// <returns>If no error occurs, returns zero. Otherwise, a value of <see cref="SOCKET_ERROR" /> is returned, and a specific error code can be retrieved by calling <see cref="WSAGetLastError" />.</returns>
			inline int setsockopt(int level, int optname, const char *optval, int optlen)
			{
				return ::setsockopt(socket, level, optname, optval, optlen);
			}

			/// <summary>
			/// Associates a local address with a socket.
			/// </summary>
			/// <param name="name">A pointer to a <see cref="SOCKADDR_IN" /> structure of the local address to assign to the bound socket .</param>
			/// <param name="namelen">The length, in bytes, of the value pointed to by the <paramref name="name" /> parameter.</param>
			/// <returns>If no error occurs, returns zero. Otherwise, returns <see cref="SOCKET_ERROR" />, and a specific error code can be retrieved by calling <see cref="WSAGetLastError" />.</returns>
			inline int bind(const struct sockaddr *name, int namelen)
			{
				return ::bind(socket, name, namelen);
			}

			/// <summary>
			/// Places a socket in a state in which it is listening for an incoming connection.
			/// </summary>
			/// <param name="backlog">The maximum length of the queue of pending connections. If set to <see cref="SOMAXCONN" />, the underlying service provider responsible for socket s will set the backlog to a maximum reasonable value. There is no standard provision to obtain the actual backlog value.</param>
			/// <returns>If no error occurs, returns zero. Otherwise, returns <see cref="SOCKET_ERROR" />, and a specific error code can be retrieved by calling <see cref="WSAGetLastError" />.</returns>
			inline int listen(int backlog)
			{
				return ::listen(socket, backlog);
			}

			#pragma endregion
		};
	}
}

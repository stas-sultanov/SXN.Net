using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Security;
using SXN.Net.Winsock;

// ReSharper disable All

namespace SXN.Net
{
	using SOCKET = UIntPtr;
	using DWORD = UInt32;
	using RIO_CQ = IntPtr;
	using RIO_RQ = IntPtr;
	using RIO_BUFFERID = IntPtr;
	using ULONG = UInt32;
	using BOOL = Boolean;
	using INT = Int32;
	using PVOID = Object;
	using PCHAR = IntPtr;

	internal sealed class RIOHandle
	{
		#region Nested Types

		[SuppressUnmanagedCodeSecurity]
		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		public delegate RIO_BUFFERID RIORegisterBuffer([In] PCHAR DataBuffer, [In] DWORD DataLength);

		[SuppressUnmanagedCodeSecurity]
		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		public delegate void RIODeregisterBuffer([In] RIO_BUFFERID BufferId);

		[SuppressUnmanagedCodeSecurity]
		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		public delegate BOOL RIOSend([In] RIO_RQ SocketQueue, [In] ref RIO_BUF pData, [In] DWORD DataBufferCount, [In] DWORD Flags, [In] PVOID RequestContext);

		[SuppressUnmanagedCodeSecurity]
		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		public delegate BOOL RIOReceive([In] RIO_RQ SocketQueue, [In] ref RIO_BUF pData, [In] ULONG DataBufferCount, [In] DWORD Flags, [In] PVOID RequestContext);

		[SuppressUnmanagedCodeSecurity]
		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		public delegate RIO_CQ RIOCreateCompletionQueue([In] DWORD QueueSize, [In] RIO_NOTIFICATION_COMPLETION NotificationCompletion);

		[SuppressUnmanagedCodeSecurity]
		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		public delegate void RIOCloseCompletionQueue([In] RIO_CQ CQ);

		[SuppressUnmanagedCodeSecurity]
		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		public delegate RIO_RQ RIOCreateRequestQueue([In] SOCKET Socket, [In] ULONG MaxOutstandingReceive, [In] ULONG MaxReceiveDataBuffers, [In] ULONG MaxOutstandingSend, [In] ULONG MaxSendDataBuffers, [In] RIO_CQ ReceiveCQ, [In] RIO_CQ SendCQ, [In] PVOID SocketContext);

		[SuppressUnmanagedCodeSecurity]
		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		public delegate ULONG RIODequeueCompletion([In] RIO_CQ CQ, [In] IntPtr Array, [In] ULONG ArraySize);

		[SuppressUnmanagedCodeSecurity]
		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		public delegate INT RIONotify([In] RIO_CQ CQ);

		[SuppressUnmanagedCodeSecurity]
		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		public delegate BOOL RIOResizeCompletionQueue([In] RIO_CQ CQ, [In] DWORD QueueSize);

		[SuppressUnmanagedCodeSecurity]
		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		public delegate BOOL RIOResizeRequestQueue([In] RIO_RQ RQ, [In] DWORD MaxOutstandingReceive, [In] DWORD MaxOutstandingSend);

		#endregion

		#region Fields

		private readonly RIOCloseCompletionQueue closeCompletionQueue;

		private readonly RIOCreateCompletionQueue createCompletionQueue;

		private readonly RIOCreateRequestQueue createRequestQueue;

		private readonly RIODequeueCompletion dequeueCompletion;

		private readonly RIODeregisterBuffer deregisterBuffer;

		private readonly RIONotify notify;

		private readonly RIOReceive receive;

		private readonly RIORegisterBuffer registerBuffer;

		private readonly RIOResizeCompletionQueue resizeCompletionQueue;

		private readonly RIOResizeRequestQueue resizeRequestQueue;

		private readonly RIOSend send;

		#endregion

		#region Constructors

		/// <summary>
		/// Initializes a new instance of <see cref="RIOHandle" /> class.
		/// </summary>
		/// <param name="table">The reference to the table of the functions pointers.</param>
		public RIOHandle(ref RIO_EXTENSION_FUNCTION_TABLE table)
		{
			registerBuffer = Marshal.GetDelegateForFunctionPointer<RIORegisterBuffer>(table.RIORegisterBuffer);

			createCompletionQueue = Marshal.GetDelegateForFunctionPointer<RIOCreateCompletionQueue>(table.RIOCreateCompletionQueue);

			createRequestQueue = Marshal.GetDelegateForFunctionPointer<RIOCreateRequestQueue>(table.RIOCreateRequestQueue);

			notify = Marshal.GetDelegateForFunctionPointer<RIONotify>(table.RIONotify);

			dequeueCompletion = Marshal.GetDelegateForFunctionPointer<RIODequeueCompletion>(table.RIODequeueCompletion);

			receive = Marshal.GetDelegateForFunctionPointer<RIOReceive>(table.RIOReceive);

			send = Marshal.GetDelegateForFunctionPointer<RIOSend>(table.RIOSend);

			closeCompletionQueue = Marshal.GetDelegateForFunctionPointer<RIOCloseCompletionQueue>(table.RIOCloseCompletionQueue);

			deregisterBuffer = Marshal.GetDelegateForFunctionPointer<RIODeregisterBuffer>(table.RIODeregisterBuffer);

			resizeCompletionQueue = Marshal.GetDelegateForFunctionPointer<RIOResizeCompletionQueue>(table.RIOResizeCompletionQueue);

			resizeRequestQueue = Marshal.GetDelegateForFunctionPointer<RIOResizeRequestQueue>(table.RIOResizeRequestQueue);
		}

		#endregion

		#region Methods

		/// <summary>
		/// Closes an existing completion queue used for I/O completion notification by send and receive requests with the Winsock registered I/O extensions.
		/// </summary>
		/// <param name="CQ">A descriptor identifying an existing completion queue.</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void CloseCompletionQueue([In] RIO_CQ CQ) => closeCompletionQueue(CQ);

		/// <summary>
		/// Creates an I/O completion queue of a specific size for use with the Winsock registered I/O extensions.
		/// </summary>
		/// <param name="QueueSize">The size, in number of entries, of the completion queue to create.</param>
		/// <param name="NotificationCompletion">The type of notification completion to use based on the Type member of the <see cref="RIO_NOTIFICATION_COMPLETION" /> structure (I/O completion or event notification).</param>
		/// <returns>
		/// If no error occurs, returns a descriptor referencing a new completion queue.
		/// Otherwise, a value of <see cref="WinsockInterop.RIO_CORRUPT_CQ" /> is returned, and a specific error code can be retrieved by calling the <see cref="WinsockInterop.WSAGetLastError" /> function.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public RIO_CQ CreateCompletionQueue([In] DWORD QueueSize, [In] RIO_NOTIFICATION_COMPLETION NotificationCompletion) => createCompletionQueue(QueueSize, NotificationCompletion);

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
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public RIO_RQ CreateRequestQueue([In] SOCKET Socket, [In] ULONG MaxOutstandingReceive, [In] ULONG MaxReceiveDataBuffers, [In] ULONG MaxOutstandingSend, [In] ULONG MaxSendDataBuffers, [In] RIO_CQ ReceiveCQ, [In] RIO_CQ SendCQ, [In] PVOID SocketContext) => createRequestQueue(Socket, MaxOutstandingReceive, MaxReceiveDataBuffers, MaxOutstandingSend, MaxSendDataBuffers, ReceiveCQ, SendCQ, SocketContext);

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
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public ULONG DequeueCompletion([In] RIO_CQ CQ, [In] IntPtr Array, [In] ULONG ArraySize) => dequeueCompletion(CQ, Array, ArraySize);

		/// <summary>
		/// Deregisters a registered buffer used with the Winsock registered I/O extensions.
		/// </summary>
		/// <param name="BufferId">A descriptor identifying a registered buffer.</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void DeregisterBuffer([In] RIO_BUFFERID BufferId) => deregisterBuffer(BufferId);

		/// <summary>
		/// Registers the method to use for notification behavior with an I/O completion queue for use with the Winsock registered I/O extensions.
		/// </summary>
		/// <param name="CQ">A descriptor that identifies an I/O completion queue.</param>
		/// <returns>
		/// If no error occurs, returns <see cref="WinsockErrorCode.None" />.
		/// Otherwise, the function failed and a specific error code is returned.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public INT Notify([In] RIO_CQ CQ) => notify(CQ);

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
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public BOOL Receive([In] RIO_RQ SocketQueue, [In] ref RIO_BUF pData, [In] ULONG DataBufferCount, [In] DWORD Flags, [In] PVOID RequestContext) => receive(SocketQueue, ref pData, DataBufferCount, Flags, RequestContext);

		/// <summary>
		/// Registers a RIO_BUFFERID, a registered buffer descriptor, with a specified buffer for use with the Winsock registered I/O extensions.
		/// </summary>
		/// <param name="DataBuffer">A pointer to the beginning of the memory buffer to register.</param>
		/// <param name="DataLength">The length, in bytes, in the buffer to register.</param>
		/// <returns>If no error occurs, returns a registered buffer descriptor. Otherwise, a value of <see cref="WinsockInterop.RIO_INVALID_BUFFERID" /> is returned, and a specific error code can be retrieved by calling the <see cref="WinsockInterop.WSAGetLastError" /> function.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public RIO_BUFFERID RegisterBuffer([In] PCHAR DataBuffer, [In] DWORD DataLength) => registerBuffer(DataBuffer, DataLength);

		/// <summary>
		/// Resizes an I/O completion queue to be either larger or smaller for use with the Winsock registered I/O extensions.
		/// </summary>
		/// <param name="CQ">A descriptor that identifies an existing I/O completion queue to resize.</param>
		/// <param name="QueueSize">The new size, in number of entries, of the completion queue.</param>
		/// <returns>
		/// If no error occurs, returns <c>true</c>.
		/// Otherwise, a value of <c>false</c> is returned, and a specific error code can be retrieved by calling the <see cref="WinsockInterop.WSAGetLastError" /> function.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public BOOL ResizeCompletionQueue([In] RIO_CQ CQ, [In] DWORD QueueSize) => resizeCompletionQueue(CQ, QueueSize);

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
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public BOOL ResizeRequestQueue([In] RIO_RQ RQ, [In] DWORD MaxOutstandingReceive, [In] DWORD MaxOutstandingSend) => resizeRequestQueue(RQ, MaxOutstandingReceive, MaxOutstandingSend);

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
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public BOOL Send([In] RIO_RQ SocketQueue, [In] ref RIO_BUF pData, [In] DWORD DataBufferCount, [In] DWORD Flags, [In] PVOID RequestContext) => send(SocketQueue, ref pData, DataBufferCount, Flags, RequestContext);

		/// <summary>
		/// Tries to initialize the Registered I/O handle.
		/// </summary>
		/// <param name="socket">A descriptor that identifies a socket.</param>
		/// <param name="result">Contains valid object if operation was successful, <c>null</c> otherwise.</param>
		/// <returns><c>true</c> if operation was successful, <c>false</c> otherwise.</returns>
		public static unsafe Boolean TryCreate(SOCKET socket, out RIOHandle result)
		{
			// get function table id
			var functionTableId = WinsockInterop.WSAID_MULTIPLE_RIO;

			// initialize functions table
			var functionTable = new RIO_EXTENSION_FUNCTION_TABLE();

			// get table size
			var tableSize = (UInt32) sizeof(RIO_EXTENSION_FUNCTION_TABLE);

			// will contain actual table size
			UInt32 actualTableSize;

			// try get registered IO functions table
			var tryGetTableResult = WinsockInterop.WSAIoctl(socket, WinsockInterop.SIO_GET_MULTIPLE_EXTENSION_FUNCTION_POINTER, &functionTableId, 16, &functionTable, tableSize, out actualTableSize, IntPtr.Zero, IntPtr.Zero);

			// check if attempt was successful
			if (tryGetTableResult == WinsockInterop.SOCKET_ERROR)
			{
				result = null;

				// return fail
				return false;
			}

			// create registered I/O handle
			result = new RIOHandle(ref functionTable);

			// return success
			return true;
		}

		#endregion
	}
}
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Security;
// ReSharper disable All

namespace SXN.Net.Winsock
{
	using DWORD = System.UInt32;

	internal sealed class RIO
	{
		#region Nested Types

		[SuppressUnmanagedCodeSecurity]
		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		public delegate IntPtr RIORegisterBuffer([In] IntPtr DataBuffer, [In] DWORD DataLength);

		[SuppressUnmanagedCodeSecurity]
		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		public delegate void RIODeregisterBuffer([In] IntPtr BufferId);

		[SuppressUnmanagedCodeSecurity]
		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		public delegate Boolean RIOSend([In] IntPtr SocketQueue, [In] ref RIO_BUF pData, [In] DWORD DataBufferCount, [In] DWORD Flags, [In] Int64 RequestContext);

		[SuppressUnmanagedCodeSecurity]
		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		public delegate Boolean RIOReceive([In] IntPtr SocketQueue, [In] ref RIO_BUF pData, [In] DWORD DataBufferCount, [In] DWORD Flags, [In] Int64 RequestContext);

		[SuppressUnmanagedCodeSecurity]
		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		public delegate IntPtr RIOCreateCompletionQueue([In] DWORD QueueSize, [In] RIO_NOTIFICATION_COMPLETION NotificationCompletion);

		[SuppressUnmanagedCodeSecurity]
		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		public delegate void RIOCloseCompletionQueue([In] IntPtr CQ);

		[SuppressUnmanagedCodeSecurity]
		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		public delegate IntPtr RIOCreateRequestQueue([In] IntPtr Socket,
			[In] DWORD MaxOutstandingReceive,
			[In] DWORD MaxReceiveDataBuffers,
			[In] DWORD MaxOutstandingSend,
			[In] DWORD MaxSendDataBuffers,
			[In] IntPtr ReceiveCQ,
			[In] IntPtr SendCQ,
			[In] Int64 ConnectionCorrelation
			);

		[SuppressUnmanagedCodeSecurity]
		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		public delegate DWORD RIODequeueCompletion([In] IntPtr CQ, [In] IntPtr ResultArray, [In] DWORD ResultArrayLength);

		[SuppressUnmanagedCodeSecurity]
		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		public delegate Int32 RIONotify([In] IntPtr CQ);

		[SuppressUnmanagedCodeSecurity]
		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		public delegate Boolean RIOResizeCompletionQueue([In] IntPtr CQ, [In] DWORD QueueSize);

		[SuppressUnmanagedCodeSecurity]
		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		public delegate Boolean RIOResizeRequestQueue([In] IntPtr RQ, [In] DWORD MaxOutstandingReceive, [In] DWORD MaxOutstandingSend);

		#endregion

		#region Constant and Static Fields

		public const Int64 CachedValue = Int64.MinValue;

		public const int RIO_INVALID_BUFFERID = 0;

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
		/// Initializes a new instance of <see cref="RIO"/> class.
		/// </summary>
		/// <param name="table">The reference to the table of the functions pointers.</param>
		public RIO(ref RIO_EXTENSION_FUNCTION_TABLE table)
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

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void CloseCompletionQueue([In] IntPtr CQ) => closeCompletionQueue(CQ);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public IntPtr CreateRequestQueue([In] IntPtr Socket,
			[In] DWORD MaxOutstandingReceive,
			[In] DWORD MaxReceiveDataBuffers,
			[In] DWORD MaxOutstandingSend,
			[In] DWORD MaxSendDataBuffers,
			[In] IntPtr ReceiveCQ,
			[In] IntPtr SendCQ,
			[In] Int64 ConnectionCorrelation
			) => createRequestQueue(Socket, MaxOutstandingReceive, MaxReceiveDataBuffers, MaxOutstandingSend, MaxSendDataBuffers, ReceiveCQ, SendCQ, ConnectionCorrelation);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public DWORD DequeueCompletion([In] IntPtr CQ, [In] IntPtr ResultArray, [In] DWORD ResultArrayLength) => dequeueCompletion(CQ, ResultArray, ResultArrayLength);

		/// <summary>
		/// Deregisters a registered buffer used with the Winsock registered I/O extensions.
		/// </summary>
		/// <param name="BufferId">A descriptor identifying a registered buffer.</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void DeregisterBuffer([In] IntPtr BufferId) => deregisterBuffer(BufferId);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Int32 Notify([In] IntPtr CQ) => notify(CQ);

		/// <summary>
		/// Receives network data on a connected registered I/O TCP socket or a bound registered I/O UDP socket for use with the Winsock registered I/O extensions.
		/// </summary>
		/// <param name="SocketQueue">A descriptor that identifies a connected registered I/O TCP socket or a bound registered I/O UDP socket.</param>
		/// <param name="pData">A description of the portion of the registered buffer in which to receive data. This parameter may be <c>null</c> for a bound registered I/O UDP socket if the application does not need to receive the data payload in the UDP datagram.</param>
		/// <param name="DataBufferCount">A data buffer count parameter that indicates if data is to be received in the buffer pointed to by the pData parameter. This parameter should be set to zero if the <paramref name="pData" /> is <c>null</c>.Otherwise, this parameter should be set to 1.</param>
		/// <param name="Flags">A set of flags that modify the behavior of the function.</param>
		/// <param name="RequestContext">The request context to associate with this receive operation.</param>
		/// <returns>
		/// If no error occurs, returns TRUE. In this case, the receive operation is successfully initiated and the completion will have already been queued or the operation has been successfully initiated and the completion will be queued at a later time.
		/// A value of FALSE indicates the function failed, the operation was not successfully initiated and no completion indication will be queued.A specific error code can be retrieved by calling the <see cref="Interop.WSAGetLastError" /> function.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Boolean Receive([In] IntPtr SocketQueue, [In] ref RIO_BUF pData, [In] DWORD DataBufferCount, [In] DWORD Flags, [In] Int64 RequestContext) => receive(SocketQueue, ref pData, DataBufferCount, Flags, RequestContext);

		/// <summary>
		/// Registers a RIO_BUFFERID, a registered buffer descriptor, with a specified buffer for use with the Winsock registered I/O extensions.
		/// </summary>
		/// <param name="DataBuffer">A pointer to the beginning of the memory buffer to register.</param>
		/// <param name="DataLength">The length, in bytes, in the buffer to register.</param>
		/// <returns>If no error occurs, returns a registered buffer descriptor. Otherwise, a value of <see cref="RIO.RIO_INVALID_BUFFERID" /> is returned, and a specific error code can be retrieved by calling the <see cref="Interop.WSAGetLastError" /> function.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public IntPtr RegisterBuffer([In] IntPtr DataBuffer, [In] DWORD DataLength) => registerBuffer(DataBuffer, DataLength);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Boolean ResizeCompletionQueue([In] IntPtr CQ, [In] DWORD QueueSize) => resizeCompletionQueue(CQ, QueueSize);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Boolean ResizeRequestQueue([In] IntPtr RQ, [In] DWORD MaxOutstandingReceive, [In] DWORD MaxOutstandingSend) => resizeRequestQueue(RQ, MaxOutstandingReceive, MaxOutstandingSend);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public IntPtr CreateCompletionQueue([In] DWORD QueueSize, [In] RIO_NOTIFICATION_COMPLETION NotificationCompletion) => createCompletionQueue(QueueSize, NotificationCompletion);

		/// <summary>
		/// Sends a network data on a connected registered I/O TCP socket or a bound registered I/O UDP socket for use with the Winsock registered I/O extensions.
		/// </summary>
		/// <param name="SocketQueue">A descriptor that identifies a connected registered I/O TCP socket or a bound registered I/O UDP socket.</param>
		/// <param name="pData">A description of the portion of the registered buffer from which to send data. This parameter may be NULL for a bound registered I/O UDP socket if the application does not need to send a data payload in the UDP datagram.</param>
		/// <param name="DataBufferCount">A data buffer count parameter that indicates if data is to be sent in the buffer pointed to by the <see cref="pData" /> parameter.</param>
		/// <param name="Flags">A set of flags that modify the behavior of the function.</param>
		/// <param name="RequestContext">The request context to associate with this send operation.</param>
		/// <returns>
		/// If no error occurs, the RIOSend function returns TRUE. In this case, the send operation is successfully initiated and the completion will have already been queued or the operation has been successfully initiated and the completion will be queued at a later time.
		/// A value of FALSE indicates the function failed, the operation was not successfully initiated and no completion indication will be queued.A specific error code can be retrieved by calling the WSAGetLastError function.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Boolean Send([In] IntPtr SocketQueue, [In] ref RIO_BUF pData, [In] DWORD DataBufferCount, [In] DWORD Flags, [In] Int64 RequestContext) => send(SocketQueue, ref pData, DataBufferCount, Flags, RequestContext);

		#endregion
	}
}
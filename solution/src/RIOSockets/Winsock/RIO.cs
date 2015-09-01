using System;
using System.Runtime.InteropServices;
using System.Security;

// ReSharper disable All

namespace SXN.Net.Winsock
{
	internal sealed class RIO
	{
		#region Nested Types

		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		[SuppressUnmanagedCodeSecurity]
		public delegate IntPtr RIORegisterBuffer([In] IntPtr DataBuffer, [In] UInt32 DataLength);

		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		[SuppressUnmanagedCodeSecurity]
		public delegate void RIODeregisterBuffer([In] IntPtr BufferId);

		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		[SuppressUnmanagedCodeSecurity]
		public delegate Boolean RIOSend([In] IntPtr SocketQueue, [In] ref RIO_BUF RioBuffer, [In] UInt32 DataBufferCount, [In] UInt32 Flags, [In] Int64 RequestCorrelation);

		/// <summary>
		/// Receives network data on a connected registered I/O TCP socket or a bound registered I/O UDP socket for use with the Winsock registered I/O extensions.
		/// </summary>
		/// <param name="SocketQueue">A descriptor that identifies a connected registered I/O TCP socket or a bound registered I/O UDP socket.</param>
		/// <param name="pData">A description of the portion of the registered buffer in which to receive data. This parameter may be <c>null</c> for a bound registered I/O UDP socket if the application does not need to receive the data payload in the UDP datagram.</param>
		/// <param name="DataBufferCount">A data buffer count parameter that indicates if data is to be received in the buffer pointed to by the pData parameter. This parameter should be set to zero if the <paramref name="pData"/> is <c>null</c>.Otherwise, this parameter should be set to 1.</param>
		/// <param name="Flags">A set of flags that modify the behavior of the function.</param>
		/// <param name="RequestContext">The request context to associate with this receive operation.</param>
		/// <returns>
		/// If no error occurs, returns TRUE. In this case, the receive operation is successfully initiated and the completion will have already been queued or the operation has been successfully initiated and the completion will be queued at a later time.
		/// A value of FALSE indicates the function failed, the operation was not successfully initiated and no completion indication will be queued.A specific error code can be retrieved by calling the <see cref="Interop.WSAGetLastError"/> function.
		/// </returns>
		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		[SuppressUnmanagedCodeSecurity]
		public delegate Boolean RIOReceive([In] IntPtr SocketQueue, [In] ref RIO_BUF pData, [In] UInt32 DataBufferCount, [In] UInt32 Flags, [In] Int64 RequestContext);

		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		[SuppressUnmanagedCodeSecurity]
		public delegate IntPtr RIOCreateCompletionQueue([In] UInt32 QueueSize, [In] RIO_NOTIFICATION_COMPLETION NotificationCompletion);

		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		[SuppressUnmanagedCodeSecurity]
		public delegate void RIOCloseCompletionQueue([In] IntPtr CQ);

		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		[SuppressUnmanagedCodeSecurity]
		public delegate IntPtr RIOCreateRequestQueue([In] IntPtr Socket,
			[In] UInt32 MaxOutstandingReceive,
			[In] UInt32 MaxReceiveDataBuffers,
			[In] UInt32 MaxOutstandingSend,
			[In] UInt32 MaxSendDataBuffers,
			[In] IntPtr ReceiveCQ,
			[In] IntPtr SendCQ,
			[In] Int64 ConnectionCorrelation
			);

		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		[SuppressUnmanagedCodeSecurity]
		public delegate UInt32 RIODequeueCompletion([In] IntPtr CQ, [In] IntPtr ResultArray, [In] UInt32 ResultArrayLength);

		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		[SuppressUnmanagedCodeSecurity]
		public delegate Int32 RIONotify([In] IntPtr CQ);

		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		[SuppressUnmanagedCodeSecurity]
		public delegate Boolean RIOResizeCompletionQueue([In] IntPtr CQ, [In] UInt32 QueueSize);

		[UnmanagedFunctionPointer(CallingConvention.StdCall, SetLastError = true)]
		[SuppressUnmanagedCodeSecurity]
		public delegate Boolean RIOResizeRequestQueue([In] IntPtr RQ, [In] UInt32 MaxOutstandingReceive, [In] UInt32 MaxOutstandingSend);

		#endregion

		#region Constant and Static Fields

		public const Int64 CachedValue = Int64.MinValue;

		#endregion

		#region Fields

		public readonly RIOCloseCompletionQueue CloseCompletionQueue;

		public readonly RIOCreateCompletionQueue CreateCompletionQueue;

		public readonly RIOCreateRequestQueue CreateRequestQueue;

		public readonly RIODequeueCompletion DequeueCompletion;

		public readonly RIODeregisterBuffer DeregisterBuffer;

		public readonly RIONotify Notify;

		public readonly RIOReceive Receive;

		public readonly RIORegisterBuffer RegisterBuffer;

		public readonly RIOResizeCompletionQueue ResizeCompletionQueue;

		public readonly RIOResizeRequestQueue ResizeRequestQueue;

		public readonly RIOSend Send;

		#endregion

		#region Constructors

		public RIO(ref RIO_EXTENSION_FUNCTION_TABLE table)
		{
			RegisterBuffer = Marshal.GetDelegateForFunctionPointer<RIORegisterBuffer>(table.RIORegisterBuffer);

			CreateCompletionQueue = Marshal.GetDelegateForFunctionPointer<RIOCreateCompletionQueue>(table.RIOCreateCompletionQueue);

			CreateRequestQueue = Marshal.GetDelegateForFunctionPointer<RIOCreateRequestQueue>(table.RIOCreateRequestQueue);

			Notify = Marshal.GetDelegateForFunctionPointer<RIONotify>(table.RIONotify);

			DequeueCompletion = Marshal.GetDelegateForFunctionPointer<RIODequeueCompletion>(table.RIODequeueCompletion);

			Receive = Marshal.GetDelegateForFunctionPointer<RIOReceive>(table.RIOReceive);

			Send = Marshal.GetDelegateForFunctionPointer<RIOSend>(table.RIOSend);

			CloseCompletionQueue = Marshal.GetDelegateForFunctionPointer<RIOCloseCompletionQueue>(table.RIOCloseCompletionQueue);

			DeregisterBuffer = Marshal.GetDelegateForFunctionPointer<RIODeregisterBuffer>(table.RIODeregisterBuffer);

			ResizeCompletionQueue = Marshal.GetDelegateForFunctionPointer<RIOResizeCompletionQueue>(table.RIOResizeCompletionQueue);

			ResizeRequestQueue = Marshal.GetDelegateForFunctionPointer<RIOResizeRequestQueue>(table.RIOResizeRequestQueue);
		}

		#endregion
	}
}
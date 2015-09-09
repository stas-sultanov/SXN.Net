using System;
using System.Runtime.InteropServices;
using System.Threading;

// ReSharper disable All

namespace SXN.Net.Winsock
{
	using HANDLE = IntPtr;
	using BOOL = Boolean;

	/// <summary>
	/// Specifies the method for I/O completion to be used with a <see cref="RIOHandle.RIONotify" /> function for sending or receiving network data with the Winsock registered I/O extensions.
	/// </summary>
	[StructLayout(LayoutKind.Sequential)]
	internal unsafe struct RIO_NOTIFICATION_COMPLETION
	{
		#region Nested Types

		[StructLayout(LayoutKind.Sequential)]
		internal struct EVENT
		{
			#region Fields

			/// <summary>
			/// The handle for the event to set following a completed RIONotify request.
			/// </summary>
			/// <remarks>This value is valid when the <see cref="RIO_NOTIFICATION_COMPLETION.Type" /> member is set to <see cref="RIO_NOTIFICATION_COMPLETION_TYPE.RIO_EVENT_COMPLETION" />.</remarks>
			public HANDLE EventHandle;

			/// <summary>
			/// The boolean value that causes the associated event to be reset when the RIONotify function is called. A non-zero value cause the associated event to be reset.
			/// </summary>
			/// <remarks>This value is valid when the <see cref="RIO_NOTIFICATION_COMPLETION.Type" /> member is set to <see cref="RIO_NOTIFICATION_COMPLETION_TYPE.RIO_EVENT_COMPLETION" />.</remarks>
			public BOOL NotifyReset;

			#endregion
		}

		[StructLayout(LayoutKind.Sequential)]
		internal struct IOCP
		{
			#region Fields

			/// <summary>
			/// The handle for the I/O completion port to use for queuing a <see cref="RIOHandle.Notify" /> request completion.
			/// </summary>
			/// <remarks>
			/// This value is valid when the <see cref="RIO_NOTIFICATION_COMPLETION.Type" /> member is set to <see cref="RIO_NOTIFICATION_COMPLETION_TYPE.RIO_IOCP_COMPLETION" />.
			/// </remarks>
			public HANDLE IocpHandle;

			/// <summary>
			/// The value to use for lpCompletionKey parameter returned by the <see cref="KernelInterop.GetQueuedCompletionStatus" /> or <see cref="KernelInterop.GetQueuedCompletionStatusEx" /> function when queuing a <see cref="RIOHandle.Notify" /> request.
			/// </summary>
			/// <remarks>
			/// This value is valid when the <see cref="RIO_NOTIFICATION_COMPLETION.Type" /> member is set to <see cref="RIO_NOTIFICATION_COMPLETION_TYPE.RIO_IOCP_COMPLETION" />.
			/// </remarks>
			public ulong CompletionKey;

			/// <summary>
			/// A pointer to the <see cref="NativeOverlapped" /> structure to use when queuing a <see cref="RIOHandle.Notify" /> request completion.
			/// This member must point to a valid OVERLAPPED structure.
			/// </summary>
			/// <remarks>
			/// This value is valid when the <see cref="RIO_NOTIFICATION_COMPLETION.Type" /> member is set to <see cref="RIO_NOTIFICATION_COMPLETION_TYPE.RIO_IOCP_COMPLETION" />.
			/// </remarks>
			public NativeOverlapped* Overlapped;

			#endregion
		}

		#endregion

		#region Fields

		/// <summary>
		/// The type of completion to use with the RIONotify function when sending or receiving data.
		/// </summary>
		//[FieldOffset(0)]
		internal RIO_NOTIFICATION_COMPLETION_TYPE Type;

		/// <summary>
		/// This value is valid when the <see cref="Type" /> member is set to <see cref="RIO_NOTIFICATION_COMPLETION_TYPE.RIO_EVENT_COMPLETION" />.
		/// </summary>
		//[FieldOffset(4)]
		//internal EVENT Event;

		/// <summary>
		/// This value is valid when the <see cref="Type" /> member is set to <see cref="RIO_NOTIFICATION_COMPLETION_TYPE.RIO_IOCP_COMPLETION" />.
		/// </summary>
		//[FieldOffset(4)]
		internal IOCP Iocp;

		#endregion
	}
}
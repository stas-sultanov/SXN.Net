using System.Runtime.InteropServices;
using System.Threading;

// ReSharper disable InconsistentNaming

namespace System.Net.RIOSockets
{
	/// <summary>
	/// Specifies the method for I/O completion to be used with a RIONotify function for sending or receiving network data.
	/// </summary>
	[StructLayout(LayoutKind.Explicit)]
	internal struct RIO_NOTIFICATION_COMPLETION
	{
		#region Nested Types

		[StructLayout(LayoutKind.Sequential)]
		internal struct IOCP
		{
			#region Fields

			/// <summary>
			/// The handle for the event to set following a completed RIONotify request.
			/// </summary>
			/// <remarks>This value is valid when the <see cref="RIO_NOTIFICATION_COMPLETION.Type" /> member is set to <see cref="RIO_NOTIFICATION_COMPLETION_TYPE.RIO_EVENT_COMPLETION" />.</remarks>
			public IntPtr EventHandle;

			/// <summary>
			/// The boolean value that causes the associated event to be reset when the RIONotify function is called. A non-zero value cause the associated event to be reset.
			/// </summary>
			/// <remarks>This value is valid when the <see cref="RIO_NOTIFICATION_COMPLETION.Type" /> member is set to <see cref="RIO_NOTIFICATION_COMPLETION_TYPE.RIO_EVENT_COMPLETION" />.</remarks>
			public Boolean NotifyReset;

			#endregion
		}

		[StructLayout(LayoutKind.Sequential)]
		internal unsafe struct EVENT
		{
			#region Fields

			/// <summary>
			/// The handle for the I/O completion port to use for queuing a RIONotify request completion.
			/// </summary>
			/// <remarks>This value is valid when the <see cref="RIO_NOTIFICATION_COMPLETION.Type" /> member is set to <see cref="RIO_NOTIFICATION_COMPLETION_TYPE.RIO_IOCP_COMPLETION" />.</remarks>
			public IntPtr IocpHandle;

			/// <summary>
			/// The value to use for lpCompletionKey parameter returned by the GetQueuedCompletionStatus or GetQueuedCompletionStatusEx function when queuing a RIONotify request.
			/// </summary>
			/// <remarks>This value is valid when the <see cref="RIO_NOTIFICATION_COMPLETION.Type" /> member is set to <see cref="RIO_NOTIFICATION_COMPLETION_TYPE.RIO_IOCP_COMPLETION" />.</remarks>
			public UInt64 CompletionKey;

			/// <summary>
			/// A pointer to the <see cref="NativeOverlapped" /> structure to use when queuing a RIONotify request completion. This member must point to a valid OVERLAPPED structure.
			/// </summary>
			/// <remarks>This value is valid when the <see cref="RIO_NOTIFICATION_COMPLETION.Type" /> member is set to <see cref="RIO_NOTIFICATION_COMPLETION_TYPE.RIO_IOCP_COMPLETION" />.</remarks>
			public NativeOverlapped* Overlapped;

			#endregion
		}

		#endregion

		#region Fields

		/// <summary>
		/// The type of completion to use with the RIONotify function when sending or receiving data.
		/// </summary>
		[FieldOffset(0)]
		internal RIO_NOTIFICATION_COMPLETION_TYPE Type;

		/// <summary>
		/// 
		/// </summary>
		/// <remarks>This value is valid when the <see cref="Type" /> member is set to <see cref="RIO_NOTIFICATION_COMPLETION_TYPE.RIO_EVENT_COMPLETION" />.</remarks>
		[FieldOffset(4)]
		internal EVENT Event;

		/// <summary>
		/// 
		/// </summary>
		/// <remarks>This value is valid when the <see cref="Type" /> member is set to <see cref="RIO_NOTIFICATION_COMPLETION_TYPE.RIO_IOCP_COMPLETION" />.</remarks>
		[FieldOffset(4)]
		internal IOCP Iocp;

		#endregion
	}
}
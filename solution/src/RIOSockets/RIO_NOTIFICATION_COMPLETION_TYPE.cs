using System;
// ReSharper disable InconsistentNaming

namespace System.Net.RIOSockets
{
	/// <summary>
	/// Specifies the type of completion queue notifications to use with the RIONotify function when sending or receiving data.
	/// </summary>
	internal enum RIO_NOTIFICATION_COMPLETION_TYPE : int
	{
		/// <summary>
		/// An event handle is used to signal completion queue notifications.
		/// </summary>
		RIO_EVENT_COMPLETION = 1,

		/// <summary>
		/// An I/O completion port handle is used to signal completion queue notifications.
		/// </summary>
		RIO_IOCP_COMPLETION = 2
	}
}
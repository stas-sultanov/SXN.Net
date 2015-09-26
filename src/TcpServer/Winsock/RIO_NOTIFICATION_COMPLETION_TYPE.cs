// ReSharper disable InconsistentNaming

namespace SXN.Net.Winsock
{
	/// <summary>
	/// Specifies the type of completion queue notifications to use with the RIONotify function when sending or receiving data.
	/// </summary>
	internal enum RIO_NOTIFICATION_COMPLETION_TYPE
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
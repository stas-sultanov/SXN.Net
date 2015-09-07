using System;

namespace SXN.Net.Winsock
{
	using LONG = Int32;
	using ULONG = UInt32;
	using ULONGULONG = UInt64;

	internal struct RIORESULT
	{
		/// <summary>
		/// The completion status of the Winsock registered I/O request.
		/// </summary>
		LONG Status;

		/// <summary>
		/// The number of bytes sent or received in the I/O request.
		/// </summary>
		ULONG BytesTransferred;

		/// <summary>
		/// An application-provided context specified in call to the <see cref="RIO.CreateRequestQueue"/> function.
		/// </summary>
		ULONGULONG SocketContext;

		/// <summary>
		/// An application-provided context specified with the registered I/O request to the <see cref="RIO.Receive"/>, <see cref="RIO.ReceiveEx"/>, <see cref="RIO.Send"/> and <see cref="RIO.SendEx"/> functions.
		/// </summary>
		ULONGULONG RequestContext;
	}
}

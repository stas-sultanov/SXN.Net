using System;

namespace SXN.Net.Winsock
{
	using LONG = Int32;
	using ULONG = UInt32;
	using SOCKET = UIntPtr;
	using ULONGULONG = UInt64;

	/// <summary>
	/// Contains data used to indicate request completion results used with the Winsock registered I/O extensions.
	/// </summary>
	internal struct RIORESULT
	{
		#region Fields

		/// <summary>
		/// The completion status of the Winsock registered I/O request.
		/// </summary>
		public LONG Status;

		/// <summary>
		/// The number of bytes sent or received in the I/O request.
		/// </summary>
		public ULONG BytesTransferred;

		/// <summary>
		/// An application-provided context specified in call to the <see cref="RIOHandle.CreateRequestQueue" /> function.
		/// </summary>
		public SOCKET SocketContext;

		/// <summary>
		/// An application-provided context specified with the registered I/O request to the <see cref="RIOHandle.Receive" />, <see cref="RIOHandle.ReceiveEx" />, <see cref="RIOHandle.Send" /> and <see cref="RIOHandle.SendEx" /> functions.
		/// </summary>
		public ULONGULONG RequestContext;

		#endregion
	}
}
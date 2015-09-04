using System;
using System.Runtime.InteropServices;

// ReSharper disable All

namespace SXN.Net.Winsock
{
	using RIO_BUFFERID = IntPtr;
	using ULONG = UInt32;

	/// <summary>
	/// Specifies a portion of a registered buffer used for sending or receiving network data with the Winsock registered I/O extensions.
	/// </summary>
	[StructLayout(LayoutKind.Sequential)]
	internal struct RIO_BUF
	{
		#region Fields

		/// <summary>
		/// The registered buffer descriptor for a Winsock registered I/O buffer used with send and receive requests.
		/// </summary>
		public RIO_BUFFERID BufferId;

		/// <summary>
		/// The offset, in bytes, into the buffer specified by the <see cref="BufferId" /> member.
		/// </summary>
		public ULONG Offset;

		/// <summary>
		/// A length, in bytes, of the buffer to use from the <see cref="Offset" /> member.
		/// </summary>
		public ULONG Length;

		#endregion
	}
}
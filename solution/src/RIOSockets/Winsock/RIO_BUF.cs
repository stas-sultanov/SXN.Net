using System;
using System.Runtime.InteropServices;
// ReSharper disable All

namespace SXN.Net.Winsock
{
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
		public IntPtr BufferId;

		/// <summary>
		/// A length, in bytes, of the buffer to use from the <see cref="Offset" /> member.
		/// </summary>
		public UInt32 Length;

		/// <summary>
		/// The offset, in bytes, into the buffer specified by the <see cref="BufferId" /> member.
		/// </summary>
		public UInt32 Offset;

		#endregion
	}
}
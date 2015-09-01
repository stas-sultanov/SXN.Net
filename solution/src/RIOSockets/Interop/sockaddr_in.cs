using System.Runtime.InteropServices;
// ReSharper disable All

namespace System.Net.RIOSockets
{
	[StructLayout(LayoutKind.Sequential, Size = 16)]
	internal unsafe struct SOCKADDR_IN
	{
		/// <summary>
		/// The size of the structure.
		/// </summary>
		public const Int32 Size = 16;

		/// <summary>
		/// Address family.
		/// </summary>
		public ADDRESS_FAMILIES sin_family;

		/// <summary>
		/// Internet Protocol (IP) port.
		/// </summary>
		public UInt16 sin_port;

		/// <summary>
		/// IP address in network byte order.
		/// </summary>
		public IN_ADDR sin_addr;

		/// <summary>
		/// Padding to make structure the same size as SOCKADDR.
		/// </summary>
		public fixed Byte sin_zero[8];
	}
}
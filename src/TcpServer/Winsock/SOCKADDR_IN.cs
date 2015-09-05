using System;
using System.Runtime.InteropServices;

// ReSharper disable All

namespace SXN.Net.Winsock
{
	using SHORT = Int16;
	using USHORT = UInt16;
	using CHAR = Byte;

	[StructLayout(LayoutKind.Sequential, Size = 16)]
	internal unsafe struct SOCKADDR_IN
	{
		#region Constant and Static Fields

		/// <summary>
		/// The size of the structure.
		/// </summary>
		public const Int32 Size = 16;

		#endregion

		#region Fields

		/// <summary>
		/// Address family.
		/// </summary>
		public SHORT sin_family;

		/// <summary>
		/// Internet Protocol (IP) port.
		/// </summary>
		public USHORT sin_port;

		/// <summary>
		/// IP address in network byte order.
		/// </summary>
		public IN_ADDR sin_addr;

		/// <summary>
		/// Padding to make structure the same size as SOCKADDR.
		/// </summary>
		public fixed CHAR sin_zero [8];

		#endregion
	}
}
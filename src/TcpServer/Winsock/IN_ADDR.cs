using System;
using System.Runtime.InteropServices;

// ReSharper disable All

namespace SXN.Net.Winsock
{
	using UCHAR = Byte;
	using USHORT = UInt16;
	using ULONG = UInt32;

	/// <summary>
	/// Represents an IPv4 Internet address.
	/// </summary>
	[StructLayout(LayoutKind.Explicit, Size = 4)]
	internal struct IN_ADDR
	{
		#region Nested Types

		[StructLayout(LayoutKind.Sequential)]
		public struct S_UN_B
		{
			#region Fields

			public UCHAR s_b1;

			public UCHAR s_b2;

			public UCHAR s_b3;

			public UCHAR s_b4;

			#endregion
		}

		[StructLayout(LayoutKind.Sequential)]
		public struct S_UN_W
		{
			#region Fields

			public USHORT s_w1;

			public USHORT s_w2;

			#endregion
		}

		#endregion

		#region Fields

		/// <summary>
		/// An IPv4 address formatted as four <see cref="UCHAR" />.
		/// </summary>
		[FieldOffset(0)]
		internal S_UN_B s_un_b;

		/// <summary>
		/// An IPv4 address formatted as two <see cref="USHORT" />.
		/// </summary>
		[FieldOffset(0)]
		internal S_UN_W s_un_w;

		/// <summary>
		/// An IPv4 address formatted as a <see cref="ULONG" />.
		/// </summary>
		[FieldOffset(0)]
		internal ULONG s_addr;

		#endregion
	}
}
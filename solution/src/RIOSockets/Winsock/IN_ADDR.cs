using System;
using System.Runtime.InteropServices;

// ReSharper disable All

namespace SXN.Net.Winsock
{
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

			public Byte s_b1;

			public Byte s_b2;

			public Byte s_b3;

			public Byte s_b4;

			#endregion
		}

		[StructLayout(LayoutKind.Sequential)]
		public struct S_UN_W
		{
			#region Fields

			public UInt16 s_w1;

			public UInt16 s_w2;

			#endregion
		}

		#endregion

		#region Fields

		/// <summary>
		/// An IPv4 address formatted as four <see cref="Byte" />.
		/// </summary>
		[FieldOffset(0)]
		internal S_UN_B s_un_b;

		/// <summary>
		/// An IPv4 address formatted as two <see cref="Int16" />.
		/// </summary>
		[FieldOffset(0)]
		internal S_UN_W s_un_w;

		/// <summary>
		/// An IPv4 address formatted as a <see cref="UInt32" />.
		/// </summary>
		[FieldOffset(0)]
		internal UInt32 s_addr;

		#endregion
	}
}
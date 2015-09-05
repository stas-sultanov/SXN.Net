using System;
using System.Runtime.InteropServices;

// ReSharper disable All

namespace SXN.Net.Winsock
{
	using SHORT = Int16;
	using CHAR = Byte;

	[StructLayout(LayoutKind.Sequential, Size = 16)]
	internal unsafe struct SOCKADDR
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
		public SHORT sa_family;

		/// <summary>
		/// Up to 14 bytes of direct address.
		/// </summary>
		public fixed CHAR sa_data [14];

		#endregion
	}
}
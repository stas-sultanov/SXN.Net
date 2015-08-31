using System.Runtime.InteropServices;

namespace System.Net.RIOSockets
{
	[StructLayout(LayoutKind.Sequential)]
	public unsafe struct sockaddr_in
	{
		public ADDRESS_FAMILIES sin_family;
		public ushort sin_port;
		public in_addr sin_addr;
		public fixed byte sin_zero[8];
	}
}
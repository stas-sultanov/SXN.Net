using System.Net.Sockets;

namespace System.Net.RIOSockets
{
	public sealed class ServerSocket
	{
		#region Constant and Static Fields

		public const Int16 Version = 0x22;

		#endregion

		#region Private methods

		private SocketTryResult<ServerSocket> TryInitialize(UInt16 port, Byte address1, Byte address2, Byte address3, Byte address4)
		{
			WSADATA data;

			// 0 initiates use of the Winsock DLL by a process.
			var startupResultCode = Interop.WSAStartup(Version, out data);

			// 0.1 check if startup was successful
			if (startupResultCode != SocketErrorCode.None)
			{
				return new SocketTryResult<ServerSocket>(startupResultCode, null);
			}

			// 1 try create socket
			var socket = Interop.WSASocket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp, IntPtr.Zero, 0, SocketCreateFlags.REGISTERED_IO);

			if (socket == Interop.INVALID_SOCKET)
			{
				goto FAIL;
			}

			// 2 bind

			FAIL:

			// get last error
			var errorCode = (SocketErrorCode) Interop.WSAGetLastError();

			// terminate use of the Winsock DLL
			Interop.WSACleanup();

			// return result
			return new SocketTryResult<ServerSocket>(errorCode, null);
		}

		#endregion
	}
}
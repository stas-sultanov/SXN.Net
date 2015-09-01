using System.Net.Sockets;

namespace System.Net.RIOSockets
{
	public sealed class TcpSocketServer
	{
		#region Constant and Static Fields

		

		#endregion

		private readonly IntPtr socket;

		/// <summary>
		/// Initializes a new instance of the <see cref="TcpSocketServer"/> class.
		/// </summary>
		private TcpSocketServer(IntPtr socket)
		{
			this.socket = socket;
		}

		#region Private methods

		public static SocketTryResult<TcpSocketServer> TryInitialize(UInt16 port)
		{
			WSADATA data;

			// 0 initiates use of the Winsock DLL by a process.
			var startupResultCode = Interop.WSAStartup(Interop.Version, out data);

			// 0.1 check if startup was successful
			if (startupResultCode != SocketErrorCode.None)
			{
				return new SocketTryResult<TcpSocketServer>(startupResultCode, null);
			}

			// 1 try create socket
			var serverSocket = Interop.WSASocket((Int32) ADDRESS_FAMILIES.AF_INET, (Int32) SocketType.Stream, (Int32) ProtocolType.Tcp, IntPtr.Zero, 0, SocketCreateFlags.REGISTERED_IO);

			if (Interop.INVALID_SOCKET == serverSocket)
			{
				goto FAIL;
			}

			// 2 compose address
			var address = new IN_ADDR
			{
				s_addr = 0
			};

			// 3 compose socket address
			var socketAddress = new SOCKADDR_IN
			{
				sin_family = ADDRESS_FAMILIES.AF_INET,
				sin_port = Interop.htons(port),
				sin_addr = address
			};

			// 4 try associate address with socket
			if (Interop.SOCKET_ERROR == Interop.bind(serverSocket, ref socketAddress, SOCKADDR_IN.Size))
			{
				goto FAIL;
			}

			// 5 try start listen
			if (Interop.SOCKET_ERROR == Interop.listen(serverSocket, 2048))
			{
				goto FAIL;
			}

			var result = new TcpSocketServer(serverSocket);

			return new SocketTryResult<TcpSocketServer>(SocketErrorCode.None, result);

			FAIL:

			// get last error
			var errorCode = (SocketErrorCode) Interop.WSAGetLastError();

			// terminate use of the Winsock DLL
			Interop.WSACleanup();

			// return result
			return new SocketTryResult<TcpSocketServer>(errorCode, null);
		}

		public SocketTryResult<IntPtr> TryAccept()
		{
			// Permits an incoming connection attempt on a socket.
			var acceptedSocket = Interop.accept(socket, IntPtr.Zero, 0);

			// ReSharper disable once InvertIf
			if (acceptedSocket == Interop.INVALID_SOCKET)
			{
				// get last error
				var errorCode = (SocketErrorCode)Interop.WSAGetLastError();

				// return result
				return new SocketTryResult<IntPtr>(errorCode, IntPtr.Zero);
			}

			return new SocketTryResult<IntPtr>(SocketErrorCode.None, acceptedSocket);
		}

		#endregion
	}
}
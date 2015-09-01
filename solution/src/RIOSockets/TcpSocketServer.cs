using System.Net.Sockets;

namespace System.Net.RIOSockets
{
	public sealed class TcpSocketServer
	{
		#region Fields

		private readonly IntPtr socket;

		#endregion

		#region Constructors

		/// <summary>
		/// Initializes a new instance of the <see cref="TcpSocketServer" /> class.
		/// </summary>
		private TcpSocketServer(IntPtr socket)
		{
			this.socket = socket;
		}

		#endregion

		#region Methods

		public SocketTryResult<IntPtr> TryAccept()
		{
			// Permits an incoming connection attempt on a socket.
			var acceptedSocket = WinsockInterop.accept(socket, IntPtr.Zero, 0);

			// ReSharper disable once InvertIf
			if (acceptedSocket == WinsockInterop.INVALID_SOCKET)
			{
				// get last error
				var errorCode = (SocketErrorCode) WinsockInterop.WSAGetLastError();

				// return result
				return new SocketTryResult<IntPtr>(errorCode, IntPtr.Zero);
			}

			return new SocketTryResult<IntPtr>(SocketErrorCode.None, acceptedSocket);
		}

		public static SocketTryResult<TcpSocketServer> TryInitialize(UInt16 port)
		{
			WSADATA data;

			// 0 initiates use of the Winsock DLL by a process.
			var startupResultCode = WinsockInterop.WSAStartup(WinsockInterop.Version, out data);

			// 0.1 check if startup was successful
			if (startupResultCode != SocketErrorCode.None)
			{
				return new SocketTryResult<TcpSocketServer>(startupResultCode, null);
			}

			// 1 try create socket
			var serverSocket = WinsockInterop.WSASocket((Int32) ADDRESS_FAMILIES.AF_INET, (Int32) SocketType.Stream, (Int32) ProtocolType.Tcp, IntPtr.Zero, 0, SocketCreateFlags.REGISTERED_IO);

			if (WinsockInterop.INVALID_SOCKET == serverSocket)
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
				sin_port = WinsockInterop.htons(port),
				sin_addr = address
			};

			// 4 try associate address with socket
			if (WinsockInterop.SOCKET_ERROR == WinsockInterop.bind(serverSocket, ref socketAddress, SOCKADDR_IN.Size))
			{
				goto FAIL;
			}

			// 5 try start listen
			if (WinsockInterop.SOCKET_ERROR == WinsockInterop.listen(serverSocket, 2048))
			{
				goto FAIL;
			}

			var result = new TcpSocketServer(serverSocket);

			return new SocketTryResult<TcpSocketServer>(SocketErrorCode.None, result);

			FAIL:

			// get last error
			var errorCode = (SocketErrorCode) WinsockInterop.WSAGetLastError();

			// terminate use of the Winsock DLL
			WinsockInterop.WSACleanup();

			// return result
			return new SocketTryResult<TcpSocketServer>(errorCode, null);
		}

		private unsafe Int32 Init()
		{
			Int32 disable = -1;

			Int32 result;

			// try disable use of the Nagle algorithm
			result = WinsockInterop.setsockopt(socket, WinsockInterop.IPPROTO_TCP, WinsockInterop.TCP_NODELAY, (Char*)&disable, 4)

			if (result == WinsockInterop.setsockopt(socket, WinsockInterop.IPPROTO_TCP, WinsockInterop.TCP_NODELAY, (Char*) &disable, 4))
			{

			}

			// try enable faster operations on the loopback interface
		}

		#endregion
	}
}
using System;
using System.Net.Sockets;
using SXN.Net.Winsock;

namespace SXN.Net
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

		#region Private methods

		private unsafe void TryGetRio()
		{
			UInt32 dwBytes;
			// try get registered IO functions table
			var rioFunctionsTableId = new Guid("8509e081-96dd-4005-b165-9e2ee8c79e3f");

			var rioTable = new RIO_EXTENSION_FUNCTION_TABLE();

			var tryGetTableResult = Interop.WSAIoctl
				(
					socket,
					Interop.SIO_GET_MULTIPLE_EXTENSION_FUNCTION_POINTER,
					&rioFunctionsTableId,
					16,
					&rioTable,
					(UInt32) sizeof(RIO_EXTENSION_FUNCTION_TABLE),
					out dwBytes, IntPtr.Zero, IntPtr.Zero
				);

			if (tryGetTableResult == Interop.SOCKET_ERROR)
			{
			}
		}

		#endregion

		#region Methods

		public SocketTryResult<IntPtr> TryAccept()
		{
			// Permits an incoming connection attempt on a socket.
			var acceptedSocket = Interop.accept(socket, IntPtr.Zero, 0);

			// ReSharper disable once InvertIf
			if (acceptedSocket == Interop.INVALID_SOCKET)
			{
				// get last error
				var errorCode = (SocketErrorCode) Interop.WSAGetLastError();

				// return result
				return new SocketTryResult<IntPtr>(errorCode, IntPtr.Zero);
			}

			return new SocketTryResult<IntPtr>(SocketErrorCode.None, acceptedSocket);
		}

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
			var serverSocket = Interop.WSASocket((Int32) Interop.AF_INET, (Int32) SocketType.Stream, (Int32) ProtocolType.Tcp, IntPtr.Zero, 0, SocketCreateFlags.REGISTERED_IO);

			if (Interop.INVALID_SOCKET == serverSocket)
			{
				goto FAIL;
			}

			var disable = -1;

			// 2 try disable use of the Nagle algorithm
			unsafe
			{
				var tryDisableNagle = Interop.setsockopt(serverSocket, Interop.IPPROTO_TCP, Interop.TCP_NODELAY, (Char*)&disable, 4);

				if (tryDisableNagle == Interop.SOCKET_ERROR)
				{
					goto FAIL;
				}
			}

			// 3 try enable faster operations on the loopback interface
			unsafe
			{
				UInt32 dwBytes = 0;

				var tryEnableFastLoopbackResult = Interop.WSAIoctl(serverSocket, Interop.SIO_LOOPBACK_FAST_PATH, &disable, 4, null, 0, out dwBytes, IntPtr.Zero, IntPtr.Zero);

				if (tryEnableFastLoopbackResult == Interop.SOCKET_ERROR)
				{
					goto FAIL;
				}
			}

			// 4 compose address
			var address = new IN_ADDR
			{
				s_addr = 0
			};

			// 5 compose socket address
			var socketAddress = new SOCKADDR_IN
			{
				sin_family = Interop.AF_INET,
				sin_port = Interop.htons(port),
				sin_addr = address
			};

			// 6 try associate address with socket
			var tryBindResult = Interop.bind(serverSocket, ref socketAddress, SOCKADDR_IN.Size);

			if (tryBindResult == Interop.SOCKET_ERROR)
			{
				goto FAIL;
			}

			// 7 try start listen
			var tryStartListen = Interop.listen(serverSocket, 2048);

			if (tryStartListen == Interop.SOCKET_ERROR)
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

		#endregion
	}
}
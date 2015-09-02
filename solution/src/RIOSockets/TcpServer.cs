using System;
using System.Net.Sockets;
using SXN.Net.Winsock;

namespace SXN.Net
{
	public sealed class TcpServer
	{
		#region Fields

		private readonly IntPtr socket;

		#endregion

		#region Constructors

		/// <summary>
		/// Initializes a new instance of the <see cref="TcpServer" /> class.
		/// </summary>
		private TcpServer(IntPtr socket)
		{
			this.socket = socket;
		}

		#endregion

		#region Private methods

		private static unsafe TryResult<RIO> TryGetRio(IntPtr socket)
		{
			UInt32 dwBytes;

			var rioFunctionsTableId = new Guid("8509e081-96dd-4005-b165-9e2ee8c79e3f");

			var rioTable = new RIO_EXTENSION_FUNCTION_TABLE();

			// try get registered IO functions table
			var tryGetTableResult = Interop.WSAIoctl(socket, Interop.SIO_GET_MULTIPLE_EXTENSION_FUNCTION_POINTER, &rioFunctionsTableId, 16, &rioTable, (UInt32) sizeof(RIO_EXTENSION_FUNCTION_TABLE), out dwBytes, IntPtr.Zero, IntPtr.Zero);

			// check if attempt was successful
			if (tryGetTableResult == Interop.SOCKET_ERROR)
			{
				// return fail
				return TryResult<RIO>.CreateFail();
			}

			var rio = new RIO(ref rioTable);

			return TryResult<RIO>.CreateSuccess(rio);
		}

		#endregion

		#region Methods

		public WinsockTryResult<IntPtr> TryAccept()
		{
			// Permits an incoming connection attempt on a socket.
			var acceptedSocket = Interop.accept(socket, IntPtr.Zero, IntPtr.Zero);

			// ReSharper disable once InvertIf
			if (acceptedSocket == Interop.INVALID_SOCKET)
			{
				// get last error
				var errorCode = (WinsockErrorCode) Interop.WSAGetLastError();

				// return result
				return WinsockTryResult<IntPtr>.CreateFail(errorCode);
			}

			return WinsockTryResult<IntPtr>.CreateSuccess(acceptedSocket);
		}

		public static WinsockTryResult<TcpServer> TryInitialize(UInt16 port)
		{
			WSADATA data;

			// 0 initiates use of the Winsock DLL by a process.
			var startupResultCode = Interop.WSAStartup(Interop.Version, out data);

			// 0.a check if startup was successful
			if (startupResultCode != WinsockErrorCode.None)
			{
				return WinsockTryResult<TcpServer>.CreateFail(startupResultCode);
			}

			// 1 try create socket
			var serverSocket = Interop.WSASocket((Int32) Interop.AF_INET, (Int32) SocketType.Stream, (Int32) ProtocolType.Tcp, IntPtr.Zero, 0, Interop.WSA_FLAG_REGISTERED_IO);

			// 1.a check if socket created
			if (Interop.INVALID_SOCKET == serverSocket)
			{
				goto FAIL;
			}

			// try initialize registered I/O extension
			var tryGetRio = TryGetRio(serverSocket);

			if (!tryGetRio.Success)
			{
				goto FAIL;
			}

			var disable = -1;

			// 2 try disable use of the Nagle algorithm
			unsafe
			{
				var tryDisableNagle = Interop.setsockopt(serverSocket, Interop.IPPROTO_TCP, Interop.TCP_NODELAY, (Char*)&disable, 4);

				// 2.a check if attempt has succeed
				if (tryDisableNagle == Interop.SOCKET_ERROR)
				{
					goto FAIL;
				}
			}

			// 3 try enable faster operations on the loopback interface
			unsafe
			{
				UInt32 dwBytes;

				var tryEnableFastLoopbackResult = Interop.WSAIoctl(serverSocket, Interop.SIO_LOOPBACK_FAST_PATH, &disable, 4, null, 0, out dwBytes, IntPtr.Zero, IntPtr.Zero);

				// 3.a check if attempt has succeed
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

			var result = new TcpServer(serverSocket);

			// return success
			return WinsockTryResult<TcpServer>.CreateSuccess(result);

			FAIL:

			// get last error
			var errorCode = (WinsockErrorCode) Interop.WSAGetLastError();

			// terminate use of the Winsock DLL
			Interop.WSACleanup();

			// return fail
			return WinsockTryResult<TcpServer>.CreateFail(errorCode);
		}

		#endregion
	}
}
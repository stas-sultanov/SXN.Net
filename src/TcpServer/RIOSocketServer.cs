using System;
using System.Net.Sockets;
using SXN.Net.Winsock;

namespace SXN.Net
{
	using SOCKET = UIntPtr;

	internal sealed class RIOSocketServer
	{
		private readonly RIO rioHandle;

		private readonly SOCKET serverSocket;

		/// <summary>
		/// The colleciton of the 
		/// </summary>
		private readonly IOCPWorker[] workers;

		/// <summary>
		/// Initializes a new instance of the <see cref="T:System.Object"/> class.
		/// </summary>
		public RIOSocketServer(UIntPtr serverSocket, RIO rioHandle, IOCPWorker[] workers)
		{
			this.serverSocket = serverSocket;

			this.rioHandle = rioHandle;

			this.workers = workers;
		}

		/// <summary>
		/// Activates server.
		/// </summary>
		public static WinsockTryResult<RIOSocketServer> TryInitialize(TcpServerSettings settings)
		{
			// try initiates use of the Winsock DLL by a process
			{
				WSADATA data;

				var startupResultCode = Interop.WSAStartup(Interop.Version, out data);

				// check if startup was successful
				if (startupResultCode != WinsockErrorCode.None)
				{
					return WinsockTryResult<RIOSocketServer>.CreateFail(startupResultCode);
				}
			}

			// try create server socket
			SOCKET serverSocket;

			{
				serverSocket = Interop.WSASocket(Interop.AF_INET, (Int32)SocketType.Stream, (Int32)ProtocolType.Tcp, IntPtr.Zero, 0, Interop.WSA_FLAG_REGISTERED_IO);

				// check if socket created
				if (Interop.INVALID_SOCKET == serverSocket)
				{
					goto FAIL;
				}
			}

			// try initialize Registered I/O extension
			RIO rioHandle;

			if (!RIO.TryInitialize(serverSocket, out rioHandle))
			{
				goto FAIL;
			}

			//
			// get count of processors
			var processorsCount = Environment.ProcessorCount;

			// create IOCP workers
			var workers = new IOCPWorker[processorsCount];

			for (var processorIndex = 0; processorIndex < processorsCount; processorIndex++)
			{
				var tryInitializeWorker = IOCPWorker.TryInitialize(rioHandle, processorIndex);

				workers[processorIndex] = tryInitializeWorker.Result;
			}

			// try configure server socket and start listen
			if (!TryConfigureBindAndStartListen(serverSocket, settings))
			{
				goto FAIL;
			}

			// success
			var server = new RIOSocketServer(serverSocket, rioHandle, null);

			return WinsockTryResult<RIOSocketServer>.CreateSuccess(server);

			FAIL:

			// get last error
			var errorCode = (WinsockErrorCode)Interop.WSAGetLastError();

			// terminate use of the Winsock DLL
			Interop.WSACleanup();

			// return fail
			return WinsockTryResult<RIOSocketServer>.CreateFail(errorCode);
		}

		private static Boolean TryConfigureBindAndStartListen(SOCKET serverSocket, TcpServerSettings settings)
		{
			// try disable use of the Nagle algorithm if requested
			if (settings.UseNagleAlgorithm == false)
			{
				var optionValue = -1;

				unsafe
				{
					var tryDisableNagle = Interop.setsockopt(serverSocket, Interop.IPPROTO_TCP, Interop.TCP_NODELAY, (Byte*)&optionValue, sizeof(Int32));

					// check if attempt has succeed
					if (tryDisableNagle == Interop.SOCKET_ERROR)
					{
						return false;
					}
				}
			}

			// try enable faster operations on the loopback if requested
			if (settings.UseFastLoopback)
			{
				unsafe
				{
					UInt32 optionValue = 1;

					UInt32 dwBytes;

					var tryEnableFastLoopbackResult = Interop.WSAIoctl(serverSocket, Interop.SIO_LOOPBACK_FAST_PATH, &optionValue, sizeof(UInt32), null, 0, out dwBytes, IntPtr.Zero, IntPtr.Zero);

					// check if attempt has succeed
					if (tryEnableFastLoopbackResult == Interop.SOCKET_ERROR)
					{
						return false;
					}
				}
			}

			// try bind
			{
				// compose address
				var address = new IN_ADDR
				{
					s_addr = 0
				};

				// compose socket address
				var socketAddress = new SOCKADDR_IN
				{
					sin_family = Interop.AF_INET,
					sin_port = Interop.htons(settings.Port),
					sin_addr = address
				};

				// try associate address with socket
				var tryBindResult = Interop.bind(serverSocket, ref socketAddress, SOCKADDR_IN.Size);

				if (tryBindResult == Interop.SOCKET_ERROR)
				{
					return false;
				}
			}

			// try start listen
			{
				var tryStartListen = Interop.listen(serverSocket, settings.AcceptBacklogLength);

				if (tryStartListen == Interop.SOCKET_ERROR)
				{
					return false;
				}
			}

			return true;
		}

		/// <summary>
		/// Deactivates server.
		/// </summary>
		public WinsockErrorCode Deactivate()
		{
			// try close socket
			var tryCloseResultCode = Interop.closesocket(serverSocket);

			if (tryCloseResultCode == Interop.SOCKET_ERROR)
			{
				goto FAIL;
			}

			return WinsockErrorCode.None;

			FAIL:

			return (WinsockErrorCode)Interop.WSAGetLastError();
		}
	}
}
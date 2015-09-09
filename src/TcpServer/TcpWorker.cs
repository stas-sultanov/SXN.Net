using System;
using System.Net.Sockets;
using SXN.Net.Winsock;

namespace SXN.Net
{
	using SOCKET = UIntPtr;

	public sealed class TcpWorker
	{
		#region Fields

		private readonly RIOHandle rioHandle;

		private readonly SOCKET serverSocket;

		/// <summary>
		/// The collection of the workers.
		/// </summary>
		private readonly IocpWorker[] workers;

		#endregion

		#region Constructors

		/// <summary>
		/// Initializes a new instance of the <see cref="TcpWorker" /> class.
		/// </summary>
		private TcpWorker(UIntPtr serverSocket, RIOHandle rioHandle, IocpWorker[] workers)
		{
			this.serverSocket = serverSocket;

			this.rioHandle = rioHandle;

			this.workers = workers;
		}

		#endregion

		#region Private methods

		private static Boolean TryConfigureBindAndStartListen(SOCKET serverSocket, TcpServerSettings settings)
		{
			// try disable use of the Nagle algorithm if requested
			if (settings.UseNagleAlgorithm == false)
			{
				var optionValue = -1;

				unsafe
				{
					var tryDisableNagle = WinsockInterop.setsockopt(serverSocket, WinsockInterop.IPPROTO_TCP, WinsockInterop.TCP_NODELAY, (Byte*) &optionValue, sizeof(Int32));

					// check if attempt has succeed
					if (tryDisableNagle == WinsockInterop.SOCKET_ERROR)
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

					var tryEnableFastLoopbackResult = WinsockInterop.WSAIoctl(serverSocket, WinsockInterop.SIO_LOOPBACK_FAST_PATH, &optionValue, sizeof(UInt32), null, 0, out dwBytes, IntPtr.Zero, IntPtr.Zero);

					// check if attempt has succeed
					if (tryEnableFastLoopbackResult == WinsockInterop.SOCKET_ERROR)
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
					sin_family = WinsockInterop.AF_INET,
					sin_port = WinsockInterop.htons(settings.Port),
					sin_addr = address
				};

				// try associate address with socket
				var tryBindResult = WinsockInterop.bind(serverSocket, ref socketAddress, SOCKADDR_IN.Size);

				if (tryBindResult == WinsockInterop.SOCKET_ERROR)
				{
					return false;
				}
			}

			// try start listen
			{
				var tryStartListen = WinsockInterop.listen(serverSocket, settings.AcceptBacklogLength);

				if (tryStartListen == WinsockInterop.SOCKET_ERROR)
				{
					return false;
				}
			}

			return true;
		}

		#endregion

		#region Methods

		/// <summary>
		/// Deactivates server.
		/// </summary>
		public WinsockErrorCode Deactivate()
		{
			// try close socket
			var tryCloseResultCode = WinsockInterop.closesocket(serverSocket);

			if (tryCloseResultCode == WinsockInterop.SOCKET_ERROR)
			{
				goto FAIL;
			}

			return WinsockErrorCode.None;

			FAIL:

			return (WinsockErrorCode) WinsockInterop.WSAGetLastError();
		}

		/// <summary>
		/// Activates server.
		/// </summary>
		public static TryResult<TcpWorker> TryInitialize(TcpServerSettings settings)
		{
			// 0 try initiates use of the Winsock DLL by a process
			{
				WSADATA data;

				var startupResultCode = WinsockInterop.WSAStartup(WinsockInterop.Version, out data);

				// check if startup was successful
				if (startupResultCode != WinsockErrorCode.None)
				{
					return TryResult<TcpWorker>.CreateFail(startupResultCode);
				}
			}

			// 1 try create server socket
			SOCKET serverSocket;

			{
				serverSocket = WinsockInterop.WSASocket(WinsockInterop.AF_INET, (Int32) SocketType.Stream, (Int32) ProtocolType.Tcp, IntPtr.Zero, 0, WinsockInterop.WSA_FLAG_REGISTERED_IO);

				// check if socket created
				if (WinsockInterop.INVALID_SOCKET == serverSocket)
				{
					goto FAIL;
				}
			}

			// 2 try initialize Registered I/O extension
			RIOHandle rioHandle;

			if (!RIOHandle.TryCreate(serverSocket, out rioHandle))
			{
				goto FAIL;
			}

			// 3 get count of processors
			var processorsCount = Environment.ProcessorCount;

			// 4 create collection of the IOCP workers
			var workers = new IocpWorker[processorsCount];

			// initialize workers
			for (var processorIndex = 0; processorIndex < processorsCount; processorIndex++)
			{
				// try create worker
				var tryCreateWorker = IocpWorker.TryCreate(rioHandle, processorIndex, 4096, 1024);

				// check if operation has succeed
				if (!tryCreateWorker.Success)
				{
					goto FAIL;
				}

				// add to collection
				workers[processorIndex] = tryCreateWorker.Result;
			}

			// try configure server socket and start listen
			if (!TryConfigureBindAndStartListen(serverSocket, settings))
			{
				goto FAIL;
			}

			// success
			var server = new TcpWorker(serverSocket, rioHandle, workers);

			foreach (var worker in workers)
			{
				worker.thread.Start();
			}

			return TryResult<TcpWorker>.CreateSuccess(server);

			FAIL:

			// get last error
			var errorCode = (WinsockErrorCode) WinsockInterop.WSAGetLastError();

			// terminate use of the Winsock DLL
			WinsockInterop.WSACleanup();

			// return fail
			return TryResult<TcpWorker>.CreateFail(errorCode);
		}

		public TryResult<TcpConnection> TryAccept()
		{
			SOCKADDR address;

			var length = SOCKADDR.Size;

			// permit an incoming connection attempt on a socket
			var acceptedSocket = WinsockInterop.accept(serverSocket, out address, ref length);

			// ReSharper disable once InvertIf
			if (acceptedSocket == WinsockInterop.INVALID_SOCKET)
			{
				// get last error
				var errorCode = (WinsockErrorCode) WinsockInterop.WSAGetLastError();

				// return result
				return TryResult<TcpConnection>.CreateFail(errorCode);
			}

			// try create connection
			return workers[0].TryCreateConnection(acceptedSocket, acceptedSocket.ToUInt32());
		}

		#endregion
	}
}
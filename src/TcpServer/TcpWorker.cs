using System;
using System.Net.Sockets;
using SXN.Net.Winsock;

namespace SXN.Net
{
	using SOCKET = UIntPtr;

	public sealed class TcpWorker
	{
		#region Fields

		private readonly RIO rioHandle;

		private readonly SOCKET serverSocket;

		/// <summary>
		/// The collection of the workers.
		/// </summary>
		private readonly IOCPWorker[] workers;

		#endregion

		#region Constructors

		/// <summary>
		/// Initializes a new instance of the <see cref="TcpWorker" /> class.
		/// </summary>
		private TcpWorker(UIntPtr serverSocket, RIO rioHandle, IOCPWorker[] workers)
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
					var tryDisableNagle = Interop.setsockopt(serverSocket, Interop.IPPROTO_TCP, Interop.TCP_NODELAY, (Byte*) &optionValue, sizeof(Int32));

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

		#endregion

		#region Methods

		/// <summary>
		/// Deactivates server.
		/// </summary>
		public ErrorCode Deactivate()
		{
			// try close socket
			var tryCloseResultCode = Interop.closesocket(serverSocket);

			if (tryCloseResultCode == Interop.SOCKET_ERROR)
			{
				goto FAIL;
			}

			return ErrorCode.None;

			FAIL:

			return (ErrorCode) Interop.WSAGetLastError();
		}

		/// <summary>
		/// Activates server.
		/// </summary>
		public static TryResult<TcpWorker> TryInitialize(TcpServerSettings settings)
		{
			// 0 try initiates use of the Winsock DLL by a process
			{
				WSADATA data;

				var startupResultCode = Interop.WSAStartup(Interop.Version, out data);

				// check if startup was successful
				if (startupResultCode != ErrorCode.None)
				{
					return TryResult<TcpWorker>.CreateFail(startupResultCode);
				}
			}

			// 1 try create server socket
			SOCKET serverSocket;

			{
				serverSocket = Interop.WSASocket(Interop.AF_INET, (Int32) SocketType.Stream, (Int32) ProtocolType.Tcp, IntPtr.Zero, 0, Interop.WSA_FLAG_REGISTERED_IO);

				// check if socket created
				if (Interop.INVALID_SOCKET == serverSocket)
				{
					goto FAIL;
				}
			}

			// 2 try initialize Registered I/O extension
			RIO rioHandle;

			if (!RIO.TryInitialize(serverSocket, out rioHandle))
			{
				goto FAIL;
			}

			// 3 get count of processors
			var processorsCount = Environment.ProcessorCount;

			// 4 create collection of the IOCP workers
			var workers = new IOCPWorker[processorsCount];

			// initialize workers
			for (var processorIndex = 0; processorIndex < processorsCount; processorIndex++)
			{
				// try create worker
				var tryCreateWorker = IOCPWorker.TryCreate(rioHandle, processorIndex, 4096, 1024);

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
			var server = new TcpWorker(serverSocket, rioHandle, null);

			return TryResult<TcpWorker>.CreateSuccess(server);

			FAIL:

			// get last error
			var errorCode = (ErrorCode) Interop.WSAGetLastError();

			// terminate use of the Winsock DLL
			Interop.WSACleanup();

			// return fail
			return TryResult<TcpWorker>.CreateFail(errorCode);
		}

		#endregion
	}
}
using System;
using System.Net.Sockets;
using SXN.Net.Winsock;

namespace SXN.Net
{
	using SOCKET = UIntPtr;

	/// <summary>
	/// </summary>
	public sealed class TcpServer
	{
		#region Fields

		private RIO rioHandle;

		private SOCKET serverSocket;

		#endregion

		#region Constructors

		/// <summary>
		/// Initializes a new instance of the <see cref="TcpServer" /> class.
		/// </summary>
		public TcpServer(TcpServerSettings settings)
		{
			Settings = settings;
		}

		#endregion

		#region Properties

		public TcpServerSettings Settings
		{
			get;
		}

		/// <summary>
		/// Determines whether server is active.
		/// </summary>
		public Boolean IsActive
		{
			get;

			private set;
		}

		#endregion

		#region Methods

		/// <summary>
		/// Activates server.
		/// </summary>
		public WinsockErrorCode Activate()
		{
			// 0 check if is active
			if (IsActive)
			{
				return WinsockErrorCode.None;
			}

			// 1 try initiates use of the Winsock DLL by a process
			{
				WSADATA data;

				var startupResultCode = Interop.WSAStartup(Interop.Version, out data);

				// check if startup was successful
				if (startupResultCode != WinsockErrorCode.None)
				{
					return startupResultCode;
				}
			}

			// 2 try create socket
			{
				serverSocket = Interop.WSASocket(Interop.AF_INET, (Int32) SocketType.Stream, (Int32) ProtocolType.Tcp, IntPtr.Zero, 0, Interop.WSA_FLAG_REGISTERED_IO);

				// check if socket created
				if (Interop.INVALID_SOCKET == serverSocket)
				{
					goto FAIL;
				}
			}

			// 3 try initialize Registered I/O extension
			if (!RIO.TryInitialize(serverSocket, out rioHandle))
			{
				rioHandle = null;

				goto FAIL;
			}

			// 4 try disable use of the Nagle algorithm if requested
			if (Settings.UseNagleAlgorithm == false)
			{
				var optionValue = -1;

				unsafe
				{
					var tryDisableNagle = Interop.setsockopt(serverSocket, Interop.IPPROTO_TCP, Interop.TCP_NODELAY, (Byte*) &optionValue, sizeof(Int32));

					// check if attempt has succeed
					if (tryDisableNagle == Interop.SOCKET_ERROR)
					{
						goto FAIL;
					}
				}
			}

			// 5 try enable faster operations on the loopback if requested
			if (Settings.UseFastLoopback)
			{
				unsafe
				{
					UInt32 optionValue = 1;

					UInt32 dwBytes;

					var tryEnableFastLoopbackResult = Interop.WSAIoctl(serverSocket, Interop.SIO_LOOPBACK_FAST_PATH, &optionValue, sizeof(UInt32), null, 0, out dwBytes, IntPtr.Zero, IntPtr.Zero);

					// check if attempt has succeed
					if (tryEnableFastLoopbackResult == Interop.SOCKET_ERROR)
					{
						goto FAIL;
					}
				}
			}

			// 6 try bind
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
					sin_port = Interop.htons(Settings.Port),
					sin_addr = address
				};

				// try associate address with socket
				var tryBindResult = Interop.bind(serverSocket, ref socketAddress, SOCKADDR_IN.Size);

				if (tryBindResult == Interop.SOCKET_ERROR)
				{
					goto FAIL;
				}
			}

			// 7 try start listen
			{
				var tryStartListen = Interop.listen(serverSocket, Settings.AcceptBacklogLength);

				if (tryStartListen == Interop.SOCKET_ERROR)
				{
					goto FAIL;
				}
			}

			// success
			IsActive = true;

			return WinsockErrorCode.None;

			FAIL:

			serverSocket = UIntPtr.Zero;

			rioHandle = null;

			// get last error
			var errorCode = (WinsockErrorCode) Interop.WSAGetLastError();

			// terminate use of the Winsock DLL
			Interop.WSACleanup();

			// return fail
			return errorCode;
		}

		/// <summary>
		/// Deactivates server.
		/// </summary>
		public WinsockErrorCode Deactivate()
		{
			// check if is not active
			if (!IsActive)
			{
				return WinsockErrorCode.None;
			}

			// try close socket
			var tryCloseResultCode = Interop.closesocket(serverSocket);

			if (tryCloseResultCode == Interop.SOCKET_ERROR)
			{
				goto FAIL;
			}

			serverSocket = UIntPtr.Zero;

			rioHandle = null;

			return WinsockErrorCode.None;

			FAIL:

			return (WinsockErrorCode) Interop.WSAGetLastError();
		}

		/// <summary>
		/// Tries to accepts a pending connection request.
		/// </summary>
		/// <returns></returns>
		public WinsockTryResult<SOCKET> TryAccept()
		{
			SOCKADDR address;

			var length = SOCKADDR.Size;

			// Permits an incoming connection attempt on a socket.
			var acceptedSocket = Interop.accept(serverSocket, out address, ref length);

			// ReSharper disable once InvertIf
			if (acceptedSocket == Interop.INVALID_SOCKET)
			{
				// get last error
				var errorCode = (WinsockErrorCode) Interop.WSAGetLastError();

				// return result
				return WinsockTryResult<SOCKET>.CreateFail(errorCode);
			}

			return WinsockTryResult<SOCKET>.CreateSuccess(acceptedSocket);
		}

		#endregion
	}
}
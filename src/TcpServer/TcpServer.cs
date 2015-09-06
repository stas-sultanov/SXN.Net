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

		private RIOSocketServer server;

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

			var tryInitializeServer = RIOSocketServer.TryInitialize(Settings);

			if (!tryInitializeServer.Success)
			{
				return tryInitializeServer.ErrorCode;
			}

			server = tryInitializeServer.Result;

			IsActive = true;

			return WinsockErrorCode.None;
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

			server.Deactivate();

			server = null;

			return WinsockErrorCode.None;
		}

		/// <summary>
		/// Tries to accepts a pending connection request.
		/// </summary>
		/// <returns></returns>
		public WinsockTryResult<SOCKET> TryAccept()
		{
			return WinsockTryResult<SOCKET>.CreateFail(WinsockErrorCode.None);

			/*
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
			*/
		}

		#endregion
	}
}
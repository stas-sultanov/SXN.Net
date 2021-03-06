﻿using System;

namespace SXN.Net
{
	using SOCKET = UIntPtr;

	/// <summary>
	/// </summary>
	public sealed class TcpServer
	{
		#region Fields

		private TcpWorker server;

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
		public Boolean Activate()
		{
			// 0 check if is active
			if (IsActive)
			{
				return false;
			}

			var tryInitializeServer = TcpWorker.TryInitialize(Settings);

			if (!tryInitializeServer.Success)
			{
				return false;
			}

			server = tryInitializeServer.Result;

			IsActive = true;

			return true;
		}

		/// <summary>
		/// Deactivates server.
		/// </summary>
		public Boolean Deactivate()
		{
			// check if is not active
			if (!IsActive)
			{
				return false;
			}

			server.Deactivate();

			server = null;

			return true;
		}

		/// <summary>
		/// Tries to accepts a pending connection request.
		/// </summary>
		/// <returns></returns>
		public TryResult<TcpConnection> TryAccept()
		{
			return server.TryAccept();
		}

		#endregion
	}
}
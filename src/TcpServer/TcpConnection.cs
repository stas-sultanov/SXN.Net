using System;
using System.Runtime.CompilerServices;

namespace SXN.Net
{
	using SOCKET = UIntPtr;

	public sealed class TcpConnection
	{
		#region Fields

		private readonly SOCKET socket;

		private readonly IntPtr requestQueue;

		#endregion

		#region Constructors

		/// <summary>
		/// Initializes a new instance of the <see cref="TcpConnection" /> class.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public TcpConnection(SOCKET socket, IntPtr requestQueue)
		{
			this.socket = socket;

			this.requestQueue = requestQueue;

			// create request queue
			//var rq = rioHandle.CreateRequestQueue(socket, 24, 1, 24, 1, )
		}

		#endregion
	}
}
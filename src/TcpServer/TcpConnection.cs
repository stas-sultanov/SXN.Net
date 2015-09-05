using System;
using System.Runtime.CompilerServices;
using SXN.Net.Winsock;

namespace SXN.Net
{
	using SOCKET = UIntPtr;

	public sealed class TcpContext
	{
		private readonly SOCKET socket;

		private readonly RIO rioHandle;

		/// <summary>
		/// Initializes a new instance of the <see cref="TcpContext"/> class.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		internal TcpContext(SOCKET socket, RIO rioHandle)
		{
			this.socket = socket;

			this.rioHandle = rioHandle;

			// create request queue
			var rq = rioHandle.CreateRequestQueue(socket, 24, 1, 24, 1, )
		}



	}
}
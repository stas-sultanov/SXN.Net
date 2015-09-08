using System;
using SXN.Net.Winsock;

namespace SXN.Net
{
	/// <summary>
	/// Specifies the configuration settings of the <see cref="TcpServer" /> class.
	/// </summary>
	public struct TcpServerSettings
	{
		#region Constant and Static Fields

		public const Int32 MaxConnections = WinsockInterop.SOMAXCONN;

		#endregion

		#region Properties

		/// <summary>
		/// Specifies the port on which to listen for incoming connection attempts.
		/// </summary>
		public UInt16 Port
		{
			get;

			set;
		}

		/// <summary>
		/// Specifies the maximum length of the queue of pending connections.
		/// </summary>
		/// <remarks>
		/// If set to <see cref="MaxConnections" />, the underlying service provider responsible will set the backlog to a maximum reasonable value.
		/// There is no standard provision to obtain the actual backlog value.
		/// </remarks>
		public Int32 AcceptBacklogLength
		{
			get;

			set;
		}

		/// <summary>
		/// Determines whether the Nagle algorithm is used by the server.
		/// </summary>
		/// <remarks>
		/// The Nagle algorithm is used to reduce network traffic by buffering small packets of data and transmitting them as a single packet.
		/// This process is also referred to as "nagling"; it is widely used because it reduces the number of packets transmitted and lowers the overhead per packet.
		/// </remarks>
		public Boolean UseNagleAlgorithm
		{
			get;

			set;
		}

		/// <summary>
		/// Determines whether the TCP Loopback optimization is used by the server.
		/// </summary>
		/// <remarks>
		/// The TCP loopback optimization provides an optimized low-latency loopback path for performance-critical applications that rely on loopback for inter-process communication.
		/// </remarks>
		public Boolean UseFastLoopback
		{
			get;

			set;
		}

		#endregion
	}
}
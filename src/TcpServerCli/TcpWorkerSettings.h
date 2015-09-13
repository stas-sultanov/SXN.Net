#pragma once

using namespace System;

namespace SXN
{
	namespace Net
	{
		/// <summary>
		/// Specifies the configuration settings of the <see cref="TcpWorker" /> class.
		/// </summary>
		public ref struct TcpWorkerSettings
		{
			#pragma region Constant and Static Fields

			//public const Int32 MaxConnections = WinsockInterop.SOMAXCONN;

			#pragma endregion

			public:

			#pragma region Properties

			/// <summary>
			/// Specifies the port on which to listen for incoming connection attempts.
			/// </summary>
			property UInt16 Port;

			/// <summary>
			/// Specifies the maximum length of the queue of pending connections.
			/// </summary>
			/// <remarks>
			/// If set to <see cref="MaxConnections" />, the underlying service provider responsible will set the backlog to a maximum reasonable value.
			/// There is no standard provision to obtain the actual backlog value.
			/// </remarks>
			property Int32 AcceptBacklogLength;

			/// <summary>
			/// Determines whether the Nagle algorithm is used by the server.
			/// </summary>
			/// <remarks>
			/// The Nagle algorithm is used to reduce network traffic by buffering small packets of data and transmitting them as a single packet.
			/// This process is also referred to as "nagling"; it is widely used because it reduces the number of packets transmitted and lowers the overhead per packet.
			/// </remarks>
			property Boolean UseNagleAlgorithm;

			/// <summary>
			/// Determines whether the TCP Loopback optimization is used by the server.
			/// </summary>
			/// <remarks>
			/// The TCP loopback optimization provides an optimized low-latency loopback path for performance-critical applications that rely on loopback for inter-process communication.
			/// </remarks>
			property Boolean UseFastLoopback;

			#pragma endregion
		};
	}
}
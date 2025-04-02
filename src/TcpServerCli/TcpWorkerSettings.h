#pragma once

#include "Stdafx.h"

using namespace System;
using namespace System::Net;
using namespace System::Net::Sockets;

namespace SXN::Net
{
	/// <summary>
	/// Specifies the configuration settings for the <see cref="TcpWorker" /> class.
	/// </summary>
	public ref class TcpWorkerSettings
	{
	private:

#pragma region Static Fields

		/// <summary>
		/// The number of logical processors available in the current system.
		/// </summary>
		static initonly Int32 processorsCount;

		/// <summary>
		/// The granularity for the starting address at which virtual memory can be allocated.
		/// </summary>
		static initonly UInt32 allocationGranularity;

#pragma endregion

		Int32 useProcessorsCount;

		IPEndPoint^ acceptPoint;

#pragma endregion

	public:

#pragma region Constant and Static Fields

		/// <summary>
		/// The maximum number of connections that can be queued for acceptance.
		/// </summary>
		const Int32 MaxConnections = SOMAXCONN;

#pragma endregion

		static TcpWorkerSettings()
		{
			// Retrieve system information
			SYSTEM_INFO sysinfo;
			::GetSystemInfo(&sysinfo);

			// Set the number of logical processors
			processorsCount = sysinfo.dwNumberOfProcessors;

			// Set the allocation granularity
			allocationGranularity = sysinfo.dwAllocationGranularity;
		}

#pragma region Properties

		/// <summary>
		/// Gets the number of logical processors available in the current system.
		/// </summary>
		static property Int32 ProcessorsCount
		{
			Int32 get()
			{
				return processorsCount;
			}
		}

		/// <summary>
		/// Gets or sets the length of the connection backlog queue.
		/// </summary>
		/// <remarks>
		/// The value must be less than the maximum number of ports and will be adjusted to the number of used processors.
		/// </remarks>
		property UInt32 ConnectionsBacklogLength;

		/// <summary>
		/// Gets or sets the length of the receive buffer in bytes.
		/// </summary>
		/// <remarks>
		/// The value will be adjusted to the nearest valid size.
		/// </remarks>
		property Int32 ReceiveBufferLength;

		/// <summary>
		/// Gets or sets the length of the send buffer in bytes.
		/// </summary>
		/// <remarks>
		/// The value will be adjusted to the nearest valid size.
		/// </remarks>
		property Int32 SendBufferLength;

		/// <summary>
		/// Gets or sets a value indicating whether the Nagle algorithm is used by the server.
		/// </summary>
		/// <remarks>
		/// The Nagle algorithm reduces network traffic by buffering small packets of data and transmitting them as a single packet.
		/// </remarks>
		property Boolean UseNagleAlgorithm;

		/// <summary>
		/// Gets or sets a value indicating whether the TCP Loopback optimization is used by the server.
		/// </summary>
		/// <remarks>
		/// The TCP loopback optimization provides an optimized low-latency loopback path for performance-critical applications that rely on loopback for inter-process communication.
		/// </remarks>
		property Boolean UseFastLoopback;

		/// <summary>
		/// Gets or sets the number of processors to use.
		/// </summary>
		/// <remarks>
		/// If the value is not specified, is zero, or is greater than the actual number of processors, then all available processors will be used.
		/// </remarks>
		property Int32 UseProcessorsCount
		{
			Int32 get()
			{
				return useProcessorsCount;
			}

			void set(Int32 value)
			{
				if ((value < 1) || (value > processorsCount))
				{
					throw gcnew ArgumentOutOfRangeException("value");
				}
				useProcessorsCount = value;
			}
		}

		/// <summary>
		/// Gets or sets the maximum number of outstanding receive operations for RIO.
		/// </summary>
		property UInt32 RIOMaxOutstandingReceive;

		/// <summary>
		/// Gets or sets the maximum number of outstanding send operations for RIO.
		/// </summary>
		property UInt32 RIOMaxOutstandingSend;

		/// <summary>
		/// Gets or sets the maximum number of entries to try to dequeue from the accept queue.
		/// </summary>
		property UInt32 AcceptQueueMaxEntriesCount
		{
			UInt32 get()
			{
				return useProcessorsCount;
			}

			void set(UInt32 value)
			{
				if (value < 1)
				{
					throw gcnew ArgumentOutOfRangeException("value");
				}
				useProcessorsCount = value;
			}
		}

		/// <summary>
		/// Gets or sets the time to wait for a request packet to appear in the accept queue.
		/// </summary>
		/// <remarks>
		/// If the value is INFINITE (0xFFFFFFFF), the function will never time out.
		/// If the value is zero and there is no I/O operation to dequeue, the function will time out immediately.
		/// </remarks>
		property TimeSpan AcceptQueueWaitTime;

		/// <summary>
		/// Gets or sets the Internet Protocol address and port on which to listen for incoming connections.
		/// </summary>
		property IPEndPoint^ AcceptPoint
		{
			IPEndPoint^ get()
			{
				return acceptPoint;
			}

			void set(IPEndPoint^ value)
			{
				if (value == nullptr)
				{
					throw gcnew ArgumentNullException("value");
				}

				if ((value->AddressFamily != AddressFamily::InterNetwork) && (value->AddressFamily != AddressFamily::InterNetworkV6))
				{
					throw gcnew ArgumentOutOfRangeException("value.AddressFamily");
				}

				acceptPoint = value;
			}
		}

#pragma endregion
	};
}

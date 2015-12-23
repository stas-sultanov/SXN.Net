#pragma once

#include "Stdafx.h"

using namespace System;
using namespace System::Net;
using namespace System::Net::Sockets;

namespace SXN
{
	namespace Net
	{
		/// <summary>
		/// Specifies the configuration settings of the TCP worker.
		/// </summary>
		public ref class TcpWorkerSettings
		{
			private:

			#pragma region Fields

			static initonly Int32 processorsCount;

			static initonly UInt32 allocationGranularity;

			Int32 useProcessorsCount;

			IPEndPoint^ acceptPoint;

			#pragma endregion

			public:

			#pragma region Constant and Static Fields

			const Int32 MaxConnections = SOMAXCONN;

			#pragma endregion

			static TcpWorkerSettings()
			{
				// get number of processors
				SYSTEM_INFO sysinfo;

				::GetSystemInfo(&sysinfo);

				processorsCount = sysinfo.dwNumberOfProcessors;

				allocationGranularity = sysinfo.dwAllocationGranularity;
			}


			#pragma region Properties

			/// <summary>
			/// The number of processors on the current machine.
			/// </summary>
			static property Int32 ProcessorsCount
			{
				Int32 get()
				{
					return processorsCount;
				}
			}

			/// <summary>
			/// The length in bytes of the memory buffer for receive operations.
			/// </summary>
			/// <remarks>
			/// Must be less than maximum number of ports.
			/// Value will be ceiled to the number of used processors.
			/// </remarks>
			property UInt32 ConnectionsBacklogLength;

			/// <summary>
			/// The length in bytes of the memory buffer for receive operations.
			/// </summary>
			/// <remarks>
			/// Value will be ceiled.
			/// </remarks>
			property Int32 ReceiveBufferLength;

			/// <summary>
			/// The length in bytes of the memory buffer for receive operations.
			/// </summary>
			/// <remarks>
			/// Value will be ceiled.
			/// </remarks>
			property Int32 SendBufferLength;

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

			/// <summary>
			/// The number of processors to use.
			/// </summary>
			/// <remarks>
			/// If value is not specified or is zero or is greater than actual number of processors, then all available processors will be used.
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
				}
			}

			property UInt32 RIOMaxOutstandingReceive;

			property UInt32 RIOMaxOutstandingSend;

			/// <summary>
			/// The maximum number of entries to try to dequeue from the accept queue.
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
				}
			}

			/// <summary>
			/// The time to wait for a request packet to appear at the accept queue.
			/// If value is INFINITE(0xFFFFFFFF), the function will never time out.
			/// If dwMilliseconds is zero and there is no I / O operation to dequeue, the function will time out immediately.
			/// </summary>
			property TimeSpan AcceptQueueWaitTime;

			/// <summary>
			/// The Internet Protocol address and port on which to listen the incoming connections.
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


			internal:

			/**
			TcpWorkerSettings^ Validate()
			{
				// get number of procssors
				SYSTEM_INFO sysinfo;

				:: GetSystemInfo(&sysinfo);

				UInt16 numCPU = sysinfo.dwNumberOfProcessors;

				TcpWorkerSettings^ result = gcnew TcpWorkerSettings();

				if (NumberOfProcessors.HasValue)
				{
					if ((NumberOfProcessors.Value == 0) || (NumberOfProcessors.Value > numCPU))
					{
						throw gcnew ArgumentOutOfRangeException();
					}
				}

				result->NumberOfProcessors = NumberOfProcessors.HasValue ? NumberOfProcessors.Value == 0  : numCPU;
			}
			/**/
		};
	}
}
#pragma once

#include "Stdafx.h"


namespace SXN
{
	namespace Net
	{
		public ref class TcpWorker sealed
		{
			private:

			#pragma region Fields

			/// <summary>
			/// The descriptor of the listening socket.
			/// </summary>
			initonly SOCKET listenSocket;

			/// <summary>
			/// The completion port of the listening socket.
			/// </summary>
			initonly HANDLE completionPort;

			/// <summary>
			/// A reference to the object that provides work with the Winsock extensions.
			/// </summary>
			initonly Winsock* pWinsock;

			/// <summary>
			/// The collection of the workers.
			/// </summary>
			initonly array<IocpWorker^>^ workers;

			initonly Thread^ mainThread;

			initonly Func<Connection^, System::Threading::Tasks::Task^>^ serveSocket;

			/// <summary>
			/// The maximum number of entries to try to dequeue from the accept queue.
			/// </summary>
			initonly UInt32 acceptQueueMaxEntriesCount;

			#pragma endregion

			public:

			#pragma region Methods

			/// <summary>
			/// Initializes a new instance of the <see cref="TcpWorker" /> class.
			/// </summary>
			TcpWorker(TcpWorkerSettings^ settings, Func<Connection^, System::Threading::Tasks::Task^>^ serveSocket)
			{
				this->serveSocket = serveSocket;

				// initialize Winsock
				{
					WSADATA data;

					auto startupResultCode = ::WSAStartup(MAKEWORD(2, 2), &data);

					// check if startup was successful
					if (startupResultCode != 0)
					{
						// get error code
						auto winsockErrorCode = (WinsockErrorCode)startupResultCode;

						// throw exception
						throw gcnew TcpServerException(winsockErrorCode);
					}
				}

				// initialize listen socket
				{
					listenSocket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_REGISTERED_IO);

					// check if operation has failed
					if (listenSocket == INVALID_SOCKET)
					{
						// get error code
						auto winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

						// throw exception
						throw gcnew TcpServerException(winsockErrorCode);
					}
				}

				// configure listen socket
				{
					auto configResult = Configure(listenSocket, settings);

					if (!configResult)
					{
						// get error code
						auto winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

						// throw exception
						throw gcnew TcpServerException(winsockErrorCode);
					}
				}

				// initialize winsock extensions
				{
					pWinsock = Winsock::Initialize(listenSocket);

					if (pWinsock == nullptr)
					{
						// get error code
						auto winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

						// throw exception
						throw gcnew TcpServerException(winsockErrorCode);
					}
				}

				// initialize IOCP
				{
					// create I/O completion port
					completionPort = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);

					// check if operation has failed
					if (completionPort == nullptr)
					{
						// get error code
						auto kernelErrorCode = ::GetLastError();

						// throw exception
						throw gcnew TcpServerException(kernelErrorCode);
					}

					// associate the listening socket with the completion port
					HANDLE associateResult = ::CreateIoCompletionPort((HANDLE)listenSocket, completionPort, 0, 0);

					if ((associateResult == nullptr) || (associateResult != completionPort))
					{
						// get error code
						auto kernelErrorCode = ::GetLastError();

						// throw exception
						throw gcnew TcpServerException(kernelErrorCode);
					}
				}

				// start listen
				{
					auto configResult = StartListen(listenSocket, settings);

					if (!configResult)
					{
						// get error code
						auto winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

						// throw exception
						throw gcnew TcpServerException(winsockErrorCode);
					}
				}

				// create and configure sub workers
				{
					// get count of processors
					auto processorsCount = TcpWorkerSettings::ProcessorsCount;

					// get the length of the connections backlog per processor
					auto perWorkerConnectionBacklogLength = settings->ConnectionsBacklogLength / processorsCount;

					// 4 create collection of the IOCP workers
					workers = gcnew array<IocpWorker^>(processorsCount);

					// initialize workers
					for (int processorIndex = 0; processorIndex < processorsCount; processorIndex++)
					{
						// create process worker
						auto worker = gcnew IocpWorker(listenSocket, *pWinsock, processorIndex, settings->ReceiveBufferLength, perWorkerConnectionBacklogLength);

						// add to collection
						workers[processorIndex] = worker;
					}
				}

				// initialize and run main thread
				{ 
					auto mainThreadDelegate = gcnew ThreadStart(this, &TcpWorker::ProcessAcceptRequests);

					mainThread = gcnew Thread(mainThreadDelegate);

					mainThread->Start();
				}
			}

			private:

			static Boolean Configure(SOCKET listenSocket, TcpWorkerSettings^ settings)
			{
				// disable use of the Nagle algorithm if requested
				if (settings->UseNagleAlgorithm == false)
				{
					auto boolValue = (BOOL) TRUE;

					auto disableNagleResult = ::setsockopt(listenSocket, IPPROTO_TCP, TCP_NODELAY, (const char *)&boolValue, sizeof(BOOL));

					// check if operation has failed
					if (disableNagleResult == SOCKET_ERROR)
					{
						return false;
					}

					auto resultLength = (int) sizeof(BOOL);

					auto getNagleStatus = getsockopt(listenSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&boolValue, &resultLength);

					Console::WriteLine("socket {0} TCP_NODELAY state {1}", listenSocket, boolValue);

					/* EXPEREMENTAL */
					int intValue = 0;

					int setBufferResult = ::setsockopt(listenSocket, SOL_SOCKET, SO_SNDBUF, (const char *)&intValue, sizeof(int));

					// check if operation has failed
					if (setBufferResult == SOCKET_ERROR)
					{
						return false;
					}/**/
				}

				// enable faster operations on the loop-back if requested
				if (settings->UseFastLoopback == true)
				{
					UInt32 optionValue = 1;

					DWORD dwBytes;

					int enableFastLoopbackResult = ::WSAIoctl(listenSocket, SIO_LOOPBACK_FAST_PATH, &optionValue, sizeof(UInt32), nullptr, 0, &dwBytes, nullptr, nullptr);

					// check if attempt has succeed
					if (enableFastLoopbackResult == SOCKET_ERROR)
					{
						return false;
					}
				}

				return true;
			}

			static Boolean StartListen(SOCKET listenSocket, TcpWorkerSettings^ settings)
			{
				// bind
				{
					auto acceptPoint = settings->AcceptPoint;

					// compose socket address
					SOCKADDR_IN socketAddress;

					// set address family
					socketAddress.sin_family = (ADDRESS_FAMILY) acceptPoint->AddressFamily;

					// set port
					socketAddress.sin_port = ::htons(acceptPoint->Port);

					// set address
					if (acceptPoint->AddressFamily == System::Net::Sockets::AddressFamily::InterNetwork)
					{
						IN_ADDR address;

						address.S_un.S_addr = 0;

						socketAddress.sin_addr = address;
					}
					else
					{
						IN6_ADDR address;

						//address.u.Word[0] = 0;

						//socketAddress.sin_addr = address;
					}

					// associate address with socket
					auto bindResult = ::bind(listenSocket, (sockaddr *)&socketAddress, sizeof(SOCKADDR_IN));

					if (bindResult == SOCKET_ERROR)
					{
						return false;
					}
				}

				// start listen
				{
					auto startListen = ::listen(listenSocket, settings->ConnectionsBacklogLength);

					if (startListen == SOCKET_ERROR)
					{
						return false;
					}
				}

				return true;
			}

			#pragma endregion

			#pragma region Static Constructor

			[System::Security::SuppressUnmanagedCodeSecurity]
			void ProcessAcceptRequests()
			{
				ULONG maxEntries = acceptQueueMaxEntriesCount;

				// allocate array of completion entries
				auto completionPortEntries = (LPOVERLAPPED_ENTRY) ::VirtualAlloc(nullptr, sizeof(OVERLAPPED_ENTRY) * maxEntries, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

				DWORD waitTime = (DWORD) -1;

				// will contain number of entries removed from the completion queue
				ULONG numEntriesRemoved;

				while (true)
				{
					// dequeue entries
					auto dequeueResult = ::GetQueuedCompletionStatusEx(completionPort, completionPortEntries, maxEntries, &numEntriesRemoved, waitTime, FALSE);

					// check if operation has failed
					if (dequeueResult == FALSE)
					{
						continue;
					}

					for (auto entryIndex = 0; entryIndex < numEntriesRemoved; entryIndex++)
					{
						// get entry
						auto entry = completionPortEntries[entryIndex];

						// get structure that was specified when the completed I/O operation was started
						auto overlapped = (Ovelapped*) entry.lpOverlapped;

						// get identifier of the worker
						auto workerId = overlapped->workerId;

						// get worker
						auto worker = this->workers[workerId];

						// get identifier of the connection
						auto connectionId = overlapped->connectionId;

						// get connection
						auto connection = worker->managedConnections[connectionId];

						//Console::WriteLine("Accepted {0}", overlapped->connectionSocket);

						// queue connection to processing chain
						auto waitCallback = gcnew WaitCallback(this, &TcpWorker::Serve);

						ThreadPool::UnsafeQueueUserWorkItem(waitCallback, (Object ^)connection);
					}
				}

				// free allocated memory
				::VirtualFree(completionPortEntries, 0, MEM_RELEASE);
			}

			void Serve(Object^ state)
			{
				auto connection = (Connection ^) state;

				connection->connection->EndAccepet();

				// #pragma warning disable CS4014 // Because this call is not awaited, execution of the current method continues before the call is completed

				serveSocket(connection);

				//#pragma warning restore CS4014
			}

			#pragma endregion
		};
	}
}
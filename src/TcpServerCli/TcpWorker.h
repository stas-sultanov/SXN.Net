#pragma once

#include "Stdafx.h"
#include "TcpWorkerSettings.h"
#include "WinsockEx.h"
#include "IocpWorker.h"

namespace SXN
{
	namespace Net
	{
		public ref class TcpWorker
		{
			private:

			#pragma region Fields

			/// <summary>
			/// The descriptor of the listening socket.
			/// <summary/>
			initonly SOCKET listenSocket;

			/// <summary>
			/// The completion port of the listening socket.
			/// </summary>
			initonly HANDLE completionPort;

			/// <summary>
			/// A pointer to the object that provides work with Winsock extensions.
			/// <summary/>
			initonly WinsockEx* pWinsockEx;

			/// <summary>
			/// The collection of the workers.
			/// </summary>
			initonly array<IocpWorker^>^ workers;

			initonly Thread^ mainThread;

			#pragma endregion

			public:

			#pragma region Methods

			/// <summary>
			/// Activates server.
			/// </summary>
			TcpWorker(TcpWorkerSettings settings)
			{
				// 0 initialize Winsock
				{
					WSADATA data;

					int startupResultCode = ::WSAStartup(MAKEWORD(2, 2), &data);

					// check if startup was successful
					if (startupResultCode != 0)
					{
						// get error code
						WinsockErrorCode winsockErrorCode = (WinsockErrorCode)startupResultCode;

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
						WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

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
						DWORD kernelErrorCode = ::GetLastError();

						// throw exception
						throw gcnew TcpServerException(kernelErrorCode);
					}

					// associate the listening socket with the completion port
					HANDLE associateResult = ::CreateIoCompletionPort((HANDLE)listenSocket, completionPort, 0, 0);

					if ((associateResult == nullptr) || (associateResult != completionPort))
					{
						// get error code
						DWORD kernelErrorCode = ::GetLastError();

						// throw exception
						throw gcnew TcpServerException(kernelErrorCode);
					}
				}

				// initialize winsock extensions
				{
					pWinsockEx = WinsockEx::Initialize(listenSocket);

					if (pWinsockEx == nullptr)
					{
						// get error code
						WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

						// throw exception
						throw gcnew TcpServerException(winsockErrorCode);
					}
				}

				// try configure server socket and start listen
				{
					Boolean configResult = ConfigureBindAndStartListen(listenSocket, settings);

					if (!configResult)
					{
						// get error code
						WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

						// throw exception
						throw gcnew TcpServerException(winsockErrorCode);
					}
				}

				// create and configure sub workers
				{
					// 3 get count of processors
					int processorsCount = TcpWorkerSettings::ProcessorsCount;

					// get the length of the connections backlog per processor
					int perWorkerConnectionBacklogLength = settings.ConnectionsBacklogLength / processorsCount;

					// 4 create collection of the IOCP workers
					workers = gcnew array<IocpWorker^>(processorsCount);

					// initialize workers
					for (int processorIndex = 0; processorIndex < processorsCount; processorIndex++)
					{
						// create process worker
						IocpWorker^ worker = gcnew IocpWorker(listenSocket, pWinsockEx, processorIndex, settings.ReceiveBufferLength, perWorkerConnectionBacklogLength);

						// add to collection
						workers[processorIndex] = worker;
					}
				}

				ThreadStart^ threadDelegate = gcnew ThreadStart(this, &TcpWorker::AcceptConnections);

				mainThread = gcnew Thread(threadDelegate);

				mainThread->Start();
			}

			private:

			static Boolean ConfigureBindAndStartListen(SOCKET listenSocket, TcpWorkerSettings settings)
			{
				// try disable use of the Nagle algorithm if requested
				if (settings.UseNagleAlgorithm == false)
				{
					DWORD optionValue = -1;

					int disableNagleResult = ::setsockopt(listenSocket, IPPROTO_TCP, TCP_NODELAY, (const char *)&optionValue, sizeof(Int32));

					// check if operation has failed
					if (disableNagleResult == SOCKET_ERROR)
					{
						return false;
					}
				}

				// try enable faster operations on the loopback if requested
				if (settings.UseFastLoopback)
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

				// try bind
				{
					// compose address
					IN_ADDR address;

					address.S_un.S_addr = 0;

					// compose socket address
					SOCKADDR_IN socketAddress;

					socketAddress.sin_family = AF_INET;
					socketAddress.sin_port = ::htons(settings.Port);
					socketAddress.sin_addr = address;

					// try associate address with socket
					int bindResult = ::bind(listenSocket, (sockaddr *)&socketAddress, sizeof(SOCKADDR_IN));

					if (bindResult == SOCKET_ERROR)
					{
						return false;
					}
				}

				// try start listen
				{
					int startListen = ::listen(listenSocket, settings.ConnectionsBacklogLength);

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
			void AcceptConnections()
			{
				// define array of completion entries
				OVERLAPPED_ENTRY completionPortEntries[128];

				// will contain number of entries removed from the completion queue
				ULONG numEntriesRemoved;

				while (true)
				{
					// dequeue completion status
					BOOL dequeueResult = ::GetQueuedCompletionStatusEx(completionPort, completionPortEntries, 128, &numEntriesRemoved, WSA_INFINITE, FALSE);

					// check if operation has failed
					if (dequeueResult == FALSE)
					{
						continue;
						// TODO: ABORT
					}

					for (unsigned int entryIndex = 0; entryIndex < numEntriesRemoved; entryIndex++)
					{
						// get entry
						OVERLAPPED_ENTRY entry = completionPortEntries[entryIndex];

						// get structure that was specified when the completed I/O operation was started
						WSAOVERLAPPEDPLUS* overlapped = (WSAOVERLAPPEDPLUS*) entry.lpOverlapped;

						// repost completion status
						::PostQueuedCompletionStatus(overlapped->completionPort, entry.dwNumberOfBytesTransferred, SOCK_ACTION_ACCEPT, overlapped);
					}

					//System::Console::WriteLine("MAIN Thread: something, completionPort: {0} numberOfBytesTransferred: {1} completionKey: {2}", (Int32)completionPort, (Int32)numberOfBytesTransferred, (Int32)completionKey);
				}
			}

			#pragma endregion
		};
	}
}
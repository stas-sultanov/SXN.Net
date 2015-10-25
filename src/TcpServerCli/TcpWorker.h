#pragma once

#include "Stdafx.h"
#include "TcpWorkerSettings.h"
#include "winsock.h"
#include "IocpWorker.h"


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

			initonly Func<ConnectionHandle^, System::Threading::Tasks::Task^>^ serveSocket;

			// queue connection to processing chain
			initonly WaitCallback^ serveWaitCallback = gcnew WaitCallback(this, &TcpWorker::Serve);

			#pragma endregion

			public:

			#pragma region Methods

			/// <summary>
			/// Activates server.
			/// </summary>
			TcpWorker(TcpWorkerSettings settings, Func<ConnectionHandle^, System::Threading::Tasks::Task^>^ serveSocket)
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
				/**
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
				/**/

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
				/**
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
				/**/


				// create and configure sub workers
				{
					// 3 get count of processors
					auto processorsCount = TcpWorkerSettings::ProcessorsCount;

					// get the length of the connections backlog per processor
					auto perWorkerConnectionBacklogLength = settings.ConnectionsBacklogLength / processorsCount;

					// 4 create collection of the IOCP workers
					workers = gcnew array<IocpWorker^>(processorsCount);

					// initialize workers
					for (int processorIndex = 0; processorIndex < processorsCount; processorIndex++)
					{
						// create process worker
						auto worker = gcnew IocpWorker(listenSocket, pWinsock, processorIndex, settings.ReceiveBufferLength, perWorkerConnectionBacklogLength);

						// add to collection
						workers[processorIndex] = worker;
					}
				}

				// initialize and run main thread
				/**
				{
					auto threadDelegate = gcnew ThreadStart(this, &TcpWorker::ProcessAcceptRequests);

					mainThread = gcnew Thread(threadDelegate);

					mainThread->Start();
				}
				/**/


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
			}

			private:

			static Boolean Configure(SOCKET listenSocket, TcpWorkerSettings settings)
			{
				// disable use of the Nagle algorithm if requested
				if (settings.UseNagleAlgorithm == false)
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
				if (settings.UseFastLoopback == true)
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

			static Boolean StartListen(SOCKET listenSocket, TcpWorkerSettings settings)
			{

				// try bind
				{
					// compose address
					IN_ADDR address;

					//address.S_un.S_addr = 0;
					address.S_un.S_un_b.s_b1 = 127;
					address.S_un.S_un_b.s_b2 = 0;
					address.S_un.S_un_b.s_b3 = 0;
					address.S_un.S_un_b.s_b4 = 1;

					// compose socket address
					SOCKADDR_IN socketAddress;

					socketAddress.sin_family = AF_INET;
					socketAddress.sin_port = ::htons(5000);
					socketAddress.sin_addr = address;

					// try associate address with socket
					auto bindResult = ::bind(listenSocket, (sockaddr *)&socketAddress, sizeof(SOCKADDR_IN));

					if (bindResult == SOCKET_ERROR)
					{
						return false;
					}
				}

				// try start listen
				{
					auto startListen = ::listen(listenSocket, 10);

					if (startListen == SOCKET_ERROR)
					{
						return false;
					}
				}

				return true;
			}

			#pragma endregion

			#pragma region Static Constructor

			public:

			[System::Security::SuppressUnmanagedCodeSecurity]
			void ProcessAcceptRequests()
			{
				auto lastWorkerId = this->workers->Length - 1;

				// the identifier of the worker that will serve connection
				auto workerId = 0;

				while (true)
				{
					// accept connection
					auto acceptedSocket = ::accept(listenSocket, nullptr, nullptr);

					// check if operation has failed
					if (acceptedSocket == INVALID_SOCKET)
					{
						break;
					}

					// get worker
					auto worker = this->workers[workerId];

					// get connection handle
					auto connectionHandle = worker->GetHandle(acceptedSocket);

					if (connectionHandle == nullptr)
					{
						continue;
					}

					//Console::WriteLine("Accepted: {0} Worker: {1} Connection: {2}", acceptedSocket, workerId, connectionHandle->Id);

					ThreadPool::UnsafeQueueUserWorkItem(serveWaitCallback, (Object ^)connectionHandle);

					workerId = workerId == lastWorkerId ? 0 : workerId + 1;
				}
			}

			[System::Security::SuppressUnmanagedCodeSecurity]
			void Serve(Object^ state)
			{
				auto connection = (ConnectionHandle ^) state;

				// connection->connection->EndAccepet();

				// #pragma warning disable CS4014 // Because this call is not awaited, execution of the current method continues before the call is completed

				serveSocket(connection);

				//#pragma warning restore CS4014
			}

			#pragma endregion
		};
	}
}
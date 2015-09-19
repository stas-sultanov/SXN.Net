#pragma once

#include "Stdafx.h"
#include "WinsockErrorCode.h"
#include "TcpWorkerSettings.h"
#include "TcpServerException.h"
#include "WinsockHandle.h"
#include "IocpWorker.h"

namespace SXN
{
	namespace Net
	{
		public ref class TcpWorker
		{
			private:

			#pragma region Fields

			///<summary>
			///The handle of the Winsock extensions socket.
			///<summary/>
			initonly WinSocket^ serverSocket;

			/// <summary>
			/// The collection of the workers.
			/// </summary>
			initonly array<IocpWorker^>^ workers;

			#pragma endregion

			public:

			#pragma region Methods

			/// <summary>
			/// Activates server.
			/// </summary>
			TcpWorker(TcpWorkerSettings settings)
			{
				// 0 initiate use of the Winsock DLL by a process
				{
					WSADATA data;

					int startupResultCode = ::WSAStartup(MAKEWORD(2, 2), &data);

					// check if startup was successful
					if (startupResultCode != 0)
					{
						// get error code
						WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

						// throw exception
						throw gcnew TcpServerException(winsockErrorCode);
					}
				}

				// 2 initialize Winsock extensions handle
				serverSocket = gcnew WinSocket();

				// try configure server socket and start listen
				if (!TryConfigureBindAndStartListen(serverSocket, settings))
				{
					// get error code
					WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

					// throw exception
					throw gcnew TcpServerException(winsockErrorCode);
				}

				// 3 get count of processors
				int processorsCount = TcpWorkerSettings::ProcessorsCount;

				// get the length of the connections backlog per processor
				int perWorkerConnectionBacklogLength = settings.ConnectinosBacklogLength / processorsCount;

				// 4 create collection of the IOCP workers
				workers = gcnew array<IocpWorker^>(processorsCount);

				// initialize workers
				for (int processorIndex = 0; processorIndex < TcpWorkerSettings::ProcessorsCount; processorIndex++)
				{
					// create process worker
					IocpWorker^ worker = gcnew IocpWorker(serverSocket, processorIndex, settings.ReciveBufferLength, perWorkerConnectionBacklogLength);

					// add to collection
					workers[processorIndex] = worker;
				}


			}


			private:

			static Boolean TryConfigureBindAndStartListen(WinSocket^ serverSocket, TcpWorkerSettings settings)
			{
				// try disable use of the Nagle algorithm if requested
				if (settings.UseNagleAlgorithm == false)
				{
					DWORD optionValue = -1;

					int disableNagleResult = serverSocket->setsockopt(IPPROTO_TCP, TCP_NODELAY, (const char *) &optionValue, sizeof(Int32));

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

					int enableFastLoopbackResult = serverSocket->WSAIoctl(SIO_LOOPBACK_FAST_PATH, &optionValue, sizeof(UInt32), NULL, 0, &dwBytes, NULL, NULL);

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
					int bindResult = serverSocket->bind((sockaddr *) &socketAddress, sizeof(SOCKADDR_IN));

					if (bindResult == SOCKET_ERROR)
					{
						return false;
					}
				}

				// try start listen
				{
					int startListen = serverSocket->listen(200);

					if (startListen == SOCKET_ERROR)
					{
						return false;
					}
				}

				return true;
			}

			#pragma endregion
		};
	}
}
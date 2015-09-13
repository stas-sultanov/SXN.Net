#pragma once

#include "Stdafx.h"
#include "WinsockErrorCode.h"
#include "TcpWorkerSettings.h"
#include "WinsockHandle.h"

namespace SXN
{
	namespace Net
	{
		public ref class TcpWorker
		{
			private:

			#pragma region Fields

			///<summary>
			///The handle of the server socket.
			///<summary/>
			initonly SOCKET serverSocket;

			///<summary>
			///The handle of the Winsock extensions socket.
			///<summary/>
			initonly WinsockHandle* winsockHandle;

			#pragma endregion

			#pragma region Methods

			/// <summary>
			/// Activates server.
			/// </summary>
			public: static TryResult<TcpWorker> TryInitialize(TcpWorkerSettings settings)
			{
				// 0 try initiates use of the Winsock DLL by a process
				/*{
					WSADATA data;

					int startupResultCode = ::WSAStartup(MAKEWORD(2, 2), &data);

					// check if startup was successful
					if (startupResultCode != WinsockErrorCode.None)
					{
						return TryResult<TcpWorker>.CreateFail(startupResultCode);
					}
				}*/

				// 1 try create server socket
				SOCKET serverSocket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_REGISTERED_IO);

				// check if socket is created
				if (INVALID_SOCKET == serverSocket)
				{
					goto FAIL;
				}

				// 2 try initialize Winsock extensions handle
				WinsockHandle* winsockHandle;

				if (!WinsockHandle::TryCreate(serverSocket, winsockHandle))
				{
					goto FAIL;
				}

				// 3 get count of processors
				var processorsCount = Environment.ProcessorCount;

				// 4 create collection of the IOCP workers
				var workers = new IocpWorker[processorsCount];

				// initialize workers
				for (var processorIndex = 0; processorIndex < processorsCount; processorIndex++)
				{
					// try create worker
					var tryCreateWorker = IocpWorker.TryCreate(rioHandle, processorIndex, 4096, 1024);

					// check if operation has succeed
					if (!tryCreateWorker.Success)
					{
						goto FAIL;
					}

					// add to collection
					workers[processorIndex] = tryCreateWorker.Result;
				}

				// try configure server socket and start listen
				if (!TryConfigureBindAndStartListen(serverSocket, settings))
				{
					goto FAIL;
				}

				// success
				var server = new TcpWorker(serverSocket, rioHandle, workers);

				foreach(var worker in workers)
				{
					worker.thread.Start();
				}

				return TryResult<TcpWorker>.CreateSuccess(server);

				FAIL:

				// get last error
				var errorCode = (WinsockErrorCode)WinsockInterop.WSAGetLastError();

				// terminate use of the Winsock DLL
				WinsockInterop.WSACleanup();

				// return fail
				return TryResult<TcpWorker>.CreateFail(errorCode);
			}
		};
	}
}
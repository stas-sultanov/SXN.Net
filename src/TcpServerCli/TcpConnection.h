#pragma once

#include "Stdafx.h"
#include "WinsockEx.h"
#include "Overlap.h"

namespace SXN
{
	namespace Net
	{
		public ref class TcpConnection
		{
			public:

			#pragma region Fields

			WinsockEx* pWinsockEx;

			initonly SOCKET listenSocket;

			initonly SOCKET socket;

			initonly RIO_RQ rioRequestQueue;

			initonly PVOID addrBuf;

			initonly WSAOVERLAPPEDPLUS* acceptOverlapped;

			initonly WSAOVERLAPPEDPLUS* disconnectOverlaped;

			int id;

			PRIO_BUF receiveBuffer;

			PRIO_BUF sendBuffer;

			#pragma endregion

			internal:

			#pragma region Constructor

			/// <summary>
			/// Initializes a new instance of the <see cref="TcpConnection" /> class.
			/// </summary>
			inline TcpConnection(SOCKET listenSocket, WinsockEx* pWinsockEx, int id, SOCKET socket, RIO_RQ requestQueue)
			{
				this->listenSocket = listenSocket;

				this->pWinsockEx = pWinsockEx;

				this->id = id;

				this->socket = socket;

				this->rioRequestQueue = requestQueue;

				this->addrBuf = new char[(sizeof(sockaddr_in) + 16) * 2];

				this->acceptOverlapped = new WSAOVERLAPPEDPLUS();

				memset(acceptOverlapped, 0, sizeof(WSAOVERLAPPEDPLUS));

				acceptOverlapped->action = SOCK_ACTION_ACCEPT;

				acceptOverlapped->connectionId = id;

				acceptOverlapped->connectionSocket = socket;

				this->disconnectOverlaped = new WSAOVERLAPPEDPLUS();

				memset(disconnectOverlaped, 0, sizeof(WSAOVERLAPPEDPLUS));

				disconnectOverlaped->action = SOCK_ACTION_DISCONNECT;

				disconnectOverlaped->connectionId = id;

				acceptOverlapped->connectionSocket = socket;
			}

			!TcpConnection()
			{
				delete addrBuf;
			}

			BOOL StartAccept()
			{
				DWORD dwBytes;

				return pWinsockEx->AcceptEx(listenSocket, socket, addrBuf, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &dwBytes, acceptOverlapped);
			}

			BOOL StartRecieve()
			{
				return pWinsockEx->RIOReceive(rioRequestQueue, receiveBuffer, 1, 0, (LPVOID)id);

				/*{
					// get error code
					WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

					// throw exception
					throw gcnew TcpServerException(winsockErrorCode);
				}*/
			}

			BOOL StartSend(DWORD dataLength)
			{
				sendBuffer->Length = dataLength;

				BOOL result = pWinsockEx->RIOSend(rioRequestQueue, sendBuffer, 1, 0, (LPVOID)id);

				if (!result)
				{
					// get error code
					WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

					Console::WriteLine("buffer id: {0}, length: {1}, offset: {2}", (Int32)sendBuffer->BufferId, (Int32)sendBuffer->Length, (Int32)sendBuffer->Offset);

					// throw exception
					throw gcnew TcpServerException(winsockErrorCode);
				}

				return result;
			}

			BOOL StartDisconnect()
			{
				return pWinsockEx->DisconnectEx(socket, disconnectOverlaped, TF_REUSE_SOCKET, 0);
			}

			#pragma endregion
		};
	}
}
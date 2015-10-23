#pragma once

#include "Stdafx.h"
#include "Winsock.h"
#include "Ovelapped.h"

#pragma unmanaged

namespace SXN
{
	namespace Net
	{
		public class TcpConnection final
		{
			public :

			#pragma region Fields

			/// <summary>
			/// A reference to the object that provides work with the Winsock extensions.
			/// </summary>
			Winsock& winsock;

			/// <summary>
			/// The descriptor of the listening socket.
			/// </summary>
			SOCKET listenSocket;

			/// <summary>
			/// The descriptor of the connection socket.
			/// </summary>
			SOCKET connectionSocket;

			/// <summary>
			/// The descriptor of the request queue of the Registered IO of the socket.
			/// </summary>
			RIO_RQ rioRequestQueue;

			PVOID addrBuf;

			Ovelapped* acceptOverlapped;

			Ovelapped* disconnectOverlaped;

			PRIO_BUF receiveBuffer;

			PRIO_BUF sendBuffer;

			ULONG id;

			#pragma endregion

			public:

			ConnectionState state;

			#pragma region Constructor & Destructor

			/// <summary>
			/// Initializes a new instance of the <see cref="TcpConnection" /> class.
			/// </summary>
			inline TcpConnection(Winsock& winsock, SOCKET listenSocket, SOCKET connectionSocket, RIO_RQ rioRequestQueue, HANDLE complitionPort, ULONG id, ULONG workerId)
				: winsock(winsock)
			{
				this->id = id;

				this->listenSocket = listenSocket;

				this->connectionSocket = connectionSocket;

				this->rioRequestQueue = rioRequestQueue;

				this->addrBuf = new char[(sizeof(sockaddr_in) + 16) * 2];

				{
					this->acceptOverlapped = new Ovelapped();

					memset(acceptOverlapped, 0, sizeof(Ovelapped));

					acceptOverlapped->connectionId = id;

					acceptOverlapped->workerId = workerId;

					acceptOverlapped->action = SOCK_ACTION_ACCEPT;

					acceptOverlapped->connection = this;

					acceptOverlapped->connectionSocket = connectionSocket;

					acceptOverlapped->completionPort = complitionPort;
				}

				{
					this->disconnectOverlaped = new Ovelapped();

					memset(disconnectOverlaped, 0, sizeof(Ovelapped));

					disconnectOverlaped->action = SOCK_ACTION_DISCONNECT;

					disconnectOverlaped->connection = this;

					disconnectOverlaped->connectionSocket = connectionSocket;

					disconnectOverlaped->completionPort = complitionPort;
				}

				state = ConnectionState::Disconnected;
			}

			inline ~TcpConnection()
			{
				delete addrBuf;
			}

			#pragma endregion

			#pragma region Methods

			inline BOOL StartAccept()
			{
				state = ConnectionState::Accepting;

				DWORD dwBytes;

				return winsock.AcceptEx(listenSocket, connectionSocket, addrBuf, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &dwBytes, acceptOverlapped);
			}

			inline int EndAccepet()
			{
				return ::setsockopt(connectionSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char *)&listenSocket, sizeof(SOCKET));
			}

			inline void GetSourceAddress()
			{
				//winsock.GetAcceptExSockaddrs();
			}

			inline BOOL StartRecieve()
			{
				state = ConnectionState::Receiving;

				return winsock.RIOReceive(rioRequestQueue, receiveBuffer, 1, 0, (PVOID) id);
			}

			inline BOOL StartSend(DWORD dataLength)
			{
				state = ConnectionState::Sending;

				sendBuffer->Length = dataLength;

				return winsock.RIOSend(rioRequestQueue, sendBuffer, 1, 0, (PVOID) id);
			}

			inline BOOL StartDisconnect()
			{
				state = ConnectionState::Disconnecting;

				//return winsock.DisconnectEx(connectionSocket, disconnectOverlaped, TF_REUSE_SOCKET, 0);

				return winsock.DisconnectEx(connectionSocket, NULL, TF_REUSE_SOCKET, 0);
			}

			#pragma endregion
		};
	}
}

#pragma managed
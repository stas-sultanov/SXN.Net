#pragma once

#include "Stdafx.h"
#include "Winsock.h"
#include "Ovelapped.h"

#pragma unmanaged

namespace SXN
{
	namespace Net
	{
		/// <summary>
		/// Provides work with a TCP connection.
		/// </summary>
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
			/// The descriptor of the socket within the Registered I/O extension.
			/// </summary>
			RIO_RQ rioRequestQueue;

			/// <summary>
			/// The descriptor of the portion of the registered buffer used for receiving data.
			/// </summary>
			PRIO_BUF rioReceiveBuffer;

			/// <summary>
			/// The descriptor of the portion of the registered buffer used for sending data.
			/// </summary>
			PRIO_BUF rioSendBuffer;

			/// <summary>
			/// The pointer to the IP address of the client which has requested the connection.
			/// </summary>
			PVOID clientAddress;

			Ovelapped* acceptOverlapped;

			Ovelapped* disconnectOverlaped;

			ULONG id;

			#pragma endregion

			public:

			ConnectionState state;

			#pragma region Constructor & Destructor

			/// <summary>
			/// Initializes a new instance of the <see cref="TcpConnection" /> class.
			/// </summary>
			/// <param name="winsock">A reference to the object that provides work with the Winsock extensions.</param>
			/// <param name="listenSocket">The descriptor of the listening socket.</param>
			/// <param name="connectionSocket">The descriptor of the connection socket.</param>
			/// <param name="rioRequestQueue">The descriptor of the socket within the Registered I/O extension.</param>
			inline TcpConnection(Winsock& winsock, SOCKET listenSocket, SOCKET connectionSocket, RIO_RQ rioRequestQueue, HANDLE complitionPort, ULONG id, ULONG workerId)
				: winsock(winsock)
			{
				this->id = id;

				this->listenSocket = listenSocket;

				this->connectionSocket = connectionSocket;

				this->rioRequestQueue = rioRequestQueue;

				this->clientAddress = new char[(sizeof(sockaddr_in) + 16) * 2];

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
				delete clientAddress;
			}

			#pragma endregion

			#pragma region Methods

			inline BOOL StartAccept()
			{
				state = ConnectionState::Accepting;

				DWORD dwBytes;

				return winsock.AcceptEx(listenSocket, connectionSocket, clientAddress, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &dwBytes, acceptOverlapped);
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

				return winsock.RIOReceive(rioRequestQueue, rioReceiveBuffer, 1, 0, (PVOID) id);
			}

			inline BOOL StartSend(DWORD dataLength)
			{
				state = ConnectionState::Sending;

				rioSendBuffer->Length = dataLength;

				return winsock.RIOSend(rioRequestQueue, rioSendBuffer, 1, 0, (PVOID) id);
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
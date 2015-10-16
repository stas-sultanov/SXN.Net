#pragma once

#include "Stdafx.h"
#include "WinsockEx.h"
#include "Ovelapped.h"

#pragma unmanaged

namespace SXN
{
	namespace Net
	{
		public class TcpConnection final
		{
			public:

			#pragma region Fields

			/// <summary>
			/// A reference to the object that provides work with Winsock extensions.
			/// </summary>
			WinsockEx& winsockEx;

			/// <summary>
			/// The descriptor of the listening socket.
			/// <summary/>
			SOCKET listenSocket;

			/// <summary>
			/// The descriptor of the connection socket.
			/// <summary/>
			SOCKET connectionSocket;

			/// <summary>
			/// The descriptor of the request queue of the Registered IO of the socket.
			/// <summary/>
			RIO_RQ rioRequestQueue;

			PVOID addrBuf;

			Ovelapped* acceptOverlapped;

			Ovelapped* disconnectOverlaped;

			PRIO_BUF receiveBuffer;

			PRIO_BUF sendBuffer;

			#pragma endregion

			public:

			ConnectionState state;

			#pragma region Constructor & Destructor

			/// <summary>
			/// Initializes a new instance of the <see cref="TcpConnection" /> class.
			/// </summary>
			inline TcpConnection(WinsockEx& winsockEx, SOCKET listenSocket, SOCKET connectionSocket, RIO_RQ rioRequestQueue, HANDLE complitionPort)
				: winsockEx(winsockEx)
			{
				this->listenSocket = listenSocket;

				this->connectionSocket = connectionSocket;

				this->rioRequestQueue = rioRequestQueue;

				this->addrBuf = new char[(sizeof(sockaddr_in) + 16) * 2];

				{
					this->acceptOverlapped = new Ovelapped();

					memset(acceptOverlapped, 0, sizeof(Ovelapped));

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

				return winsockEx.AcceptEx(listenSocket, connectionSocket, addrBuf, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &dwBytes, acceptOverlapped);
			}

			inline int EndAccepet()
			{
				return ::setsockopt(connectionSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char *)&listenSocket, sizeof(SOCKET));
			}

			inline BOOL StartRecieve()
			{
				state = ConnectionState::Receiving;

				return winsockEx.RIOReceive(rioRequestQueue, receiveBuffer, 1, 0, this);
			}

			inline BOOL StartSend(DWORD dataLength)
			{
				state = ConnectionState::Sending;

				sendBuffer->Length = dataLength;

				return winsockEx.RIOSend(rioRequestQueue, sendBuffer, 1, RIO_MSG_DONT_NOTIFY, this);

				//sendBuffer->Length = 0;

				// TODO: bug here
				//return winsockEx.RIOSend(rioRequestQueue, nullptr, 0, RIO_MSG_COMMIT_ONLY, this);
			}

			inline BOOL StartDisconnect()
			{
				state = ConnectionState::Disconnecting;

				//return winsockEx.DisconnectEx(connectionSocket, disconnectOverlaped, TF_REUSE_SOCKET, 0);

				return winsockEx.DisconnectEx(connectionSocket, NULL, TF_REUSE_SOCKET, 0);

			}

			#pragma endregion
		};
	}
}

#pragma managed
#pragma once
#pragma once

#include "Stdafx.h"
#include "winsock.h"
#include "TcpServerException.h"
#include "RioTask.h"

using namespace System;
using namespace System::Threading;
using namespace System::Collections::Generic;

namespace SXN
{
	namespace Net
	{
		/// <summary>
		/// Provides handle to the connection.
		/// </summary>
		public ref class ConnectionHandle sealed
		{
			private:

			#pragma region Fields

			const char* testMessage = "HTTP/1.1 200 OK\r\nServer:SXN.Ion\r\nContent-Length:0\r\nDate:Sat, 26 Sep 2015 17:45:57 GMT\r\n\r\n";

			/// <summary>
			/// A reference to the object that provides work with the Winsock extensions.
			/// </summary>
			Winsock* winsock;

			/// <summary>
			/// The identifier of the handle within the worker.
			/// </summary>
			initonly ULONG id;

			/// <summary>
			/// The receive buffer.
			/// </summary>
			initonly PRIO_BUF receiveBuffer;

			/// <summary>
			/// The send buffer.
			/// </summary>
			initonly PRIO_BUF sendBuffer;

			/// <summary>
			/// The data receive task.
			/// </summary>
			initonly RioTask^ receiveTask;

			/// <summary>
			/// The data send task.
			/// </summary>
			initonly RioTask^ sendTask;

			/// <summary>
			/// The descriptor of the connection socket.
			/// </summary>
			SOCKET socket;

			/// <summary>
			/// The descriptor of the request queue of the Registered IO of the socket.
			/// </summary>
			RIO_RQ rioRequestQueue;

			ConnectionState state;

			PVOID addrBuf;

			#pragma endregion

			internal:

			#pragma region Constructor

			/// <summary>
			/// Initializes a new instance of the <see cref="ConnectionHandle" /> class.
			/// </summary>
			/// <param name="id">The identifier of the handle within the worker.</param>
			/// <param name="winsock">A reference to the object that provides work with the Winsock extensions.</param>
			inline ConnectionHandle(ULONG id, Winsock* winsock, PRIO_BUF receiveBuffer, PRIO_BUF sendBuffer)
			{
				// set id
				this->id = id;

				// set reference to winsock
				this->winsock = winsock;

				// set receive buffer
				this->receiveBuffer = receiveBuffer;

				// set send buffer
				this->sendBuffer = sendBuffer;

				// initialize receive task
				receiveTask = gcnew RioTask();

				// initialize send task
				sendTask = gcnew RioTask();

				// set state of the connection
				state = ConnectionState::Disconnected;
			}

			#pragma endregion

			#pragma region Methods

			inline Boolean BeginReceive()
			{
				state = ConnectionState::Receiving;

				return winsock->RIOReceive(rioRequestQueue, receiveBuffer, 1, 0, (PVOID) id);
			}

			inline void EndReceive(unsigned int bytesTransferred)
			{
				//Console::WriteLine("Connection[{0}]::EndReceive {1} bytes", connection->connectionSocket, bytesTransferred);

				state = ConnectionState::Received;

				receiveTask->Complete(bytesTransferred);
			}

			property UInt32 Id
			{
				UInt32 get()
				{
					return id;
				}
			}

			property ConnectionState State
			{
				ConnectionState get()
				{
					return state;
				}
			}

			inline BOOL StartSend(DWORD dataLength)
			{
				state = ConnectionState::Sending;

				sendBuffer->Length = dataLength;

				return winsock->RIOSend(rioRequestQueue, sendBuffer, 1, 0, (PVOID)id);
			}

			inline void EndSend(unsigned int bytesTransferred)
			{
				state = ConnectionState::Sent;

				//Console::WriteLine("Connection[{0}]::EndSend {1} bytes", connection->connectionSocket, bytesTransferred);

				sendTask->Complete(bytesTransferred);
			}


			inline void Reuse(SOCKET socket, RIO_RQ rioRequestQueue)
			{
				this->state = ConnectionState::Accepted;

				this->socket = socket;

				this->rioRequestQueue = rioRequestQueue;
			}

			public:

			#pragma region Public methods

			inline RioTask^ ReceiveAsync()
			{
				//Console::WriteLine("Connection[{0}]::ReceiveAsync", connection->connectionSocket);

				BeginReceive();

				return receiveTask;
			}

			inline RioTask^ SendAsync()
			{
				//Console::WriteLine("Connection[{0}]::SendAsync", connection->connectionSocket);

				StartSend(strlen(testMessage));

				return receiveTask;
			}

			inline void Disconnect()
			{
				state = ConnectionState::Disconnected;

				::closesocket(socket);
			}

			#pragma endregion
		};
	}
}
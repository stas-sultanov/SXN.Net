#pragma once

#include "pch.h"
#include "TcpConnection.h"
#include "ReceiveTask.h"

namespace SXN::Net
{
	public ref class Connection sealed
	{
	private:

	internal:

		const char* testMessage = "HTTP/1.1 200 OK\r\nServer:SXN.Ion\r\nContent-Length:0\r\nDate:Sat, 26 Sep 2015 17:45:57 GMT\r\n\r\n";

		TcpConnection* connection;

		initonly ReceiveTask^ receiveTask;

		initonly ReceiveTask^ sendTask;

		/// <summary>
		/// Initializes a new instance of the <see cref="Connection" /> class.
		/// </summary>
		/// <param name="connection">The connection.</param>
		inline Connection(TcpConnection* connection)
		{
			this->connection = connection;

			receiveTask = gcnew ReceiveTask(this);

			sendTask = gcnew ReceiveTask(this);
		}

		inline Boolean BeginReceive()
		{
			auto res = connection->StartRecieve();

			//Console::WriteLine("Connection[{0}]::BeginReceive {1}", connection->connectionSocket, res);

			return res;
		}

		inline void EndReceive(unsigned int bytesTransferred)
		{
			connection->state = ConnectionState::Received;

			//Console::WriteLine("Connection[{0}]::EndReceive {1} bytes", connection->connectionSocket, bytesTransferred);

			receiveTask->Complete(bytesTransferred);
		}

	public:

		inline ReceiveTask^ ReceiveAsync()
		{
			//Console::WriteLine("Connection[{0}]::ReceiveAsync", connection->connectionSocket);

			BeginReceive();

			return receiveTask;
		}

		property UInt32 Id
		{
			UInt32 get()
			{
				return connection->id;
			}
		}

		property ConnectionState State
		{
			ConnectionState get()
			{
				return connection->state;
			}
		}

		inline ReceiveTask^ SendAsync()
		{
			//Console::WriteLine("Connection[{0}]::SendAsync", connection->connectionSocket);

			connection->StartSend(strlen(testMessage));

			return receiveTask;
		}

		inline void EndSend(unsigned int bytesTransferred)
		{
			connection->state = ConnectionState::Transmitted;

			//Console::WriteLine("Connection[{0}]::EndSend {1} bytes", connection->connectionSocket, bytesTransferred);

			sendTask->Complete(bytesTransferred);
		}

		inline void Disconnect()
		{
			connection->state = ConnectionState::Disconnected;

			connection->StartDisconnect();

			connection->StartAccept();
		}
	};
}
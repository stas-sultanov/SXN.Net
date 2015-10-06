#pragma once

#include "Stdafx.h"

#define SOCK_ACTION_ACCEPT 2

#define SOCK_ACTION_RECEIVE 4

#define SOCK_ACTION_SEND 8

#define SOCK_ACTION_DISCONNECT 16

#pragma unmanaged




namespace SXN
{
	namespace Net
	{
		enum ConnectionState : USHORT
		{
			None = 0,

			Accepting,

			Accepted,

			Receiving,

			Received,

			Sending,

			Sent,

			Disconnecting,

			Disconnected
		};

		extern class TcpConnection;

		private struct Ovelapped final : OVERLAPPED
		{
			public:

			int action;

			TcpConnection* connection;

			int status;

			SOCKET connectionSocket;

			HANDLE completionPort;
		};
	}
}

#pragma managed
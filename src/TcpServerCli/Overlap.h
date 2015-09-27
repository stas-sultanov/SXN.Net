#pragma once

#include "Stdafx.h"

#define SOCK_ACTION_ACCEPT 2

#define SOCK_ACTION_RECEIVE 4

#define SOCK_ACTION_SEND 8

#define SOCK_ACTION_DISCONNECT 16

namespace SXN
{
	namespace Net
	{
		private struct WSAOVERLAPPEDPLUS final : OVERLAPPED
		{
			public:

			int action;

			int connectionId;

			SOCKET connectionSocket;
		};
	}
}
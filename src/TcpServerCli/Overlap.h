#pragma once

#include "Stdafx.h"

#define SOCK_ACTION_ACCEPT 2

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
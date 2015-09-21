#pragma once

#include "Stdafx.h"

namespace SXN
{
	namespace Net
	{
		private struct WSAOVERLAPPEDPLUS final : OVERLAPPED
		{
			public:

			int action;
		};
	}
}
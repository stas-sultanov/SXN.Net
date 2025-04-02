#pragma once

#include "Stdafx.h"

namespace SXN::Net
{
	enum ConnectionState : USHORT
	{
		Accepted,

		Accepting,

		Disconnected,

		Disconnecting,

		Received,

		Receiving,

		Transmitted,

		Transmitting
	};
}

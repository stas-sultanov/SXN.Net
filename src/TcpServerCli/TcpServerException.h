#pragma once

#include "Stdafx.h"
#include "WinsockErrorCode.h"

using namespace System;

namespace SXN
{
	namespace Net
	{
		[Serializable]
		public ref class TcpServerException sealed: Exception
		{
			internal:

			TcpServerException(WinsockErrorCode winsockErrorCode)
				: TcpServerException(winsockErrorCode, 0)
			{
			}

			TcpServerException(int kernelErrorCode)
				: TcpServerException((WinsockErrorCode) 0, kernelErrorCode)
			{
			}

			TcpServerException(WinsockErrorCode winsockErrorCode, int kernelErrorCode)
			{
			}
		};
	}
}
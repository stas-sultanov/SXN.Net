#pragma once

#include "pch.h"
#include "WinSockErrorCode.h"

using namespace System;

namespace SXN::Net
{
		[Serializable]
		public ref class TcpServerException sealed : Exception
		{
			internal:

			TcpServerException(WinSockErrorCode winsockErrorCode)
				: TcpServerException(winsockErrorCode, 0)
			{
			}

			TcpServerException(int kernelErrorCode)
				: TcpServerException(WinSockErrorCode::None, kernelErrorCode)
			{
			}

			TcpServerException(WinSockErrorCode winsockErrorCode, int kernelErrorCode)
			{
				WErrorCode = winsockErrorCode;

				KErrorCode = kernelErrorCode;
			}

			public:
			
			property WinSockErrorCode WErrorCode;

			property int KErrorCode;
		};
}
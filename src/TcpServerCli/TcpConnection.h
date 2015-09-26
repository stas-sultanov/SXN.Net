#pragma once

#include "Stdafx.h"
#include "WinsockEx.h"

namespace SXN
{
	namespace Net
	{
		public ref class TcpConnection
		{
			public:

			#pragma region Fields

			WinsockEx* pWinsockEx;

			initonly SOCKET socket;

			initonly RIO_RQ rioRequestQueue;

			initonly PVOID addrBuf;

			initonly LPOVERLAPPED acceptOverlapped;

			int id;

			PRIO_BUF receiveBuffer;

			PRIO_BUF sendBuffer;

			#pragma endregion

			internal:

			#pragma region Constructor

			/// <summary>
			/// Initializes a new instance of the <see cref="TcpConnection" /> class.
			/// </summary>
			inline TcpConnection(WinsockEx* pWinsockEx, int id, SOCKET socket, PVOID addrBuf, RIO_RQ requestQueue, LPOVERLAPPED accceptk)
			{
				this->pWinsockEx = pWinsockEx;

				this->id = id;

				this->socket = socket;

				this->rioRequestQueue = requestQueue;

				this->addrBuf = addrBuf;

				this->acceptOverlapped = accceptk;
			}

			!TcpConnection()
			{
				delete addrBuf;
			}

			BOOL StartRecieve()
			{
				return pWinsockEx->RIOReceive(rioRequestQueue, receiveBuffer, 1, 0, (LPVOID)id);

				/*{
					// get error code
					WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

					// throw exception
					throw gcnew TcpServerException(winsockErrorCode);
				}*/
			}

			BOOL StartSend(DWORD dataLength)
			{
				sendBuffer->Length = dataLength;

				BOOL result = pWinsockEx->RIOSend(rioRequestQueue, sendBuffer, 1, 0, (LPVOID)id);

				if (!result)
				{
					// get error code
					WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

					Console::WriteLine("buffer id: {0}, length: {1}, offset: {2}", (Int32)sendBuffer->BufferId, (Int32)sendBuffer->Length, (Int32)sendBuffer->Offset);

					// throw exception
					throw gcnew TcpServerException(winsockErrorCode);
				}

				return result;
			}

			#pragma endregion
		};
	}
}
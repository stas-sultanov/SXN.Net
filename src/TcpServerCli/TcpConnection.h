#pragma once

#include "Stdafx.h"

namespace SXN
{
	namespace Net
	{
		public ref class TcpConnection
		{
			private:

			#pragma region Fields

			initonly SOCKET socket;

			initonly RIO_RQ requestQueue;

			initonly PVOID addrBuf;

			initonly LPOVERLAPPED acceptOverlapped;

			#pragma endregion

			internal:

			#pragma region Constructor

			/// <summary>
			/// Initializes a new instance of the <see cref="TcpConnection" /> class.
			/// </summary>
			inline TcpConnection(SOCKET socket, PVOID addrBuf, RIO_RQ requestQueue, LPOVERLAPPED accceptk)
			{
				this->socket = socket;

				this->requestQueue = requestQueue;

				this->addrBuf = addrBuf;

				this->acceptOverlapped = accceptk;

				// create request queue
				//var rq = rioHandle.CreateRequestQueue(socket, 24, 1, 24, 1, )
			}

			!TcpConnection()
			{
				delete addrBuf;
			}

			#pragma endregion
		};
	}
}
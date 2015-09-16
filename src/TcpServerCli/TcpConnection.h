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

#pragma endregion

		internal:

#pragma region Constructor

			/// <summary>
			/// Initializes a new instance of the <see cref="TcpConnection" /> class.
			/// </summary>
			inline TcpConnection(SOCKET socket, RIO_RQ requestQueue)
			{
				this->socket = socket;

				this->requestQueue = requestQueue;

				// create request queue
				//var rq = rioHandle.CreateRequestQueue(socket, 24, 1, 24, 1, )
			}

#pragma endregion
		};
	}
}
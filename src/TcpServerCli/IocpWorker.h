#pragma once

#include "Stdafx.h"
#include "WinsockHandle.h"
#include "TcpServerException.h"
#include "RioBufferPool.h"

using namespace System;
using namespace System::Collections::Generic;

namespace SXN
{
	namespace Net
	{
		private ref class IocpWorker
		{
			private:

			#pragma region Fields

			/// <summary>
			/// A pointer to the object that provides work with Winsock extensions.
			/// </summary>
			initonly WinsockHandle^ winsockHandle;

			/// <summary>
			/// The unique identifier of the worker within the <see cref = "TcpWorker"/>.
			/// </summary>
			initonly Int32 Id;

			/// <summary>
			/// The completion port.
			/// </summary>
			initonly HANDLE completionPort;

			/// <summary>
			/// The completion queue.
			/// </summary>
			initonly RIO_CQ completionQueue;

			/// <summary>
			/// The dictionary of the connections.
			/// </summary>
			// ConcurrentDictionary<UInt64, TcpConnection> Connections;

			/// <summary>
			/// The Registered I/O buffer pool.
			/// </summary>
			initonly RioBufferPool^ bufferPool;

			#pragma endregion

			internal:

			#pragma region Constructor & Destructor

			/// <summary>
			/// Initializes a new instance of the <see cref="IocpWorker" /> class.
			/// </summary>
			/// <param name="rioHandle">A pointer to the object that provides work with Winsock extensions.</param>
			/// <param name="processorIndex">The index of the processor.</param>
			/// <param name="segmentLength">The length of the segment.</param>
			/// <param name="segmentsCount">The count of the segments.</param>
			/// <returns>
			/// An instance of <see cref="TryResult{T}" /> which encapsulates result of the operation.
			/// <see cref="TryResult{T}.Success" /> contains <c>true</c> if operation was successful, <c>false</c> otherwise.
			/// <see cref="TryResult{T}.Result" /> contains valid object if operation was successful, <c>null</c> otherwise.
			/// </returns>
			IocpWorker(WinsockHandle ^winsockHandle, Int32 processorIndex, UInt32 segmentLength, UInt32 segmentsCount)
			{
				// set winsock handle
				this->winsockHandle = winsockHandle;

				// create I/O completion port
				completionPort = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);

				// check if operation has failed
				if (completionPort == NULL)
				{
					// get error code
					DWORD kernelErrorCode = ::GetLastError();

					// throw exception
					throw gcnew TcpServerException(kernelErrorCode);
				}

				#pragma region create Registered I/O completion queue

				// compose completion method structure
				RIO_NOTIFICATION_COMPLETION completionSettings;

				// set type to IOCP
				completionSettings.Type = RIO_IOCP_COMPLETION;

				// set IOCP handle to completion port
				completionSettings.Iocp.IocpHandle = completionPort;

				// set IOCP completion key to id of current worker
				completionSettings.Iocp.CompletionKey = (PVOID) processorIndex;

				// set IOCP overlapped to invalid
				completionSettings.Iocp.Overlapped = (PVOID) -1;

				// create completion queue
				completionQueue = winsockHandle->RIOCreateCompletionQueue(100, &completionSettings);

				// check if operation has failed
				if (completionQueue == RIO_INVALID_CQ)
				{
					// get error code
					WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

					// throw exception
					throw gcnew TcpServerException(winsockErrorCode);
				}

				#pragma endregion

				// create buffer pool
				bufferPool = gcnew RioBufferPool(winsockHandle, segmentLength, segmentsCount);
			}

			/// <summary>
			/// Releases all associated resources.
			/// </summary>
			~IocpWorker()
			{
				// release buffer pool
				delete bufferPool;

				// close completion queue
				winsockHandle->RIOCloseCompletionQueue(completionQueue);

				// clo
				//::Clos
			}

			#pragma endregion
		};
	}
}
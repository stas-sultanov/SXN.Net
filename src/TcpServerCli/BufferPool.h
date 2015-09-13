#pragma once

#include "Stdafx.h"
#include "WinsockHandle.h"
#include "TcpServerException.h"

using namespace System;

namespace SXN
{
	namespace Net
	{
		/// <summary>
		/// Provides management of the memory segments within the Winsock Registered I/O extensions.
		/// </summary>
		private ref class BufferPool sealed
		{
			private:

			#pragma region Fields

			/// <summary>
			/// A pointer to the object that provides work with Winsock.
			/// </summary>
			initonly WinsockHandle* winsockHandle;

			/// <summary>
			/// A pointer to the memory buffer.
			/// </summary>
			initonly LPVOID buffer;

			/// <summary>
			/// The length of the <see cref="buffer" />.
			/// </summary>
			initonly UInt32 bufferLength;

			/// <summary>
			/// The identifier of the <see cref="buffer" /> within the Winsock Registered I/O extensions.
			/// </summary>
			initonly RIO_BUFFERID rioBufferId;

			/// <summary>
			/// The collection of the identifiers of the available segments.
			/// </summary>
			//ConcurrentQueue<Int32> availableSegments;

			/// <summary>
			/// The collection of the items of the <see cref="RIO_BUF" /> type.
			/// </summary>
			initonly RIO_BUF* segments;

			#pragma endregion

			internal:

			#pragma region Constructors

			/// <summary>
			/// Initializes a new instance of the <see cref="BufferPool" /> class.
			/// </summary>
			/// <param name="rioHandle">A pointer to the object that provides work with Winsock.</param>
			/// <param name="segmentLength">The length of the single segment.</param>
			/// <param name="segmentsCount">The count of the segments to manage.</param>
			/// <exception cref="TcpServerException">If any error occurs.</exception>
			/// <remarks>
			/// The multiplication of <paramref name="segmentLength" /> and <paramref name="segmentsCount" /> must produce value that is aligned to Memory Allocation Granularity.
			/// </remarks>
			BufferPool(WinsockHandle* winsockHandle, unsigned int segmentLength, unsigned int segmentsCount)
			{
				// set winsock handle
				this->winsockHandle = winsockHandle;

				// calculate and set the length of the memory buffer
				bufferLength = segmentLength * segmentsCount;

				// reserve and commit memory block
				LPVOID buffer = ::VirtualAlloc(NULL, bufferLength, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

				// check if operation has failed
				if (buffer == NULL)
				{
					// get kernel error code
					int kernelErrorCode = ::GetLastError();

					// throw exception
					throw gcnew TcpServerException(kernelErrorCode);
				}

				// register and set the identifier of the buffer
				rioBufferId = winsockHandle->RIORegisterBuffer((PCHAR) buffer, bufferLength);

				// check if operation has failed
				if (rioBufferId == RIO_INVALID_BUFFERID)
				{
					// get winsock error code
					WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

					// try free allocated memory and ignore result
					::VirtualFree(buffer, 0, MEM_RELEASE);

					// throw exception
					throw gcnew TcpServerException(winsockErrorCode);
				}

				// initialize available segments collection
				//availableSegments = new ConcurrentQueue<Int32>();

				// initialize collection of the buffer segments
				segments = new RIO_BUF[segmentsCount];

				int offset = 0u;

				// initialize items of the collection
				for (int segmentIndex = 0; segmentIndex < segmentsCount; segmentIndex++)
				{
					// initialize item
					RIO_BUF rioBuf;

					rioBuf.BufferId = rioBufferId;

					rioBuf.Offset = offset;

					rioBuf.Length = segmentLength;

					segments[segmentIndex] = rioBuf;

					// add id of the segment into the collection of the available segments
					//availableSegments.Enqueue(segmentIndex);

					// increment offset
					offset += segmentLength;
				}
			}



			#pragma endregion

			#pragma region Methods

			void ReleaseBuffer(Int32 segmentId)
			{
				// availableSegments.Enqueue(segmentId);
			}

			/*
			/// <summary>
			/// Tries to release all allocated resources.
			/// </summary>
			/// <param name="rioHandle">The object that provides work with Winsock Registered I/O extensions.</param>
			/// <param name="kernelErrorCode">Contains <c>0</c> if operation was successful, error code otherwise.</param>
			/// <returns><c>true</c> if operation was successful, <c>false</c> otherwise.</returns>
			public unsafe Boolean TryRelease(RIOHandle rioHandle, out UInt32 kernelErrorCode)
			{
				// 0 deregister buffer with Registered I/O extension
				{
					rioHandle.DeregisterBuffer(bufferId);
				}

				// 1 try free memory
				{
					var freeResult = KernelInterop.VirtualFree((void*)buffer, 0, KernelInterop.MEM_RELEASE);

					if (freeResult == false)
					{
						kernelErrorCode = KernelInterop.GetLastError();

						return false;
					}
				}

				// 2 success
				kernelErrorCode = 0;

				return true;
			}

			public Boolean TryGetBufferSegmentId(out Int32 segmentId) = > availableSegments.TryDequeue(out segmentId);
			*/

			#pragma endregion
		};
	}
}
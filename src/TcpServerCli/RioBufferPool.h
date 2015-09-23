#pragma once

#include "Stdafx.h"
#include "WinsockEx.h"
#include "TcpServerException.h"

using namespace System;

namespace SXN
{
	namespace Net
	{
		/// <summary>
		/// Provides management of the memory buffers within the Winsock Registered I/O extensions.
		/// </summary>
		private ref class RioBufferPool sealed
		{
			private:

			#pragma region Fields

			/// <summary>
			/// A pointer to the object that provides work with Winsock extensions.
			/// </summary>
			initonly WinsockEx* pWinsockEx;

			/// <summary>
			/// A pointer to the aligned memory block.
			/// </summary>
			initonly LPVOID memoryBlock;

			/// <summary>
			/// The length of the <see cref="memoryBlock" />.
			/// </summary>
			initonly UInt32 memoryBlockLength;

			/// <summary>
			/// The identifier of the <see cref="memoryBlock" /> within the Winsock Registered I/O extensions.
			/// </summary>
			initonly RIO_BUFFERID rioBufferId;

			/// <summary>
			/// The collection of the identifiers of the available segments.
			/// </summary>
			//ConcurrentQueue<Int32> availableSegments;

			/// <summary>
			/// The collection of the items of the <see cref="RIO_BUF" /> type.
			/// </summary>
			initonly RIO_BUF* buffers;

			INT bufferLength;

			#pragma endregion

			internal:

			#pragma region Constructor & Destructor

			/// <summary>
			/// Initializes a new instance of the <see cref="BufferPool" /> class.
			/// </summary>
			/// <param name="pWinsockEx">A pointer to the object that provides work with Winsock extensions.</param>
			/// <param name="bufferLength">The length of the single buffer to manage.</param>
			/// <param name="segmentsCount">The count of the buffers to manage.</param>
			/// <remarks>
			/// The multiplication of <paramref name="segmentLength" /> and <paramref name="segmentsCount" /> must produce value that is aligned to Memory Allocation Granularity.
			/// </remarks>
			/// <exception cref="TcpServerException">If error occurs.</exception>
			RioBufferPool(WinsockEx* pWinsockEx, unsigned int bufferLength, unsigned int buffersCount)
			{
				// set winsock handle
				this->pWinsockEx = pWinsockEx;

				this->bufferLength = bufferLength;

				// calculate and set the length of the memory buffer
				memoryBlockLength = bufferLength * buffersCount;

				// reserve and commit aligned memory block
				memoryBlock = ::VirtualAlloc(NULL, memoryBlockLength, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

				// check if operation has failed
				if (memoryBlock == NULL)
				{
					// get kernel error code
					int kernelErrorCode = ::GetLastError();

					// throw exception
					throw gcnew TcpServerException(kernelErrorCode);
				}

				// register and set the identifier of the buffer
				rioBufferId = pWinsockEx->RIORegisterBuffer((PCHAR)memoryBlock, memoryBlockLength);

				// check if operation has failed
				if (rioBufferId == RIO_INVALID_BUFFERID)
				{
					// get winsock error code
					WinsockErrorCode winsockErrorCode = (WinsockErrorCode) ::WSAGetLastError();

					// try free allocated memory and ignore result
					::VirtualFree(memoryBlock, 0, MEM_RELEASE);

					// throw exception
					throw gcnew TcpServerException(winsockErrorCode);
				}

				// initialize available segments collection
				//availableSegments = new ConcurrentQueue<Int32>();

				// initialize collection of the buffer segments
				buffers = new RIO_BUF[buffersCount];

				int offset = 0u;

				// initialize items of the collection
				for (int segmentIndex = 0; segmentIndex < buffersCount; segmentIndex++)
				{
					// initialize item
					RIO_BUF rioBuf;

					rioBuf.BufferId = rioBufferId;

					rioBuf.Offset = offset;

					rioBuf.Length = bufferLength;

					buffers[segmentIndex] = rioBuf;

					// add id of the segment into the collection of the available segments
					//availableSegments.Enqueue(segmentIndex);

					// increment offset
					offset += bufferLength;
				}
			}

			/// <summary>
			/// Releases all associated resources.
			/// </summary>
			~RioBufferPool()
			{
				// delete buffers array
				delete buffers;

				// deregister buffer within the Registered I/O extensions
				// ignore result
				//winsockHandle->RIODeregisterBuffer(rioBufferId);

				// free allocated memory
				// ignore result
				::VirtualFree(memoryBlock, 0, MEM_RELEASE);
			}

			#pragma endregion

			#pragma region Methods

			void ReleaseBuffer(Int32 segmentId)
			{
				// availableSegments.Enqueue(segmentId);
			}

			/*
			public Boolean TryGetBufferSegmentId(out Int32 segmentId) = > availableSegments.TryDequeue(out segmentId);
			*/

			#pragma endregion

			PRIO_BUF GetBuffer(int id)
			{
				return this->buffers + id;
			}

			char* GetData(int id)
			{
				return ((PCHAR) this->memoryBlock) + id * bufferLength;
			}
		};
	}
}
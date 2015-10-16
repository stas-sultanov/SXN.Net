#pragma once

#include "Stdafx.h"
#include "WinsockEx.h"

#pragma unmanaged

namespace SXN
{
	namespace Net
	{
		/// <summary>
		/// Provides management of the memory buffers within the Winsock Registered I/O extensions.
		/// </summary>
		private class RioBufferPool final
		{
			private:

			#pragma region Fields

			/// <summary>
			/// A reference to the object that provides work with Winsock extensions.
			/// </summary>
			WinsockEx& winsockEx;

			/// <summary>
			/// The length of the buffer.
			/// </summary>
			ULONG bufferLength;

			/// <summary>
			/// A pointer to the aligned memory block.
			/// </summary>
			LPVOID memoryBlock;

			/// <summary>
			/// The identifier of the <see cref="memoryBlock" /> within the Winsock Registered I/O extensions.
			/// </summary>
			RIO_BUFFERID rioBufferId;

			/// <summary>
			/// The collection of the items of the <see cref="RIO_BUF" /> type.
			/// </summary>
			RIO_BUF* buffers;

			#pragma endregion

			#pragma region Constructor

			/// <summary>
			/// Initializes a new instance of the <see cref="BufferPool" /> class.
			/// </summary>
			/// <param name="winsockEx">A reference to the object that provides work with Winsock extensions.</param>
			/// <param name="bufferLength">The length of the buffer.</param>
			/// <param name="buffersCount">The count of the buffers.</param>
			/// <param name="memoryBlock">A pointer to the aligned memory block.</param>
			/// <param name="rioBufferId"> The identifier of the <see cref="memoryBlock" /> within the Winsock Registered I/O extensions.</param>
			RioBufferPool(WinsockEx& winsockEx, ULONG bufferLength, ULONG buffersCount, LPVOID memoryBlock, RIO_BUFFERID rioBufferId)
				: winsockEx(winsockEx)
			{
				// set buffer length
				this->bufferLength = bufferLength;

				// set memory block pointer
				this->memoryBlock = memoryBlock;

				// set the identifier of the memory block within the Winsock Registered I/O extensions.
				this->rioBufferId = rioBufferId;

				// initialize collection of the buffer segments
				buffers = new RIO_BUF[buffersCount];

				// initialize items of the collection
				for (ULONG segmentIndex = 0, offset = 0; segmentIndex < buffersCount; segmentIndex++, offset += bufferLength)
				{
					// initialize item
					RIO_BUF rioBuf;

					// set buffer identifier
					rioBuf.BufferId = rioBufferId;

					// set offset
					rioBuf.Offset = offset;

					// set length
					rioBuf.Length = bufferLength;

					// put structure into the collection
					buffers[segmentIndex] = rioBuf;
				}
			}

			#pragma endregion

			public:

			#pragma region Create and Destroy

			/// <summary>
			/// Initializes a new instance of the <see cref="BufferPool" /> class.
			/// </summary>
			/// <param name="pWinsockEx">A reference to the object that provides work with Winsock extensions.</param>
			/// <param name="bufferLength">The length of the buffer.</param>
			/// <param name="segmentsCount">The count of the buffers.</param>
			/// <remarks>
			/// The multiplication of <paramref name="segmentLength" /> and <paramref name="segmentsCount" /> must produce value that is aligned to Memory Allocation Granularity.
			/// </remarks>
			static RioBufferPool* Create(WinsockEx& winsockEx, ULONG bufferLength, ULONG buffersCount, DWORD& kernelErrorCode, int& winsockErrorCode)
			{
				// calculate and set the length of the memory block
				ULONG memoryBlockLength = bufferLength * buffersCount;

				// reserve and commit aligned memory block
				//LPVOID memoryBlock = ::VirtualAlloc(nullptr, memoryBlockLength, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

				LPVOID memoryBlock = new char [memoryBlockLength];

				memset(memoryBlock, 0, memoryBlockLength);

				// check if operation has failed
				if (memoryBlock == nullptr)
				{
					// get kernel error code
					kernelErrorCode = ::GetLastError();

					// set winsock error code
					winsockErrorCode = 0;

					return nullptr;
				}

				// register and set the identifier of the buffer
				RIO_BUFFERID rioBufferId = winsockEx.RIORegisterBuffer((PCHAR)memoryBlock, memoryBlockLength);

				// check if operation has failed
				if (rioBufferId == RIO_INVALID_BUFFERID)
				{
					// get winsock error code
					winsockErrorCode = ::WSAGetLastError();

					// try free allocated memory and ignore result
					if (::VirtualFree(memoryBlock, 0, MEM_RELEASE))
					{
						// set kernel error code
						kernelErrorCode = 0;
					}
					else
					{
						// get kernel error code
						kernelErrorCode = ::GetLastError();
					}

					return nullptr;
				}

				// initialize and return result
				return new RioBufferPool(winsockEx, bufferLength, buffersCount, memoryBlock, rioBufferId);
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
				winsockEx.RIODeregisterBuffer(rioBufferId);

				// free allocated memory
				// ignore result
				//::VirtualFree(memoryBlock, 0, MEM_RELEASE);

				delete[] memoryBlock;
			}

			#pragma endregion

			#pragma region Methods

			PRIO_BUF GetBuffer(int id)
			{
				return this->buffers + id;
			}

			char* GetData(int id)
			{
				return ((PCHAR) this->memoryBlock) + id * bufferLength;
			}

			#pragma endregion
		};
	}
}

#pragma managed
#pragma once

#include "Stdafx.h"
#include "winsock.h"

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
			Winsock* winsock;

			/// <summary>
			/// The length of the single buffer.
			/// </summary>
			ULONG bufferLength;

			/// <summary>
			/// The count of the buffers.
			/// </summary>
			ULONG buffersCount;

			/// <summary>
			/// A pointer to the memory block.
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
			/// Initializes a new instance of the <see cref="RioBufferPool" /> class.
			/// </summary>
			/// <param name="winsock">A reference to the object that provides work with Winsock extensions.</param>
			/// <param name="bufferLength">The length of the single buffer.</param>
			/// <param name="buffersCount">The count of the buffers to manage.</param>
			/// <param name="memoryBlock">A pointer to the aligned memory block.</param>
			/// <param name="rioBufferId">The identifier of the <see cref="memoryBlock" /> within the Winsock Registered I/O extensions.</param>
			inline RioBufferPool(Winsock* winsock, ULONG bufferLength, ULONG buffersCount, LPVOID memoryBlock, RIO_BUFFERID rioBufferId)
			{
				this->winsock = winsock;

				// set buffer length
				this->bufferLength = bufferLength;

				// set buffers count
				this->buffersCount = buffersCount;

				// set memory block pointer
				this->memoryBlock = memoryBlock;

				// set the identifier of the memory block within the Winsock Registered I/O extensions.
				this->rioBufferId = rioBufferId;

				// initialize collection of the buffer segments
				//buffers = new RIO_BUF[buffersCount];

				buffers = (PRIO_BUF) ::VirtualAlloc(nullptr, sizeof(RIO_BUF) * buffersCount, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

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
			/// Initializes a new instance of the <see cref="RioBufferPool" /> class.
			/// </summary>
			/// <param name="winsock">A reference to the object that provides work with Winsock extensions.</param>
			/// <param name="bufferLength">The length of the single buffer.</param>
			/// <param name="buffersCount">The count of the buffers to manage.</param>
			inline static RioBufferPool* Create(Winsock* winsock, ULONG bufferLength, ULONG buffersCount, DWORD& kernelErrorCode, int& winsockErrorCode)
			{
				// calculate and set the length of the memory block
				auto memoryBlockLength = bufferLength * buffersCount;

				// reserve and commit aligned memory block
				auto memoryBlock = ::VirtualAlloc(nullptr, memoryBlockLength, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

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
				auto rioBufferId = winsock->RIORegisterBuffer((PCHAR)memoryBlock, memoryBlockLength);

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
				return new RioBufferPool(winsock, bufferLength, buffersCount, memoryBlock, rioBufferId);
			}

			/// <summary>
			/// Releases all associated resources.
			/// </summary>
			inline ~RioBufferPool()
			{
				// delete buffers array
				//delete buffers;

				// free allocated memory
				// ignore result
				::VirtualFree(buffers, 0, MEM_RELEASE);

				// deregister buffer within the Registered I/O extensions
				// ignore result
				winsock->RIODeregisterBuffer(rioBufferId);

				// free allocated memory
				// ignore result
				::VirtualFree(memoryBlock, 0, MEM_RELEASE);
			}

			#pragma endregion

			#pragma region Methods

			/// <summary>
			/// Gets a pointer to the <see cref="RIO_BUF" /> structure that specifies a portion of the registered buffer.
			/// </summary>
			/// <param name="bufferIndex">The identifier of the buffer to retrieve.</param>
			/// <returns>Structure that specifies a portion of the registered buffer.</returns>
			/// <remarks><paramref name="bufferIndex" /> must be less then <see cref="buffersCount">.</remarks>
			inline PRIO_BUF GetBuffer(ULONG bufferIndex)
			{
				return this->buffers + bufferIndex;
			}

			/// <summary>
			/// Gets a pointer to the memory block that is associated with the specified buffer.
			/// </summary>
			/// <param name="bufferIndex">The identifier of the buffer to retrieve.</param>
			/// <returns>A pointer to the memory block.</returns>
			/// <remarks><paramref name="bufferIndex" /> must be less then <see cref="buffersCount">.</remarks>
			inline char* GetBufferData(ULONG bufferIndex)
			{
				return ((PCHAR) this->memoryBlock) + bufferLength * bufferIndex;
			}

			#pragma endregion
		};
	}
}

#pragma managed
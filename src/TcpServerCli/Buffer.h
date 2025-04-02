#pragma once

#include "pch.h"
#include "KernelException.h"
#include "WinSock.h"

namespace SXN::Net
{
	/// <summary>
	/// Represents the buffer for the Windows Sockets Registered I/O.
	/// </summary>
	private class Buffer final
	{
	public:

		#pragma region Static Methods

		/// <summary>
		/// Creates a new instance of the <see cref="RIOBuffer" /> class.
		/// </summary>
		/// <param name="winsock">A reference to the object that provides work with the WinSock extensions.</param>
		/// <param name="portionLength">The length of the portion, in bytes.</param>
		/// <param name="portionsCount">The count of portions within the buffer.</param>
		inline static Buffer* Create
		(
			WinSock& winsock,
			UINT portionLength,
			UINT portionsCount
		)
		{
			// initialize a vector of specificators
			auto specificators = (PRIO_BUF) ::VirtualAlloc(nullptr, sizeof(RIO_BUF) * portionsCount, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

			if (specificators == nullptr)
			{
				// get kernel error code
				auto errorCode = ::GetLastError();

				throw gcnew KernelException(errorCode, "Can not allocate memory for descriptors.");
			}

			// calculate the size of the buffer
			auto bufferSize = portionLength * portionsCount;

			// reserve and commit aligned memory block
			auto portions = reinterpret_cast<PBYTE> (::VirtualAlloc(nullptr, bufferSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));

			// check if operation has failed
			if (portions == nullptr)
			{
				// free allocated memory
				auto _ = ::VirtualFree(specificators, 0, MEM_RELEASE);

				// get kernel error code
				auto errorCode = ::GetLastError();

				throw gcnew KernelException(errorCode, "Can not allocate memory for buffer.");
			}

			// register and set the identifier of the buffer
			auto id = winsock.RIORegisterBuffer((PCHAR)portions, bufferSize);

			// check if operation has failed
			if (id == RIO_INVALID_BUFFERID)
			{
				// free allocated memory
				VirtualFree(portions, 0, MEM_RELEASE);

				// free allocated memory
				VirtualFree(specificators, 0, MEM_RELEASE);

				return nullptr;
			}

			// initialize specificators within the vector
			for (ULONG portionIndex = 0, offset = 0; portionIndex < portionsCount; portionIndex++, offset += portionLength)
			{
				// initialize item
				RIO_BUF rioBuf =
				{
					// set the buffer identifier
					.BufferId = id,

					// set offset
					.Offset = offset,

					// set length
					.Length = portionLength
				};

				// put structure into the collection
				specificators[portionIndex] = rioBuf;
			}

			// initialize
			auto result = new Buffer(id, portionLength, portions, specificators);

			return result;
		}

		/// <summary>
		/// Disposes a new instance of the <see cref="RIOBuffer" /> class.
		/// </summary>
		/// <param name="winsock">A reference to the object that provides work with the WinSock extensions.</param>
		inline void Dispose
		(
			WinSock& winsock
		)
			const
		{
			// free allocated memory
			// ignore result
			::VirtualFree(specificators, 0, MEM_RELEASE);

			// deregister buffer within the Registered I/O extensions
			// ignore result
			winsock.RIODeregisterBuffer(id);

			// free allocated memory
			// ignore result
			::VirtualFree(portions, 0, MEM_RELEASE);
		}

		#pragma endregion

	private:

		#pragma region Fields

		/// <summary>
		/// The identifier of the buffer within the Windos Sockets Registered I/O.
		/// </summary>
		RIO_BUFFERID id;

		/// <summary>
		/// The length of the portion, in bytes.
		/// </summary>
		UINT portionLength;

		/// <summary>
		/// The pointer to the data portions within the buffer.
		/// </summary>
		PBYTE portions;

		/// <summary>
		/// The vector of items of the <see cref="RIO_BUF" /> type.
		/// </summary>
		PRIO_BUF specificators;

		#pragma endregion

		#pragma region Constructor

		/// <summary>
		/// Initializes a new instance of the <see cref="RIOBuffer" /> class.
		/// </summary>
		/// <param name="bufferId">The identifier of the buffer within the Windos Sockets Registered I/O.</param>
		/// <param name="portionLength">The length of the portion, in bytes.</param>
		/// <param name="portions">The pointer to the data portions within the buffer.</param>
		/// <param name="specificators">The vector of items of the <see cref="RIO_BUF" /> type.</param>
		inline Buffer
		(
			RIO_BUFFERID id,
			ULONG portionLength,
			PBYTE portions,
			PRIO_BUF specificators
		)
		{
			this->id = id;

			this->portionLength = portionLength;

			this->portions = portions;

			this->specificators = specificators;
		}

		#pragma endregion

	public:

		#pragma region Methods

		/// <summary>
		/// Gets a pointer to the <see cref="RIO_BUF" /> structure that specifies a portion of the registered buffer.
		/// </summary>
		/// <param name="portionIndex">The identifier of the portion to retrieve.</param>
		/// <returns>Structure that specifies a portion of the registered buffer.</returns>
		inline PRIO_BUF GetSpecificator(UINT portionIndex) const
		{
			return this->specificators + portionIndex;
		}

		/// <summary>
		/// Gets a pointer to the memory block that is associated with the specified portion within the buffer.
		/// </summary>
		/// <param name="portionIndex">The identifier of the portion to retrieve.</param>
		/// <returns>A pointer to the memory block.</returns>
		inline PBYTE GetData(UINT portionIndex) const
		{
			return this->portions + portionLength * portionIndex;
		}

		#pragma endregion
	};
}

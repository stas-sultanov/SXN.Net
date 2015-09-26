using System;
using System.Collections.Concurrent;
using System.Runtime.CompilerServices;
using SXN.Net.Kernel;
using SXN.Net.Winsock;

namespace SXN.Net
{
	/// <summary>
	/// Provides management of the memory buffers required for the Winsock Registered I/O extensions.
	/// </summary>
	internal sealed class RIOBufferPool
	{
		#region Fields

		/// <summary>
		/// The pointer to the memory buffer.
		/// </summary>
		private readonly IntPtr buffer;

		/// <summary>
		/// The length of the <see cref="buffer" />.
		/// </summary>
		private readonly UInt32 bufferLength;

		/// <summary>
		/// The identifier of the <see cref="buffer" /> within the Winsock Registered I/O extensions.
		/// </summary>
		private readonly IntPtr bufferId;

		/// <summary>
		/// The collection of the identifiers of the available segments.
		/// </summary>
		private readonly ConcurrentQueue<Int32> availableSegments;

		/// <summary>
		/// The collection of the items of the <see cref="RIO_BUF" /> type.
		/// </summary>
		private readonly RIO_BUF[] segments;

		#endregion

		#region Constructors

		/// <summary>
		/// Initializes a new instance of the <see cref="RIOBufferPool" /> class.
		/// </summary>
		private RIOBufferPool(IntPtr buffer, UInt32 bufferLength, IntPtr bufferId, UInt32 bufferSegmentLength, UInt32 bufferSegmentsCount)
		{
			this.buffer = buffer;

			this.bufferLength = bufferLength;

			this.bufferId = bufferId;

			// initialize available segments collection
			availableSegments = new ConcurrentQueue<Int32>();

			// initialize collection of the buffer segments
			segments = new RIO_BUF[bufferSegmentsCount];

			var offset = 0u;

			// initialize items of the collection
			for (var segmentIndex = 0; segmentIndex < bufferSegmentsCount; segmentIndex++)
			{
				// initialize item
				segments[segmentIndex] = new RIO_BUF
				{
					BufferId = bufferId,
					Offset = offset,
					Length = bufferSegmentLength
				};

				// add id of the segment into the collection of the available segments
				availableSegments.Enqueue(segmentIndex);

				// increment offset
				offset += bufferSegmentLength;
			}
		}

		#endregion

		#region Methods

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		internal void ReleaseBuffer(Int32 segmentId) => availableSegments.Enqueue(segmentId);

		/// <summary>
		/// Tries to create a new instance of the <see cref="RIOBufferPool" />.
		/// </summary>
		/// <param name="rioHandle">The object that provides work with Winsock Registered I/O extensions.</param>
		/// <param name="segmentLength">The length of the segment.</param>
		/// <param name="segmentsCount">The count of the segments.</param>
		/// <returns>
		/// An instance of <see cref="TryResult{T}" /> which encapsulates result of the operation.
		/// <see cref="TryResult{T}.Success" /> contains <c>true</c> if operation was successful, <c>false</c> otherwise.
		/// <see cref="TryResult{T}.Result" /> contains valid object if operation was successful, <c>null</c> otherwise.
		/// </returns>
		/// <remarks>
		/// The multiplication of <paramref name="segmentLength" /> and <paramref name="segmentsCount" /> must produce value that is aligned to Memory Allocation Granularity.
		/// </remarks>
		public static unsafe TryResult<RIOBufferPool> TryCreate(RIOHandle rioHandle, UInt32 segmentLength, UInt32 segmentsCount)
		{
			// calculate size of the memory buffer to allocate
			var bufferLength = segmentLength * segmentsCount;

			void* memoryPointer;

			IntPtr buffer;

			IntPtr bufferId;

			// 1 try allocate memory block
			{
				// try reserve and commit memory block
				memoryPointer = KernelInterop.VirtualAlloc(null, bufferLength, KernelInterop.MEM_COMMIT | KernelInterop.MEM_RESERVE, KernelInterop.PAGE_READWRITE);

				// check if allocation has failed
				if (memoryPointer == null)
				{
					// get kernel error code
					var kernelErrorCode = (KernelErrorCode) KernelInterop.GetLastError();

					// return result
					return TryResult<RIOBufferPool>.CreateFail(kernelErrorCode);
				}

				// set buffer
				buffer = (IntPtr) memoryPointer;
			}

			// 2 try register buffer with Registered I/O extensions
			{
				bufferId = rioHandle.RegisterBuffer(buffer, bufferLength);

				if (bufferId == WinsockInterop.RIO_INVALID_BUFFERID)
				{
					// get winsock error code
					var winsockErrorCode = (WinsockErrorCode) WinsockInterop.WSAGetLastError();

					// free allocated memory
					var freeResult = KernelInterop.VirtualFree(memoryPointer, 0, KernelInterop.MEM_RELEASE);

					// set kernel error code
					if (freeResult)
					{
						// return result
						return TryResult<RIOBufferPool>.CreateFail(winsockErrorCode);
					}

					// get kernel error code
					var kernelErrorCode = (KernelErrorCode) KernelInterop.GetLastError();

					// return result
					return TryResult<RIOBufferPool>.CreateFail(kernelErrorCode, winsockErrorCode);
				}
			}

			// 3 success
			var result = new RIOBufferPool(buffer, bufferLength, bufferId, segmentLength, segmentsCount);

			// return result
			return TryResult<RIOBufferPool>.CreateSuccess(result);
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Boolean TryGetBufferSegmentId(out Int32 segmentId) => availableSegments.TryDequeue(out segmentId);

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
				var freeResult = KernelInterop.VirtualFree((void*) buffer, 0, KernelInterop.MEM_RELEASE);

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

		#endregion
	}
}
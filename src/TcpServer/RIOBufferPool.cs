using System;
using System.Collections.Concurrent;
using System.Runtime.CompilerServices;
using SXN.Net.Winsock;

namespace SXN.Net
{
	/// <summary>
	/// Provides mangement of the memory requried for the Registered I/O extension.
	/// </summary>
	internal sealed class RIOBufferPool : IDisposable
	{
		#region Fields

		/// <summary>
		/// The collection of the identfiers of the available segments.
		/// </summary>
		private readonly ConcurrentQueue<Int32> availableSegments;

		/// <summary>
		/// The collecion of the items of the <see cref="RIO_BUF"/> type.
		/// </summary>
		private readonly RIO_BUF[] segments;

		private RIO rioHandle;

		#endregion

		#region Constructors

		/// <summary>
		/// Initializes a new instance of the <see cref="RIOBufferPool"/> class.
		/// </summary>
		private RIOBufferPool(RIO rioHandle, IntPtr bufferId, UInt32 bufferSegmentsLength, UInt32 bufferSegmentsCount)
		{
			this.rioHandle = rioHandle;

			IsDisposed = false;

			// initialize available segments collection
			availableSegments = new ConcurrentQueue<Int32>();

			// initialize collection of the buffer segments
			segments = new RIO_BUF[bufferSegmentsCount];

			var offset = 0u;

			// initialize items of the collection
			for (var segmentIndex = 0; segmentIndex < bufferSegmentsCount; segmentIndex++)
			{
				// initializ item
				segments[segmentIndex] = new RIO_BUF
				{
					BufferId = bufferId,
					Offset = offset,
					Length = bufferSegmentsLength
				};

				// add id of the segment into the collection of the availabl segments
				availableSegments.Enqueue(segmentIndex);

				// increment offset
				offset += bufferSegmentsLength;
			}
		}

		#endregion

		#region Properties

		/// <summary>
		/// Gets a <see cref="Boolean" /> value which indicates whether instance is disposed.
		/// </summary>
		public Boolean IsDisposed
		{
			get;

			private set;
		}

		#endregion

		#region Overrides of object

		~RIOBufferPool()
		{
			Dispose(false);
		}

		#endregion

		#region Methods of IDisposable

		/// <summary>
		/// Releases resources associated with the instance.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void Dispose()
		{
			// Release the resources
			Dispose(true);

			// Request system not to call the finalize method for a specified object
			GC.SuppressFinalize(this);
		}

		#endregion

		#region Private methods

		/// <summary>
		/// Releases resources associated with the instance.
		/// </summary>
		/// <param name="fromDispose">Value indicating whether method was called from the <see cref="Dispose()" /> method.</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		private void Dispose(Boolean fromDispose)
		{
			// Check if object is disposed already
			if (IsDisposed)
			{
				return;
			}

			// Set disposed
			IsDisposed = true;
		}

		#endregion

		#region Methods

		public static unsafe RIOBufferPool TryInitialize(RIO rioHandle)
		{
			var bufferSegmentLength = 2u * 1024u;

			var bufferSegmentsCount = 1024u * 10u;

			// calculate size of the memory buffer to allocate
			var bufferLength = bufferSegmentLength * bufferSegmentsCount;

			// try allocate memory buffer
			var memoryPointer = Kernel.Interop.VirtualAlloc(null, bufferLength, Kernel.Interop.MEM_COMMIT | Kernel.Interop.MEM_RESERVE, Kernel.Interop.PAGE_READWRITE);

			if (memoryPointer == null)
			{
				throw new NotImplementedException();
				// TODO: FAIL, get last error
			}

			// try register buffer with Regsitered I/O extension
			var bufferId = rioHandle.RegisterBuffer(new IntPtr(memoryPointer), bufferLength);

			if (bufferId == RIO.RIO_INVALID_BUFFERID)
			{
				throw new NotImplementedException();
				// TODO: FAIL, get last error
			}

			return new RIOBufferPool(rioHandle, bufferId, bufferSegmentLength, bufferSegmentsCount);
		}

		#endregion
	}
}
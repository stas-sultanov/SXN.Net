using System;
using System.Collections.Concurrent;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using SXN.Net.Kernel;
using SXN.Net.Winsock;

namespace SXN.Net
{
	using SOCKET = UIntPtr;

	/// <summary>
	/// Encapsulates data and methods required to process IOCP requests.
	/// </summary>
	internal sealed class IocpWorker
	{
		#region Constant and Static Fields

		// TODO: move to settings
		private const UInt32 maxOutsandingCompletions = 100u;

		#endregion

		#region Fields

		public Thread thread;

		#endregion

		#region Constructors

		/// <summary>
		/// Initializes a new instance of the <see cref="IocpWorker" /> class.
		/// </summary>
		private IocpWorker(RIOHandle rioHandle, Int32 processorId, IntPtr completionPort, IntPtr completionQueue, RIOBufferPool bufferPool)
		{
			RioHandle = rioHandle;

			ProcessorId = processorId;

			CompletionPort = completionPort;

			CompletionQueue = completionQueue;

			// set buffer pool
			BufferPool = bufferPool;

			// initialize dictionary of the connections
			Connections = new ConcurrentDictionary<UInt64, TcpConnection>();
		}

		#endregion

		#region Properties

		/// <summary>
		/// The Registered I/O buffer pool.
		/// </summary>
		public RIOBufferPool BufferPool
		{
			get;
		}

		/// <summary>
		/// The completion port.
		/// </summary>
		private IntPtr CompletionPort
		{
			get;
		}

		/// <summary>
		/// The completion queue.
		/// </summary>
		private IntPtr CompletionQueue
		{
			get;
		}

		/// <summary>
		/// The identifier of the associated processor.
		/// </summary>
		private Int32 ProcessorId
		{
			get;
		}

		/// <summary>
		/// The dictionary of the connections.
		/// </summary>
		private ConcurrentDictionary<UInt64, TcpConnection> Connections
		{
			get;
		}

		/// <summary>
		/// The object that provides work with Winsock Registered I/O extensions.
		/// </summary>
		private RIOHandle RioHandle
		{
			get;
		}

		#endregion

		#region Private methods

		private unsafe async Task Process(CancellationToken cancellationToken)
		{
			const Int32 maxResults = 1024;

			RIORESULT* results = stackalloc RIORESULT[maxResults];

			UIntPtr completionKey;

			UInt32 bytesCount;

			NativeOverlapped* overlapped;
			/*
			RIOPooledSegment cachedBadBuffer = worker.bufferPool.GetBuffer();
			Buffer.BlockCopy(_badResponseBytes, 0, cachedBadBuffer.Buffer, cachedBadBuffer.Offset, _badResponseBytes.Length);
			cachedBadBuffer.RioBuffer.Length = (uint)_badResponseBytes.Length;
			worker.cachedBad = cachedBadBuffer.RioBuffer;

			RIOPooledSegment cachedBusyBuffer = worker.bufferPool.GetBuffer();
			Buffer.BlockCopy(_busyResponseBytes, 0, cachedBusyBuffer.Buffer, cachedBusyBuffer.Offset, _busyResponseBytes.Length);
			cachedBusyBuffer.RioBuffer.Length = (uint)_busyResponseBytes.Length;
			worker.cachedBusy = cachedBusyBuffer.RioBuffer;
*/
			RIORESULT result;

			while (!cancellationToken.IsCancellationRequested)
			{
				// try register method to use for notification behavior with the I/O completion queue
				var notifyResult = (WinsockErrorCode) RioHandle.Notify(CompletionQueue);

				if (notifyResult != WinsockErrorCode.None)
				{
					// TODO: fail
				}

				// try to dequeue the I/O completion packet from the specified I/O completion port
				var getQueuedCompletionStatusResult = KernelInterop.GetQueuedCompletionStatus(CompletionPort, out bytesCount, out completionKey, out overlapped, UInt32.MaxValue);

				if (getQueuedCompletionStatusResult == false)
				{
					// TODO: fail
				}

				// try remove entries from the I/O completion queue
				var resultsCount = RioHandle.DequeueCompletion(CompletionQueue, (IntPtr) results, maxResults);

				/*
				if (resultsCount == RIOHandle.RIO_CORRUPT_CQ.)
				{
					// TODO: fail
				}
				*/

				var activatedCompletionPort = false;

				while (resultsCount > 0)
				{
					for (var resultIndex = 0; resultIndex < resultsCount; resultIndex++)
					{
						// get RIORESULT
						result = results[resultIndex];

						// TODO : wtf ???
						// if (result.RequestContext >= 0)

						TcpConnection connection;

						if (Connections.TryGetValue(result.SocketContext.ToUInt64(), out connection))
						{
							connection.CompleteReceive(result.RequestContext, result.BytesTransferred);
						}
					}

					if (!activatedCompletionPort)
					{
						notifyResult = (WinsockErrorCode) RioHandle.Notify(CompletionQueue);

						if (notifyResult != WinsockErrorCode.None)
						{
							// TODO: fail
						}

						activatedCompletionPort = true;
					}
				}
			}

			/*
							else
				{
					var error = GetLastError();
					if (error != 258)
					{
						throw new Exception(string.Format("ERROR: GetQueuedCompletionStatusEx returned {0}", error));
					}
}

			cachedBadBuffer.Dispose();
			cachedBusyBuffer.Dispose();
			*/
		}

		#endregion

		#region Methods

		/// <summary>
		/// Tries to create a new instance of the <see cref="IocpWorker" />.
		/// </summary>
		/// <param name="rioHandle">The object that provides work with Winsock Registered I/O extensions.</param>
		/// <param name="processorIndex">The index of the processor.</param>
		/// <param name="segmentLength">The length of the segment.</param>
		/// <param name="segmentsCount">The count of the segments.</param>
		/// <returns>
		/// An instance of <see cref="TryResult{T}" /> which encapsulates result of the operation.
		/// <see cref="TryResult{T}.Success" /> contains <c>true</c> if operation was successful, <c>false</c> otherwise.
		/// <see cref="TryResult{T}.Result" /> contains valid object if operation was successful, <c>null</c> otherwise.
		/// </returns>
		public static unsafe TryResult<IocpWorker> TryCreate(RIOHandle rioHandle, Int32 processorIndex, UInt32 segmentLength, UInt32 segmentsCount)
		{
			#region 0 try create completion port

			var completionPort = KernelInterop.CreateIoCompletionPort(KernelInterop.INVALID_HANDLE_VALUE, IntPtr.Zero, UIntPtr.Zero, 0);

			// check if operation has succeed
			if (completionPort == IntPtr.Zero)
			{
				// get error code
				var kernelErrorCode = (KernelErrorCode) KernelInterop.GetLastError();

				// return fail result
				TryResult<IocpWorker>.CreateFail(kernelErrorCode);
			}

			#endregion

			#region 1 try create completion queue

			// compose completion method structure
			var completionMethod = new RIO_NOTIFICATION_COMPLETION
			{
				// set type to IOCP
				Type = RIO_NOTIFICATION_COMPLETION_TYPE.RIO_IOCP_COMPLETION,

				// set IOCP parameters
				Iocp = new RIO_NOTIFICATION_COMPLETION.IOCP
				{
					// set completion port
					IocpHandle = completionPort,
					CompletionKey = (void*) processorIndex,
					Overlapped = (void*) (-1)
				}
			};

			// create completion queue
			var completionQueue = rioHandle.CreateCompletionQueue(maxOutsandingCompletions, completionMethod);

			if (completionQueue == WinsockInterop.RIO_CORRUPT_CQ)
			{
				// get error code
				var winsockErrorCode = (WinsockErrorCode) WinsockInterop.WSAGetLastError();

				// return fail result
				TryResult<IocpWorker>.CreateFail(winsockErrorCode);
			}

			#endregion

			#region 2 try create buffer pool

			var tryCreateBufferPool = RIOBufferPool.TryCreate(rioHandle, segmentLength, segmentsCount);

			if (tryCreateBufferPool.Success == false)
			{
				// return result
				return TryResult<IocpWorker>.CreateFail(tryCreateBufferPool.KernelErrorCode, tryCreateBufferPool.WinsockErrorCode);
			}

			#endregion

			// success
			var result = new IocpWorker(rioHandle, processorIndex, completionPort, completionQueue, tryCreateBufferPool.Result);

			// return success result
			return TryResult<IocpWorker>.CreateSuccess(result);
		}

		/// <summary>
		/// Tries to free all allocated unmanaged resources.
		/// </summary>
		/// <param name="kernelErrorCode">Contains <c>0</c> if operation was successful, error code otherwise.</param>
		/// <returns><c>true</c> if operation was successful, <c>false</c> otherwise.</returns>
		public Boolean TryRelease(out UInt32 kernelErrorCode)
		{
			return BufferPool.TryRelease(RioHandle, out kernelErrorCode);
		}

		#endregion

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		internal TryResult<TcpConnection> TryCreateConnection(SOCKET socket, UInt64 id)
		{
			var maxOutstandingReceive = 10u;

			var maxOutstandingSend = 10u;

			// try create request queue
			var requestQueue = RioHandle.CreateRequestQueue(socket, maxOutstandingReceive, 1, maxOutstandingSend, 1, CompletionQueue, CompletionQueue, id);

			if (requestQueue == WinsockInterop.RIO_INVALID_RQ)
			{
				var errorCode = (WinsockErrorCode) WinsockInterop.WSAGetLastError();

				TryResult<TcpConnection>.CreateFail(errorCode);
			}

			var connection = new TcpConnection(socket, requestQueue);

			// add connection into the collection of the connections
			Connections.TryAdd(id, connection);

			return TryResult<TcpConnection>.CreateSuccess(connection);
		}
	}
}
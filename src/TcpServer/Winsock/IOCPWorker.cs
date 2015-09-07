using System;
using System.Threading;
using System.Threading.Tasks;

namespace SXN.Net.Winsock
{
	using WinsockInterop = Interop;
	using WinsockErrorCode = ErrorCode;
	using KernelInterop = Kernel.Interop;
	using KernelErrorCode = Kernel.ErrorCode;

	/// <summary>
	/// Encapsulates data and methods required to process IOCP requests.
	/// </summary>
	internal sealed class IOCPWorker
	{
		#region Fields

		public readonly RIOBufferPool bufferPool;

		public RIO_BUF cachedBad;

		public RIO_BUF cachedBusy;

		//public ConcurrentDictionary<long, RIOTcpConnection> connections;

		public Thread thread;

		#endregion

		#region Constructors

		/// <summary>
		/// Initializes a new instance of the <see cref="T:System.Object" /> class.
		/// </summary>
		public IOCPWorker(Int32 processorId, IntPtr completionPort, IntPtr completionQueue, RIOBufferPool bufferPool)
		{
			ProcessorId = processorId;

			CompletionPort = completionPort;

			CompletionQueue = completionQueue;

			this.bufferPool = bufferPool;
		}

		#endregion

		#region Properties

		/// <summary>
		/// The completion port.
		/// </summary>
		public IntPtr CompletionPort
		{
			get;
		}

		/// <summary>
		/// The completion queue.
		/// </summary>
		public IntPtr CompletionQueue
		{
			get;
		}

		/// <summary>
		/// The identifier of the associated processor.
		/// </summary>
		public Int32 ProcessorId
		{
			get;
		}

		#endregion

		#region Methods

		/// <summary>
		/// Tries to create a new instance of the <see cref="IOCPWorker" />.
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
		public static unsafe TryResult<IOCPWorker> TryCreate(RIO rioHandle, Int32 processorIndex, UInt32 segmentLength, UInt32 segmentsCount)
		{
			var MaxOutsandingCompletions = 100u;

			#region 0 try create completion port

			var completionPort = KernelInterop.CreateIoCompletionPort(KernelInterop.INVALID_HANDLE_VALUE, IntPtr.Zero, UIntPtr.Zero, 0);

			// check if operation has succeed
			if (completionPort == IntPtr.Zero)
			{
				// get error code
				var kernelErrorCode = (KernelErrorCode) KernelInterop.GetLastError();

				// return fail result
				TryResult<IOCPWorker>.CreateFail(kernelErrorCode);
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
			var completionQueue = rioHandle.CreateCompletionQueue(MaxOutsandingCompletions, completionMethod);

			if (completionQueue == RIO.RIO_CORRUPT_CQ)
			{
				// get error code
				var winsockErrorCode = (WinsockErrorCode) WinsockInterop.WSAGetLastError();

				// return fail result
				TryResult<IOCPWorker>.CreateFail(winsockErrorCode);
			}

			#endregion

			#region 2 try create buffer pool

			var tryCreateBufferPool = RIOBufferPool.TryCreate(rioHandle, segmentLength, segmentsCount);

			if (tryCreateBufferPool.Success == false)
			{
				// return result
				return TryResult<IOCPWorker>.CreateFail(tryCreateBufferPool.KernelErrorCode, tryCreateBufferPool.WinsockErrorCode);
			}

			#endregion

			// success
			var result = new IOCPWorker(processorIndex, completionPort, completionQueue, tryCreateBufferPool.Result);

			// return success result
			return TryResult<IOCPWorker>.CreateSuccess(result);
		}

		/// <summary>
		/// Tries to free all allocated unmanaged resources.
		/// </summary>
		/// <param name="rioHandle">The object that provides work with Winsock Registered I/O extensions.</param>
		/// <param name="kernelErrorCode">Contains <c>0</c> if operation was successful, error code otherwise.</param>
		/// <returns><c>true</c> if operation was successful, <c>false</c> otherwise.</returns>
		public Boolean TryRelease(RIO rioHandle, out UInt32 kernelErrorCode)
		{
			return bufferPool.TryRelease(rioHandle, out kernelErrorCode);
		}


		private unsafe async Task Process(CancellationToken cancellationToken)
		{
			/*
			RIO_RESULT* results = stackalloc RIO_RESULT[maxResults];
			uint bytes, key;
			NativeOverlapped* overlapped;

			var worker = _workers[id];
			var completionPort = worker.completionPort;
			var cq = worker.completionQueue;

			RIOPooledSegment cachedBadBuffer = worker.bufferPool.GetBuffer();
			Buffer.BlockCopy(_badResponseBytes, 0, cachedBadBuffer.Buffer, cachedBadBuffer.Offset, _badResponseBytes.Length);
			cachedBadBuffer.RioBuffer.Length = (uint)_badResponseBytes.Length;
			worker.cachedBad = cachedBadBuffer.RioBuffer;

			RIOPooledSegment cachedBusyBuffer = worker.bufferPool.GetBuffer();
			Buffer.BlockCopy(_busyResponseBytes, 0, cachedBusyBuffer.Buffer, cachedBusyBuffer.Offset, _busyResponseBytes.Length);
			cachedBusyBuffer.RioBuffer.Length = (uint)_busyResponseBytes.Length;
			worker.cachedBusy = cachedBusyBuffer.RioBuffer;

			uint count;
			RIO_RESULT result;
			while (!_token.IsCancellationRequested)
			{
				_rio.Notify(cq);
				var sucess = GetQueuedCompletionStatus(completionPort, out bytes, out key, out overlapped, -1);
				if (sucess)
				{
					var activatedCompletionPort = false;
					while ((count = _rio.DequeueCompletion(cq, (IntPtr)results, maxResults)) > 0)
					{
						for (var i = 0; i < count; i++)
						{
							result = results[i];
							if (result.RequestCorrelation >= 0)
							{
								// receive
								RIOTcpConnection connection;
								if (worker.connections.TryGetValue(result.ConnectionCorrelation, out connection))
								{

									connection.CompleteReceive(result.RequestCorrelation, result.BytesTransferred);
								}
							}
						}

						if (!activatedCompletionPort)
						{
							_rio.Notify(cq);
							activatedCompletionPort = true;
						}
					}
				}
				else
				{
					var error = GetLastError();
					if (error != 258)
					{
						throw new Exception(string.Format("ERROR: GetQueuedCompletionStatusEx returned {0}", error));
					}
				}
			}

			cachedBadBuffer.Dispose();
			cachedBusyBuffer.Dispose();
			*/
		}

		#endregion
	}
}
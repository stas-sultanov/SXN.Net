using System;
using System.Threading;

namespace SXN.Net.Winsock
{
	using WinsockInterop = Interop;
	using WinsockErrorCode = ErrorCode;
	using KernelInterop = Kernel.Interop;
	using KernelErrorCode = Kernel.ErrorCode;

	/// <summary>
	/// Encapsulates data and methods required to process IOCP request.
	/// </summary>
	internal sealed class IOCPWorker
	{
		#region Fields

		public RIOBufferPool bufferPool;

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
		/// <returns>
		/// An instance of <see cref="TryResult{T}" /> which encapsulates result of the operation.
		/// <see cref="TryResult{T}.Success" /> contains <c>true</c> if operation was successful, <c>false</c> otherwise.
		/// <see cref="TryResult{T}.Result" /> contains valid object if operation was successful, <c>null</c> otherwise.
		/// </returns>
		public static unsafe TryResult<IOCPWorker> TryCreate(RIO rioHandle, Int32 processorIndex)
		{
			var MaxOutsandingCompletions = 100u;

			IntPtr completionPort;

			IntPtr completionQueue;

			// 0 try create completion port
			{
				completionPort = KernelInterop.CreateIoCompletionPort(KernelInterop.INVALID_HANDLE_VALUE, IntPtr.Zero, UIntPtr.Zero, 0);

				// check if operation has succeed
				if (completionPort == IntPtr.Zero)
				{
					// get error code
					var kernelErrorCode = (KernelErrorCode) KernelInterop.GetLastError();

					// return fail result
					TryResult<IOCPWorker>.CreateFail(kernelErrorCode);
				}
			}

			// 1 try create Registered I/O completion queue
			{
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
				completionQueue = rioHandle.CreateCompletionQueue(MaxOutsandingCompletions, completionMethod);

				if (completionQueue == RIO.RIO_CORRUPT_CQ)
				{
					// get error code
					var winsockErrorCode = (WinsockErrorCode) WinsockInterop.WSAGetLastError();

					// return fail result
					TryResult<IOCPWorker>.CreateFail(winsockErrorCode);
				}
			}

			// 2 try initialize buffer pool
			var tryInitializeBufferPool = RIOBufferPool.TryCreate(rioHandle, 4096, 1024);

			if (tryInitializeBufferPool.Success == false)
			{
				// return result
				return TryResult<IOCPWorker>.CreateFail(tryInitializeBufferPool.KernelErrorCode, tryInitializeBufferPool.WinsockErrorCode);
			}

			// success
			var result = new IOCPWorker(processorIndex, completionPort, completionQueue, tryInitializeBufferPool.Result);

			// return success result
			return TryResult<IOCPWorker>.CreateSuccess(result);
		}

		#endregion
	}
}
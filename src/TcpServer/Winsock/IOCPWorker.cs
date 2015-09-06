using System;
using System.Threading;

namespace SXN.Net.Winsock
{
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

		/// <summary>
		/// Initializes a new instance of the <see cref="T:System.Object"/> class.
		/// </summary>
		public IOCPWorker(Int32 processorId, IntPtr completionPort, IntPtr completionQueue)
		{
			ProcessorId = processorId;

			CompletionPort = completionPort;

			CompletionQueue = completionQueue;
		}

		#region Methods

		public static unsafe WinsockTryResult<IOCPWorker> TryInitialize(RIO rioHandle, Int32 processorId)
		{
			// try create new completion port
			var completionPort = Kernel.Interop.CreateIoCompletionPort(Kernel.Interop.INVALID_HANDLE_VALUE, IntPtr.Zero, UIntPtr.Zero, 0);

			if (completionPort == IntPtr.Zero)
			{
				var errorCode = Kernel.Interop.GetLastError();

				WinsockTryResult<IOCPWorker>.CreateFail((WinsockErrorCode) (Int32) errorCode);
			}

			// compose completion method structure
			var completionMethod = new RIO_NOTIFICATION_COMPLETION
			{
				// set type to IOCP
				Type = RIO_NOTIFICATION_COMPLETION_TYPE.RIO_IOCP_COMPLETION,
				// set IOCP params
				Iocp = new RIO_NOTIFICATION_COMPLETION.IOCP
				{
					// set completion port
					IocpHandle = completionPort,
					CompletionKey = (void*) processorId,
					Overlapped = (void*) (-1)
				}
			};

			var MaxOutsandingCompletions = 100u;

			// create completion queue
			var completionQueue = rioHandle.CreateCompletionQueue(MaxOutsandingCompletions, completionMethod);

			if (completionQueue == RIO.RIO_CORRUPT_CQ)
			{
				var errorCode = (WinsockErrorCode) Interop.WSAGetLastError();

				WinsockTryResult<IOCPWorker>.CreateFail(errorCode);
			}

			var rioBufferPool = RIOBufferPool.TryInitialize(rioHandle);

			var result = new IOCPWorker(processorId, completionPort, completionQueue);

			return WinsockTryResult<IOCPWorker>.CreateSuccess(result);
		}

		#endregion
	}
}
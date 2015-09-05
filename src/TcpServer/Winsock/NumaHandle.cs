using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace SXN.Net.Winsock
{
	internal class NumaHandle
	{
			public Int32 id;
			public IntPtr completionPort;
			public IntPtr completionQueue;

			//public ConcurrentDictionary<long, RIOTcpConnection> connections;
			public Thread thread;

			//public RIOBufferPool bufferPool;

			public RIO_BUF cachedBad;

			public RIO_BUF cachedBusy;
	}
}

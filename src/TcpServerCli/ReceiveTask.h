#pragma once

#include "Stdafx.h"
#include "TcpConnection.h"

using namespace System;
using namespace System::Runtime::CompilerServices;
using namespace System::Threading;

namespace SXN
{
	namespace NET
	{
		public ref class ReceiveTask sealed
			: INotifyCompletion, ICriticalNotifyCompletion
		{
			private:

			// readonly static Action CALLBACK_RAN = () = > { };

			bool _isCompleted;

			Action _continuation;

			uint _bytesTransferred;

			uint _requestCorrelation;

			ArraySegment<byte> _buffer;

			TcpConnection _connection;

			public:

			ReceiveTask(TcpConnection connection)
			{
				this->connection = connection;
			}

			void Reset()
			{
				_bytesTransferred = 0;

				_isCompleted = false;

				_continuation = null;
			}

			void SetBuffer(ArraySegment<byte> buffer)
			{
				_buffer = buffer;
			}

			void Complete(uint bytesTransferred, uint requestCorrelation)
			{
				_bytesTransferred = bytesTransferred;
				_requestCorrelation = requestCorrelation;
				_isCompleted = true;

				Action continuation = _continuation ? ? Interlocked.CompareExchange(ref _continuation, CALLBACK_RAN, null);
				if (continuation != null)
				{
					CompleteCallback(continuation);
				}
			}

			internal void CompleteCallback(Action continuation)
			{
				ThreadPool.UnsafeQueueUserWorkItem(UnsafeCallback, continuation);
			}

			public ReceiveTask GetAwaiter() { return this; }

			bool IsCompleted{ get{ return _isCompleted; } }

			private void UnsafeCallback(object state)
			{
				((Action)state)();
			}

			public void OnCompleted(Action continuation)
			{
				throw new NotImplementedException();
			}

			[System.Security.SecurityCritical]
			public void UnsafeOnCompleted(Action continuation)
			{
				if (_continuation == CALLBACK_RAN ||
					Interlocked.CompareExchange(
						ref _continuation, continuation, null) == CALLBACK_RAN)
				{
					CompleteCallback(continuation);
				}
			}
			public uint GetResult()
			{
				var bytesTransferred = _bytesTransferred;
				
				Buffer.BlockCopy(_segment.Buffer, _segment.Offset, _buffer.Array, _buffer.Offset, (int)bytesTransferred);
				
				Reset();

				connection.PostReceive(_requestCorrelation);
				
				return bytesTransferred;
			}

		}
	}
}

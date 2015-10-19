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
		public ref class ReceiveTask sealed : INotifyCompletion, ICriticalNotifyCompletion
		{
			private:

			// readonly static Action CALLBACK_RAN = () = > { };

			/// <summary>
			/// Indicates whether task is completed.
			/// <summary/>
			Boolean isCompleted;

			Action^ continuation;

			/// <summary>
			/// The amount of bytes transferred.
			/// <summary/>
			UInt32 bytesTransferred;

			UInt32 requestCorrelation;

			ArraySegment<byte> _buffer;

			TcpConnection* connection;

			public:

			ReceiveTask(TcpConnection* connection)
			{
				this->connection = connection;
			}

			void Reset()
			{
				// reset amount of data transferred
				bytesTransferred = 0;

				// reset completion flag
				isCompleted = false;

				// reset continuation delegate
				continuation = nullptr;
			}

			void SetBuffer(ArraySegment<byte> buffer)
			{
				_buffer = buffer;
			}

			void Complete(UInt32 bytesTransferred, UInt32 requestCorrelation)
			{
				bytesTransferred = bytesTransferred;

				requestCorrelation = requestCorrelation;

				isCompleted = true;

				/*
				Action continuation = continuation ? ? Interlocked.CompareExchange(ref _continuation, CALLBACK_RAN, null);

				if (continuation != null)
				{
					CompleteCallback(continuation);
				}*/
			}

			void CompleteCallback(Action^ continuation)
			{
				ThreadPool::UnsafeQueueUserWorkItem(UnsafeCallback, continuation);
			}

			ReceiveTask^ GetAwaiter() { return this; }

			property Boolean IsCompleted
			{
				Boolean get()
				{
					return isCompleted;
				}
			}

			void UnsafeCallback(Object^ state)
			{
				((Action^)state)();
			}

			virtual void OnCompleted(Action^ continuation)
			{
				throw gcnew NotImplementedException();
			}

			[System::Security::SecurityCritical]
			virtual void UnsafeOnCompleted(Action^ continuation)
			{
				if (continuation == CALLBACK_RAN || Interlocked::CompareExchange(continuation, continuation, nullptr) == CALLBACK_RAN)
				{
					CompleteCallback(continuation);
				}
			}

			UInt32 GetResult()
			{
				auto bytesTransferred = this->bytesTransferred;

				//Buffer.BlockCopy(_segment.Buffer, _segment.Offset, _buffer.Array, _buffer.Offset, (int)bytesTransferred)
				Reset();

				connection->PostReceive(requestCorrelation);

				return bytesTransferred;
			}
		}
	};
}
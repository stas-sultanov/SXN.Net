#pragma once

#include "Stdafx.h"
#include "TcpConnection.h"

using namespace System;
using namespace System::Runtime::CompilerServices;
using namespace System::Threading;

namespace SXN
{
	namespace Net
	{
		public ref class ReceiveTask sealed : ICriticalNotifyCompletion
		{
			private:

			//readonly static Action CALLBACK_RAN = () = > { };

			/// <summary>
			/// Indicates whether task is completed.
			/// </summary>
			Boolean isCompleted;

			Action^ continuation;

			/// <summary>
			/// The amount of bytes transferred.
			/// </summary>
			UInt32 bytesTransferred;

			UInt32 requestCorrelation;

			ArraySegment<byte> _buffer;

			TcpConnection* connection;

			internal:

			#pragma region Constructors

			/// <summary>
			/// Initialize a new instance of the <see cref="ReceiveTask" /> class.
			/// </summary>
			ReceiveTask(TcpConnection* connection)
			{
				this->connection = connection;
			}

			#pragma endregion

			public:

			#pragma region Methods implementation of ICriticalNotifyCompletion

			/// <summary>
			/// Schedules the continuation action that's invoked when the instance completes.
			/// </summary>
			/// <param name="continuation">The action to invoke when the operation completes.</param>
			/// <exception cref="ArgumentNullException"><paramref name="continuation" /> is <c>null</c>.</exception>
			virtual void OnCompleted(Action^ continuation)
			{
				// check argument
				if (continuation == nullptr)
				{
					throw gcnew ArgumentNullException("continuation");
				}

				throw gcnew NotImplementedException();
			}

			/// <summary>
			/// Schedules the continuation action that's invoked when the instance completes.
			/// </summary>
			/// <param name="continuation">The action to invoke when the operation completes.</param>
			/// <exception cref="ArgumentNullException"><paramref name="continuation" /> is <c>null</c>.</exception>
			[System::Security::SecurityCritical]
			virtual void UnsafeOnCompleted(Action^ continuation)
			{
				// check argument
				if (continuation == nullptr)
				{
					throw gcnew ArgumentNullException("continuation");
				}

				/**
				if (continuation == CALLBACK_RAN || Interlocked::CompareExchange(continuation, continuation, nullptr) == CALLBACK_RAN)
				{
					CompleteCallback(continuation);
				}/**/

				throw gcnew NotImplementedException();
			}

			#pragma endregion

			#pragma region Methods

			/// <summary>
			/// Gets an awaiter used to await this task.
			/// </summary>
			/// <returs>An awaiter instance.</returns>
			/// <remarks>This method is intended for compiler use rather than for use in application code.</remarks>
			ReceiveTask^ GetAwaiter()
			{
				return this;
			}

			#pragma endregion

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
				//ThreadPool::UnsafeQueueUserWorkItem(UnsafeCallback, continuation);
			}

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

			UInt32 GetResult()
			{
				auto bytesTransferred = this->bytesTransferred;

				//Buffer.BlockCopy(_segment.Buffer, _segment.Offset, _buffer.Array, _buffer.Offset, (int)bytesTransferred)
				Reset();

				//connection->PostReceive(requestCorrelation);

				return bytesTransferred;
			}
		};
	}
}
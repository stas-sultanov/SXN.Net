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

			static initonly Action^ CALLBACK_RAN = gcnew Action(&ReceiveTask::DoNothing);

			static void DoNothing()
			{
			}

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

			TcpConnection* connection;

			internal:

			#pragma region Constructors

			/// <summary>
			/// Initialize a new instance of the <see cref="ReceiveTask" /> class.
			/// </summary>
			ReceiveTask(TcpConnection* connection)
			{
				this->connection = connection;

				isCompleted = false;

				continuation = nullptr;
			}

			#pragma endregion

			public:

			#pragma region Implementation of the awaiter routine

			/// <summary>
			/// Gets an awaiter used to await this task.
			/// </summary>
			/// <returs>An awaiter instance.</returns>
			/// <remarks>This method is intended for compiler use rather than for use in application code.</remarks>
			ReceiveTask^ GetAwaiter()
			{
				return this;
			}

			/// <summary>
			/// Gets a value that indicates whether the asynchronous task has completed.
			/// </summary>
			property Boolean IsCompleted
			{
				Boolean get()
				{
					if (connection->state == ConnectionState::Received)
					{
						isCompleted = true;
					}

					return isCompleted;
				}
			}

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

				// TODO: must fix this
				// throw gcnew NotImplementedException();
			}

			/// <summary>
			/// Schedules the continuation action that's invoked when the instance completes.
			/// </summary>
			/// <param name="continuation">The action to invoke when the operation completes.</param>
			/// <exception cref="ArgumentNullException"><paramref name="continuation" /> is <c>null</c>.</exception>
			[System::Security::SecurityCritical]
			virtual void UnsafeOnCompleted(Action^ newContinuation)
			{
				if (continuation == CALLBACK_RAN || Interlocked::CompareExchange(continuation, newContinuation, (Action ^) nullptr) == CALLBACK_RAN)
				{
					CompleteCallback(continuation);
				}
			}

			/// <summary>
			/// Ends the wait for the completion of the asynchronous task.
			/// </summary>
			UInt32 GetResult()
			{
				auto bytesTransferred = this->bytesTransferred;

				//Buffer.BlockCopy(_segment.Buffer, _segment.Offset, _buffer.Array, _buffer.Offset, (int)bytesTransferred)
				Reset();

				//connection->PostReceive(requestCorrelation);

				return bytesTransferred;
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

			void Complete(UInt32 bytesTransferred, UInt32 requestCorrelation)
			{
				bytesTransferred = bytesTransferred;

				requestCorrelation = requestCorrelation;

				// set completed
				isCompleted = true;

				Action ^continueCall = continuation == nullptr ? Interlocked::CompareExchange(continuation, CALLBACK_RAN, (Action ^) nullptr) : continuation;

				if (continuation != nullptr)
				{
					CompleteCallback(continueCall);
				}
			}

			void CompleteCallback(Action^ continuation)
			{
				auto waitCallBack = gcnew WaitCallback(this, &ReceiveTask::UnsafeCallback);

				ThreadPool::UnsafeQueueUserWorkItem(waitCallBack, continuation);
			}

			void UnsafeCallback(Object^ state)
			{
				auto action = (Action ^)state;

				if (action != nullptr)
				{
					action();
				}
			}
		};
	}
}
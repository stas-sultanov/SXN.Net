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
		extern ref class Connection;

		public ref class SendTask sealed : ICriticalNotifyCompletion
		{
			private:

			static initonly Action^ CALLBACK_RAN2 = gcnew Action(&SendTask::DoNothing);

			static void DoNothing()
			{
				Console::WriteLine("ReceiveTask::DoNothing WTF??");
			}

			/// <summary>
			/// Indicates whether task is completed.
			/// </summary>
			Boolean isCompleted;

			Action^ _continuation;

			/// <summary>
			/// The amount of bytes transferred.
			/// </summary>
			UInt32 bytesTransferred;

			UInt32 requestCorrelation;

			Connection^ connection;

			internal:

			#pragma region Constructors

			/// <summary>
			/// Initialize a new instance of the <see cref="SendTask" /> class.
			/// </summary>
			SendTask(Connection^ connection)
			{
				this->connection = connection;

				isCompleted = false;

				_continuation = nullptr;
			}

			#pragma endregion

			public:

			#pragma region Implementation of the awaiter routine

			/// <summary>
			/// Gets an awaiter used to await this task.
			/// </summary>
			/// <returs>An awaiter instance.</returns>
			/// <remarks>This method is intended for compiler use rather than for use in application code.</remarks>
			SendTask^ GetAwaiter()
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
					return isCompleted;
				}
			}

			/// <summary>
			/// Schedules the continuation action that's invoked when the instance completes.
			/// </summary>
			/// <param name="continuation">The action to invoke when the operation completes.</param>
			/// <exception cref="ArgumentNullException"><paramref name="continuation" /> is <c>null</c>.</exception>
			[System::Security::SecuritySafeCritical]
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
			virtual void UnsafeOnCompleted(Action^ continuation)
			{
				if (_continuation == CALLBACK_RAN2)
				{
					CompleteCallback(_continuation);

					return;
				}

				if (Interlocked::CompareExchange(_continuation, continuation, (Action ^) nullptr) == CALLBACK_RAN2)
				{
					CompleteCallback(_continuation);

					return;
				}
			}

			/// <summary>
			/// Ends the wait for the completion of the asynchronous task.
			/// </summary>
			UInt32 GetResult()
			{
				//Console::WriteLine("ReceiveTask::GetResult Connection[{0}]", this->connection->Id);

				auto result = this->bytesTransferred;

				//Buffer.BlockCopy(_segment.Buffer, _segment.Offset, _buffer.Array, _buffer.Offset, (int)bytesTransferred)
				//Reset();

				//connection->PostReceive(requestCorrelation);

				return result;
			}

			#pragma endregion

			void Reset()
			{
				// reset amount of data transferred
				bytesTransferred = 0;

				// reset completion flag
				isCompleted = false;

				// reset continuation delegate
				_continuation = nullptr;
			}

			void Complete(UInt32 bytesTransferred)
			{
				// set completed
				isCompleted = true;

				this->bytesTransferred = bytesTransferred;

				//Action ^continuation = _continuation != nullptr ? _continuation : Interlocked::CompareExchange(_continuation, CALLBACK_RAN2, (Action ^) nullptr);

				if (_continuation == nullptr)
				{

				}
				else
				{
					CompleteCallback(_continuation);
				}
			}

			void CompleteCallback(Action^ continuation1)
			{
				auto waitCallBack = gcnew WaitCallback(this, &SendTask::UnsafeCallback);

				ThreadPool::UnsafeQueueUserWorkItem(waitCallBack, continuation1);
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
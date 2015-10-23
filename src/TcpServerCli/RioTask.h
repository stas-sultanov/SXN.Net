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
		[System::Security::SuppressUnmanagedCodeSecurity]
		public ref class RioTask sealed : ICriticalNotifyCompletion
		{
			private:

			static initonly Action^ EmptyContinuation = gcnew Action(&RioTask::DoNothing);

			static void DoNothing()
			{
			}

			static initonly WaitCallback^ continueWaitCallback = gcnew WaitCallback(&RioTask::UnsafeCallback);

			/// <summary>
			/// Indicates whether task is completed.
			/// </summary>
			Boolean isCompleted;

			/// <summary>
			/// The number of bytes sent or received in the I/O request.
			/// </summary>
			UInt32 bytesTransferred;

			Action^ continuation;

			internal:

			#pragma region Constructors

			/// <summary>
			/// Initialize a new instance of the <see cref="RioTask" /> class.
			/// </summary>
			RioTask()
			{
				isCompleted = false;

				continuation = nullptr;
			}

			#pragma endregion

			public:

			#pragma region Methohs implementation of the awaiter routine

			/// <summary>
			/// Gets an awaiter used to await this task.
			/// </summary>
			/// <returs>An awaiter instance.</returns>
			/// <remarks>This method is intended for compiler use rather than for use in application code.</remarks>
			[System::Security::SuppressUnmanagedCodeSecurity]
			RioTask^ GetAwaiter()
			{
				return this;
			}

			/// <summary>
			/// Gets a value that indicates whether the asynchronous task has completed.
			/// </summary>
			property Boolean IsCompleted
			{
				[System::Security::SuppressUnmanagedCodeSecurity]
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
			[System::Security::SuppressUnmanagedCodeSecurity]
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
			[System::Security::SuppressUnmanagedCodeSecurity]
			[System::Security::SecurityCritical]
			virtual void UnsafeOnCompleted(Action^ newContinuation)
			{
				// replace reference
				//Interlocked::Exchange(continuation, newContinuation);

				ThreadPool::UnsafeQueueUserWorkItem(continueWaitCallback, newContinuation);

				/**
				if (continuation == EmptyContinuation || Interlocked::CompareExchange(continuation, continuation, (Action ^) nullptr) == EmptyContinuation)
				{
					CompleteCallback(continuation);
				}
				/**/
			}

			/// <summary>
			/// Ends the wait for the completion of the asynchronous task.
			/// </summary>
			[System::Security::SuppressUnmanagedCodeSecurity]
			UInt32 GetResult()
			{
				//Console::WriteLine("ReceiveTask::GetResult Connection[{0}]", this->connection->Id);

				auto result = this->bytesTransferred;

				//Buffer.BlockCopy(_segment.Buffer, _segment.Offset, _buffer.Array, _buffer.Offset, (int)bytesTransferred)
				Reset();

				//connection->PostReceive(requestCorrelation);

				return result;
			}

			#pragma endregion

			#pragma region

			[System::Security::SuppressUnmanagedCodeSecurity]

			[MethodImplAttribute(MethodImplOptions::AggressiveInlining)]
			inline void Reset()
			{
				// reset amount of data transferred
				bytesTransferred = 0;

				// reset completion flag
				isCompleted = false;

				// reset continuation delegate
				continuation = nullptr;
			}

			[System::Security::SuppressUnmanagedCodeSecurity]

			[MethodImplAttribute(MethodImplOptions::AggressiveInlining)]
			inline void Complete(UInt32 bytesTransferred)
			{
				this->bytesTransferred = bytesTransferred;

				// set completed
				isCompleted = true;

				/*
				Action^ continueAction;

				if (continuation == nullptr)
				{
					continueAction = Interlocked::CompareExchange(continuation, EmptyContinuation, (Action ^) nullptr);
				}
				else
				{
					continueAction = continuation;
				}
				*/

				/**
				if (continuation != nullptr)
				{
					CompleteCallback(continuation);
				}/**/
			}

			[System::Security::SuppressUnmanagedCodeSecurity]
			static inline void CompleteCallback(Action^ continuation)
			{
				ThreadPool::UnsafeQueueUserWorkItem(continueWaitCallback, continuation);
			}

			[System::Security::SuppressUnmanagedCodeSecurity]
			[MethodImplAttribute(MethodImplOptions::AggressiveInlining)]
			static inline void UnsafeCallback(Object^ state)
			{
				auto action = (Action ^)state;

				if (action != nullptr)
				{
					action();
				}
			}

			#pragma endregion
		};
	}
}
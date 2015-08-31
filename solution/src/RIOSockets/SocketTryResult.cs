
using System.Runtime.CompilerServices;


namespace System.Net.RIOSockets
{
	/// <summary>
	/// Encapsulates the results of the execution of the Try-Do methods.
	/// </summary>
	/// <typeparam name="T">The type of the result returned by a Try-Do method.</typeparam>
	public struct SocketTryResult<T> : ITryResult<T>
	{
		#region Constructors

		/// <summary>
		/// Initializes a new instance of <see cref="TryResult{T}" /> structure.
		/// </summary>
		/// <param name="errorCode">A <see cref="Boolean" /> value that indicates whether an operation was successful.</param>
		/// <param name="result">A valid <typeparamref name="T" /> object if operation was successful, default value of the <typeparamref name="T"/> type otherwise.</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		internal SocketTryResult(SocketErrorCode errorCode, T result)
		{
			ErrorCode = errorCode;

			Result = result;
		}

		#endregion

		#region Implementation of ITryResult<TResult>

		/// <summary>
		/// Gets an error code.
		/// </summary>
		public SocketErrorCode ErrorCode
		{
			get;
		}

		/// <summary>
		/// Gets an instance of <typeparamref name="T" /> if operation was successful, default value of the <typeparamref name="T"/> type otherwise.
		/// </summary>
		public T Result
		{
			get;
		}

		/// <summary>
		/// Gets a <see cref="Boolean" /> value that indicates whether an operation was successful.
		/// </summary>
		public Boolean Success => ErrorCode == SocketErrorCode.None;

		#endregion
	}
}
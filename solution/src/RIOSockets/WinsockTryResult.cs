using System;
using System.Runtime.CompilerServices;

namespace SXN.Net
{
	/// <summary>
	/// Encapsulates the results of the execution of the Try-Do methods.
	/// </summary>
	/// <typeparam name="T">The type of the result returned by a Try-Do method.</typeparam>
	public struct WinsockTryResult<T> : ITryResult<T>
	{
		#region Constructors

		/// <summary>
		/// Initializes a new instance of <see cref="TryResult{T}" /> structure.
		/// </summary>
		/// <param name="errorCode">A <see cref="Boolean" /> value that indicates whether an operation was successful.</param>
		/// <param name="result">A valid <typeparamref name="T" /> object if operation was successful, default value of the <typeparamref name="T" /> type otherwise.</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		private WinsockTryResult(WinsockErrorCode errorCode, T result)
		{
			ErrorCode = errorCode;

			Result = result;
		}

		#endregion

		#region Methods

		/// <summary>
		/// Crates a failed result of the operation.
		/// </summary>
		/// <param name="errorCode">The code of the error.</param>
		/// <returns>The failed result.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		internal static WinsockTryResult<T> CreateFail(WinsockErrorCode errorCode)
		{
			return new WinsockTryResult<T>(errorCode, default(T));
		}

		/// <summary>
		/// Crates a successful result of the operation.
		/// </summary>
		/// <param name="result">The data returned by the operation.</param>
		/// <returns>The successful result.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		internal static WinsockTryResult<T> CreateSuccess(T result)
		{
			return new WinsockTryResult<T>(WinsockErrorCode.None, result);
		}

		#endregion

		#region Implementation of ITryResult<TResult>

		/// <summary>
		/// Gets an error code.
		/// </summary>
		public WinsockErrorCode ErrorCode
		{
			get;
		}

		/// <summary>
		/// Gets an instance of <typeparamref name="T" /> if operation was successful, default value of the <typeparamref name="T" /> type otherwise.
		/// </summary>
		public T Result
		{
			get;
		}

		/// <summary>
		/// Gets a <see cref="Boolean" /> value that indicates whether an operation was successful.
		/// </summary>
		public Boolean Success => ErrorCode == WinsockErrorCode.None;

		#endregion
	}
}
using System;
using System.Runtime.CompilerServices;
using SXN.Net.Kernel;
using SXN.Net.Winsock;

namespace SXN.Net
{
	/// <summary>
	/// Encapsulates the results of the execution of the Try-Do methods.
	/// </summary>
	/// <typeparam name="T">The type of the result returned by a Try-Do method.</typeparam>
	public struct TryResult<T>
	{
		#region Constructors

		/// <summary>
		/// Initializes a new instance of <see cref="TryResult{T}" /> structure.
		/// </summary>
		/// <param name="kernelErrorCode"><see cref="Kernel.KernelErrorCode.None" /> if there was no error in Kernel library, error code otherwise. </param>
		/// <param name="winsockErrorCode"><see cref="winsock->WinsockErrorCode.None" /> if there was no error in Winsock library, error code otherwise. </param>
		/// <param name="result">A valid <typeparamref name="T" /> object if operation was successful, default value of the <typeparamref name="T" /> type otherwise.</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		private TryResult(KernelErrorCode kernelErrorCode, WinsockErrorCode winsockErrorCode, T result = default(T))
		{
			KernelErrorCode = kernelErrorCode;

			WinsockErrorCode = winsockErrorCode;

			Result = result;
		}

		#endregion

		#region Properties

		/// <summary>
		/// Gets an error code occurred in the Kernel library.
		/// </summary>
		public KernelErrorCode KernelErrorCode
		{
			get;
		}

		/// <summary>
		/// Gets an error code occurred in the Winsock library.
		/// </summary>
		public WinsockErrorCode WinsockErrorCode
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
		/// Specifies whether an operation was successful.
		/// </summary>
		public Boolean Success => KernelErrorCode == KernelErrorCode.None && WinsockErrorCode == WinsockErrorCode.None;

		#endregion

		#region Methods

		/// <summary>
		/// Crates a failed result of the operation.
		/// </summary>
		/// <param name="kernelErrorCode">The code of the error occurred in Kernel library.</param>
		/// <param name="winsockErrorCode">The code of the error occurred in Winsock library.</param>
		/// <returns>The failed result.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		internal static TryResult<T> CreateFail(KernelErrorCode kernelErrorCode, WinsockErrorCode winsockErrorCode) => new TryResult<T>(kernelErrorCode, winsockErrorCode);

		/// <summary>
		/// Crates a failed result of the operation.
		/// </summary>
		/// <param name="kernelErrorCode">The code of the error occurred in Kernel library.</param>
		/// <returns>The failed result.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		internal static TryResult<T> CreateFail(KernelErrorCode kernelErrorCode) => new TryResult<T>(kernelErrorCode, WinsockErrorCode.None);

		/// <summary>
		/// Crates a failed result of the operation.
		/// </summary>
		/// <param name="winsockErrorCode">The code of the error occurred in Winsock library.</param>
		/// <returns>The failed result.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		internal static TryResult<T> CreateFail(WinsockErrorCode winsockErrorCode) => new TryResult<T>(KernelErrorCode.None, winsockErrorCode);

		/// <summary>
		/// Crates a successful result of the operation.
		/// </summary>
		/// <param name="result">The data returned by the operation.</param>
		/// <returns>The successful result.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		internal static TryResult<T> CreateSuccess(T result) => new TryResult<T>(KernelErrorCode.None, WinsockErrorCode.None, result);

		#endregion
	}
}
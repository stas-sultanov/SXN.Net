#pragma once

#include "pch.h"

using namespace System;

namespace SXN::Net
{
	/// <summary>
	/// Represents an exception that is thrown when a Kernel error occurs.
	/// </summary>
	[Serializable]
	public ref class KernelException sealed : Exception
	{
	private:

		#pragma region Fields

		/// <summary>
		/// Error code.
		/// </summary>

		Int32 errorCode;

		#pragma endregion

	internal:

		#pragma region Constructor

		/// <summary>
		/// Initializes a new instance of the <see cref="KernelException" /> class.
		/// </summary>
		/// <param name="errorCode">The Kernel error code.</param>
		/// <param name="message">The message that describes the error.</param>

		KernelException(Int32 errorCode, String^ message) : Exception(message)
		{
			this->errorCode = errorCode;
		}

		#pragma endregion

	public:

		#pragma region Properties

		/// <summary>
		/// The Kernel error code.
		/// </summary>
		property Int32 ErrorCode
		{
			Int32 get()
			{
				return this->errorCode;
			}
		}

		#pragma endregion
	};
}
using System;
using System.Runtime.InteropServices;
// ReSharper disable All

namespace SXN.Net.Winsock
{
	using DWORD = UInt32;

	/// <summary>
	/// Contains information on the functions that implement the Winsock registered I/O extensions.
	/// </summary>
	[StructLayout(LayoutKind.Sequential)]
	internal struct RIO_EXTENSION_FUNCTION_TABLE
	{
		/// <summary>
		/// The size, in bytes, of the structure.
		/// </summary>
		public DWORD cbSize;

		/// <summary>
		/// A pointer to the RIOReceive function.
		/// </summary>
		public IntPtr RIOReceive;

		/// <summary>
		/// A pointer to the RIOReceiveEx function.
		/// </summary>
		public IntPtr RIOReceiveEx;

		/// <summary>
		/// A pointer to the RIOSend function.
		/// </summary>
		public IntPtr RIOSend;

		/// <summary>
		/// A pointer to the RIOSendEx function.
		/// </summary>
		public IntPtr RIOSendEx;

		/// <summary>
		/// A pointer to the RIOCloseCompletionQueue function.
		/// </summary>
		public IntPtr RIOCloseCompletionQueue;

		/// <summary>
		/// A pointer to the RIOCreateCompletionQueue function.
		/// </summary>
		public IntPtr RIOCreateCompletionQueue;

		/// <summary>
		/// A pointer to the RIOCreateRequestQueue function.
		/// </summary>
		public IntPtr RIOCreateRequestQueue;

		/// <summary>
		/// A pointer to the RIODequeueCompletion function.
		/// </summary>
		public IntPtr RIODequeueCompletion;

		/// <summary>
		/// A pointer to the RIODeregisterBuffer function.
		/// </summary>
		public IntPtr RIODeregisterBuffer;

		/// <summary>
		/// A pointer to the RIONotify function.
		/// </summary>
		public IntPtr RIONotify;

		/// <summary>
		/// A pointer to the RIORegisterBuffer function.
		/// </summary>
		public IntPtr RIORegisterBuffer;

		/// <summary>
		/// A pointer to the RIOResizeCompletionQueue function.
		/// </summary>
		public IntPtr RIOResizeCompletionQueue;

		/// <summary>
		/// A pointer to the RIOResizeRequestQueue function.
		/// </summary>
		public IntPtr RIOResizeRequestQueue;
	}
}
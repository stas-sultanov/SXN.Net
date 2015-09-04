using System;
using System.Runtime.InteropServices;

// ReSharper disable All

namespace SXN.Net.Winsock
{
	using DWORD = UInt32;
	using LPFN = IntPtr;

	/// <summary>
	/// Contains information on the functions that implement the Winsock registered I/O extensions.
	/// </summary>
	[StructLayout(LayoutKind.Sequential)]
	internal struct RIO_EXTENSION_FUNCTION_TABLE
	{
		#region Fields

		/// <summary>
		/// The size, in bytes, of the structure.
		/// </summary>
		public DWORD cbSize;

		/// <summary>
		/// A pointer to the RIOReceive function.
		/// </summary>
		public LPFN RIOReceive;

		/// <summary>
		/// A pointer to the RIOReceiveEx function.
		/// </summary>
		public LPFN RIOReceiveEx;

		/// <summary>
		/// A pointer to the RIOSend function.
		/// </summary>
		public LPFN RIOSend;

		/// <summary>
		/// A pointer to the RIOSendEx function.
		/// </summary>
		public LPFN RIOSendEx;

		/// <summary>
		/// A pointer to the RIOCloseCompletionQueue function.
		/// </summary>
		public LPFN RIOCloseCompletionQueue;

		/// <summary>
		/// A pointer to the RIOCreateCompletionQueue function.
		/// </summary>
		public LPFN RIOCreateCompletionQueue;

		/// <summary>
		/// A pointer to the RIOCreateRequestQueue function.
		/// </summary>
		public LPFN RIOCreateRequestQueue;

		/// <summary>
		/// A pointer to the RIODequeueCompletion function.
		/// </summary>
		public LPFN RIODequeueCompletion;

		/// <summary>
		/// A pointer to the RIODeregisterBuffer function.
		/// </summary>
		public LPFN RIODeregisterBuffer;

		/// <summary>
		/// A pointer to the RIONotify function.
		/// </summary>
		public LPFN RIONotify;

		/// <summary>
		/// A pointer to the RIORegisterBuffer function.
		/// </summary>
		public LPFN RIORegisterBuffer;

		/// <summary>
		/// A pointer to the RIOResizeCompletionQueue function.
		/// </summary>
		public LPFN RIOResizeCompletionQueue;

		/// <summary>
		/// A pointer to the RIOResizeRequestQueue function.
		/// </summary>
		public LPFN RIOResizeRequestQueue;

		#endregion
	}
}
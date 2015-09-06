using System;
using System.Runtime.ConstrainedExecution;
using System.Runtime.InteropServices;
using System.Runtime.Versioning;
using System.Security;
using System.Threading;

// ReSharper disable All

namespace SXN.Net.Kernel
{
	using HANDLE = IntPtr;
	using ULONG_PTR = UIntPtr;
	using DWORD = UInt32;
	using OVERLAPPED = NativeOverlapped;
	using SIZE_T = UInt32;

	/// <summary>
	/// Encapsulates functions of the 'kernel32.dll'.
	/// </summary>
	internal static class Interop
	{
		#region Constant and Static Fields

		public static readonly IntPtr INVALID_HANDLE_VALUE = new IntPtr(-1);

		/// <summary>
		/// Name of the source DLL file.
		/// </summary>
		private const String KERNEL32DLL = "kernel32.dll";

		/// <summary>
		/// Allocates memory charges (from the overall size of memory and the paging files on disk) for the specified reserved memory pages.
		/// The function also guarantees that when the caller later initially accesses the memory, the contents will be zero.
		/// Actual physical pages are not allocated unless/until the virtual addresses are actually accessed.
		/// </summary>
		public const DWORD MEM_COMMIT = 0x00001000;

		/// <summary>
		/// Reserves a range of the process's virtual address space without allocating any actual physical storage in memory or in the paging file on disk.
		/// </summary>
		public const DWORD MEM_RESERVE = 0x00002000;

		/// <summary>
		/// Decommits the specified region of committed pages.
		/// </summary>
		/// <remarks>
		/// After the operation, the pages are in the reserved state.
		/// The function does not fail if you attempt to decommit an uncommitted page.
		/// This means that you can decommit a range of pages without first determining the current commitment state.
		/// Do not use this value with <see cref="MEM_RELEASE" />.
		/// </remarks>
		public const UInt32 MEM_DECOMMIT = 0x4000;

		/// <summary>
		/// Releases the specified region of pages.
		/// </summary>
		/// <remarks>
		/// After this operation, the pages are in the free state.
		/// If you specify this value, dwSize must be 0 (zero), and lpAddress must point to the base address returned by the <see cref="VirtualAlloc" /> function when the region is reserved.
		/// The function fails if either of these conditions is not met.
		/// If any pages in the region are committed currently, the function first decommits, and then releases them.
		/// The function does not fail if you attempt to release pages that are in different states, some reserved and some committed.
		/// This means that you can release a range of pages without first determining the current commitment state.
		/// Do not use this value with <see cref="MEM_DECOMMIT" />.
		/// </remarks>
		public const UInt32 MEM_RELEASE = 0x8000;

		/// <summary>
		/// Enables read-only or read/write access to the committed region of pages.
		/// </summary>
		/// <remarks>
		/// If Data Execution Prevention is enabled, attempting to execute code in the committed region results in an access violation.
		/// </remarks>
		public const DWORD PAGE_READWRITE = 0x04;

		#endregion

		#region Methods

		/// <summary>
		/// Attempts to dequeue an I/O completion packet from the specified I/O completion port.
		/// If there is no completion packet queued, the function waits for a pending I/O operation associated with the completion port to complete.
		/// </summary>
		/// <param name="CompletionPort">A handle to the completion port.</param>
		/// <param name="lpNumberOfBytes">A pointer to a variable that receives the number of bytes transferred during an I/O operation that has completed.</param>
		/// <param name="lpCompletionKey">A pointer to a variable that receives the completion key value associated with the file handle whose I/O operation has completed.</param>
		/// <param name="lpOverlapped">A pointer to a variable that receives the address of the <see cref="OVERLAPPED" /> structure that was specified when the completed I/O operation was started.</param>
		/// <param name="dwMilliseconds">The number of milliseconds that the caller is willing to wait for a completion packet to appear at the completion port. If a completion packet does not appear within the specified time, the function times out, returns <c>false</c>, and sets <paramref name="lpOverlapped" /> to <c>null</c>.</param>
		/// <returns>
		/// Returns nonzero <c>true</c> if successful or <c>false</c> otherwise.
		/// To get extended error information, call <see cref="GetLastError" />.
		/// </returns>
		[SuppressUnmanagedCodeSecurity]
		[DllImport(KERNEL32DLL, SetLastError = true)]
		public static extern unsafe Boolean GetQueuedCompletionStatus(HANDLE CompletionPort, out DWORD lpNumberOfBytes, out ULONG_PTR lpCompletionKey, out OVERLAPPED* lpOverlapped, DWORD dwMilliseconds);

		/// <summary>
		/// Creates an input/output (I/O) completion port and associates it with a specified handle, or creates an I/O completion port that is not yet associated with a handle, allowing association at a later time.
		/// Associating an instance of an opened handle with an I/O completion port allows a process to receive notification of the completion of asynchronous I/O operations involving that handle.
		/// </summary>
		/// <param name="FileHandle">An open file handle or <see cref="INVALID_HANDLE_VALUE" />.</param>
		/// <param name="ExistingCompletionPort">A handle to an existing I/O completion port or <c>null</c>.</param>
		/// <param name="CompletionKey">The per-handle user-defined completion key that is included in every I/O completion packet for the specified file handle.</param>
		/// <param name="NumberOfConcurrentThreads">The maximum number of threads that the operating system can allow to concurrently process I/O completion packets for the I/O completion port.</param>
		/// <returns>
		/// If the function succeeds, the return value is the handle to an I/O completion port:
		/// <list type="bullet">
		///     <item>
		///         <description>If the <paramref name="ExistingCompletionPort" /> parameter was <c>null</c>, the return value is a new handle.</description>
		///     </item>
		///     <item>
		///         <description>If the <paramref name="ExistingCompletionPort" /> parameter was a valid I/O completion port handle, the return value is that same handle.</description>
		///     </item>
		///     <item>
		///         <description>If the <paramref name="FileHandle" /> parameter was a valid handle, that file handle is now associated with the returned I/O completion port.</description>
		///     </item>
		/// </list>
		/// If the function fails, the return value is <c>null</c>.
		/// To get extended error information, call the <see cref="GetLastError" /> function.
		/// </returns>
		[SuppressUnmanagedCodeSecurity]
		[DllImport(KERNEL32DLL, SetLastError = true)]
		public static extern HANDLE CreateIoCompletionPort([In] HANDLE FileHandle, [In] HANDLE ExistingCompletionPort, [In] ULONG_PTR CompletionKey, [In] DWORD NumberOfConcurrentThreads);

		/// <summary>
		/// Retrieves the calling thread's last-error code value. The last-error code is maintained on a per-thread basis.
		/// </summary>
		/// <returns>
		/// The return value is the calling thread's last-error code.
		/// </returns>
		/// <remarks>
		/// Multiple threads do not overwrite each other's last-error code.
		/// </remarks>
		[SuppressUnmanagedCodeSecurity]
		[DllImport(KERNEL32DLL)]
		public static extern DWORD GetLastError();

		/// <summary>
		/// Reserves, commits, or changes the state of a region of pages in the virtual address space of the calling process.
		/// Memory allocated by this function is automatically initialized to zero.
		/// </summary>
		/// <param name="lpAddress">
		/// The starting address of the region to allocate.
		/// If the memory is being reserved, the specified address is rounded down to the nearest multiple of the allocation granularity.
		/// If the memory is already reserved and is being committed, the address is rounded down to the next page boundary.
		/// To determine the size of a page and the allocation granularity on the host computer, use the GetSystemInfo function.
		/// If this parameter is <c>null</c>, the system determines where to allocate the region.
		/// </param>
		/// <param name="dwSize">
		/// The size of the region, in bytes. If the <paramref name="lpAddress" /> parameter is <c>null</c>, this value is rounded up to the next page boundary.
		/// Otherwise, the allocated pages include all pages containing one or more bytes in the range from <paramref name="lpAddress" /> to <paramref name="lpAddress" />+<paramref name="dwSize" />.
		/// This means that a 2-byte range straddling a page boundary causes both pages to be included in the allocated region.
		/// </param>
		/// <param name="flAllocationType">The type of memory allocation.</param>
		/// <param name="flProtect">The memory protection for the region of pages to be allocated.</param>
		/// <returns>
		/// If the function succeeds, the return value is the base address of the allocated region of pages.
		/// If the function fails, the return value is <c>null</c>.
		/// To get extended error information, call <see cref="GetLastError" />.
		/// </returns>
		[ResourceExposure(ResourceScope.Process)]
		[ReliabilityContract(Consistency.WillNotCorruptState, Cer.MayFail)]
		[SuppressUnmanagedCodeSecurity]
		[DllImport(KERNEL32DLL, SetLastError = true)]
		internal static extern unsafe void* VirtualAlloc([In] void* lpAddress, [In] SIZE_T dwSize, [In] DWORD flAllocationType, [In] DWORD flProtect);

		/// <summary>
		/// Releases, decommits, or releases and decommits a region of pages within the virtual address space of the calling process.
		/// </summary>
		/// <param name="lpAddress"></param>
		/// <param name="dwSize">The size of the region of memory to be freed, in bytes.</param>
		/// <param name="dwFreeType">The type of free operation.</param>
		/// <returns>The type of free operation.</returns>
		[ResourceExposure(ResourceScope.Process)]
		[ReliabilityContract(Consistency.WillNotCorruptState, Cer.MayFail)]
		[SuppressUnmanagedCodeSecurity]
		[DllImport(KERNEL32DLL, SetLastError = true)]
		internal static extern unsafe Boolean VirtualFree([In] void* lpAddress, [In] UIntPtr dwSize, Int32 dwFreeType);

		#endregion
	}
}
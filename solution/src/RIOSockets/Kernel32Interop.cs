using System;
using System.Runtime.ConstrainedExecution;
using System.Runtime.InteropServices;
using System.Runtime.Versioning;

namespace SXN.Net
{
	/// <summary>
	/// Encapsulates functions of the 'kernel32'.
	/// </summary>
	internal static class Kernel32Interop
	{
		#region Constant and Static Fields

		/// <summary>
		/// Name of the source DLL file.
		/// </summary>
		private const String KERNEL32 = "kernel32.dll";

		/// <summary>
		/// Decommits the specified region of committed pages.
		/// </summary>
		/// <remarks>
		/// After the operation, the pages are in the reserved state.
		/// The function does not fail if you attempt to decommit an uncommitted page. This means that you can decommit a range of pages without first determining the current commitment state.
		/// Do not use this value with <see cref="MEM_RELEASE" />.
		/// </remarks>
		public const UInt32 MEM_DECOMMIT = 0x4000;

		/// <summary>
		/// Releases the specified region of pages.
		/// </summary>
		/// <remarks>
		/// After this operation, the pages are in the free state.
		/// If you specify this value, dwSize must be 0 (zero), and lpAddress must point to the base address returned by the <see cref="VirtualAlloc" /> function when the region is reserved. The function fails if either of these conditions is not met.
		/// If any pages in the region are committed currently, the function first decommits, and then releases them.
		/// The function does not fail if you attempt to release pages that are in different states, some reserved and some committed.This means that you can release a range of pages without first determining the current commitment state.
		/// Do not use this value with <see cref="MEM_DECOMMIT" />.
		/// </remarks>
		public const UInt32 MEM_RELEASE = 0x8000;

		#endregion

		#region Methods

		/// <summary>
		/// Reserves, commits, or changes the state of a region of pages in the virtual address space of the calling process. Memory allocated by this function is automatically initialized to zero.
		/// </summary>
		/// <param name="lpAddress">The starting address of the region to allocate. If the memory is being reserved, the specified address is rounded down to the nearest multiple of the allocation granularity. If the memory is already reserved and is being committed, the address is rounded down to the next page boundary. To determine the size of a page and the allocation granularity on the host computer, use the GetSystemInfo function. If this parameter is NULL, the system determines where to allocate the region.</param>
		/// <param name="dwSize">The size of the region, in bytes. If the lpAddress parameter is NULL, this value is rounded up to the next page boundary. Otherwise, the allocated pages include all pages containing one or more bytes in the range from lpAddress to lpAddress+dwSize. This means that a 2-byte range straddling a page boundary causes both pages to be included in the allocated region.</param>
		/// <param name="flAllocationType">The type of memory allocation.</param>
		/// <param name="flProtect">The memory protection for the region of pages to be allocated.</param>
		/// <returns>
		/// If the function succeeds, the return value is the base address of the allocated region of pages.
		/// If the function fails, the return value is NULL.To get extended error information, call GetLastError.
		/// </returns>
		[ResourceExposure(ResourceScope.Process)]
		[ReliabilityContract(Consistency.WillNotCorruptState, Cer.MayFail)]
		[DllImport(KERNEL32, SetLastError = true)]
		internal static extern unsafe void* VirtualAlloc([In] void* lpAddress, [In] UIntPtr dwSize, [In] UInt32 flAllocationType, [In] UInt32 flProtect);

		/// <summary>
		/// Releases, decommits, or releases and decommits a region of pages within the virtual address space of the calling process.
		/// </summary>
		/// <param name="lpAddress"></param>
		/// <param name="dwSize">The size of the region of memory to be freed, in bytes.</param>
		/// <param name="dwFreeType">The type of free operation.</param>
		/// <returns>The type of free operation.</returns>
		[ResourceExposure(ResourceScope.Process)]
		[ReliabilityContract(Consistency.WillNotCorruptState, Cer.MayFail)]
		[DllImport(KERNEL32, SetLastError = true)]
		internal static extern unsafe Boolean VirtualFree([In] void* lpAddress, [In] UIntPtr dwSize, Int32 dwFreeType);

		#endregion
	}
}
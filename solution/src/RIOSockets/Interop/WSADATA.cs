using System.Runtime.InteropServices;

namespace System.Net.RIOSockets
{
	/// <summary>
	/// Contains information about the Windows Sockets implementation.
	/// </summary>
	/// <remarks>
	/// The <see cref="WinsockInterop.WSAStartup"/> function initiates the use of the Windows Sockets DLL by a process.
	/// The <see cref="WinsockInterop.WSAStartup"/> function returns a pointer to the <see cref="WSADATA"/> structure in the lpWSAData parameter.
	/// The current version of the Windows Sockets specification returned in the <see cref="wHighVersion"/> member of the <see cref="WSADATA"/> structure is version 2.2 encoded with the major version number in the low-byte and the minor version number in the high-byte.
	/// </remarks>
	[StructLayout(LayoutKind.Sequential)]
	internal struct WSADATA
	{
		/// <summary>
		/// The version of the Windows Sockets specification that the Ws2_32.dll expects the caller to use.
		/// The high-order byte specifies the minor version number; the low-order byte specifies the major version number.
		/// </summary>
		internal Int16 wVersion;

		/// <summary>
		/// The highest version of the Windows Sockets specification that the Ws2_32.dll can support.
		/// The high-order byte specifies the minor version number; the low-order byte specifies the major version number.
		/// </summary>
		/// <remarks>
		/// This is the same value as the <see cref="wVersion"/> member when the version requested in the wVersionRequested parameter passed to the <see cref="WinsockInterop.WSAStartup"/> function is the highest version of the Windows Sockets specification that the Ws2_32.dll can support.
		/// </remarks>
		internal Int16 wHighVersion;

		/// <summary>
		/// A NULL-terminated ASCII string into which the Ws2_32.dll copies a description of the Windows Sockets implementation.
		/// The text (up to 256 characters in length) can contain any characters except control and formatting characters.
		/// The most likely use that an application would have for this member is to display it (possibly truncated) in a status message.
		/// </summary>
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 257)]
		internal String szDescription;

		/// <summary>
		/// A NULL-terminated ASCII string into which the Ws2_32.dll copies relevant status or configuration information.
		/// The Ws2_32.dll should use this parameter only if the information might be useful to the user or support staff.
		/// This member should not be considered as an extension of the <see cref="szDescription"/> parameter.
		/// </summary>
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 129)]
		internal String szSystemStatus;

		/// <summary>
		/// The maximum number of sockets that may be opened.
		/// This member should be ignored for Windows Sockets version 2 and later.
		/// </summary>
		/// <remarks>
		/// The <see cref="iMaxSockets"/> member is retained for compatibility with Windows Sockets specification 1.1, but should not be used when developing new applications.
		/// No single value can be appropriate for all underlying service providers.
		/// The architecture of Windows Sockets changed in version 2 to support multiple providers, and the <see cref="WSADATA"/> structure no longer applies to a single vendor's stack.
		/// </remarks>
		internal Int16 iMaxSockets;

		/// <summary>
		/// The maximum datagram message size. This member is ignored for Windows Sockets version 2 and later.
		/// </summary>
		/// <remarks>
		/// The <see cref="iMaxUdpDg"/> member is retained for compatibility with Windows Sockets specification 1.1, but should not be used when developing new applications.
		/// The architecture of Windows Sockets changed in version 2 to support multiple providers, and the <see cref="WSADATA"/> structure no longer applies to a single vendor's stack.
		/// For the actual maximum message size specific to a particular Windows Sockets service provider and socket type, applications should use getsockopt to retrieve the value of option SO_MAX_MSG_SIZE after a socket has been created.
		/// </remarks>
		internal Int16 iMaxUdpDg;

		/// <summary>
		/// A pointer to vendor-specific information. This member should be ignored for Windows Sockets version 2 and later.
		/// </summary>
		/// <remarks>
		/// The <see cref="lpVendorInfo"/> member is retained for compatibility with Windows Sockets specification 1.1.
		/// The architecture of Windows Sockets changed in version 2 to support multiple providers, and the <see cref="WSADATA"/> structure no longer applies to a single vendor's stack.
		/// Applications needing to access vendor-specific configuration information should use getsockopt to retrieve the value of option PVD_CONFIG for vendor-specific information.
		/// </remarks>
		internal IntPtr lpVendorInfo;
	}
}
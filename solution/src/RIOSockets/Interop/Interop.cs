using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Security;

// ReSharper disable InconsistentNaming

namespace System.Net.RIOSockets
{
	internal static class Interop
	{
		#region Constant and Static Fields

		public static readonly IntPtr INVALID_SOCKET = new IntPtr(-1);

		public static readonly IntPtr SOCKET_ERROR = new IntPtr(-1);

		#endregion

		#region Methods

		[DllImport("WS2_32.dll", SetLastError = true, CharSet = CharSet.Ansi)]
		public static extern Int32 bind(IntPtr s, ref sockaddr_in name, Int32 namelen);

		/// <summary>
		/// Terminates use of the Winsock DLL.
		/// </summary>
		/// <returns>
		/// The return value is <c>0</c> if the operation was successful.Otherwise, the value <see cref="SOCKET_ERROR" /> is returned, and a specific error number can be retrieved by calling <see cref="WSAGetLastError" />.
		/// </returns>
		[SuppressUnmanagedCodeSecurity]
		[DllImport("WS2_32.dll", SetLastError = true)]
		public static extern Int32 WSACleanup();

		/// <summary>
		/// Returns the error status for the last Windows Sockets operation that failed.
		/// </summary>
		/// <returns>The value that indicates the error code for this thread's last Windows Sockets operation that failed.</returns>
		[SuppressUnmanagedCodeSecurity]
		[DllImport("WS2_32.dll")]
		public static extern Int32 WSAGetLastError();

		/// <summary>
		/// Creates a socket that is bound to a specific transport-service provider.
		/// </summary>
		/// <param name="af">The address family specification.</param>
		/// <param name="type">The type specification for the new socket.</param>
		/// <param name="protocol">The protocol to be used.</param>
		/// <param name="lpProtocolInfo">A pointer to a WSAPROTOCOL_INFO structure that defines the characteristics of the socket to be created. If this parameter is not NULL, the socket will be bound to the provider associated with the indicated WSAPROTOCOL_INFO structure.</param>
		/// <param name="group">An existing socket group ID or an appropriate action to take when creating a new socket and a new socket group.</param>
		/// <param name="dwFlags">A set of flags used to specify additional socket attributes.</param>
		/// <returns>If no error occurs, returns a descriptor referencing the new socket. Otherwise, a value of <see cref="INVALID_SOCKET" /> is returned, and a specific error code can be retrieved by calling <see cref="WSAGetLastError" />.</returns>
		[SuppressUnmanagedCodeSecurity]
		[DllImport("WS2_32.dll", SetLastError = true, CharSet = CharSet.Ansi)]
		internal static extern IntPtr WSASocket([In] AddressFamily af, [In] SocketType type, [In] ProtocolType protocol, [In] IntPtr lpProtocolInfo, [In] Int32 group, [In] SocketCreateFlags dwFlags);

		/// <summary>
		/// Initiates use of the Winsock DLL by a process.
		/// </summary>
		/// <param name="wVersionRequested">The highest version of Windows Sockets specification that the caller can use. The high-order byte specifies the minor version number; the low-order byte specifies the major version number.</param>
		/// <param name="lpWSAData">A pointer to the <see cref="WSADATA" /> data structure that is to receive details of the Windows Sockets implementation.</param>
		/// <returns>
		/// If successful, the function returns <see cref="SocketErrorCode.None" />. Otherwise, it returns one of the error codes listed below.
		/// <list type="table">
		///     <item>
		///         <term>
		///             <see cref="SocketErrorCode.WSASYSNOTREADY" />
		///         </term>
		///         <description>The underlying network subsystem is not ready for network communication.</description>
		///     </item>
		///     <item>
		///         <term>
		///             <see cref="SocketErrorCode.WSAVERNOTSUPPORTED" />
		///         </term>
		///         <description>The version of Windows Sockets support requested is not provided by this particular Windows Sockets implementation.</description>
		///     </item>
		///     <item>
		///         <term>
		///             <see cref="SocketErrorCode.WSAEINPROGRESS" />
		///         </term>
		///         <description>A blocking Windows Sockets 1.1 operation is in progress.</description>
		///     </item>
		///     <item>
		///         <term>
		///             <see cref="SocketErrorCode.WSAEPROCLIM" />
		///         </term>
		///         <description>A limit on the number of tasks supported by the Windows Sockets implementation has been reached.</description>
		///     </item>
		///     <item>
		///         <term>
		///             <see cref="SocketErrorCode.WSAEFAULT" />
		///         </term>
		///         <description>The <paramref name="lpWSAData" /> parameter is not a valid pointer.</description>
		///     </item>
		/// </list>
		/// </returns>
		[SuppressUnmanagedCodeSecurity]
		[DllImport("WS2_32.dll", SetLastError = true, CharSet = CharSet.Ansi, BestFitMapping = true, ThrowOnUnmappableChar = true)]
		internal static extern SocketErrorCode WSAStartup([In] Int16 wVersionRequested, [Out] out WSADATA lpWSAData);

		#endregion
	}
}
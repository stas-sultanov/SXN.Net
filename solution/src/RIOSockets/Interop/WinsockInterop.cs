using System.Runtime.InteropServices;
using System.Security;

// ReSharper disable InconsistentNaming

namespace System.Net.RIOSockets
{
	internal static class WinsockInterop
	{
		#region Constant and Static Fields

		public const Int32 SOCKET_ERROR = -1;

		public const Int16 Version = 0x22;

		private const String WS232DLL = "WS2_32.dll";

		public static readonly IntPtr INVALID_SOCKET = new IntPtr(-1);

		/// <summary>
		/// Control code that allows to configure a TCP socket for faster operations on the loopback interface.
		/// </summary>
		public const Int32 SIO_LOOPBACK_FAST_PATH = (-1744830448);

		/// <summary>
		/// 
		/// </summary>
		public const Int32 IPPROTO_TCP = 6;

		/// <summary>
		/// Enables or disables the Nagle algorithm for TCP sockets. This option is disabled (set to FALSE) by default.
		/// </summary>
		public const Int32 TCP_NODELAY = 0x0001;

		#endregion

		#region Private methods

		/// <summary>
		/// Controls the mode of a socket.
		/// </summary>
		/// <param name="s">A descriptor identifying a socket.</param>
		/// <param name="dwIoControlCode">The control code of operation to perform.</param>
		/// <param name="lpvInBuffer">A pointer to the input buffer.</param>
		/// <param name="cbInBuffer">The size, in bytes, of the input buffer.</param>
		/// <param name="lpvOutBuffer">A pointer to the output buffer.</param>
		/// <param name="cbOutBuffer">The size, in bytes, of the output buffer.</param>
		/// <param name="lpcbBytesReturned">A pointer to actual number of bytes of output.</param>
		/// <param name="lpOverlapped">A pointer to a WSAOVERLAPPED structure (ignored for non-overlapped sockets).</param>
		/// <param name="lpCompletionRoutine">A pointer to the completion routine called when the operation has been completed.</param>
		/// <returns>
		/// Upon successful completion, returns zero. Otherwise, a value of <see cref="SOCKET_ERROR" /> is returned, and a specific error code can be retrieved by calling <see cref="WSAGetLastError" />.
		/// </returns>
		/// <remarks>
		/// The function is used to set or retrieve operating parameters associated with the socket, the transport protocol, or the communications subsystem.
		/// </remarks>
		[SuppressUnmanagedCodeSecurity]
		[DllImport(WS232DLL, SetLastError = true)]
		private static extern unsafe Int32 WSAIoctl
			(
			[In] IntPtr s,
			[In] UInt32 dwIoControlCode,
			[In] void* lpvInBuffer,
			[In] UInt32 cbInBuffer,
			[Out] void* lpvOutBuffer,
			[In] UInt32 cbOutBuffer,
			[Out] out UInt32 lpcbBytesReturned,
			[In] IntPtr lpOverlapped,
			[In] IntPtr lpCompletionRoutine
			);

		#endregion

		#region Methods

		/// <summary>
		/// Permits an incoming connection attempt on a socket.
		/// </summary>
		/// <param name="s">A descriptor that identifies a socket that has been placed in a listening state with the <see cref="listen" /> function. The connection is actually made with the socket that is returned by accept.</param>
		/// <param name="addr">An optional pointer to a buffer that receives the address of the connecting entity, as known to the communications layer. The exact format of the <paramref name="addr" /> parameter is determined by the address family that was established when the socket from the sockaddr structure was created.</param>
		/// <param name="addrlen">An optional pointer to an integer that contains the length of structure pointed to by the <paramref name="addr" /> parameter.</param>
		/// <returns>
		/// If no error occurs, accept returns a value of type SOCKET that is a descriptor for the new socket.
		/// This returned value is a handle for the socket on which the actual connection is made.
		/// Otherwise, a value of <see cref="INVALID_SOCKET" /> is returned, and a specific error code can be retrieved by calling <see cref="WSAGetLastError" />.
		/// </returns>
		[SuppressUnmanagedCodeSecurity]
		[DllImport(WS232DLL, SetLastError = true)]
		public static extern IntPtr accept([In] IntPtr s, [Out] IntPtr addr, [In] [Out] Int32 addrlen);

		/// <summary>
		/// Associates a local address with a socket.
		/// </summary>
		/// <param name="s">A descriptor identifying an unbound socket.</param>
		/// <param name="name">A pointer to a <see cref="SOCKADDR_IN" /> structure of the local address to assign to the bound socket .</param>
		/// <param name="namelen">The length, in bytes, of the value pointed to by the <paramref name="name" /> parameter.</param>
		/// <returns>If no error occurs, returns zero. Otherwise, returns <see cref="SOCKET_ERROR" />, and a specific error code can be retrieved by calling <see cref="WSAGetLastError" />.</returns>
		[DllImport(WS232DLL, SetLastError = true, CharSet = CharSet.Ansi)]
		public static extern Int32 bind([In] IntPtr s, [In] ref SOCKADDR_IN name, [In] Int32 namelen);

		/// <summary>
		/// Converts a <paramref name="hostshort" /> from host to TCP/IP network byte order (which is big-endian).
		/// </summary>
		/// <param name="hostshort">A 16-bit number in host byte order.</param>
		/// <returns>The value in TCP/IP network byte order</returns>
		[SuppressUnmanagedCodeSecurity]
		[DllImport(WS232DLL, SetLastError = true)]
		public static extern UInt16 htons([In] UInt16 hostshort);

		/// <summary>
		/// Places a socket in a state in which it is listening for an incoming connection.
		/// </summary>
		/// <param name="s">A descriptor identifying a bound, unconnected socket.</param>
		/// <param name="backlog">The maximum length of the queue of pending connections. If set to SOMAXCONN, the underlying service provider responsible for socket s will set the backlog to a maximum reasonable value. There is no standard provision to obtain the actual backlog value.</param>
		/// <returns>If no error occurs, returns zero. Otherwise, returns <see cref="SOCKET_ERROR" />, and a specific error code can be retrieved by calling <see cref="WSAGetLastError" />.</returns>
		[SuppressUnmanagedCodeSecurity]
		[DllImport(WS232DLL, SetLastError = true)]
		public static extern Int32 listen([In] IntPtr s, [In] Int32 backlog);

		/// <summary>
		/// Sets a socket option.
		/// </summary>
		/// <param name="s">A descriptor that identifies a socket.</param>
		/// <param name="level">The level at which the option is defined.</param>
		/// <param name="optname">The socket option for which the value is to be set.</param>
		/// <param name="optval">A pointer to the buffer in which the value for the requested option is specified.</param>
		/// <param name="optlen">The size, in bytes, of the buffer pointed to by the <paramref name="optval" /> parameter.</param>
		/// <returns>If no error occurs, setsockopt returns zero. Otherwise, a value of <see cref="SOCKET_ERROR" /> is returned, and a specific error code can be retrieved by calling <see cref="WSAGetLastError" />.</returns>
		[SuppressUnmanagedCodeSecurity]
		[DllImport(WS232DLL, SetLastError = true)]
		public static extern unsafe Int32 setsockopt(IntPtr s, Int32 level, Int32 optname, Char* optval, Int32 optlen);

		/// <summary>
		/// Terminates use of the Winsock DLL.
		/// </summary>
		/// <returns>
		/// The return value is <c>0</c> if the operation was successful.Otherwise, the value <see cref="SOCKET_ERROR" /> is returned, and a specific error number can be retrieved by calling <see cref="WSAGetLastError" />.
		/// </returns>
		[SuppressUnmanagedCodeSecurity]
		[DllImport(WS232DLL, SetLastError = true)]
		public static extern Int32 WSACleanup();

		/// <summary>
		/// Returns the error status for the last Windows Sockets operation that failed.
		/// </summary>
		/// <returns>The value that indicates the error code for this thread's last Windows Sockets operation that failed.</returns>
		[SuppressUnmanagedCodeSecurity]
		[DllImport(WS232DLL)]
		public static extern Int32 WSAGetLastError();

		/// <summary>
		/// Creates a socket that is bound to a specific transport-service provider.
		/// </summary>
		/// <param name="af">The address family specification.</param>
		/// <param name="type">The type specification for the new socket.</param>
		/// <param name="protocol">The protocol to be used.</param>
		/// <param name="lpProtocolInfo">A pointer to a WSAPROTOCOL_INFO structure that defines the characteristics of the socket to be created. If this parameter is not NULL, the socket will be bound to the provider associated with the indicated WSAPROTOCOL_INFO structure.</param>
		/// <param name="g">An existing socket group ID or an appropriate action to take when creating a new socket and a new socket group.</param>
		/// <param name="dwFlags">A set of flags used to specify additional socket attributes.</param>
		/// <returns>If no error occurs, returns a descriptor referencing the new socket. Otherwise, a value of <see cref="INVALID_SOCKET" /> is returned, and a specific error code can be retrieved by calling <see cref="WSAGetLastError" />.</returns>
		[SuppressUnmanagedCodeSecurity]
		[DllImport(WS232DLL, SetLastError = true, CharSet = CharSet.Ansi)]
		internal static extern IntPtr WSASocket([In] Int32 af, [In] Int32 type, [In] Int32 protocol, [In] IntPtr lpProtocolInfo, [In] Int32 g, [In] SocketCreateFlags dwFlags);

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
		[DllImport(WS232DLL, SetLastError = true, CharSet = CharSet.Ansi, BestFitMapping = true, ThrowOnUnmappableChar = true)]
		internal static extern SocketErrorCode WSAStartup([In] Int16 wVersionRequested, [Out] out WSADATA lpWSAData);

		#endregion
	}
}
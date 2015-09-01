namespace System.Net.RIOSockets
{
	/// <summary>
	/// A set of flags used to specify additional socket attributes for <see cref="WinsockInterop.WSASocket"/> method.
	/// </summary>
	[Flags]
	internal enum SocketCreateFlags : uint
	{
		/// <summary>
		/// Indicates to create a socket that supports overlapped I/O operations.
		/// </summary>
		OVERLAPPED = 0x01,

		/// <summary>
		/// Indicates to create a socket that will be a c_root in a multipoint session.
		/// </summary>
		MULTIPOINT_C_ROOT = 0x02,

		/// <summary>
		/// Indicates to create a socket that will be a c_leaf in a multipoint session.
		/// </summary>
		MULTIPOINT_C_LEAF = 0x04,

		/// <summary>
		/// Indicates to create a socket that will be a d_root in a multipoint session.
		/// </summary>
		MULTIPOINT_D_ROOT = 0x08,

		/// <summary>
		/// Indicates to create a socket that will be a d_leaf in a multipoint session.
		/// </summary>
		MULTIPOINT_D_LEAF = 0x10,

		/// <summary>
		/// Indicates to create a socket that allows the the ability to set a security descriptor on the socket that contains a security access control list (SACL) as opposed to just a discretionary access control list (DACL).
		/// </summary>
		ACCESS_SYSTEM_SECURITY = 0x40,

		/// <summary>
		/// Indicates to create a socket that is non-inheritable.
		/// </summary>
		NO_HANDLE_INHERIT = 0x80,

		/// <summary>
		/// Indicates to create a registered IO socket.
		/// </summary>
		REGISTERED_IO = 0x100
	}
}
#pragma once

#include "stdafx.h"

namespace SXN
{
	namespace Net
	{
		/// <summary>
		/// Specifies the error code occured within the winsock library.
		/// </summary>
		public enum class WinsockErrorCode: int
		{
			/// <summary>
			/// No error.
			/// </summary>
			None = 0,

			/// <summary>
			/// Specified event object handle is invalid.
			/// </summary>
			/// <remarks>
			/// An application attempts to use an event object, but the specified handle is not valid.
			/// Note that this error is returned by the operating system, so the error number may change in future releases of Windows.
			/// </remarks>
			InvalidHandle = WSA_INVALID_HANDLE,

			/// <summary>
			/// Insufficient memory available.
			/// </summary>
			/// <remarks>
			/// An application used a Windows Sockets function that directly maps to a Windows function.
			/// The Windows function is indicating a lack of required memory resources.
			/// Note that this error is returned by the operating system, so the error number may change in future releases of Windows.
			/// </remarks>
			NotEnoughMemory = WSA_NOT_ENOUGH_MEMORY,

			/// <summary>
			/// One or more parameters are invalid.
			/// </summary>
			/// <remarks>
			/// An application used a Windows Sockets function which directly maps to a Windows function.
			/// The Windows function is indicating a problem with one or more parameters.
			/// Note that this error is returned by the operating system, so the error number may change in future releases of Windows.
			/// </remarks>
			InvalidParameter = WSA_INVALID_PARAMETER,

			/// <summary>
			/// Overlapped operation aborted.
			/// </summary>
			/// <remarks>
			/// An overlapped operation was canceled due to the closure of the socket, or the execution of the SIO_FLUSH command in <see cref="WinsockInterop.WSAIoctl" />.
			/// Note that this error is returned by the operating system, so the error number may change in future releases of Windows.
			/// </remarks>
			OperationAborted = WSA_OPERATION_ABORTED,

			/// <summary>
			/// Overlapped I/O event object not in signaled state.
			/// </summary>
			/// <remarks>
			/// The application has tried to determine the status of an overlapped operation which is not yet completed.
			/// Applications that use WSAGetOverlappedResult (with the fWait flag set to FALSE) in a polling mode to determine when an overlapped operation has completed, get this error code until the operation is complete.
			/// Note that this error is returned by the operating system, so the error number may change in future releases of Windows.
			/// </remarks>
			IoComplete = WSA_IO_INCOMPLETE,

			/// <summary>
			/// Overlapped operations will complete later.
			/// </summary>
			/// <remarks>
			/// The application has initiated an overlapped operation that cannot be completed immediately.
			/// A completion indication will be given later when the operation has been completed.
			/// Note that this error is returned by the operating system, so the error number may change in future releases of Windows.
			/// </remarks>
			IoPending = WSA_IO_PENDING,

			/// <summary>
			/// Interrupted function call.
			/// </summary>
			/// <remarks>
			/// A blocking operation was interrupted by a call to WSACancelBlockingCall.
			/// </remarks>
			Interrupted = WSAEINTR,

			/// <summary>
			/// File handle is not valid.
			/// </summary>
			/// <remarks>
			/// The file handle supplied is not valid.
			/// </remarks>
			BadFile = WSAEBADF,

			/// <summary>
			/// Permission denied.
			/// </summary>
			/// <remarks>
			/// An attempt was made to access a socket in a way forbidden by its access permissions. An example is using a broadcast address for sendto without broadcast permission being set using setsockopt(SO_BROADCAST).
			/// Another possible reason for the WSAEACCES error is that when the bind function is called(on Windows NT 4.0 with SP4 and later), another application, service, or kernel mode driver is bound to the same address with exclusive access.Such exclusive access is a new feature of Windows NT 4.0 with SP4 and later, and is implemented by using the SO_EXCLUSIVEADDRUSE option.
			/// </remarks>
			Access = WSAEACCES,

			/// <summary>
			/// Bad address.
			/// </summary>
			/// <remarks>
			/// The system detected an invalid pointer address in attempting to use a pointer argument of a call.This error occurs if an application passes an invalid pointer value, or if the length of the buffer is too small. For instance, if the length of an argument, which is a sockaddr structure, is smaller than the sizeof(sockaddr).
			/// </remarks>
			Fault = WSAEFAULT,

			/// <summary>
			/// Invalid argument.
			/// </summary>
			/// <remarks>
			/// Some invalid argument was supplied (for example, specifying an invalid level to the setsockopt function). In some instances, it also refers to the current state of the socket—for instance, calling accept on a socket that is not listening.
			/// </remarks>
			InvalidValue = WSAEINVAL,

			/// <summary>
			/// Too many open files.
			/// </summary>
			/// <remarks>
			/// Too many open sockets. Each implementation may have a maximum number of socket handles available, either globally, per process, or per thread.
			/// </remarks>
			ManyFile = WSAEMFILE,

			/// <summary>
			/// Resource temporarily unavailable.
			/// </summary>
			/// <remarks>
			/// This error is returned from operations on nonblocking sockets that cannot be completed immediately, for example recv when no data is queued to be read from the socket.It is a nonfatal error, and the operation should be retried later.It is normal for WSAEWOULDBLOCK to be reported as the result from calling connect on a nonblocking SOCK_STREAM socket, since some time must elapse for the connection to be established.
			/// </remarks>
			WouldBlock = WSAEWOULDBLOCK,

			/// <summary>
			/// Operation now in progress.
			/// </summary>
			/// <remarks>
			/// A blocking operation is currently executing. Windows Sockets only allows a single blocking operation—per- task or thread—to be outstanding, and if any other function call is made (whether or not it references that or any other socket) the function fails with the WSAEINPROGRESS error.
			/// </remarks>
			InProgress = WSAEINPROGRESS,

			/// <summary>
			/// Operation already in progress.
			/// </summary>
			/// <remarks>
			/// An operation was attempted on a nonblocking socket with an operation already in progress—that is, calling connect a second time on a nonblocking socket that is already connecting, or canceling an asynchronous request(WSAAsyncGetXbyY) that has already been canceled or completed.
			/// </remarks>
			Already = WSAEALREADY,

			/// <summary>
			/// Socket operation on nonsocket.
			/// </summary>
			/// <remarks>
			/// An operation was attempted on something that is not a socket.Either the socket handle parameter did not reference a valid socket, or for select, a member of an fd_set was not valid.
			/// </remarks>
			NotSocket = WSAENOTSOCK,

			/// <summary>
			/// Destination address required.
			/// </summary>
			/// <remarks>
			/// A required address was omitted from an operation on a socket.For example, this error is returned if sendto is called with the remote address of ADDR_ANY.
			/// </remarks>
			DestinationAddressRequired = WSAEDESTADDRREQ,

			/// <summary>
			/// Message too long.
			/// </summary>
			/// <remarks>
			/// A message sent on a datagram socket was larger than the internal message buffer or some other network limit, or the buffer used to receive a datagram was smaller than the datagram itself.
			/// </remarks>
			MessageSize = WSAEMSGSIZE,

			/// <summary>
			/// Protocol wrong type for socket.
			/// </summary>
			/// <remarks>
			/// A protocol was specified in the socket function call that does not support the semantics of the socket type requested.For example, the ARPA Internet UDP protocol cannot be specified with a socket type of SOCK_STREAM.
			/// </remarks>
			ProtocolType = WSAEPROTOTYPE,

			/// <summary>
			/// Bad protocol option.
			/// </summary>
			/// <remarks>
			/// An unknown, invalid or unsupported option or level was specified in a getsockopt or setsockopt call.
			/// </remarks>
			NoProtocolOption = WSAENOPROTOOPT,

			/// <summary>
			/// Protocol not supported.
			/// </summary>
			/// <remarks>
			/// The requested protocol has not been configured into the system, or no implementation for it exists. For example, a socket call requests a SOCK_DGRAM socket, but specifies a stream protocol.
			/// </remarks>
			ProtocolNotSupported = WSAEPROTONOSUPPORT,

			/// <summary>
			/// Socket type not supported.
			/// </summary>
			/// <remarks>
			/// The support for the specified socket type does not exist in this address family. For example, the optional type SOCK_RAW might be selected in a socket call, and the implementation does not support SOCK_RAW sockets at all.
			/// </remarks>
			SocketTypeNotSupported = WSAESOCKTNOSUPPORT,

			/// <summary>
			/// Operation not supported.
			/// </summary>
			/// <remarks>
			/// The attempted operation is not supported for the type of object referenced. Usually this occurs when a socket descriptor to a socket that cannot support this operation is trying to accept a connection on a datagram socket.
			/// </remarks>
			OperationNotSupported = WSAEOPNOTSUPP,

			/// <summary>
			/// Protocol family not supported.
			/// </summary>
			/// <remarks>
			/// The protocol family has not been configured into the system or no implementation for it exists. This message has a slightly different meaning from WSAEAFNOSUPPORT.However, it is interchangeable in most cases, and all Windows Sockets functions that return one of these messages also specify WSAEAFNOSUPPORT.
			/// </remarks>
			ProtocolFamilyNotSupported = WSAEPFNOSUPPORT,

			/// <summary>
			/// Address family not supported by protocol family.
			/// </summary>
			/// <remarks>
			/// An address incompatible with the requested protocol was used.All sockets are created with an associated address family (that is, AF_INET for Internet Protocols) and a generic protocol type(that is, SOCK_STREAM). This error is returned if an incorrect protocol is explicitly requested in the socket call, or if an address of the wrong family is used for a socket, for example, in sendto.
			/// </remarks>
			AddressFamilyNotSupported = WSAEAFNOSUPPORT
			/*	,

			/// <summary>
			/// Address already in use.
			/// </summary>
			/// <remarks>
			/// Typically, only one usage of each socket address(protocol/IP address/port) is permitted.This error occurs if an application attempts to bind a socket to an IP address/port that has already been used for an existing socket, or a socket that was not closed properly, or one that is still in the process of closing.For server applications that need to bind multiple sockets to the same port number, consider using setsockopt (SO_REUSEADDR). Client applications usually need not call bind at all—connect chooses an unused port automatically.When bind is called with a wildcard address (involving ADDR_ANY), a WSAEADDRINUSE error could be delayed until the specific address is committed.This could happen with a call to another function later, including connect, listen, WSAConnect, or WSAJoinLeaf.
			/// </remarks>
			WSAEADDRINUSE = 10048,

			/// <summary>
			/// Cannot assign requested address.
			/// </summary>
			/// <remarks>
			/// The requested address is not valid in its context. This normally results from an attempt to bind to an address that is not valid for the local computer.This can also result from connect, sendto, WSAConnect, WSAJoinLeaf, or WSASendTo when the remote address or port is not valid for a remote computer (for example, address or port 0).
			/// </remarks>
			WSAEADDRNOTAVAIL = 10049,

			/// <summary>
			/// Network is down.
			/// </summary>
			/// <remarks>
			/// A socket operation encountered a dead network.This could indicate a serious failure of the network system(that is, the protocol stack that the Windows Sockets DLL runs over), the network interface, or the local network itself.
			/// </remarks>
			WSAENETDOWN = 10050,

			/// <summary>
			/// Network is unreachable.
			/// </summary>
			/// <remarks>
			/// A socket operation was attempted to an unreachable network.This usually means the local software knows no route to reach the remote host.
			/// </remarks>
			WSAENETUNREACH = 10051,

			/// <summary>
			/// Network dropped connection on reset.
			/// </summary>
			/// <remarks>
			/// The connection has been broken due to keep-alive activity detecting a failure while the operation was in progress.It can also be returned by setsockopt if an attempt is made to set SO_KEEPALIVE on a connection that has already failed.
			/// </remarks>
			WSAENETRESET = 10052,

			/// <summary>
			/// Software caused connection abort.
			/// </summary>
			/// <remarks>
			/// An established connection was aborted by the software in your host computer, possibly due to a data transmission time-out or protocol error.
			/// </remarks>
			WSAECONNABORTED = 10053,

			/// <summary>
			/// Connection reset by peer.
			/// </summary>
			/// <remarks>
			/// An existing connection was forcibly closed by the remote host. This normally results if the peer application on the remote host is suddenly stopped, the host is rebooted, the host or remote network interface is disabled, or the remote host uses a hard close(see setsockopt for more information on the SO_LINGER option on the remote socket). This error may also result if a connection was broken due to keep-alive activity detecting a failure while one or more operations are in progress.Operations that were in progress fail with WSAENETRESET.Subsequent operations fail with WSAECONNRESET.
			/// </remarks>
			WSAECONNRESET = 10054,

			/// <summary>
			/// No buffer space available.
			/// </summary>
			/// <remarks>
			/// An operation on a socket could not be performed because the system lacked sufficient buffer space or because a queue was full.
			/// </remarks>
			WSAENOBUFS = 10055,

			/// <summary>
			/// Socket is already connected.
			/// </summary>
			/// <remarks>
			/// A connect request was made on an already-connected socket. Some implementations also return this error if sendto is called on a connected SOCK_DGRAM socket (for SOCK_STREAM sockets, the to parameter in sendto is ignored) although other implementations treat this as a legal occurrence.
			/// </remarks>
			WSAEISCONN = 10056,

			/// <summary>
			/// Socket is not connected.
			/// </summary>
			/// <remarks>
			/// A request to send or receive data was disallowed because the socket is not connected and (when sending on a datagram socket using sendto) no address was supplied.Any other type of operation might also return this error—for example, setsockopt setting SO_KEEPALIVE if the connection has been reset.
			/// </remarks>
			WSAENOTCONN = 10057,

			/// <summary>
			/// Cannot send after socket shutdown.
			/// </summary>
			/// <remarks>
			/// A request to send or receive data was disallowed because the socket had already been shut down in that direction with a previous shutdown call.By calling shutdown a partial close of a socket is requested, which is a signal that sending or receiving, or both have been discontinued.
			/// </remarks>
			WSAESHUTDOWN = 10058,

			/// <summary>
			/// Too many references.
			/// </summary>
			/// <remarks>
			/// Too many references to some kernel object.
			/// </remarks>
			WSAETOOMANYREFS = 10059,

			/// <summary>
			/// Connection timed out.
			/// </summary>
			/// <remarks>
			/// A connection attempt failed because the connected party did not properly respond after a period of time, or the established connection failed because the connected host has failed to respond.
			/// </remarks>
			WSAETIMEDOUT = 10060,

			/// <summary>
			/// Connection refused.
			/// </summary>
			/// <remarks>
			/// No connection could be made because the target computer actively refused it. This usually results from trying to connect to a service that is inactive on the foreign host—that is, one with no server application running.
			/// </remarks>
			WSAECONNREFUSED = 10061,

			/// <summary>
			/// Cannot translate name.
			/// </summary>
			/// <remarks>
			/// Cannot translate a name.
			/// </remarks>
			WSAELOOP = 10062,

			/// <summary>
			/// Name too long.
			/// </summary>
			/// <remarks>
			/// A name component or a name was too long.
			/// </remarks>
			WSAENAMETOOLONG = 10063,

			/// <summary>
			/// Host is down.
			/// </summary>
			/// <remarks>
			/// A socket operation failed because the destination host is down.A socket operation encountered a dead host.Networking activity on the local host has not been initiated. These conditions are more likely to be indicated by the error WSAETIMEDOUT.
			/// </remarks>
			WSAEHOSTDOWN = 10064,

			/// <summary>
			/// No route to host.
			/// </summary>
			/// <remarks>
			/// A socket operation was attempted to an unreachable host.See WSAENETUNREACH.
			/// </remarks>
			WSAEHOSTUNREACH = 10065,

			/// <summary>
			/// Directory not empty.
			/// </summary>
			/// <remarks>
			/// Cannot remove a directory that is not empty.
			/// </remarks>
			WSAENOTEMPTY = 10066,

			/// <summary>
			/// Too many processes.
			/// </summary>
			/// <remarks>
			/// A Windows Sockets implementation may have a limit on the number of applications that can use it simultaneously. WSAStartup may fail with this error if the limit has been reached.
			/// </remarks>
			WSAEPROCLIM = 10067,

			/// <summary>
			/// User quota exceeded.
			/// </summary>
			/// <remarks>
			/// Ran out of user quota.
			/// </remarks>
			WSAEUSERS = 10068,

			/// <summary>
			/// Disk quota exceeded.
			/// </summary>
			/// <remarks>
			/// Ran out of disk quota.
			/// </remarks>
			WSAEDQUOT = 10069,

			/// <summary>
			/// Stale file handle reference.
			/// </summary>
			/// <remarks>
			/// The file handle reference is no longer available.
			/// </remarks>
			WSAESTALE = 10070,

			/// <summary>
			/// Item is remote.
			/// </summary>
			/// <remarks>
			/// The item is not available locally.
			/// </remarks>
			WSAEREMOTE = 10071,

			/// <summary>
			/// Network subsystem is unavailable.
			/// </summary>
			/// <remarks>
			/// This error is returned by WSAStartup if the Windows Sockets implementation cannot function at this time because the underlying system it uses to provide network services is currently unavailable. Users should check:
			/// That the appropriate Windows Sockets DLL file is in the current path.
			/// That they are not trying to use more than one Windows Sockets implementation simultaneously. If there is more than one Winsock DLL on your system, be sure the first one in the path is appropriate for the network subsystem currently loaded.
			/// The Windows Sockets implementation documentation to be sure all necessary components are currently installed and configured correctly.
			/// </remarks>
			WSASYSNOTREADY = 10091,

			/// <summary>
			/// Winsock.dll version out of range.
			/// </summary>
			/// <remarks>
			/// The current Windows Sockets implementation does not support the Windows Sockets specification version requested by the application.Check that no old Windows Sockets DLL files are being accessed.
			/// </remarks>
			WSAVERNOTSUPPORTED = 10092,

			/// <summary>
			/// Successful WSAStartup not yet performed.
			/// </summary>
			/// <remarks>
			/// Either the application has not called WSAStartup or WSAStartup failed. The application may be accessing a socket that the current active task does not own (that is, trying to share a socket between tasks), or WSACleanup has been called too many times.
			/// </remarks>
			WSANOTINITIALISED = 10093,

			/// <summary>
			/// Graceful shutdown in progress.
			/// </summary>
			/// <remarks>
			/// Returned by WSARecv and WSARecvFrom to indicate that the remote party has initiated a graceful shutdown sequence.
			/// </remarks>
			WSAEDISCON = 10101,

			/// <summary>
			/// No more results.
			/// </summary>
			/// <remarks>
			/// No more results can be returned by the WSALookupServiceNext function.
			/// </remarks>
			WSAENOMORE = 10102,

			/// <summary>
			/// Call has been canceled.
			/// </summary>
			/// <remarks>
			/// A call to the WSALookupServiceEnd function was made while this call was still processing. The call has been canceled.
			/// </remarks>
			WSAECANCELLED = 10103,

			/// <summary>
			/// Procedure call table is invalid.
			/// </summary>
			/// <remarks>
			/// The service provider procedure call table is invalid.A service provider returned a bogus procedure table to Ws2_32.dll.This is usually caused by one or more of the function pointers being NULL.
			/// </remarks>
			WSAEINVALIDPROCTABLE = 10104,

			/// <summary>
			/// Service provider is invalid.
			/// </summary>
			/// <remarks>
			/// The requested service provider is invalid.This error is returned by the WSCGetProviderInfo and WSCGetProviderInfo32 functions if the protocol entry specified could not be found. This error is also returned if the service provider returned a version number other than 2.0.
			/// </remarks>
			WSAEINVALIDPROVIDER = 10105,

			/// <summary>
			/// Service provider failed to initialize.
			/// </summary>
			/// <remarks>
			/// The requested service provider could not be loaded or initialized. This error is returned if either a service provider's DLL could not be loaded (LoadLibrary failed) or the provider's WSPStartup or NSPStartup function failed.
			/// </remarks>
			WSAEPROVIDERFAILEDINIT = 10106,

			/// <summary>
			/// System call failure.
			/// </summary>
			/// <remarks>
			/// A system call that should never fail has failed.This is a generic error code, returned under various conditions.
			/// Returned when a system call that should never fail does fail.For example, if a call to WaitForMultipleEvents fails or one of the registry functions fails trying to manipulate the protocol/namespace catalogs.
			/// Returned when a provider does not return SUCCESS and does not provide an extended error code.Can indicate a service provider implementation error.
			/// </remarks>
			WSASYSCALLFAILURE = 10107,

			/// <summary>
			/// Service not found.
			/// </summary>
			/// <remarks>
			/// No such service is known.The service cannot be found in the specified name space.
			/// </remarks>
			WSASERVICE_NOT_FOUND = 10108,

			/// <summary>
			/// Class type not found.
			/// </summary>
			/// <remarks>
			/// The specified class was not found.
			/// </remarks>
			WSATYPE_NOT_FOUND = 10109,

			/// <summary>
			/// No more results.
			/// </summary>
			/// <remarks>
			/// No more results can be returned by the WSALookupServiceNext function.
			/// </remarks>
			WSA_E_NO_MORE = 10110,

			/// <summary>
			/// Call was canceled.
			/// </summary>
			/// <remarks>
			/// A call to the WSALookupServiceEnd function was made while this call was still processing. The call has been canceled.
			/// </remarks>
			WSA_E_CANCELLED = 10111,

			/// <summary>
			/// Database query was refused.
			/// </summary>
			/// <remarks>
			/// A database query failed because it was actively refused.
			/// </remarks>
			WSAEREFUSED = 10112,

			/// <summary>
			/// Host not found.
			/// </summary>
			/// <remarks>
			/// No such host is known.The name is not an official host name or alias, or it cannot be found in the database(s) being queried.This error may also be returned for protocol and service queries, and means that the specified name could not be found in the relevant database.
			/// </remarks>
			WSAHOST_NOT_FOUND = 11001,

			/// <summary>
			/// Nonauthoritative host not found.
			/// </summary>
			/// <remarks>
			/// This is usually a temporary error during host name resolution and means that the local server did not receive a response from an authoritative server.A retry at some time later may be successful.
			/// </remarks>
			WSATRY_AGAIN = 11002,

			/// <summary>
			/// This is a nonrecoverable error.
			/// </summary>
			/// <remarks>
			/// This indicates that some sort of nonrecoverable error occurred during a database lookup.This may be because the database files (for example, BSD-compatible HOSTS, SERVICES, or PROTOCOLS files) could not be found, or a DNS request was returned by the server with a severe error.
			/// </remarks>
			WSANO_RECOVERY = 11003,

			/// <summary>
			/// Valid name, no data record of requested type.
			/// </summary>
			/// <remarks>
			/// The requested name is valid and was found in the database, but it does not have the correct associated data being resolved for. The usual example for this is a host name-to-address translation attempt (using gethostbyname or WSAAsyncGetHostByName) which uses the DNS(Domain Name Server). An MX record is returned but no A record—indicating the host itself exists, but is not directly reachable.
			/// </remarks>
			WSANO_DATA = 11004,

			/// <summary>
			/// QoS receivers.
			/// </summary>
			/// <remarks>
			/// At least one QoS reserve has arrived.
			/// </remarks>
			WSA_QOS_RECEIVERS = 11005,

			/// <summary>
			/// QoS senders.
			/// </summary>
			/// <remarks>
			/// At least one QoS send path has arrived.
			/// </remarks>
			WSA_QOS_SENDERS = 11006,

			/// <summary>
			/// No QoS senders.
			/// </summary>
			/// <remarks>
			/// There are no QoS senders.
			/// </remarks>
			WSA_QOS_NO_SENDERS = 11007,

			/// <summary>
			/// QoS no receivers.
			/// </summary>
			/// <remarks>
			/// There are no QoS receivers.
			/// </remarks>
			WSA_QOS_NO_RECEIVERS = 11008,

			/// <summary>
			/// QoS request confirmed.
			/// </summary>
			/// <remarks>
			/// The QoS reserve request has been confirmed.
			/// </remarks>
			WSA_QOS_REQUEST_CONFIRMED = 11009,

			/// <summary>
			/// QoS admission error.
			/// </summary>
			/// <remarks>
			/// A QoS error occurred due to lack of resources.
			/// </remarks>
			WSA_QOS_ADMISSION_FAILURE = 11010,

			/// <summary>
			/// QoS policy failure.
			/// </summary>
			/// <remarks>
			/// The QoS request was rejected because the policy system couldn't allocate the requested resource within the existing policy.
			/// </remarks>
			WSA_QOS_POLICY_FAILURE = 11011,

			/// <summary>
			/// QoS bad style.
			/// </summary>
			/// <remarks>
			/// An unknown or conflicting QoS style was encountered.
			/// </remarks>
			WSA_QOS_BAD_STYLE = 11012,

			/// <summary>
			/// QoS bad object.
			/// </summary>
			/// <remarks>
			/// A problem was encountered with some part of the filterspec or the provider-specific buffer in general.
			/// </remarks>
			WSA_QOS_BAD_OBJECT = 11013,

			/// <summary>
			/// QoS traffic control error.
			/// </summary>
			/// <remarks>
			/// An error with the underlying traffic control (TC) API as the generic QoS request was converted for local enforcement by the TC API.This could be due to an out of memory error or to an internal QoS provider error.
			/// </remarks>
			WSA_QOS_TRAFFIC_CTRL_ERROR = 11014,

			/// <summary>
			/// QoS generic error.
			/// </summary>
			/// <remarks>
			/// A general QoS error.
			/// </remarks>
			WSA_QOS_GENERIC_ERROR = 11015,

			/// <summary>
			/// QoS service type error.
			/// </summary>
			/// <remarks>
			/// An invalid or unrecognized service type was found in the QoS flowspec.
			/// </remarks>
			WSA_QOS_ESERVICETYPE = 11016,

			/// <summary>
			/// QoS flowspec error.
			/// </summary>
			/// <remarks>
			/// An invalid or inconsistent flowspec was found in the QOS structure.
			/// </remarks>
			WSA_QOS_EFLOWSPEC = 11017,

			/// <summary>
			/// Invalid QoS provider buffer.
			/// </summary>
			/// <remarks>
			/// An invalid QoS provider-specific buffer.
			/// </remarks>
			WSA_QOS_EPROVSPECBUF = 11018,

			/// <summary>
			/// Invalid QoS filter style.
			/// </summary>
			/// <remarks>
			/// An invalid QoS filter style was used.
			/// </remarks>
			WSA_QOS_EFILTERSTYLE = 11019,

			/// <summary>
			/// Invalid QoS filter type.
			/// </summary>
			/// <remarks>
			/// An invalid QoS filter type was used.
			/// </remarks>
			WSA_QOS_EFILTERTYPE = 11020,

			/// <summary>
			/// Incorrect QoS filter count.
			/// </summary>
			/// <remarks>
			/// An incorrect number of QoS FILTERSPECs were specified in the FLOWDESCRIPTOR.
			/// </remarks>
			WSA_QOS_EFILTERCOUNT = 11021,

			/// <summary>
			/// Invalid QoS object length.
			/// </summary>
			/// <remarks>
			/// An object with an invalid ObjectLength field was specified in the QoS provider-specific buffer.
			/// </remarks>
			WSA_QOS_EOBJLENGTH = 11022,

			/// <summary>
			/// Incorrect QoS flow count.
			/// </summary>
			/// <remarks>
			/// An incorrect number of flow descriptors was specified in the QoS structure.
			/// </remarks>
			WSA_QOS_EFLOWCOUNT = 11023,

			/// <summary>
			/// Unrecognized QoS object.
			/// </summary>
			/// <remarks>
			/// An unrecognized object was found in the QoS provider-specific buffer.
			/// </remarks>
			WSA_QOS_EUNKOWNPSOBJ = 11024,

			/// <summary>
			/// Invalid QoS policy object.
			/// </summary>
			/// <remarks>
			/// An invalid policy object was found in the QoS provider-specific buffer.
			/// </remarks>
			WSA_QOS_EPOLICYOBJ = 11025,

			/// <summary>
			/// Invalid QoS flow descriptor.
			/// </summary>
			/// <remarks>
			/// An invalid QoS flow descriptor was found in the flow descriptor list.
			/// </remarks>
			WSA_QOS_EFLOWDESC = 11026,

			/// <summary>
			/// Invalid QoS provider-specific flowspec.
			/// </summary>
			/// <remarks>
			/// An invalid or inconsistent flowspec was found in the QoS provider-specific buffer.
			/// </remarks>
			WSA_QOS_EPSFLOWSPEC = 11027,

			/// <summary>
			/// Invalid QoS provider-specific filterspec.
			/// </summary>
			/// <remarks>
			/// An invalid FILTERSPEC was found in the QoS provider-specific buffer.
			/// </remarks>
			WSA_QOS_EPSFILTERSPEC = 11028,

			/// <summary>
			/// Invalid QoS shape discard mode object.
			/// </summary>
			/// <remarks>
			/// An invalid shape discard mode object was found in the QoS provider-specific buffer.
			/// </remarks>
			WSA_QOS_ESDMODEOBJ = 11029,

			/// <summary>
			/// Invalid QoS shaping rate object.
			/// </summary>
			/// <remarks>
			/// An invalid shaping rate object was found in the QoS provider-specific buffer.
			/// </remarks>
			WSA_QOS_ESHAPERATEOBJ = 11030,

			/// <summary>
			/// Reserved policy QoS element type.
			/// </summary>
			/// <remarks>
			/// A reserved policy element was found in the QoS provider-specific buffer.
			/// </remarks>
			WSA_QOS_RESERVED_PETYPE = 11031
		*/
		};
	}
}
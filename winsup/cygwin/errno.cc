/* errno.cc: errno-related functions

   Copyright 1996, 1997, 1998, 1999, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#define _REENT_ONLY
#include <stdio.h>
#include <errno.h>
#include "cygerrno.h"
#include "thread.h"

/* Table to map Windows error codes to Errno values.  */
/* FIXME: Doing things this way is a little slow.  It's trivial to change
   this into a big case statement if necessary.  Left as is for now. */

#define X(w, e) {ERROR_##w, #w, e}

static const struct
  {
    DWORD w;		 /* windows version of error */
    const char *s;	 /* text of windows version */
    int e;		 /* errno version of error */
  }
errmap[] =
{
  /* FIXME: Some of these choices are arbitrary! */
  X (INVALID_FUNCTION,		EBADRQC),
  X (FILE_NOT_FOUND,		ENOENT),
  X (PATH_NOT_FOUND,		ENOENT),
  X (TOO_MANY_OPEN_FILES,	EMFILE),
  X (ACCESS_DENIED,		EACCES),
  X (INVALID_HANDLE,		EBADF),
  X (NOT_ENOUGH_MEMORY,		ENOMEM),
  X (INVALID_DATA,		EINVAL),
  X (OUTOFMEMORY,		ENOMEM),
  X (INVALID_DRIVE,		ENODEV),
  X (NOT_SAME_DEVICE,		EXDEV),
  X (NO_MORE_FILES,		ENMFILE),
  X (WRITE_PROTECT,		EROFS),
  X (BAD_UNIT,			ENODEV),
  X (SHARING_VIOLATION,		EACCES),
  X (LOCK_VIOLATION,		EACCES),
  X (SHARING_BUFFER_EXCEEDED,	ENOLCK),
  X (HANDLE_EOF,		ENODATA),
  X (HANDLE_DISK_FULL,		ENOSPC),
  X (NOT_SUPPORTED,		ENOSYS),
  X (REM_NOT_LIST,		ENONET),
  X (DUP_NAME,			ENOTUNIQ),
  X (BAD_NETPATH,		ENOSHARE),
  X (BAD_NET_NAME,		ENOSHARE),
  X (FILE_EXISTS,		EEXIST),
  X (CANNOT_MAKE,		EPERM),
  X (INVALID_PARAMETER,		EINVAL),
  X (NO_PROC_SLOTS,		EAGAIN),
  X (BROKEN_PIPE,		EPIPE),
  X (OPEN_FAILED,		EIO),
  X (NO_MORE_SEARCH_HANDLES,	ENFILE),
  X (CALL_NOT_IMPLEMENTED,	ENOSYS),
  X (INVALID_NAME,		ENOENT),
  X (WAIT_NO_CHILDREN,		ECHILD),
  X (CHILD_NOT_COMPLETE,	EBUSY),
  X (DIR_NOT_EMPTY,		ENOTEMPTY),
  X (SIGNAL_REFUSED,		EIO),
  X (BAD_PATHNAME,		ENOENT),
  X (SIGNAL_PENDING,		EBUSY),
  X (MAX_THRDS_REACHED,		EAGAIN),
  X (BUSY,			EBUSY),
  X (ALREADY_EXISTS,		EEXIST),
  X (NO_SIGNAL_SENT,		EIO),
  X (FILENAME_EXCED_RANGE,	EINVAL),
  X (META_EXPANSION_TOO_LONG,	EINVAL),
  X (INVALID_SIGNAL_NUMBER,	EINVAL),
  X (THREAD_1_INACTIVE,		EINVAL),
  X (BAD_PIPE,			EINVAL),
  X (PIPE_BUSY,			EBUSY),
  X (NO_DATA,			EPIPE),
  X (PIPE_NOT_CONNECTED,	ECOMM),
  X (MORE_DATA,			EAGAIN),
  X (DIRECTORY,			ENOTDIR),
  X (PIPE_CONNECTED,		EBUSY),
  X (PIPE_LISTENING,		ECOMM),
  X (NO_TOKEN,			EINVAL),
  X (PROCESS_ABORTED,		EFAULT),
  X (BAD_DEVICE,		ENODEV),
  X (BAD_USERNAME,		EINVAL),
  X (NOT_CONNECTED,		ENOLINK),
  X (OPEN_FILES,		EAGAIN),
  X (ACTIVE_CONNECTIONS,	EAGAIN),
  X (DEVICE_IN_USE,		EAGAIN),
  X (INVALID_AT_INTERRUPT_TIME,	EINTR),
  X (IO_DEVICE,			EIO),
  X (NOT_OWNER,			EPERM),
  X (END_OF_MEDIA,		ENOSPC),
  X (EOM_OVERFLOW,		ENOSPC),
  X (BEGINNING_OF_MEDIA,	ESPIPE),
  X (SETMARK_DETECTED,		ESPIPE),
  X (NO_DATA_DETECTED,		ENOSPC),
  X (POSSIBLE_DEADLOCK,		EDEADLOCK),
  X (CRC,			EIO),
  X (NEGATIVE_SEEK,		EINVAL),
  X (NOT_READY,			ENOMEDIUM),
  X (DISK_FULL,			ENOSPC),
  X (NOACCESS,			EFAULT),
  { 0, NULL, 0}
};

int __stdcall
geterrno_from_win_error (DWORD code, int deferrno)
{
  for (int i = 0; errmap[i].w != 0; ++i)
    if (code == errmap[i].w)
      {
	syscall_printf ("windows error %u == errno %d", code, errmap[i].e);
	return errmap[i].e;
      }

  syscall_printf ("unknown windows error %u, setting errno to %d", code,
		  deferrno);
  return deferrno;	/* FIXME: what's so special about EACCESS? */
}

/* seterrno_from_win_error: Given a Windows error code, set errno
   as appropriate. */
void __stdcall
seterrno_from_win_error (const char *file, int line, DWORD code)
{
  syscall_printf ("%s:%d errno %d", file, line, code);
  set_errno (geterrno_from_win_error (code, EACCES));
  return;
}

/* seterrno: Set `errno' based on GetLastError (). */
void __stdcall
seterrno (const char *file, int line)
{
  seterrno_from_win_error (file, line, GetLastError ());
}

extern char *_user_strerror _PARAMS ((int));

extern const char __declspec(dllexport) * const _sys_errlist[]=
{
/*      NOERROR 0       */ "No error",
/*	EPERM 1		*/ "Not super-user",
/*	ENOENT 2	*/ "No such file or directory",
/*	ESRCH 3		*/ "No such process",
/*	EINTR 4		*/ "Interrupted system call",
/*	EIO 5		*/ "I/O error",
/*	ENXIO 6		*/ "No such device or address",
/*	E2BIG 7		*/ "Arg list too long",
/*	ENOEXEC 8	*/ "Exec format error",
/*	EBADF 9		*/ "Bad file number",
/*	ECHILD 10	*/ "No children",
/*	EAGAIN 11	*/ "Resource temporarily unavailable",
/*	ENOMEM 12	*/ "Not enough core",
/*	EACCES 13	*/ "Permission denied",
/*	EFAULT 14	*/ "Bad address",
/*	ENOTBLK 15	*/ "Block device required",
/*	EBUSY 16	*/ "Mount device busy",
/*	EEXIST 17	*/ "File exists",
/*	EXDEV 18	*/ "Cross-device link",
/*	ENODEV 19	*/ "No such device",
/*	ENOTDIR 20	*/ "Not a directory",
/*	EISDIR 21	*/ "Is a directory",
/*	EINVAL 22	*/ "Invalid argument",
/*	ENFILE 23	*/ "Too many open files in system",
/*	EMFILE 24	*/ "Too many open files",
/*	ENOTTY 25	*/ "Not a typewriter",
/*	ETXTBSY 26	*/ "Text file busy",
/*	EFBIG 27	*/ "File too large",
/*	ENOSPC 28	*/ "No space left on device",
/*	ESPIPE 29	*/ "Illegal seek",
/*	EROFS 30	*/ "Read only file system",
/*	EMLINK 31	*/ "Too many links",
/*	EPIPE 32	*/ "Broken pipe",
/*	EDOM 33		*/ "Math arg out of domain of func",
/*	ERANGE 34	*/ "Math result not representable",
/*	ENOMSG 35	*/ "No message of desired type",
/*	EIDRM 36	*/ "Identifier removed",
/*	ECHRNG 37	*/ "Channel number out of range",
/*	EL2NSYNC 38	*/ "Level 2 not synchronized",
/*	EL3HLT 39	*/ "Level 3 halted",
/*	EL3RST 40	*/ "Level 3 reset",
/*	ELNRNG 41	*/ "Link number out of range",
/*	EUNATCH 42	*/ "Protocol driver not attached",
/*	ENOCSI 43	*/ "No CSI structure available",
/*	EL2HLT 44	*/ "Level 2 halted",
/*	EDEADLK 45	*/ "Deadlock condition",
/*	ENOLCK 46	*/ "No record locks available",
			   "47",
			   "48",
			   "49",
/* EBADE 50	*/ "Invalid exchange",
/* EBADR 51	*/ "Invalid request descriptor",
/* EXFULL 52	*/ "Exchange full",
/* ENOANO 53	*/ "No anode",
/* EBADRQC 54	*/ "Invalid request code",
/* EBADSLT 55	*/ "Invalid slot",
/* EDEADLOCK 56	*/ "File locking deadlock error",
/* EBFONT 57	*/ "Bad font file fmt",
			   "58",
			   "59",
/* ENOSTR 60	*/ "Device not a stream",
/* ENODATA 61	*/ "No data (for no delay io)",
/* ETIME 62	*/ "Timer expired",
/* ENOSR 63	*/ "Out of streams resources",
/* ENONET 64	*/ "Machine is not on the network",
/* ENOPKG 65	*/ "Package not installed",
/* EREMOTE 66	*/ "The object is remote",
/* ENOLINK 67	*/ "The link has been severed",
/* EADV 68		*/ "Advertise error",
/* ESRMNT 69	*/ "Srmount error",
/*	ECOMM 70	*/ "Communication error on send",
/* EPROTO 71	*/ "Protocol error",
			   "72",
			   "73",
/*	EMULTIHOP 74	*/ "Multihop attempted",
/*	ELBIN 75	*/ "Inode is remote (not really error)",
/*	EDOTDOT 76	*/ "Cross mount point (not really error)",
/* EBADMSG 77	*/ "Trying to read unreadable message",
			   "78",
			   "79",
/* ENOTUNIQ 80	*/ "Given log. name not unique",
/* EBADFD 81	*/ "f.d. invalid for this operation",
/* EREMCHG 82	*/ "Remote address changed",
/* ELIBACC 83	*/ "Can't access a needed shared lib",
/* ELIBBAD 84	*/ "Accessing a corrupted shared lib",
/* ELIBSCN 85	*/ ".lib section in a.out corrupted",
/* ELIBMAX 86	*/ "Attempting to link in too many libs",
/* ELIBEXEC 87	*/ "Attempting to exec a shared library",
/* ENOSYS 88	*/ "Function not implemented",
/* ENMFILE 89      */ "No more files",
/* ENOTEMPTY 90	*/ "Directory not empty",
/* ENAMETOOLONG 91	*/ "File or path name too long",
/* ELOOP 92     */ "Too many symbolic links",
		   "93",
		   "94",
/* EOPNOTSUPP 95 */ "Operation not supported on transport endpoint",
/* EPFNOSUPPORT 96 */ "Protocol family not supported",
		   "97",
		   "98",
		   "99",
		   "100",
		   "101",
		   "102",
		   "103",
/* ECONNRESET 104 */ "Connection reset by peer",
/* ENOBUFS 105 */ "No buffer space available",
/* EAFNOSUPPORT 106 */ "Address family not supported by protocol",
/* EPROTOTYPE 107 */ "Protocol wrong type for transport endpoint",
/* ENOTSOCK 108 */  "Socket operation on non-socket"
/* ENOPROTOOPT 109 */ "Protocol not available",
/* ESHUTDOWN 110 */ "Cannot send after transport endpoint shutdown",
/* ECONNREFUSED 111 */ "Connection refused",
/* EADDRINUSE 112 */ "Address already in use"
/* ECONNABORTED 113 */ "Connection aborted",
/* ENETUNREACH 114 */ "Network is unreachable",
/* ENETDOWN 115 */ "Network is down",
/* ETIMEDOUT 116 */ "Connection timed out",
/* EHOSTDOWN 117 */ "Host is down",
/* EHOSTUNREACH 118 */ "No route to host",
/* EINPROGRESS 119 */ "Operation now in progress",
/* EALREADY 120 */ "Operation already in progress",
/* EDESTADDRREQ 121 */ "Destination address required",
/* EMSGSIZE 122 */ "Message too long",
/* EPROTONOSUPPORT 123 */ "Protocol not supported",
/* ESOCKTNOSUPPORT 124 */ "Socket type not supported",
/* EADDRNOTAVAIL 125 */ "Cannot assign requested address",
/* ENETRESET 126 */ "Network dropped connection because of reset",
/* EISCONN 127 */ "Transport endpoint is already connected",
/* ENOTCONN 128 */ "Transport endpoint is not connected",
/* ETOOMANYREFS 129 */ "Too many references: cannot splice",
/* EPROCLIM 130 */ "Process limit exceeded",
/* EUSERS 131 */ "Too many users",
/* EDQUOT 132 */ "Quota exceeded",
/* ESTALE 133 */ "Stale NFS file handle",
/* ENOTSUP 134 */   "134",
/* ENOMEDIUM 135 */ "no medium",
/* ENOSHARE 136 */   "No such host or network path"
};

int __declspec(dllexport) _sys_nerr =
  sizeof (_sys_errlist) / sizeof (_sys_errlist[0]);

/* FIXME: Why is strerror() a long switch and not just:
	return sys_errlist[errnum];
	(or moral equivalent).
	Some entries in sys_errlist[] don't match the corresponding
	entries in strerror().  This seems odd.
*/

/* CYGWIN internal */
/* strerror: convert from errno values to error strings */
extern "C" char *
strerror (int errnum)
{
  const char *error;
  switch (errnum)
    {
    case EPERM:
      error = "Not owner";
      break;
    case ENOENT:
      error = "No such file or directory";
      break;
    case ESRCH:
      error = "No such process";
      break;
    case EINTR:
      error = "Interrupted system call";
      break;
    case EIO:
      error = "I/O error";
      break;
    case ENXIO:
      error = "No such device or address";
      break;
    case E2BIG:
      error = "Arg list too long";
      break;
    case ENOEXEC:
      error = "Exec format error";
      break;
    case EBADF:
      error = "Bad file number";
      break;
    case ECHILD:
      error = "No children";
      break;
    case EAGAIN:
      error = "No more processes";
      break;
    case ENOMEM:
      error = "Not enough memory";
      break;
    case EACCES:
      error = "Permission denied";
      break;
    case EFAULT:
      error = "Bad address";
      break;
    case ENOTBLK:
      error = "Block device required";
      break;
    case EBUSY:
      error = "Device or resource busy";
      break;
    case EEXIST:
      error = "File exists";
      break;
    case EXDEV:
      error = "Cross-device link";
      break;
    case ENODEV:
      error = "No such device";
      break;
    case ENOTDIR:
      error = "Not a directory";
      break;
    case EISDIR:
      error = "Is a directory";
      break;
    case EINVAL:
      error = "Invalid argument";
      break;
    case ENFILE:
      error = "Too many open files in system";
      break;
    case EMFILE:
      error = "Too many open files";
      break;
    case ENOTTY:
      error = "Not a character device";
      break;
    case ETXTBSY:
      error = "Text file busy";
      break;
    case EFBIG:
      error = "File too large";
      break;
    case ENOSPC:
      error = "No space left on device";
      break;
    case ESPIPE:
      error = "Illegal seek";
      break;
    case EROFS:
      error = "Read-only file system";
      break;
    case EMLINK:
      error = "Too many links";
      break;
    case EPIPE:
      error = "Broken pipe";
      break;
    case EDOM:
      error = "Math arg out of domain of func";
      break;
    case ERANGE:
      error = "Math result out of range";
      break;
    case ENOMSG:
      error = "No message of desired type";
      break;
    case EIDRM:
      error = "Identifier removed";
      break;
    case ECHRNG:
      error = "Channel number out of range";
      break;
    case EL2NSYNC:
      error = "Level 2 not synchronized";
      break;
    case EL3HLT:
      error = "Level 3 halted";
      break;
    case EL3RST:
      error = "Level 3 reset";
      break;
    case ELNRNG:
      error = "Link number out of range";
      break;
    case EUNATCH:
      error = "Protocol driver not attached";
      break;
    case ENOCSI:
      error = "No CSI structure available";
      break;
    case EL2HLT:
      error = "Level 2 halted";
      break;
    case EDEADLK:
      error = "Deadlock condition";
      break;
    case ENOLCK:
      error = "No lock";
      break;
    case EBADE:
      error = "Invalid exchange";
      break;
    case EBADR:
      error = "Invalid request descriptor";
      break;
    case EXFULL:
      error = "Exchange full";
      break;
    case ENOANO:
      error = "No anode";
      break;
    case EBADRQC:
      error = "Invalid request code";
      break;
    case EBADSLT:
      error = "Invalid slot";
      break;
    case EDEADLOCK:
      error = "File locking deadlock error";
      break;
    case EBFONT:
      error = "Bad font file fmt";
      break;
    case ENOSTR:
      error = "Not a stream";
      break;
    case ENODATA:
      error = "No data (for no delay io)";
      break;
    case ETIME:
      error = "Stream ioctl timeout";
      break;
    case ENOSR:
      error = "No stream resources";
      break;
    case ENONET:
      error = "Machine is not on the network";
      break;
    case ENOPKG:
      error = "No package";
      break;
    case EREMOTE:
      error = "Resource is remote";
      break;
    case ENOLINK:
      error = "Virtual circuit is gone";
      break;
    case EADV:
      error = "Advertise error";
      break;
    case ESRMNT:
      error = "Srmount error";
      break;
    case ECOMM:
      error = "Communication error";
      break;
    case EPROTO:
      error = "Protocol error";
      break;
    case EMULTIHOP:
      error = "Multihop attempted";
      break;
    case ELBIN:
      error = "Inode is remote (not really error)";
      break;
    case EDOTDOT:
      error = "Cross mount point (not really error)";
      break;
    case EBADMSG:
      error = "Bad message";
      break;
    case ENOTUNIQ:
      error = "Given log. name not unique";
      break;
    case EBADFD:
      error = "f.d. invalid for this operation";
      break;
    case EREMCHG:
      error = "Remote address changed";
      break;
    case ELIBACC:
      error = "Cannot access a needed shared library";
      break;
    case ELIBBAD:
      error = "Accessing a corrupted shared library";
      break;
    case ELIBSCN:
      error = ".lib section in a.out corrupted";
      break;
    case ELIBMAX:
      error = "Attempting to link in more shared libraries than system limit";
      break;
    case ELIBEXEC:
      error = "Cannot exec a shared library directly";
      break;
    case ENOSYS:
      error = "Function not implemented";
      break;
    case ENMFILE:
      error = "No more files";
      break;
    case ENOTEMPTY:
      error = "Directory not empty";
      break;
    case ENAMETOOLONG:
      error = "File or path name too long";
      break;
    case ELOOP:
      error = "Too many symbolic links";
      break;
    case EOPNOTSUPP:
      error = "Operation not supported on transport endpoint";
      break;
    case EPFNOSUPPORT:
      error = "Protocol family not supported";
      break;
    case ECONNRESET:
      error = "Connection reset by peer";
      break;
    case ENOBUFS:
      error = "No buffer space available; the socket cannot be connected";
      break;
    case EAFNOSUPPORT:
      error = "Addresses in the specified family cannot be used with this socket";
      break;
    case EPROTOTYPE:
      error = "errno EPROTOTYPE triggered";
      break;
    case ENOTSOCK:
      error = "The descriptor is a file, not a socket";
      break;
    case ENOPROTOOPT:
      error = "This option is unsupported";
      break;
    case ESHUTDOWN:
      error = "errno ESHUTDOWN triggered";
      break;
    case ECONNREFUSED:
      error = "Connection refused";
      break;
    case EADDRINUSE:
      error = "Address already in use";
      break;
    case ECONNABORTED:
      error = "The connection was aborted";
      break;
    case ENETUNREACH:
      error ="The network can't be reached from this host at this time";
      break;
    case ENETDOWN:
      error = "Network failed.";
      break;
    case ETIMEDOUT:
      error = "Attempt to connect timed out without establishing a connection";
      break;
    case EHOSTDOWN:
      error = "errno EHOSTDOWN triggered";
      break;
    case EHOSTUNREACH:
      error = "errno EHOSTUNREACH triggered";
      break;
    case EINPROGRESS:
      error = "errno EINPROGRESS triggered";
      break;
    case EALREADY:
      error = "errno EALREADY triggered";
      break;
    case EDESTADDRREQ:
      error = "errno EDESTADDRREQ triggered";
      break;
    case EMSGSIZE:
      error = "errno EMSGSIZE triggered";
      break;

    case EPROTONOSUPPORT:
      error = "errno EPROTONOSUPPORT triggered";
      break;
    case ESOCKTNOSUPPORT:
      error = "errno ESOCKTNOSUPPORT triggered";
      break;
    case EADDRNOTAVAIL:
      error = "errno EADDRNOTAVAIL triggered";
      break;
    case ENETRESET:
      error = "errno ENETRESET triggered";
      break;
    case EISCONN:
      error = "The socket is already connected";
      break;
    case ENOTCONN:
      error = "The socket is not connected";
      break;
    case ETOOMANYREFS:
      error = "errno ETOOMANYREFS triggered";
      break;
    case EPROCLIM:
      error = "errno EPROCLIM triggered";
      break;
    case EUSERS:
      error = "errno EUSERS triggered";
      break;
    case EDQUOT:
      error = "errno EDQUOT triggered";
      break;
    case ESTALE:
      error = "errno ESTALE triggered";
      break;
    case ENOTSUP:
      error = "errno ENOTSUP triggered";
      break;
    case ENOMEDIUM:
      error = "no medium";
      break;
    case ENOSHARE:
      error = "No such host or network path";
      break;
    default:
#ifdef _MT_SAFE
      char *buf= _reent_winsup()->_strerror_buf;
#else
      static NO_COPY char buf[20];
#endif
      __small_sprintf (buf, "error %d", errnum);
      error = buf;
      break;
    }

  /* FIXME: strerror should really be const in the appropriate newlib
     include files. */
  return (char *) error;
}

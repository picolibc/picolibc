/* errno.cc: errno-related functions

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005,
   2006, 2008 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define _sys_nerr FOO_sys_nerr
#define sys_nerr FOOsys_nerr
#define _sys_errlist FOO_sys_errlist
#include "winsup.h"
#include "cygtls.h"
#undef _sys_nerr
#undef sys_nerr
#undef _sys_errlist

/* Table to map Windows error codes to Errno values.  */
/* FIXME: Doing things this way is a little slow.  It's trivial to change
   this into a big case statement if necessary.  Left as is for now. */

#define X(w, e) {ERROR_##w, #w, e}

static NO_COPY struct
{
  DWORD w;		 /* windows version of error */
  const char *s;	 /* text of windows version */
  int e;		 /* errno version of error */
} errmap[] =
{
  /* FIXME: Some of these choices are arbitrary! */
  X (ACCESS_DENIED,		EACCES),
  X (ACTIVE_CONNECTIONS,	EAGAIN),
  X (ALREADY_EXISTS,		EEXIST),
  X (BAD_DEVICE,		ENODEV),
  X (BAD_NETPATH,		ENOSHARE),
  X (BAD_NET_NAME,		ENOSHARE),
  X (BAD_PATHNAME,		ENOENT),
  X (BAD_PIPE,			EINVAL),
  X (BAD_UNIT,			ENODEV),
  X (BAD_USERNAME,		EINVAL),
  X (BEGINNING_OF_MEDIA,	EIO),
  X (BROKEN_PIPE,		EPIPE),
  X (BUSY,			EBUSY),
  X (BUS_RESET,			EIO),
  X (CALL_NOT_IMPLEMENTED,	ENOSYS),
  X (CANNOT_MAKE,		EPERM),
  X (CHILD_NOT_COMPLETE,	EBUSY),
  X (COMMITMENT_LIMIT,		EAGAIN),
  X (CRC,			EIO),
  X (DEVICE_DOOR_OPEN,		EIO),
  X (DEVICE_IN_USE,		EAGAIN),
  X (DEVICE_REQUIRES_CLEANING,	EIO),
  X (DIRECTORY,			ENOTDIR),
  X (DIR_NOT_EMPTY,		ENOTEMPTY),
  X (DISK_CORRUPT,		EIO),
  X (DISK_FULL,			ENOSPC),
  X (DUP_NAME,			ENOTUNIQ),
  X (EAS_DIDNT_FIT,		ENOSPC),
  X (EAS_NOT_SUPPORTED,		ENOTSUP),
  X (EA_LIST_INCONSISTENT,	EINVAL),
  X (EA_TABLE_FULL,		ENOSPC),
  X (END_OF_MEDIA,		ENOSPC),
  X (EOM_OVERFLOW,		EIO),
  X (FILEMARK_DETECTED,		EIO),
  X (FILENAME_EXCED_RANGE,	ENAMETOOLONG),
  X (FILE_CORRUPT,		EEXIST),
  X (FILE_EXISTS,		EEXIST),
  X (FILE_INVALID,		ENXIO),
  X (FILE_NOT_FOUND,		ENOENT),
  X (HANDLE_DISK_FULL,		ENOSPC),
  X (HANDLE_EOF,		ENODATA),
  X (INVALID_ADDRESS,		EINVAL),
  X (INVALID_AT_INTERRUPT_TIME,	EINTR),
  X (INVALID_BLOCK_LENGTH,	EIO),
  X (INVALID_DATA,		EINVAL),
  X (INVALID_DRIVE,		ENODEV),
  X (INVALID_EA_NAME,		EINVAL),
  X (INVALID_FUNCTION,		EBADRQC),
  X (INVALID_HANDLE,		EBADF),
  X (INVALID_NAME,		ENOENT),
  X (INVALID_PARAMETER,		EINVAL),
  X (INVALID_SIGNAL_NUMBER,	EINVAL),
  X (IO_DEVICE,			EIO),
  X (IO_PENDING,		EAGAIN),
  X (LOCK_VIOLATION,		EACCES),
  X (MAX_THRDS_REACHED,		EAGAIN),
  X (META_EXPANSION_TOO_LONG,	EINVAL),
  X (MOD_NOT_FOUND,		ENOENT),
  X (MORE_DATA,			EMSGSIZE),
  X (NEGATIVE_SEEK,		EINVAL),
  X (NETNAME_DELETED,		ENOSHARE),
  X (NOACCESS,			EFAULT),
  X (NONPAGED_SYSTEM_RESOURCES,	EAGAIN),
  X (NOT_CONNECTED,		ENOLINK),
  X (NOT_ENOUGH_MEMORY,		ENOMEM),
  X (NOT_OWNER,			EPERM),
  X (NOT_READY,			ENOMEDIUM),
  X (NOT_SAME_DEVICE,		EXDEV),
  X (NOT_SUPPORTED,		ENOSYS),
  X (NO_DATA,			EPIPE),
  X (NO_DATA_DETECTED,		EIO),
  X (NO_MEDIA_IN_DRIVE,		ENOMEDIUM),
  X (NO_MORE_FILES,		ENMFILE),
  X (NO_MORE_ITEMS,		ENMFILE),
  X (NO_MORE_SEARCH_HANDLES,	ENFILE),
  X (NO_PROC_SLOTS,		EAGAIN),
  X (NO_SIGNAL_SENT,		EIO),
  X (NO_SYSTEM_RESOURCES,	EAGAIN),
  X (NO_TOKEN,			EINVAL),
  X (OPEN_FAILED,		EIO),
  X (OPEN_FILES,		EAGAIN),
  X (OUTOFMEMORY,		ENOMEM),
  X (PAGED_SYSTEM_RESOURCES,	EAGAIN),
  X (PAGEFILE_QUOTA,		EAGAIN),
  X (PATH_NOT_FOUND,		ENOENT),
  X (PIPE_BUSY,			EBUSY),
  X (PIPE_CONNECTED,		EBUSY),
  X (PIPE_LISTENING,		ECOMM),
  X (PIPE_NOT_CONNECTED,	ECOMM),
  X (POSSIBLE_DEADLOCK,		EDEADLOCK),
  X (PROCESS_ABORTED,		EFAULT),
  X (PROC_NOT_FOUND,		ESRCH),
  X (REM_NOT_LIST,		ENONET),
  X (SETMARK_DETECTED,		EIO),
  X (SHARING_BUFFER_EXCEEDED,	ENOLCK),
  X (SHARING_VIOLATION,		EBUSY),
  X (SIGNAL_PENDING,		EBUSY),
  X (SIGNAL_REFUSED,		EIO),
  X (THREAD_1_INACTIVE,		EINVAL),
  X (TOO_MANY_LINKS,		EMLINK),
  X (TOO_MANY_OPEN_FILES,	EMFILE),
  X (WAIT_NO_CHILDREN,		ECHILD),
  X (WORKING_SET_QUOTA,		EAGAIN),
  X (WRITE_PROTECT,		EROFS),
  X (SEEK,			EINVAL),
  X (SECTOR_NOT_FOUND,		EINVAL),
  { 0, NULL, 0}
};

extern "C" {
const char __declspec(dllexport) * _sys_errlist[] NO_COPY_INIT =
{
/* NOERROR 0 */		  "No error",
/* EPERM 1 */		  "Operation not permitted",
/* ENOENT 2 */		  "No such file or directory",
/* ESRCH 3 */		  "No such process",
/* EINTR 4 */		  "Interrupted system call",
/* EIO 5 */		  "Input/Output error",
/* ENXIO 6 */		  "No such device or address",
/* E2BIG 7 */		  "Argument list too long",
/* ENOEXEC 8 */		  "Exec format error",
/* EBADF 9 */		  "Bad file descriptor",
/* ECHILD 10 */		  "No child processes",
/* EAGAIN 11 */		  "Resource temporarily unavailable",
/* ENOMEM 12 */		  "Cannot allocate memory",
/* EACCES 13 */		  "Permission denied",
/* EFAULT 14 */		  "Bad address",
/* ENOTBLK 15 */	  "Block device required",
/* EBUSY 16 */		  "Device or resource busy",
/* EEXIST 17 */		  "File exists",
/* EXDEV 18 */		  "Invalid cross-device link",
/* ENODEV 19 */		  "No such device",
/* ENOTDIR 20 */	  "Not a directory",
/* EISDIR 21 */		  "Is a directory",
/* EINVAL 22 */		  "Invalid argument",
/* ENFILE 23 */		  "Too many open files in system",
/* EMFILE 24 */		  "Too many open files",
/* ENOTTY 25 */		  "Inappropriate ioctl for device",
/* ETXTBSY 26 */	  "Text file busy",
/* EFBIG 27 */		  "File too large",
/* ENOSPC 28 */		  "No space left on device",
/* ESPIPE 29 */		  "Illegal seek",
/* EROFS 30 */		  "Read-only file system",
/* EMLINK 31 */		  "Too many links",
/* EPIPE 32 */		  "Broken pipe",
/* EDOM 33 */		  "Numerical argument out of domain",
/* ERANGE 34 */		  "Numerical result out of range",
/* ENOMSG 35 */		  "No message of desired type",
/* EIDRM 36 */		  "Identifier removed",
/* ECHRNG 37 */		  "Channel number out of range",
/* EL2NSYNC 38 */	  "Level 2 not synchronized",
/* EL3HLT 39 */		  "Level 3 halted",
/* EL3RST 40 */		  "Level 3 reset",
/* ELNRNG 41 */		  "Link number out of range",
/* EUNATCH 42 */	  "Protocol driver not attached",
/* ENOCSI 43 */		  "No CSI structure available",
/* EL2HLT 44 */		  "Level 2 halted",
/* EDEADLK 45 */	  "Resource deadlock avoided",
/* ENOLCK 46 */		  "No locks available",
		  	  "error 47",
		  	  "error 48",
		  	  "error 49",
/* EBADE 50 */		  "Invalid exchange",
/* EBADR 51 */		  "Invalid request descriptor",
/* EXFULL 52 */		  "Exchange full",
/* ENOANO 53 */		  "No anode",
/* EBADRQC 54 */	  "Invalid request code",
/* EBADSLT 55 */	  "Invalid slot",
/* EDEADLOCK 56 */	  "File locking deadlock error",
/* EBFONT 57 */		  "Bad font file format",
		  	  "error 58",
		  	  "error 59",
/* ENOSTR 60 */		  "Device not a stream",
/* ENODATA 61 */	  "No data available",
/* ETIME 62 */		  "Timer expired",
/* ENOSR 63 */		  "Out of streams resources",
/* ENONET 64 */		  "Machine is not on the network",
/* ENOPKG 65 */		  "Package not installed",
/* EREMOTE 66 */	  "Object is remote",
/* ENOLINK 67 */	  "Link has been severed",
/* EADV 68 */		  "Advertise error",
/* ESRMNT 69 */		  "Srmount error",
/* ECOMM 70 */		  "Communication error on send",
/* EPROTO 71 */		  "Protocol error",
		  	  "error 72",
		  	  "error 73",
/* EMULTIHOP 74 */	  "Multihop attempted",
/* ELBIN 75 */		  "Inode is remote (not really error)",
/* EDOTDOT 76 */	  "RFS specific error",
/* EBADMSG 77 */	  "Bad message",
		  	  "error 78",
/* EFTYPE 79 */		  "Inappropriate file type or format",
/* ENOTUNIQ 80 */	  "Name not unique on network",
/* EBADFD 81 */		  "File descriptor in bad state",
/* EREMCHG 82 */	  "Remote address changed",
/* ELIBACC 83 */	  "Can not access a needed shared library",
/* ELIBBAD 84 */	  "Accessing a corrupted shared library",
/* ELIBSCN 85 */	  ".lib section in a.out corrupted",
/* ELIBMAX 86 */	  "Attempting to link in too many shared libraries",
/* ELIBEXEC 87 */	  "Cannot exec a shared library directly",
/* ENOSYS 88 */		  "Function not implemented",
/* ENMFILE 89 */	  "No more files",
/* ENOTEMPTY 90	*/	  "Directory not empty",
/* ENAMETOOLONG 91 */	  "File name too long",
/* ELOOP 92 */		  "Too many levels of symbolic links",
		  	  "error 93",
		  	  "error 94",
/* EOPNOTSUPP 95 */	  "Operation not supported",
/* EPFNOSUPPORT 96 */	  "Protocol family not supported",
		  	  "error 97",
		  	  "error 98",
		  	  "error 99",
		  	  "error 100",
		  	  "error 101",
		  	  "error 102",
		  	  "error 103",
/* ECONNRESET 104 */	  "Connection reset by peer",
/* ENOBUFS 105 */	  "No buffer space available",
/* EAFNOSUPPORT 106 */	  "Address family not supported by protocol",
/* EPROTOTYPE 107 */	  "Protocol wrong type for socket",
/* ENOTSOCK 108 */ 	  "Socket operation on non-socket",
/* ENOPROTOOPT 109 */	  "Protocol not available",
/* ESHUTDOWN 110 */	  "Cannot send after transport endpoint shutdown",
/* ECONNREFUSED 111 */	  "Connection refused",
/* EADDRINUSE 112 */	  "Address already in use",
/* ECONNABORTED 113 */	  "Software caused connection abort",
/* ENETUNREACH 114 */	  "Network is unreachable",
/* ENETDOWN 115 */	  "Network is down",
/* ETIMEDOUT 116 */	  "Connection timed out",
/* EHOSTDOWN 117 */	  "Host is down",
/* EHOSTUNREACH 118 */	  "No route to host",
/* EINPROGRESS 119 */	  "Operation now in progress",
/* EALREADY 120 */	  "Operation already in progress",
/* EDESTADDRREQ 121 */	  "Destination address required",
/* EMSGSIZE 122 */	  "Message too long",
/* EPROTONOSUPPORT 123 */ "Protocol not supported",
/* ESOCKTNOSUPPORT 124 */ "Socket type not supported",
/* EADDRNOTAVAIL 125 */	  "Cannot assign requested address",
/* ENETRESET 126 */	  "Network dropped connection on reset",
/* EISCONN 127 */	  "Transport endpoint is already connected",
/* ENOTCONN 128 */	  "Transport endpoint is not connected",
/* ETOOMANYREFS 129 */	  "Too many references: cannot splice",
/* EPROCLIM 130 */	  "Too many processes",
/* EUSERS 131 */	  "Too many users",
/* EDQUOT 132 */	  "Disk quota exceeded",
/* ESTALE 133 */	  "Stale NFS file handle",
/* ENOTSUP 134 */  	  "Not supported",
/* ENOMEDIUM 135 */	  "No medium found",
/* ENOSHARE 136 */  	  "No such host or network path",
/* ECASECLASH 137 */	  "Filename exists with different case",
/* EILSEQ 138 */	  "Invalid or incomplete multibyte or wide character",
/* EOVERFLOW 139 */	  "Value too large for defined data type"
};

int NO_COPY_INIT _sys_nerr = sizeof (_sys_errlist) / sizeof (_sys_errlist[0]);
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
  syscall_printf ("%s:%d windows error %d", file, line, code);
  set_errno (geterrno_from_win_error (code, EACCES));
}

/* seterrno: Set `errno' based on GetLastError (). */
void __stdcall
seterrno (const char *file, int line)
{
  seterrno_from_win_error (file, line, GetLastError ());
}

extern char *_user_strerror _PARAMS ((int));

static char *
strerror_worker (int errnum)
{
  char *res;
  if (errnum >= 0 && errnum < _sys_nerr)
    res = (char *) _sys_errlist [errnum];
  else
    res = NULL;
  return res;
}

/* strerror: convert from errno values to error strings */
extern "C" char *
strerror (int errnum)
{
  char *errstr = strerror_worker (errnum);
  if (!errstr)
    __small_sprintf (errstr = _my_tls.locals.strerror_buf, "Unknown error %u",
		     (unsigned) errnum);
  return errstr;
}

#if 0
extern "C" int
strerror_r (int errnum, char *buf, size_t n)
{
  char *errstr = strerror_worker (errnum);
  if (!errstr)
    return EINVAL;
  if (strlen (errstr) >= n)
    return ERANGE;
  strcpy (buf, errstr);
  return 0;
}
#endif

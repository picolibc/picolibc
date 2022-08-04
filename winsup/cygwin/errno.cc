/* errno.cc: errno-related functions

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define _sys_nerr FOO_sys_nerr
#define sys_nerr FOOsys_nerr
#define _sys_errlist FOO_sys_errlist
#define strerror_r FOO_strerror_r
#define __INSIDE_CYGWIN__
#include <errno.h>
#include <error.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "winsup.h"
#include "cygtls.h"
#include "ntdll.h"
#undef _sys_nerr
#undef sys_nerr
#undef _sys_errlist
#undef strerror_r

/* Table to map Windows error codes to errno values. */
#include "errmap.h"

extern "C" {
const char *_sys_errlist[] =
{
/* NOERROR 0 */		  "No error",
/* EPERM 1 */		  "Operation not permitted",
/* ENOENT 2 */		  "No such file or directory",
/* ESRCH 3 */		  "No such process",
/* EINTR 4 */		  "Interrupted system call",
/* EIO 5 */		  "Input/output error",
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
			  NULL,
			  NULL,
			  NULL,
/* EBADE 50 */		  "Invalid exchange",
/* EBADR 51 */		  "Invalid request descriptor",
/* EXFULL 52 */		  "Exchange full",
/* ENOANO 53 */		  "No anode",
/* EBADRQC 54 */	  "Invalid request code",
/* EBADSLT 55 */	  "Invalid slot",
/* EDEADLOCK 56 */	  "File locking deadlock error",
/* EBFONT 57 */		  "Bad font file format",
			  NULL,
			  NULL,
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
			  NULL,
			  NULL,
/* EMULTIHOP 74 */	  "Multihop attempted",
/* ELBIN 75 */		  "Inode is remote (not really error)",
/* EDOTDOT 76 */	  "RFS specific error",
/* EBADMSG 77 */	  "Bad message",
			  NULL,
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
			  NULL,
			  NULL,
/* EOPNOTSUPP 95 */	  "Operation not supported",
/* EPFNOSUPPORT 96 */	  "Protocol family not supported",
			  NULL,
			  NULL,
			  NULL,
			  NULL,
			  NULL,
			  NULL,
			  NULL,
/* ECONNRESET 104 */	  "Connection reset by peer",
/* ENOBUFS 105 */	  "No buffer space available",
/* EAFNOSUPPORT 106 */	  "Address family not supported by protocol",
/* EPROTOTYPE 107 */	  "Protocol wrong type for socket",
/* ENOTSOCK 108 */	  "Socket operation on non-socket",
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
/* ENOTSUP 134 */	  "Not supported",
/* ENOMEDIUM 135 */	  "No medium found",
/* ENOSHARE 136 */	  "No such host or network path",
/* ECASECLASH 137 */	  "Filename exists with different case",
/* EILSEQ 138 */	  "Invalid or incomplete multibyte or wide character",
/* EOVERFLOW 139 */	  "Value too large for defined data type",
/* ECANCELED 140 */	  "Operation canceled",
/* ENOTRECOVERABLE 141 */ "State not recoverable",
/* EOWNERDEAD 142 */	  "Previous owner died",
/* ESTRPIPE 143 */	  "Streams pipe error"
};

int NO_COPY_INIT _sys_nerr = sizeof (_sys_errlist) / sizeof (_sys_errlist[0]);
};

int
geterrno_from_win_error (DWORD code, int deferrno)
{
  /* A 0-value in errmap means, we don't handle this windows error
     explicitely.  Fall back to deferrno in these cases. */
  if (code < sizeof errmap / sizeof errmap[0] && errmap[code])
    {
      syscall_printf ("windows error %u == errno %d", code, errmap[code]);
      return errmap[code];
    }

  syscall_printf ("unknown windows error %u, setting errno to %d", code,
		  deferrno);
  return deferrno;	/* FIXME: what's so special about EACCESS? */
}

/* seterrno_from_win_error: Given a Windows error code, set errno
   as appropriate. */
void
seterrno_from_win_error (const char *file, int line, DWORD code)
{
  syscall_printf ("%s:%d windows error %u", file, line, code);
  errno = _REENT_ERRNO(_impure_ptr) =  geterrno_from_win_error (code, EACCES);
}

int
geterrno_from_nt_status (NTSTATUS status, int deferrno)
{
  return geterrno_from_win_error (RtlNtStatusToDosError (status));
}

/* seterrno_from_nt_status: Given a NT status code, set errno
   as appropriate. */
void
seterrno_from_nt_status (const char *file, int line, NTSTATUS status)
{
  DWORD code = RtlNtStatusToDosError (status);
  SetLastError (code);
  syscall_printf ("%s:%d status %y -> windows error %u",
		  file, line, status, code);
  errno = _REENT_ERRNO(_impure_ptr) =  geterrno_from_win_error (code, EACCES);
}

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

/* Newlib requires this override for perror and friends to avoid
   clobbering strerror() buffer, without having to differentiate
   between strerror_r signatures.  This function is intentionally not
   exported, so that only newlib can use it.  */
extern "C" char *
_strerror_r (struct _reent *, int errnum, int internal, int *errptr)
{
  char *errstr = strerror_worker (errnum);
  if (!errstr)
    {
      errstr = internal ? _my_tls.locals.strerror_r_buf
	: _my_tls.locals.strerror_buf;
      __small_sprintf (errstr, "Unknown error %d", errnum);
      if (errptr)
	*errptr = EINVAL;
    }
  return errstr;
}

/* strerror: convert from errno values to error strings.  Newlib's
   strerror_r returns "" for unknown values, so we override it to
   provide a nicer thread-safe result string and set errno.  */
extern "C" char *
strerror (int errnum)
{
  int error = 0;
  char *result = _strerror_r (NULL, errnum, 0, &error);
  if (error)
    set_errno (error);
  return result;
}

extern "C" char *
strerror_l (int errnum, locale_t locale)
{
  /* We don't provide localized system error messages (yet?). */
  return strerror (errnum);
}

/* Newlib's <string.h> provides declarations for two strerror_r
   variants, according to preprocessor feature macros.  However, it
   returns "" instead of "Unknown error ...", so we override both
   versions.  */
extern "C" char *
strerror_r (int errnum, char *buf, size_t n)
{
  int error = 0;
  char *errstr = _strerror_r (NULL, errnum, 1, &error);
  if (error)
    set_errno (error);
  if (strlen (errstr) >= n)
    return errstr;
  return strcpy (buf, errstr);
}

extern "C" int
__xpg_strerror_r (int errnum, char *buf, size_t n)
{
  if (!n)
    return ERANGE;
  int result = 0;
  char *error = strerror_worker (errnum);
  char tmp[sizeof "Unknown error -2147483648"];
  if (!error)
    {
      __small_sprintf (error = tmp, "Unknown error %d", errnum);
      result = EINVAL;
    }
  if (strlen (error) >= n)
    {
      memcpy (buf, error, n - 1);
      buf[n - 1] = '\0';
      return ERANGE;
    }
  strcpy (buf, error);
  return result;
}

unsigned int error_message_count = 0;
int error_one_per_line = 0;
void (*error_print_progname) (void) = NULL;

static void
_verror (int status, int errnum, const char *filename, unsigned int lineno, const char *fmt, va_list ap)
{
  error_message_count++;

  fflush (stdout);

  if (error_print_progname)
    (*error_print_progname) ();
  else
    fprintf (stderr, "%s:%s", program_invocation_name, filename ? "" : " ");

  if (filename)
    fprintf (stderr, "%s:%d: ", filename, lineno);

  vfprintf (stderr, fmt, ap);

  if (errnum != 0)
    fprintf (stderr, ": %s", strerror (errnum));

  fprintf (stderr, "\n");

  if (status != 0)
    exit (status);
}

extern "C" void
error (int status, int errnum, const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  _verror (status, errnum, NULL, 0, fmt, ap);
  va_end (ap);
}

extern "C" void
error_at_line (int status, int errnum, const char *filename, unsigned int lineno, const char *fmt, ...)
{
  va_list ap;

  if (error_one_per_line != 0)
    {
      static const char *last_filename;
      static unsigned int last_lineno;

      /* strcmp(3) will SEGV if filename or last_filename are NULL */
      if (lineno == last_lineno
	  && ((!filename && !last_filename)
	      || (filename && last_filename && strcmp (filename, last_filename) == 0)))
	return;

      last_filename = filename;
      last_lineno = lineno;
    }

  va_start (ap, fmt);
  _verror (status, errnum, filename, lineno, fmt, ap);
  va_end (ap);
}

/*
Copyright (c) 1982, 1986, 1993
The Regents of the University of California.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. Neither the name of the University nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
 */
/* errno is not a global variable, because that would make using it
   non-reentrant.  Instead, its address is returned by the function
   __errno.  */

#ifndef _SYS_ERRNO_H_
#ifdef __cplusplus
extern "C" {
#endif
#define _SYS_ERRNO_H_

#include <sys/config.h>

/* Please don't use these variables directly.
   Use strerror instead. */
extern __IMPORT const char * const _sys_errlist[];
extern __IMPORT int _sys_nerr;
#ifdef __CYGWIN__
extern __IMPORT const char * const sys_errlist[];
extern __IMPORT int sys_nerr;
extern __IMPORT char *program_invocation_name;
extern __IMPORT char *program_invocation_short_name;
#endif

#ifdef NEWLIB_GLOBAL_ERRNO
#define NEWLIB_THREAD_LOCAL_ERRNO
#else
#define NEWLIB_THREAD_LOCAL_ERRNO NEWLIB_THREAD_LOCAL
#endif

#ifdef __PICOLIBC_ERRNO_FUNCTION
int *__PICOLIBC_ERRNO_FUNCTION(void);
#define errno (*__PICOLIBC_ERRNO_FUNCTION())
#else
extern NEWLIB_THREAD_LOCAL_ERRNO int errno;
#define errno		errno
#endif

#define __errno_r(ptr)	(errno)
#define _REENT_ERRNO(r) (errno)

#define	EPERM 1			/* Not owner */
#define	ENOENT 2		/* No such file or directory */
#define	ESRCH 3			/* No such process */
#define	EINTR 4			/* Interrupted system call */
#define	EIO 5			/* I/O error */
#define	ENXIO 6			/* No such device or address */
#define	E2BIG 7			/* Arg list too long */
#define	ENOEXEC 8		/* Exec format error */
#define	EBADF 9			/* Bad file number */
#define	ECHILD 10		/* No children */
#define	EAGAIN 11		/* No more processes */
#define	ENOMEM 12		/* Not enough space */
#define	EACCES 13		/* Permission denied */
#define	EFAULT 14		/* Bad address */
#ifdef __LINUX_ERRNO_EXTENSIONS__
#define	ENOTBLK 15	        /* Block device required */
#endif
#define	EBUSY 16		/* Device or resource busy */
#define	EEXIST 17		/* File exists */
#define	EXDEV 18		/* Cross-device link */
#define	ENODEV 19		/* No such device */
#define	ENOTDIR 20		/* Not a directory */
#define	EISDIR 21		/* Is a directory */
#define	EINVAL 22		/* Invalid argument */
#define	ENFILE 23		/* Too many open files in system */
#define	EMFILE 24		/* File descriptor value too large */
#define	ENOTTY 25		/* Not a character device */
#define	ETXTBSY 26		/* Text file busy */
#define	EFBIG 27		/* File too large */
#define	ENOSPC 28		/* No space left on device */
#define	ESPIPE 29		/* Illegal seek */
#define	EROFS 30		/* Read-only file system */
#define	EMLINK 31		/* Too many links */
#define	EPIPE 32		/* Broken pipe */
#define	EDOM 33			/* Mathematics argument out of domain of function */
#define	ERANGE 34		/* Result too large */
#define	ENOMSG 35		/* No message of desired type */
#define	EIDRM 36		/* Identifier removed */
#ifdef __LINUX_ERRNO_EXTENSIONS__
#define	ECHRNG 37       	/* Channel number out of range */
#define	EL2NSYNC 38             /* Level 2 not synchronized */
#define	EL3HLT 39	        /* Level 3 halted */
#define	EL3RST 40       	/* Level 3 reset */
#define	ELNRNG 41	        /* Link number out of range */
#define	EUNATCH 42	        /* Protocol driver not attached */
#define	ENOCSI 43	        /* No CSI structure available */
#define	EL2HLT 44	        /* Level 2 halted */
#endif
#define	EDEADLK 45		/* Deadlock */
#define	ENOLCK 46		/* No lock */
#ifdef __LINUX_ERRNO_EXTENSIONS__
#define EBADE 50		/* Invalid exchange */
#define EBADR 51		/* Invalid request descriptor */
#define EXFULL 52		/* Exchange full */
#define ENOANO 53		/* No anode */
#define EBADRQC 54		/* Invalid request code */
#define EBADSLT 55		/* Invalid slot */
#define EDEADLOCK 56		/* File locking deadlock error */
#define EBFONT 57		/* Bad font file fmt */
#endif
#define ENOSTR 60		/* Not a stream */
#define ENODATA 61		/* No data (for no delay io) */
#define ETIME 62		/* Stream ioctl timeout */
#define ENOSR 63		/* No stream resources */
#ifdef __LINUX_ERRNO_EXTENSIONS__
#define ENONET 64		/* Machine is not on the network */
#define ENOPKG 65		/* Package not installed */
#define EREMOTE 66		/* The object is remote */
#endif
#define ENOLINK 67		/* Virtual circuit is gone */
#ifdef __LINUX_ERRNO_EXTENSIONS__
#define EADV 68			/* Advertise error */
#define ESRMNT 69		/* Srmount error */
#define	ECOMM 70		/* Communication error on send */
#endif
#define EPROTO 71		/* Protocol error */
#define	EMULTIHOP 74		/* Multihop attempted */
#ifdef __LINUX_ERRNO_EXTENSIONS__
#define	ELBIN 75		/* Inode is remote (not really error) */
#define	EDOTDOT 76		/* Cross mount point (not really error) */
#endif
#define EBADMSG 77		/* Bad message */
#ifdef __LINUX_ERRNO_EXTENSIONS__
#define EFTYPE 79		/* Inappropriate file type or format */
#define ENOTUNIQ 80		/* Given log. name not unique */
#define EBADFD 81		/* File descriptor in bad state */
#define EREMCHG 82		/* Remote address changed */
#define ELIBACC 83		/* Can't access a needed shared lib */
#define ELIBBAD 84		/* Accessing a corrupted shared lib */
#define ELIBSCN 85		/* .lib section in a.out corrupted */
#define ELIBMAX 86		/* Attempting to link in too many libs */
#define ELIBEXEC 87		/* Attempting to exec a shared library */
#endif
#define ENOSYS 88		/* Function not implemented */
#ifdef __CYGWIN__
#define ENMFILE 89      	/* No more files */
#endif
#define ENOTEMPTY 90		/* Directory not empty */
#define ENAMETOOLONG 91		/* File or path name too long */
#define ELOOP 92		/* Too many symbolic links */
#define EOPNOTSUPP 95		/* Operation not supported on socket */
#ifdef __LINUX_ERRNO_EXTENSIONS__
#define EPFNOSUPPORT 96 	/* Protocol family not supported */
#endif
#define ECONNRESET 104  	/* Connection reset by peer */
#define ENOBUFS 105		/* No buffer space available */
#define EAFNOSUPPORT 106	/* Address family not supported by protocol family */
#define EPROTOTYPE 107		/* Protocol wrong type for socket */
#define ENOTSOCK 108		/* Socket operation on non-socket */
#define ENOPROTOOPT 109		/* Protocol not available */
#ifdef __LINUX_ERRNO_EXTENSIONS__
#define ESHUTDOWN 110		/* Can't send after socket shutdown */
#endif
#define ECONNREFUSED 111	/* Connection refused */
#define EADDRINUSE 112		/* Address already in use */
#define ECONNABORTED 113	/* Software caused connection abort */
#define ENETUNREACH 114		/* Network is unreachable */
#define ENETDOWN 115		/* Network interface is not configured */
#define ETIMEDOUT 116		/* Connection timed out */
#ifdef __LINUX_ERRNO_EXTENSIONS__
#define EHOSTDOWN 117		/* Host is down */
#endif
#define EHOSTUNREACH 118	/* Host is unreachable */
#define EINPROGRESS 119		/* Connection already in progress */
#define EALREADY 120		/* Socket already connected */
#define EDESTADDRREQ 121	/* Destination address required */
#define EMSGSIZE 122		/* Message too long */
#define EPROTONOSUPPORT 123	/* Unknown protocol */
#ifdef __LINUX_ERRNO_EXTENSIONS__
#define ESOCKTNOSUPPORT 124	/* Socket type not supported */
#endif
#define EADDRNOTAVAIL 125	/* Address not available */
#define ENETRESET 126   	/* Connection aborted by network */
#define EISCONN 127		/* Socket is already connected */
#define ENOTCONN 128		/* Socket is not connected */
#ifdef __LINUX_ERRNO_EXTENSIONS__
#define ETOOMANYREFS 129        /* Too many references: cannot splice */
#define EPROCLIM 130            /* Too many processes */
#define EUSERS 131              /* Too many users */
#endif
#define EDQUOT 132      	/* Reserved */
#define ESTALE 133      	/* Reserved */
#define ENOTSUP 134     	/* Not supported */
#ifdef __LINUX_ERRNO_EXTENSIONS__
#define ENOMEDIUM 135   	/* No medium found */
#endif
#define EILSEQ 138		/* Illegal byte sequence */
#define EOVERFLOW 139		/* Value too large for defined data type */
#define ECANCELED 140		/* Operation canceled */
#define ENOTRECOVERABLE 141	/* State not recoverable */
#define EOWNERDEAD 142		/* Previous owner died */
#ifdef __LINUX_ERRNO_EXTENSIONS__
#define ESTRPIPE 143		/* Streams pipe error */
#define EHWPOISON 144           /* Memory page has hardware error */
#define EISNAM 145              /* Is a named type file */
#define EKEYEXPIRED 146         /* Key has expired */
#define EKEYREJECTED 147        /* Key was rejected by service */
#define EKEYREVOKED 148         /* Key has been revoked */
#endif
#define EWOULDBLOCK EAGAIN	/* Operation would block */

#define __ELASTERROR 2000       /* Users can add values starting here */

#ifdef __cplusplus
}
#endif
#endif /* _SYS_ERRNO_H */

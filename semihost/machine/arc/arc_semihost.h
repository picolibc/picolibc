/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2023 Keith Packard
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _ARC_SEMIHOST_H_
#define _ARC_SEMIHOST_H_

#include <stdint.h>
#include <errno.h>

#define SYS_SEMIHOST_exit	1
#define SYS_SEMIHOST_read	3
#define SYS_SEMIHOST_write	4
#define SYS_SEMIHOST_open	5
#define SYS_SEMIHOST_close	6
#define SYS_SEMIHOST_unlink	10
#define SYS_SEMIHOST_time	13
#define SYS_SEMIHOST_lseek	19
#define SYS_SEMIHOST_times	43
#define SYS_SEMIHOST_gettimeofday	78
#define SYS_SEMIHOST_stat	106 /* nsim stat's is corupted.  */
#define SYS_SEMIHOST_fstat	108
#define SYS_SEMIHOST_argc	1000
#define SYS_SEMIHOST_argv_sz	1001
#define SYS_SEMIHOST_argv	1002
#define SYS_SEMIHOST_memset	1004
#define SYS_SEMIHOST_errno      2000

enum {
    TARGET_ERRNO_EPERM = 1,			/* Not owner */
    TARGET_ERRNO_ENOENT = 2,		/* No such file or directory */
    TARGET_ERRNO_ESRCH = 3,			/* No such process */
    TARGET_ERRNO_EINTR = 4,			/* Interrupted system call */
    TARGET_ERRNO_EIO = 5,			/* I/O error */
    TARGET_ERRNO_ENXIO = 6,			/* No such device or address */
    TARGET_ERRNO_E2BIG = 7,			/* Arg list too long */
    TARGET_ERRNO_ENOEXEC = 8,		/* Exec format error */
    TARGET_ERRNO_EBADF = 9,			/* Bad file number */
    TARGET_ERRNO_ECHILD = 10,		/* No children */
    TARGET_ERRNO_EAGAIN = 11,		/* No more processes */
    TARGET_ERRNO_ENOMEM = 12,		/* Not enough space */
    TARGET_ERRNO_EACCES = 13,		/* Permission denied */
    TARGET_ERRNO_EFAULT = 14,		/* Bad address */
    TARGET_ERRNO_ENOTBLK = 15,	    /* Block device required */
    TARGET_ERRNO_EBUSY = 16,		/* Device or resource busy */
    TARGET_ERRNO_EEXIST = 17,		/* File exists */
    TARGET_ERRNO_EXDEV = 18,		/* Cross-device link */
    TARGET_ERRNO_ENODEV = 19,		/* No such device */
    TARGET_ERRNO_ENOTDIR = 20,		/* Not a directory */
    TARGET_ERRNO_EISDIR = 21,		/* Is a directory */
    TARGET_ERRNO_EINVAL = 22,		/* Invalid argument */
    TARGET_ERRNO_ENFILE = 23,		/* Too many open files in system */
    TARGET_ERRNO_EMFILE = 24,		/* File descriptor value too large */
    TARGET_ERRNO_ENOTTY = 25,		/* Not a character device */
    TARGET_ERRNO_ETXTBSY = 26,		/* Text file busy */
    TARGET_ERRNO_EFBIG = 27,		/* File too large */
    TARGET_ERRNO_ENOSPC = 28,		/* No space left on device */
    TARGET_ERRNO_ESPIPE = 29,		/* Illegal seek */
    TARGET_ERRNO_EROFS = 30,		/* Read-only file system */
    TARGET_ERRNO_EMLINK = 31,		/* Too many links */
    TARGET_ERRNO_EPIPE = 32,		/* Broken pipe */
    TARGET_ERRNO_EDOM = 33,			/* Mathematics argument out of domain of function */
    TARGET_ERRNO_ERANGE = 34,		/* Result too large */
    TARGET_ERRNO_ENOMSG = 35,		/* No message of desired type */
    TARGET_ERRNO_EIDRM = 36,		/* Identifier removed */
    TARGET_ERRNO_ECHRNG = 37,       /* Channel number out of range */
    TARGET_ERRNO_EL2NSYNC = 38,     /* Level 2 not synchronized */
    TARGET_ERRNO_EL3HLT = 39,	    /* Level 3 halted */
    TARGET_ERRNO_EL3RST = 40,       /* Level 3 reset */
    TARGET_ERRNO_ELNRNG = 41,	    /* Link number out of range */
    TARGET_ERRNO_EUNATCH = 42,	    /* Protocol driver not attached */
    TARGET_ERRNO_ENOCSI = 43,	    /* No CSI structure available */
    TARGET_ERRNO_EL2HLT = 44,	    /* Level 2 halted */
    TARGET_ERRNO_EDEADLK = 45,		/* Deadlock */
    TARGET_ERRNO_ENOLCK = 46,		/* No lock */
    TARGET_ERRNO_EBADE = 50,		/* Invalid exchange */
    TARGET_ERRNO_EBADR = 51,		/* Invalid request descriptor */
    TARGET_ERRNO_EXFULL = 52,		/* Exchange full */
    TARGET_ERRNO_ENOANO = 53,		/* No anode */
    TARGET_ERRNO_EBADRQC = 54,		/* Invalid request code */
    TARGET_ERRNO_EBADSLT = 55,		/* Invalid slot */
    TARGET_ERRNO_EDEADLOCK = 56,	/* File locking deadlock error */
    TARGET_ERRNO_EBFONT = 57,		/* Bad font file fmt */
    TARGET_ERRNO_ENOSTR = 60,		/* Not a stream */
    TARGET_ERRNO_ENODATA = 61,		/* No data (for no delay io) */
    TARGET_ERRNO_ETIME = 62,		/* Stream ioctl timeout */
    TARGET_ERRNO_ENOSR = 63,		/* No stream resources */
    TARGET_ERRNO_ENONET = 64,		/* Machine is not on the network */
    TARGET_ERRNO_ENOPKG = 65,		/* Package not installed */
    TARGET_ERRNO_EREMOTE = 66,		/* The object is remote */
    TARGET_ERRNO_ENOLINK = 67,		/* Virtual circuit is gone */
    TARGET_ERRNO_EADV = 68,			/* Advertise error */
    TARGET_ERRNO_ESRMNT = 69,		/* Srmount error */
    TARGET_ERRNO_ECOMM = 70,		/* Communication error on send */
    TARGET_ERRNO_EPROTO = 71,		/* Protocol error */
    TARGET_ERRNO_EMULTIHOP = 74,	/* Multihop attempted */
    TARGET_ERRNO_ELBIN = 75,		/* Inode is remote (not really error) */
    TARGET_ERRNO_EDOTDOT = 76,		/* Cross mount point (not really error) */
    TARGET_ERRNO_EBADMSG = 77,		/* Bad message */
    TARGET_ERRNO_EFTYPE = 79,		/* Inappropriate file type or format */
    TARGET_ERRNO_ENOTUNIQ = 80,		/* Given log. name not unique */
    TARGET_ERRNO_EBADFD = 81,		/* File descriptor in bad state */
    TARGET_ERRNO_EREMCHG = 82,		/* Remote address changed */
    TARGET_ERRNO_ELIBACC = 83,		/* Can't access a needed shared lib */
    TARGET_ERRNO_ELIBBAD = 84,		/* Accessing a corrupted shared lib */
    TARGET_ERRNO_ELIBSCN = 85,		/* .lib section in a.out corrupted */
    TARGET_ERRNO_ELIBMAX = 86,		/* Attempting to link in too many libs */
    TARGET_ERRNO_ELIBEXEC = 87,		/* Attempting to exec a shared library */
    TARGET_ERRNO_ENOSYS = 88,		/* Function not implemented */
    TARGET_ERRNO_ENMFILE = 89,      /* No more files */
    TARGET_ERRNO_ENOTEMPTY = 90,	/* Directory not empty */
    TARGET_ERRNO_ENAMETOOLONG = 91,	/* File or path name too long */
    TARGET_ERRNO_ELOOP = 92,		/* Too many symbolic links */
    TARGET_ERRNO_EOPNOTSUPP = 95,	/* Operation not supported on socket */
    TARGET_ERRNO_EPFNOSUPPORT = 96, /* Protocol family not supported */
    TARGET_ERRNO_ECONNRESET = 104,  /* Connection reset by peer */
    TARGET_ERRNO_ENOBUFS = 105,		/* No buffer space available */
    TARGET_ERRNO_EAFNOSUPPORT = 106,/* Address family not supported by protocol family */
    TARGET_ERRNO_EPROTOTYPE = 107,	/* Protocol wrong type for socket */
    TARGET_ERRNO_ENOTSOCK = 108,	/* Socket operation on non-socket */
    TARGET_ERRNO_ENOPROTOOPT = 109,	/* Protocol not available */
    TARGET_ERRNO_ESHUTDOWN = 110,	/* Can't send after socket shutdown */
    TARGET_ERRNO_ECONNREFUSED = 111,/* Connection refused */
    TARGET_ERRNO_EADDRINUSE = 112,	/* Address already in use */
    TARGET_ERRNO_ECONNABORTED = 113,/* Software caused connection abort */
    TARGET_ERRNO_ENETUNREACH = 114,	/* Network is unreachable */
    TARGET_ERRNO_ENETDOWN = 115,	/* Network interface is not configured */
    TARGET_ERRNO_ETIMEDOUT = 116,	/* Connection timed out */
    TARGET_ERRNO_EHOSTDOWN = 117,	/* Host is down */
    TARGET_ERRNO_EHOSTUNREACH = 118,/* Host is unreachable */
    TARGET_ERRNO_EINPROGRESS = 119,	/* Connection already in progress */
    TARGET_ERRNO_EALREADY = 120,	/* Socket already connected */
    TARGET_ERRNO_EDESTADDRREQ = 121,/* Destination address required */
    TARGET_ERRNO_EMSGSIZE = 122,	/* Message too long */
    TARGET_ERRNO_EPROTONOSUPPORT = 123,	/* Unknown protocol */
    TARGET_ERRNO_ESOCKTNOSUPPORT = 124,	/* Socket type not supported */
    TARGET_ERRNO_EADDRNOTAVAIL = 125,	/* Address not available */
    TARGET_ERRNO_ENETRESET = 126,   /* Connection aborted by network */
    TARGET_ERRNO_EISCONN = 127,		/* Socket is already connected */
    TARGET_ERRNO_ENOTCONN = 128,	/* Socket is not connected */
    TARGET_ERRNO_ETOOMANYREFS = 129,/* Too many references: cannot splice */
    TARGET_ERRNO_EPROCLIM = 130,    /* Too many processes */
    TARGET_ERRNO_EUSERS = 131,      /* Too many users */
    TARGET_ERRNO_EDQUOT = 132,      /* Reserved */
    TARGET_ERRNO_ESTALE = 133,      /* Reserved */
    TARGET_ERRNO_ENOTSUP = 134,     /* Not supported */
    TARGET_ERRNO_ENOMEDIUM = 135,   /* No medium found */
    TARGET_ERRNO_ENOSHARE = 136,    /* No such host or network path */
    TARGET_ERRNO_ECASECLASH = 137,  /* Filename exists with different case */
    TARGET_ERRNO_EILSEQ = 138,		/* Illegal byte sequence */
    TARGET_ERRNO_EOVERFLOW = 139,	/* Value too large for defined data type */
    TARGET_ERRNO_ECANCELED = 140,	/* Operation canceled */
    TARGET_ERRNO_ENOTRECOVERABLE = 141,	/* State not recoverable */
    TARGET_ERRNO_EOWNERDEAD = 142,	/* Previous owner died */
    TARGET_ERRNO_ESTRPIPE = 143,	/* Streams pipe error */
    TARGET_ERRNO_EHWPOISON = 144,   /* Memory page has hardware error */
    TARGET_ERRNO_EISNAM = 145,      /* Is a named type file */
    TARGET_ERRNO_EKEYEXPIRED = 146, /* Key has expired */
    TARGET_ERRNO_EKEYREJECTED = 147,/* Key was rejected by service */
    TARGET_ERRNO_EKEYREVOKED = 148, /* Key has been revoked */
};

#ifdef __ARC700__
#define SWI     "trap0"
#else
#define SWI     "swi"
#endif

static inline uintptr_t
arc_semihost0(uintptr_t op)
{
    register uintptr_t r_op __asm__("r8") = op;
    register uintptr_t r_r0 __asm__("r0");

    __asm__ volatile (SWI : "=r" (r_r0) : "r" (r_op));
    return r_r0;
}

static inline uintptr_t
arc_semihost1(uintptr_t op, uintptr_t r0)
{
    register uintptr_t r_op __asm__("r8") = op;
    register uintptr_t r_r0 __asm__("r0") = r0;

    __asm__ volatile (SWI : "=r" (r_r0) : "r" (r_op), "r" (r_r0));
    return r_r0;
}

static inline uintptr_t
arc_semihost2(uintptr_t op, uintptr_t r0, uintptr_t r1)
{
    register uintptr_t r_op __asm__("r8") = op;
    register uintptr_t r_r0 __asm__("r0") = r0;
    register uintptr_t r_r1 __asm__("r1") = r1;

    __asm__ volatile (SWI : "=r" (r_r0) : "r" (r_op), "r" (r_r0), "r" (r_r1));
    return r_r0;
}

static inline uintptr_t
arc_semihost3(uintptr_t op, uintptr_t r0, uintptr_t r1, uintptr_t r2)
{
    register uintptr_t r_op __asm__("r8") = op;
    register uintptr_t r_r0 __asm__("r0") = r0;
    register uintptr_t r_r1 __asm__("r1") = r1;
    register uintptr_t r_r2 __asm__("r2") = r2;

    __asm__ volatile (SWI : "=r" (r_r0) : "r" (r_op), "r" (r_r0), "r" (r_r1), "r" (r_r2));
    return r_r0;
}

void arc_semihost_errno(int def);

#endif /* _ARC_SEMIHOST_H_ */

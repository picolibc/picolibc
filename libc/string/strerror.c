/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Keith Packard
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

#define _DEFAULT_SOURCE
#include <errno.h>
#include <string.h>
#include "string_private.h"
#include "local.h"

static const char * const errnames[] = {
    [0] = "Success",
#if defined(EPERM) && (!defined(EACCES) || (EPERM != EACCES))
    [EPERM] = "Not owner",
#endif
#ifdef ENOENT
    [ENOENT] = "No such file or directory",
#endif
#ifdef ESRCH
    [ESRCH] = "No such process",
#endif
#ifdef EINTR
    [EINTR] = "Interrupted system call",
#endif
#ifdef EIO
    [EIO] = "I/O error",
#endif
#if defined(ENXIO) && (!defined(ENODEV) || (ENXIO != ENODEV))
    [ENXIO] = "No such device or address",
#endif
#ifdef E2BIG
    [E2BIG] = "Arg list too long",
#endif
#ifdef ENOEXEC
    [ENOEXEC] = "Exec format error",
#endif
#ifdef EALREADY
    [EALREADY] = "Socket already connected",
#endif
#ifdef EBADF
    [EBADF] = "Bad file number",
#endif
#ifdef ECHILD
    [ECHILD] = "No children",
#endif
#ifdef EDESTADDRREQ
    [EDESTADDRREQ] = "Destination address required",
#endif
#ifdef EAGAIN
    [EAGAIN] = "No more processes",
#endif
#ifdef ENOMEM
    [ENOMEM] = "Not enough space",
#endif
#ifdef EACCES
    [EACCES] = "Permission denied",
#endif
#ifdef EFAULT
    [EFAULT] = "Bad address",
#endif
#ifdef ENOTBLK
    [ENOTBLK] = "Block device required",
#endif
#ifdef EBUSY
    [EBUSY] = "Device or resource busy",
#endif
#ifdef EEXIST
    [EEXIST] = "File exists",
#endif
#ifdef EXDEV
    [EXDEV] = "Cross-device link",
#endif
#ifdef ENODEV
    [ENODEV] = "No such device",
#endif
#ifdef ENOTDIR
    [ENOTDIR] = "Not a directory",
#endif
#ifdef EHOSTDOWN
    [EHOSTDOWN] = "Host is down",
#endif
#ifdef EINPROGRESS
    [EINPROGRESS] = "Connection already in progress",
#endif
#ifdef EISDIR
    [EISDIR] = "Is a directory",
#endif
#ifdef EINVAL
    [EINVAL] = "Invalid argument",
#endif
#ifdef ENETDOWN
    [ENETDOWN] = "Network interface is not configured",
#endif
#ifdef ENETRESET
    [ENETRESET] = "Connection aborted by network",
#endif
#ifdef ENFILE
    [ENFILE] = "Too many open files in system",
#endif
#ifdef EMFILE
    [EMFILE] = "File descriptor value too large",
#endif
#ifdef ENOTTY
    [ENOTTY] = "Not a character device",
#endif
#ifdef ETXTBSY
    [ETXTBSY] = "Text file busy",
#endif
#ifdef EFBIG
    [EFBIG] = "File too large",
#endif
#ifdef EHOSTUNREACH
    [EHOSTUNREACH] = "Host is unreachable",
#endif
#ifdef ENOSPC
    [ENOSPC] = "No space left on device",
#endif
#ifdef ENOTSUP
    [ENOTSUP] = "Not supported",
#endif
#ifdef ESPIPE
    [ESPIPE] = "Illegal seek",
#endif
#ifdef EROFS
    [EROFS] = "Read-only file system",
#endif
#ifdef EMLINK
    [EMLINK] = "Too many links",
#endif
#ifdef EPIPE
    [EPIPE] = "Broken pipe",
#endif
#ifdef EDOM
    [EDOM] = "Mathematics argument out of domain of function",
#endif
#ifdef ERANGE
    [ERANGE] = "Result too large",
#endif
#ifdef ENOMSG
    [ENOMSG] = "No message of desired type",
#endif
#ifdef EIDRM
    [EIDRM] = "Identifier removed",
#endif
#ifdef EILSEQ
    [EILSEQ] = "Illegal byte sequence",
#endif
#ifdef EDEADLK
    [EDEADLK] = "Deadlock",
#endif
#ifdef ENETUNREACH
    [ENETUNREACH] = "Network is unreachable",
#endif
#ifdef ENOLCK
    [ENOLCK] = "No lock",
#endif
#ifdef ENOSTR
    [ENOSTR] = "Not a stream",
#endif
#ifdef ETIME
    [ETIME] = "Stream ioctl timeout",
#endif
#ifdef ENOSR
    [ENOSR] = "No stream resources",
#endif
#ifdef ENONET
    [ENONET] = "Machine is not on the network",
#endif
#ifdef ENOPKG
    [ENOPKG] = "No package",
#endif
#ifdef EREMOTE
    [EREMOTE] = "Resource is remote",
#endif
#ifdef ENOLINK
    [ENOLINK] = "Virtual circuit is gone",
#endif
#ifdef EADV
    [EADV] = "Advertise error",
#endif
#ifdef ESRMNT
    [ESRMNT] = "Srmount error",
#endif
#ifdef ECOMM
    [ECOMM] = "Communication error",
#endif
#ifdef EPROTO
    [EPROTO] = "Protocol error",
#endif
#ifdef EPROTONOSUPPORT
    [EPROTONOSUPPORT] = "Unknown protocol",
#endif
#ifdef EMULTIHOP
    [EMULTIHOP] = "Multihop attempted",
#endif
#ifdef EBADMSG
    [EBADMSG] = "Bad message",
#endif
#ifdef ELIBACC
    [ELIBACC] = "Cannot access a needed shared library",
#endif
#ifdef ELIBBAD
    [ELIBBAD] = "Accessing a corrupted shared library",
#endif
#ifdef ELIBSCN
    [ELIBSCN] = ".lib section in a.out corrupted",
#endif
#ifdef ELIBMAX
    [ELIBMAX] = "Attempting to link in more shared libraries than system limit",
#endif
#ifdef ELIBEXEC
    [ELIBEXEC] = "Cannot exec a shared library directly",
#endif
#ifdef ENOSYS
    [ENOSYS] = "Function not implemented",
#endif
#ifdef ENMFILE
    [ENMFILE] = "No more files",
#endif
#ifdef ENOTEMPTY
    [ENOTEMPTY] = "Directory not empty",
#endif
#ifdef ENAMETOOLONG
    [ENAMETOOLONG] = "File or path name too long",
#endif
#ifdef ELOOP
    [ELOOP] = "Too many symbolic links",
#endif
#ifdef ENOBUFS
    [ENOBUFS] = "No buffer space available",
#endif
#ifdef ENODATA
    [ENODATA] = "No data",
#endif
#ifdef EAFNOSUPPORT
    [EAFNOSUPPORT] = "Address family not supported by protocol family",
#endif
#ifdef EPROTOTYPE
    [EPROTOTYPE] = "Protocol wrong type for socket",
#endif
#ifdef ENOTSOCK
    [ENOTSOCK] = "Socket operation on non-socket",
#endif
#ifdef ENOPROTOOPT
    [ENOPROTOOPT] = "Protocol not available",
#endif
#ifdef ESHUTDOWN
    [ESHUTDOWN] = "Can't send after socket shutdown",
#endif
#ifdef ECONNREFUSED
    [ECONNREFUSED] = "Connection refused",
#endif
#ifdef ECONNRESET
    [ECONNRESET] = "Connection reset by peer",
#endif
#ifdef EADDRINUSE
    [EADDRINUSE] = "Address already in use",
#endif
#ifdef EADDRNOTAVAIL
    [EADDRNOTAVAIL] = "Address not available",
#endif
#ifdef ECONNABORTED
    [ECONNABORTED] = "Software caused connection abort",
#endif
#if (defined(EWOULDBLOCK) && (!defined(EAGAIN) || (EWOULDBLOCK != EAGAIN)))
    [EWOULDBLOCK] = "Operation would block",
#endif
#ifdef ENOTCONN
    [ENOTCONN] = "Socket is not connected",
#endif
#ifdef ESOCKTNOSUPPORT
    [ESOCKTNOSUPPORT] = "Socket type not supported",
#endif
#ifdef EISCONN
    [EISCONN] = "Socket is already connected",
#endif
#ifdef ECANCELED
    [ECANCELED] = "Operation canceled",
#endif
#ifdef ENOTRECOVERABLE
    [ENOTRECOVERABLE] = "State not recoverable",
#endif
#ifdef EOWNERDEAD
    [EOWNERDEAD] = "Previous owner died",
#endif
#ifdef ESTRPIPE
    [ESTRPIPE] = "Streams pipe error",
#endif
#if defined(EOPNOTSUPP) && (!defined(ENOTSUP) || (ENOTSUP != EOPNOTSUPP))
    [EOPNOTSUPP] = "Operation not supported on socket",
#endif
#ifdef EOVERFLOW
    [EOVERFLOW] = "Value too large for defined data type",
#endif
#ifdef EMSGSIZE
    [EMSGSIZE] = "Message too long",
#endif
#ifdef ETIMEDOUT
    [ETIMEDOUT] = "Connection timed out",
#endif
};

#define NERRNAMES (sizeof(errnames) / sizeof(errnames[0]))

char *_user_strerror(int, int, int *) __weak;

char *
_strerror_r(int errnum, int internal, int *errptr)
{
    char *error;

    if ((unsigned)errnum < NERRNAMES) {
        error = (char *)errnames[(unsigned)errnum];
        if (error)
            return error;
    }

    if (!errptr)
        errptr = &errno;
    if (&_user_strerror == NULL || (error = _user_strerror(errnum, internal, errptr)) == 0)
        error = "Unknown error";
    return error;
}

char *
strerror(int errnum)
{
    return _strerror_r(errnum, 0, NULL);
}

char *
strerror_l(int errnum, locale_t locale)
{
    (void)locale;
    /* We don't support per-locale error messages. */
    return _strerror_r(errnum, 0, NULL);
}

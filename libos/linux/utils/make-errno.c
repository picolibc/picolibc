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

#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

static int has[10240];

static bool
check_has(int e)
{
    int i;
    for (i = 0; i < sizeof(has) / sizeof(has[0]); i++) {
        if (has[i] == e)
            return true;
        if (has[i] == 0) {
            has[i] = e;
            return false;
        }
    }
    return true;
}

static void
reset_has(void)
{
    memset(has, 0, sizeof(has));
}

struct {
    int         value;
    const char *name;
} linux_errno[] = {
#define declare_error(n) { .value = n, .name = #n }
#ifdef EPERM
    declare_error(EPERM),
#endif
#ifdef ENOENT
    declare_error(ENOENT),
#endif
#ifdef ESRCH
    declare_error(ESRCH),
#endif
#ifdef EINTR
    declare_error(EINTR),
#endif
#ifdef EIO
    declare_error(EIO),
#endif
#ifdef ENXIO
    declare_error(ENXIO),
#endif
#ifdef E2BIG
    declare_error(E2BIG),
#endif
#ifdef ENOEXEC
    declare_error(ENOEXEC),
#endif
#ifdef EBADF
    declare_error(EBADF),
#endif
#ifdef ECHILD
    declare_error(ECHILD),
#endif
#ifdef EAGAIN
    declare_error(EAGAIN),
#endif
#ifdef ENOMEM
    declare_error(ENOMEM),
#endif
#ifdef EACCES
    declare_error(EACCES),
#endif
#ifdef EFAULT
    declare_error(EFAULT),
#endif
#ifdef ENOTBLK
    declare_error(ENOTBLK),
#endif
#ifdef EBUSY
    declare_error(EBUSY),
#endif
#ifdef EEXIST
    declare_error(EEXIST),
#endif
#ifdef EXDEV
    declare_error(EXDEV),
#endif
#ifdef ENODEV
    declare_error(ENODEV),
#endif
#ifdef ENOTDIR
    declare_error(ENOTDIR),
#endif
#ifdef EISDIR
    declare_error(EISDIR),
#endif
#ifdef EINVAL
    declare_error(EINVAL),
#endif
#ifdef ENFILE
    declare_error(ENFILE),
#endif
#ifdef EMFILE
    declare_error(EMFILE),
#endif
#ifdef ENOTTY
    declare_error(ENOTTY),
#endif
#ifdef ETXTBSY
    declare_error(ETXTBSY),
#endif
#ifdef EFBIG
    declare_error(EFBIG),
#endif
#ifdef ENOSPC
    declare_error(ENOSPC),
#endif
#ifdef ESPIPE
    declare_error(ESPIPE),
#endif
#ifdef EROFS
    declare_error(EROFS),
#endif
#ifdef EMLINK
    declare_error(EMLINK),
#endif
#ifdef EPIPE
    declare_error(EPIPE),
#endif
#ifdef EDOM
    declare_error(EDOM),
#endif
#ifdef ERANGE
    declare_error(ERANGE),
#endif
#ifdef ENOMSG
    declare_error(ENOMSG),
#endif
#ifdef EIDRM
    declare_error(EIDRM),
#endif
#ifdef ECHRNG
    declare_error(ECHRNG),
#endif
#ifdef EL2NSYNC
    declare_error(EL2NSYNC),
#endif
#ifdef EL3HLT
    declare_error(EL3HLT),
#endif
#ifdef EL3RST
    declare_error(EL3RST),
#endif
#ifdef ELNRNG
    declare_error(ELNRNG),
#endif
#ifdef EUNATCH
    declare_error(EUNATCH),
#endif
#ifdef ENOCSI
    declare_error(ENOCSI),
#endif
#ifdef EL2HLT
    declare_error(EL2HLT),
#endif
#ifdef EDEADLK
    declare_error(EDEADLK),
#endif
#ifdef ENOLCK
    declare_error(ENOLCK),
#endif
#ifdef EBADE
    declare_error(EBADE),
#endif
#ifdef EBADR
    declare_error(EBADR),
#endif
#ifdef EXFULL
    declare_error(EXFULL),
#endif
#ifdef ENOANO
    declare_error(ENOANO),
#endif
#ifdef EBADRQC
    declare_error(EBADRQC),
#endif
#ifdef EBADSLT
    declare_error(EBADSLT),
#endif
#ifdef EDEADLOCK
    declare_error(EDEADLOCK),
#endif
#ifdef EBFONT
    declare_error(EBFONT),
#endif
#ifdef ENOSTR
    declare_error(ENOSTR),
#endif
#ifdef ENODATA
    declare_error(ENODATA),
#endif
#ifdef ETIME
    declare_error(ETIME),
#endif
#ifdef ENOSR
    declare_error(ENOSR),
#endif
#ifdef ENONET
    declare_error(ENONET),
#endif
#ifdef ENOPKG
    declare_error(ENOPKG),
#endif
#ifdef EREMOTE
    declare_error(EREMOTE),
#endif
#ifdef ENOLINK
    declare_error(ENOLINK),
#endif
#ifdef EADV
    declare_error(EADV),
#endif
#ifdef ESRMNT
    declare_error(ESRMNT),
#endif
#ifdef ECOMM
    declare_error(ECOMM),
#endif
#ifdef EPROTO
    declare_error(EPROTO),
#endif
#ifdef EMULTIHOP
    declare_error(EMULTIHOP),
#endif
#ifdef ELBIN
    declare_error(ELBIN),
#endif
#ifdef EDOTDOT
    declare_error(EDOTDOT),
#endif
#ifdef EBADMSG
    declare_error(EBADMSG),
#endif
#ifdef EFTYPE
    declare_error(EFTYPE),
#endif
#ifdef ENOTUNIQ
    declare_error(ENOTUNIQ),
#endif
#ifdef EBADFD
    declare_error(EBADFD),
#endif
#ifdef EREMCHG
    declare_error(EREMCHG),
#endif
#ifdef ELIBACC
    declare_error(ELIBACC),
#endif
#ifdef ELIBBAD
    declare_error(ELIBBAD),
#endif
#ifdef ELIBSCN
    declare_error(ELIBSCN),
#endif
#ifdef ELIBMAX
    declare_error(ELIBMAX),
#endif
#ifdef ELIBEXEC
    declare_error(ELIBEXEC),
#endif
#ifdef ENOSYS
    declare_error(ENOSYS),
#endif
#ifdef ENOTEMPTY
    declare_error(ENOTEMPTY),
#endif
#ifdef ENAMETOOLONG
    declare_error(ENAMETOOLONG),
#endif
#ifdef ELOOP
    declare_error(ELOOP),
#endif
#ifdef EOPNOTSUPP
    declare_error(EOPNOTSUPP),
#endif
#ifdef EPFNOSUPPORT
    declare_error(EPFNOSUPPORT),
#endif
#ifdef ECONNRESET
    declare_error(ECONNRESET),
#endif
#ifdef ENOBUFS
    declare_error(ENOBUFS),
#endif
#ifdef EAFNOSUPPORT
    declare_error(EAFNOSUPPORT),
#endif
#ifdef EPROTOTYPE
    declare_error(EPROTOTYPE),
#endif
#ifdef ENOTSOCK
    declare_error(ENOTSOCK),
#endif
#ifdef ENOPROTOOPT
    declare_error(ENOPROTOOPT),
#endif
#ifdef ESHUTDOWN
    declare_error(ESHUTDOWN),
#endif
#ifdef ECONNREFUSED
    declare_error(ECONNREFUSED),
#endif
#ifdef EADDRINUSE
    declare_error(EADDRINUSE),
#endif
#ifdef ECONNABORTED
    declare_error(ECONNABORTED),
#endif
#ifdef ENETUNREACH
    declare_error(ENETUNREACH),
#endif
#ifdef ENETDOWN
    declare_error(ENETDOWN),
#endif
#ifdef ETIMEDOUT
    declare_error(ETIMEDOUT),
#endif
#ifdef EHOSTDOWN
    declare_error(EHOSTDOWN),
#endif
#ifdef EHOSTUNREACH
    declare_error(EHOSTUNREACH),
#endif
#ifdef EINPROGRESS
    declare_error(EINPROGRESS),
#endif
#ifdef EALREADY
    declare_error(EALREADY),
#endif
#ifdef EDESTADDRREQ
    declare_error(EDESTADDRREQ),
#endif
#ifdef EMSGSIZE
    declare_error(EMSGSIZE),
#endif
#ifdef EPROTONOSUPPORT
    declare_error(EPROTONOSUPPORT),
#endif
#ifdef ESOCKTNOSUPPORT
    declare_error(ESOCKTNOSUPPORT),
#endif
#ifdef EADDRNOTAVAIL
    declare_error(EADDRNOTAVAIL),
#endif
#ifdef ENETRESET
    declare_error(ENETRESET),
#endif
#ifdef EISCONN
    declare_error(EISCONN),
#endif
#ifdef ENOTCONN
    declare_error(ENOTCONN),
#endif
#ifdef ETOOMANYREFS
    declare_error(ETOOMANYREFS),
#endif
#ifdef EPROCLIM
    declare_error(EPROCLIM),
#endif
#ifdef EUSERS
    declare_error(EUSERS),
#endif
#ifdef EDQUOT
    declare_error(EDQUOT),
#endif
#ifdef ESTALE
    declare_error(ESTALE),
#endif
#ifdef ENOTSUP
    declare_error(ENOTSUP),
#endif
#ifdef ENOMEDIUM
    declare_error(ENOMEDIUM),
#endif
#ifdef EILSEQ
    declare_error(EILSEQ),
#endif
#ifdef EOVERFLOW
    declare_error(EOVERFLOW),
#endif
#ifdef ECANCELED
    declare_error(ECANCELED),
#endif
#ifdef ENOTRECOVERABLE
    declare_error(ENOTRECOVERABLE),
#endif
#ifdef EOWNERDEAD
    declare_error(EOWNERDEAD),
#endif
#ifdef ESTRPIPE
    declare_error(ESTRPIPE),
#endif
#ifdef EHWPOISON
    declare_error(EHWPOISON),
#endif
#ifdef EISNAM
    declare_error(EISNAM),
#endif
#ifdef EKEYEXPIRED
    declare_error(EKEYEXPIRED),
#endif
#ifdef EKEYREJECTED
    declare_error(EKEYREJECTED),
#endif
#ifdef EKEYREVOKED
    declare_error(EKEYREVOKED),
#endif
#ifdef EWOULDBLOCK
    declare_error(EWOULDBLOCK),
#endif
};

#define NLINUX_ERRNO (sizeof(linux_errno) / sizeof(linux_errno[0]))

int
main(void)
{
    int    max_errno = 0, e;
    size_t i;

    printf("\n");

    for (i = 0; i < NLINUX_ERRNO; i++) {
        printf("#define LINUX_%s %d\n", linux_errno[i].name, linux_errno[i].value);
        if (linux_errno[i].value > max_errno)
            max_errno = linux_errno[i].value;
    }

    printf("\n");

    /*
     * Generate an enumeration containing all of the picolibc errnos which
     * are present in the linux errno list
     */
    reset_has();

    printf("enum __errno {\n");
    for (i = 0; i < NLINUX_ERRNO; i++)
        if (!check_has(linux_errno[i].value))
            printf("    __%s = %s,\n", linux_errno[i].name, linux_errno[i].name);
    printf("} __attribute__((packed));\n\n");

    /*
     * Generate a mapping from linux errno values to picolibc
     * errno values
     */

    printf("static const enum __errno __errno_map[] = {\n");

    for (e = 0; e <= max_errno; e++) {
        for (i = 0; i < NLINUX_ERRNO; i++) {
            if (linux_errno[i].value == e)
                break;
        }
        if (i < NLINUX_ERRNO)
            printf("    [LINUX_%s] = __%s,\n", linux_errno[i].name, linux_errno[i].name);
        else
            printf("    [%d] = __EINVAL,\n", e);
    }
    printf("};\n\n");
    return 0;
}

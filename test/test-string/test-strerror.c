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
#include <string.h>
#include <stdio.h>

/*
 * Every error number <errno.h> defines, POSIX's and the others Linux
 * returns, has a message of its own, not the one for unknown numbers
 */

#define E(name) { name, #name }

static const struct {
    int         value;
    const char *name;
} errors[] = {
#ifdef E2BIG
    E(E2BIG),
#endif
#ifdef EACCES
    E(EACCES),
#endif
#ifdef EADDRINUSE
    E(EADDRINUSE),
#endif
#ifdef EADDRNOTAVAIL
    E(EADDRNOTAVAIL),
#endif
#ifdef EAFNOSUPPORT
    E(EAFNOSUPPORT),
#endif
#ifdef EAGAIN
    E(EAGAIN),
#endif
#ifdef EALREADY
    E(EALREADY),
#endif
#ifdef EBADF
    E(EBADF),
#endif
#ifdef EBADMSG
    E(EBADMSG),
#endif
#ifdef EBUSY
    E(EBUSY),
#endif
#ifdef ECANCELED
    E(ECANCELED),
#endif
#ifdef ECHILD
    E(ECHILD),
#endif
#ifdef ECONNABORTED
    E(ECONNABORTED),
#endif
#ifdef ECONNREFUSED
    E(ECONNREFUSED),
#endif
#ifdef ECONNRESET
    E(ECONNRESET),
#endif
#ifdef EDEADLK
    E(EDEADLK),
#endif
#ifdef EDESTADDRREQ
    E(EDESTADDRREQ),
#endif
#ifdef EDOM
    E(EDOM),
#endif
#ifdef EDQUOT
    E(EDQUOT),
#endif
#ifdef EEXIST
    E(EEXIST),
#endif
#ifdef EFAULT
    E(EFAULT),
#endif
#ifdef EFBIG
    E(EFBIG),
#endif
#ifdef EHOSTUNREACH
    E(EHOSTUNREACH),
#endif
#ifdef EIDRM
    E(EIDRM),
#endif
#ifdef EILSEQ
    E(EILSEQ),
#endif
#ifdef EINPROGRESS
    E(EINPROGRESS),
#endif
#ifdef EINTR
    E(EINTR),
#endif
#ifdef EINVAL
    E(EINVAL),
#endif
#ifdef EIO
    E(EIO),
#endif
#ifdef EISCONN
    E(EISCONN),
#endif
#ifdef EISDIR
    E(EISDIR),
#endif
#ifdef ELOOP
    E(ELOOP),
#endif
#ifdef EMFILE
    E(EMFILE),
#endif
#ifdef EMLINK
    E(EMLINK),
#endif
#ifdef EMSGSIZE
    E(EMSGSIZE),
#endif
#ifdef EMULTIHOP
    E(EMULTIHOP),
#endif
#ifdef ENAMETOOLONG
    E(ENAMETOOLONG),
#endif
#ifdef ENETDOWN
    E(ENETDOWN),
#endif
#ifdef ENETRESET
    E(ENETRESET),
#endif
#ifdef ENETUNREACH
    E(ENETUNREACH),
#endif
#ifdef ENFILE
    E(ENFILE),
#endif
#ifdef ENOBUFS
    E(ENOBUFS),
#endif
#ifdef ENODEV
    E(ENODEV),
#endif
#ifdef ENOENT
    E(ENOENT),
#endif
#ifdef ENOEXEC
    E(ENOEXEC),
#endif
#ifdef ENOLCK
    E(ENOLCK),
#endif
#ifdef ENOLINK
    E(ENOLINK),
#endif
#ifdef ENOMEM
    E(ENOMEM),
#endif
#ifdef ENOMSG
    E(ENOMSG),
#endif
#ifdef ENOPROTOOPT
    E(ENOPROTOOPT),
#endif
#ifdef ENOSPC
    E(ENOSPC),
#endif
#ifdef ENOSYS
    E(ENOSYS),
#endif
#ifdef ENOTCONN
    E(ENOTCONN),
#endif
#ifdef ENOTDIR
    E(ENOTDIR),
#endif
#ifdef ENOTEMPTY
    E(ENOTEMPTY),
#endif
#ifdef ENOTRECOVERABLE
    E(ENOTRECOVERABLE),
#endif
#ifdef ENOTSOCK
    E(ENOTSOCK),
#endif
#ifdef ENOTSUP
    E(ENOTSUP),
#endif
#ifdef ENOTTY
    E(ENOTTY),
#endif
#ifdef ENXIO
    E(ENXIO),
#endif
#ifdef EOPNOTSUPP
    E(EOPNOTSUPP),
#endif
#ifdef EOVERFLOW
    E(EOVERFLOW),
#endif
#ifdef EOWNERDEAD
    E(EOWNERDEAD),
#endif
#ifdef EPERM
    E(EPERM),
#endif
#ifdef EPIPE
    E(EPIPE),
#endif
#ifdef EPROTO
    E(EPROTO),
#endif
#ifdef EPROTONOSUPPORT
    E(EPROTONOSUPPORT),
#endif
#ifdef EPROTOTYPE
    E(EPROTOTYPE),
#endif
#ifdef ERANGE
    E(ERANGE),
#endif
#ifdef EROFS
    E(EROFS),
#endif
#ifdef ESOCKTNOSUPPORT
    E(ESOCKTNOSUPPORT),
#endif
#ifdef ESPIPE
    E(ESPIPE),
#endif
#ifdef ESRCH
    E(ESRCH),
#endif
#ifdef ESTALE
    E(ESTALE),
#endif
#ifdef ETIMEDOUT
    E(ETIMEDOUT),
#endif
#ifdef ETXTBSY
    E(ETXTBSY),
#endif
#ifdef EWOULDBLOCK
    E(EWOULDBLOCK),
#endif
#ifdef EXDEV
    E(EXDEV),
#endif
#ifdef EADV
    E(EADV),
#endif
#ifdef EBADE
    E(EBADE),
#endif
#ifdef EBADFD
    E(EBADFD),
#endif
#ifdef EBADR
    E(EBADR),
#endif
#ifdef EBADRQC
    E(EBADRQC),
#endif
#ifdef EBADSLT
    E(EBADSLT),
#endif
#ifdef EBFONT
    E(EBFONT),
#endif
#ifdef ECHRNG
    E(ECHRNG),
#endif
#ifdef ECOMM
    E(ECOMM),
#endif
#ifdef EDEADLOCK
    E(EDEADLOCK),
#endif
#ifdef EDOTDOT
    E(EDOTDOT),
#endif
#ifdef EHOSTDOWN
    E(EHOSTDOWN),
#endif
#ifdef EHWPOISON
    E(EHWPOISON),
#endif
#ifdef EISNAM
    E(EISNAM),
#endif
#ifdef EKEYEXPIRED
    E(EKEYEXPIRED),
#endif
#ifdef EKEYREJECTED
    E(EKEYREJECTED),
#endif
#ifdef EKEYREVOKED
    E(EKEYREVOKED),
#endif
#ifdef EL2HLT
    E(EL2HLT),
#endif
#ifdef EL2NSYNC
    E(EL2NSYNC),
#endif
#ifdef EL3HLT
    E(EL3HLT),
#endif
#ifdef EL3RST
    E(EL3RST),
#endif
#ifdef ELIBACC
    E(ELIBACC),
#endif
#ifdef ELIBBAD
    E(ELIBBAD),
#endif
#ifdef ELIBEXEC
    E(ELIBEXEC),
#endif
#ifdef ELIBMAX
    E(ELIBMAX),
#endif
#ifdef ELIBSCN
    E(ELIBSCN),
#endif
#ifdef ELNRNG
    E(ELNRNG),
#endif
#ifdef EMEDIUMTYPE
    E(EMEDIUMTYPE),
#endif
#ifdef ENOANO
    E(ENOANO),
#endif
#ifdef ENOCSI
    E(ENOCSI),
#endif
#ifdef ENODATA
    E(ENODATA),
#endif
#ifdef ENOKEY
    E(ENOKEY),
#endif
#ifdef ENOMEDIUM
    E(ENOMEDIUM),
#endif
#ifdef ENONET
    E(ENONET),
#endif
#ifdef ENOPKG
    E(ENOPKG),
#endif
#ifdef ENOSR
    E(ENOSR),
#endif
#ifdef ENOSTR
    E(ENOSTR),
#endif
#ifdef ENOTBLK
    E(ENOTBLK),
#endif
#ifdef ENOTNAM
    E(ENOTNAM),
#endif
#ifdef ENOTUNIQ
    E(ENOTUNIQ),
#endif
#ifdef EPFNOSUPPORT
    E(EPFNOSUPPORT),
#endif
#ifdef EREMCHG
    E(EREMCHG),
#endif
#ifdef EREMOTE
    E(EREMOTE),
#endif
#ifdef EREMOTEIO
    E(EREMOTEIO),
#endif
#ifdef ERFKILL
    E(ERFKILL),
#endif
#ifdef ESHUTDOWN
    E(ESHUTDOWN),
#endif
#ifdef ESRMNT
    E(ESRMNT),
#endif
#ifdef ESTRPIPE
    E(ESTRPIPE),
#endif
#ifdef ETIME
    E(ETIME),
#endif
#ifdef ETOOMANYREFS
    E(ETOOMANYREFS),
#endif
#ifdef EUCLEAN
    E(EUCLEAN),
#endif
#ifdef EUNATCH
    E(EUNATCH),
#endif
#ifdef EUSERS
    E(EUSERS),
#endif
#ifdef EXFULL
    E(EXFULL),
#endif
};

#define N_ERRORS (sizeof(errors) / sizeof(errors[0]))

#define MSG_MAX  128

int
main(void)
{
    char   unknown[MSG_MAX];
    size_t i, j, len;
    int    ret = 0;

    if (!*strerror(-2)) {
        printf("strerror of invalid errno returns empty string\n");
        return 1;
    }

    /* What an unknown number looks like, up to any number in it */
    snprintf(unknown, sizeof(unknown), "%s", strerror(12345));
    for (len = 0; unknown[len]; len++)
        if (unknown[len] == '-' || (unknown[len] >= '0' && unknown[len] <= '9'))
            break;
    while (len > 0 && unknown[len - 1] == ' ')
        len--;
    unknown[len] = '\0';

    for (i = 0; i < N_ERRORS; i++) {
        const char *p = strerror(errors[i].value);

        if (p == NULL || *p == '\0' || (len && strncmp(p, unknown, len) == 0)) {
            printf("strerror(%s) is \"%s\"\n", errors[i].name, p ? p : "(null)");
            ret = 1;
        }
    }

    /* Each distinct value has its own message */
    for (i = 0; i < N_ERRORS; i++) {
        char msg[MSG_MAX];

        snprintf(msg, sizeof(msg), "%s", strerror(errors[i].value));
        for (j = i + 1; j < N_ERRORS; j++)
            if (errors[i].value != errors[j].value && strcmp(msg, strerror(errors[j].value)) == 0) {
                printf("%s and %s share the message \"%s\"\n", errors[i].name, errors[j].name, msg);
                ret = 1;
            }
    }
    return ret;
}

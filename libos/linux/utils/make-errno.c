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

int
main(void)
{
#ifdef EPERM
    if (!check_has(EPERM))
        printf("#define %-32.32s %d\n", "LINUX_EPERM", EPERM);
#endif
#ifdef ENOENT
    if (!check_has(ENOENT))
        printf("#define %-32.32s %d\n", "LINUX_ENOENT", ENOENT);
#endif
#ifdef ESRCH
    if (!check_has(ESRCH))
        printf("#define %-32.32s %d\n", "LINUX_ESRCH", ESRCH);
#endif
#ifdef EINTR
    if (!check_has(EINTR))
        printf("#define %-32.32s %d\n", "LINUX_EINTR", EINTR);
#endif
#ifdef EIO
    if (!check_has(EIO))
        printf("#define %-32.32s %d\n", "LINUX_EIO", EIO);
#endif
#ifdef ENXIO
    if (!check_has(ENXIO))
        printf("#define %-32.32s %d\n", "LINUX_ENXIO", ENXIO);
#endif
#ifdef E2BIG
    if (!check_has(E2BIG))
        printf("#define %-32.32s %d\n", "LINUX_E2BIG", E2BIG);
#endif
#ifdef ENOEXEC
    if (!check_has(ENOEXEC))
        printf("#define %-32.32s %d\n", "LINUX_ENOEXEC", ENOEXEC);
#endif
#ifdef EBADF
    if (!check_has(EBADF))
        printf("#define %-32.32s %d\n", "LINUX_EBADF", EBADF);
#endif
#ifdef ECHILD
    if (!check_has(ECHILD))
        printf("#define %-32.32s %d\n", "LINUX_ECHILD", ECHILD);
#endif
#ifdef EAGAIN
    if (!check_has(EAGAIN))
        printf("#define %-32.32s %d\n", "LINUX_EAGAIN", EAGAIN);
#endif
#ifdef ENOMEM
    if (!check_has(ENOMEM))
        printf("#define %-32.32s %d\n", "LINUX_ENOMEM", ENOMEM);
#endif
#ifdef EACCES
    if (!check_has(EACCES))
        printf("#define %-32.32s %d\n", "LINUX_EACCES", EACCES);
#endif
#ifdef EFAULT
    if (!check_has(EFAULT))
        printf("#define %-32.32s %d\n", "LINUX_EFAULT", EFAULT);
#endif
#ifdef ENOTBLK
    if (!check_has(ENOTBLK))
        printf("#define %-32.32s %d\n", "LINUX_ENOTBLK", ENOTBLK);
#endif
#ifdef EBUSY
    if (!check_has(EBUSY))
        printf("#define %-32.32s %d\n", "LINUX_EBUSY", EBUSY);
#endif
#ifdef EEXIST
    if (!check_has(EEXIST))
        printf("#define %-32.32s %d\n", "LINUX_EEXIST", EEXIST);
#endif
#ifdef EXDEV
    if (!check_has(EXDEV))
        printf("#define %-32.32s %d\n", "LINUX_EXDEV", EXDEV);
#endif
#ifdef ENODEV
    if (!check_has(ENODEV))
        printf("#define %-32.32s %d\n", "LINUX_ENODEV", ENODEV);
#endif
#ifdef ENOTDIR
    if (!check_has(ENOTDIR))
        printf("#define %-32.32s %d\n", "LINUX_ENOTDIR", ENOTDIR);
#endif
#ifdef EISDIR
    if (!check_has(EISDIR))
        printf("#define %-32.32s %d\n", "LINUX_EISDIR", EISDIR);
#endif
#ifdef EINVAL
    if (!check_has(EINVAL))
        printf("#define %-32.32s %d\n", "LINUX_EINVAL", EINVAL);
#endif
#ifdef ENFILE
    if (!check_has(ENFILE))
        printf("#define %-32.32s %d\n", "LINUX_ENFILE", ENFILE);
#endif
#ifdef EMFILE
    if (!check_has(EMFILE))
        printf("#define %-32.32s %d\n", "LINUX_EMFILE", EMFILE);
#endif
#ifdef ENOTTY
    if (!check_has(ENOTTY))
        printf("#define %-32.32s %d\n", "LINUX_ENOTTY", ENOTTY);
#endif
#ifdef ETXTBSY
    if (!check_has(ETXTBSY))
        printf("#define %-32.32s %d\n", "LINUX_ETXTBSY", ETXTBSY);
#endif
#ifdef EFBIG
    if (!check_has(EFBIG))
        printf("#define %-32.32s %d\n", "LINUX_EFBIG", EFBIG);
#endif
#ifdef ENOSPC
    if (!check_has(ENOSPC))
        printf("#define %-32.32s %d\n", "LINUX_ENOSPC", ENOSPC);
#endif
#ifdef ESPIPE
    if (!check_has(ESPIPE))
        printf("#define %-32.32s %d\n", "LINUX_ESPIPE", ESPIPE);
#endif
#ifdef EROFS
    if (!check_has(EROFS))
        printf("#define %-32.32s %d\n", "LINUX_EROFS", EROFS);
#endif
#ifdef EMLINK
    if (!check_has(EMLINK))
        printf("#define %-32.32s %d\n", "LINUX_EMLINK", EMLINK);
#endif
#ifdef EPIPE
    if (!check_has(EPIPE))
        printf("#define %-32.32s %d\n", "LINUX_EPIPE", EPIPE);
#endif
#ifdef EDOM
    if (!check_has(EDOM))
        printf("#define %-32.32s %d\n", "LINUX_EDOM", EDOM);
#endif
#ifdef ERANGE
    if (!check_has(ERANGE))
        printf("#define %-32.32s %d\n", "LINUX_ERANGE", ERANGE);
#endif
#ifdef ENOMSG
    if (!check_has(ENOMSG))
        printf("#define %-32.32s %d\n", "LINUX_ENOMSG", ENOMSG);
#endif
#ifdef EIDRM
    if (!check_has(EIDRM))
        printf("#define %-32.32s %d\n", "LINUX_EIDRM", EIDRM);
#endif
#ifdef ECHRNG
    if (!check_has(ECHRNG))
        printf("#define %-32.32s %d\n", "LINUX_ECHRNG", ECHRNG);
#endif
#ifdef EL2NSYNC
    if (!check_has(EL2NSYNC))
        printf("#define %-32.32s %d\n", "LINUX_EL2NSYNC", EL2NSYNC);
#endif
#ifdef EL3HLT
    if (!check_has(EL3HLT))
        printf("#define %-32.32s %d\n", "LINUX_EL3HLT", EL3HLT);
#endif
#ifdef EL3RST
    if (!check_has(EL3RST))
        printf("#define %-32.32s %d\n", "LINUX_EL3RST", EL3RST);
#endif
#ifdef ELNRNG
    if (!check_has(ELNRNG))
        printf("#define %-32.32s %d\n", "LINUX_ELNRNG", ELNRNG);
#endif
#ifdef EUNATCH
    if (!check_has(EUNATCH))
        printf("#define %-32.32s %d\n", "LINUX_EUNATCH", EUNATCH);
#endif
#ifdef ENOCSI
    if (!check_has(ENOCSI))
        printf("#define %-32.32s %d\n", "LINUX_ENOCSI", ENOCSI);
#endif
#ifdef EL2HLT
    if (!check_has(EL2HLT))
        printf("#define %-32.32s %d\n", "LINUX_EL2HLT", EL2HLT);
#endif
#ifdef EDEADLK
    if (!check_has(EDEADLK))
        printf("#define %-32.32s %d\n", "LINUX_EDEADLK", EDEADLK);
#endif
#ifdef ENOLCK
    if (!check_has(ENOLCK))
        printf("#define %-32.32s %d\n", "LINUX_ENOLCK", ENOLCK);
#endif
#ifdef EBADE
    if (!check_has(EBADE))
        printf("#define %-32.32s %d\n", "LINUX_EBADE", EBADE);
#endif
#ifdef EBADR
    if (!check_has(EBADR))
        printf("#define %-32.32s %d\n", "LINUX_EBADR", EBADR);
#endif
#ifdef EXFULL
    if (!check_has(EXFULL))
        printf("#define %-32.32s %d\n", "LINUX_EXFULL", EXFULL);
#endif
#ifdef ENOANO
    if (!check_has(ENOANO))
        printf("#define %-32.32s %d\n", "LINUX_ENOANO", ENOANO);
#endif
#ifdef EBADRQC
    if (!check_has(EBADRQC))
        printf("#define %-32.32s %d\n", "LINUX_EBADRQC", EBADRQC);
#endif
#ifdef EBADSLT
    if (!check_has(EBADSLT))
        printf("#define %-32.32s %d\n", "LINUX_EBADSLT", EBADSLT);
#endif
#ifdef EDEADLOCK
    if (!check_has(EDEADLOCK))
        printf("#define %-32.32s %d\n", "LINUX_EDEADLOCK", EDEADLOCK);
#endif
#ifdef EBFONT
    if (!check_has(EBFONT))
        printf("#define %-32.32s %d\n", "LINUX_EBFONT", EBFONT);
#endif
#ifdef ENOSTR
    if (!check_has(ENOSTR))
        printf("#define %-32.32s %d\n", "LINUX_ENOSTR", ENOSTR);
#endif
#ifdef ENODATA
    if (!check_has(ENODATA))
        printf("#define %-32.32s %d\n", "LINUX_ENODATA", ENODATA);
#endif
#ifdef ETIME
    if (!check_has(ETIME))
        printf("#define %-32.32s %d\n", "LINUX_ETIME", ETIME);
#endif
#ifdef ENOSR
    if (!check_has(ENOSR))
        printf("#define %-32.32s %d\n", "LINUX_ENOSR", ENOSR);
#endif
#ifdef ENONET
    if (!check_has(ENONET))
        printf("#define %-32.32s %d\n", "LINUX_ENONET", ENONET);
#endif
#ifdef ENOPKG
    if (!check_has(ENOPKG))
        printf("#define %-32.32s %d\n", "LINUX_ENOPKG", ENOPKG);
#endif
#ifdef EREMOTE
    if (!check_has(EREMOTE))
        printf("#define %-32.32s %d\n", "LINUX_EREMOTE", EREMOTE);
#endif
#ifdef ENOLINK
    if (!check_has(ENOLINK))
        printf("#define %-32.32s %d\n", "LINUX_ENOLINK", ENOLINK);
#endif
#ifdef EADV
    if (!check_has(EADV))
        printf("#define %-32.32s %d\n", "LINUX_EADV", EADV);
#endif
#ifdef ESRMNT
    if (!check_has(ESRMNT))
        printf("#define %-32.32s %d\n", "LINUX_ESRMNT", ESRMNT);
#endif
#ifdef ECOMM
    if (!check_has(ECOMM))
        printf("#define %-32.32s %d\n", "LINUX_ECOMM", ECOMM);
#endif
#ifdef EPROTO
    if (!check_has(EPROTO))
        printf("#define %-32.32s %d\n", "LINUX_EPROTO", EPROTO);
#endif
#ifdef EMULTIHOP
    if (!check_has(EMULTIHOP))
        printf("#define %-32.32s %d\n", "LINUX_EMULTIHOP", EMULTIHOP);
#endif
#ifdef ELBIN
    if (!check_has(ELBIN))
        printf("#define %-32.32s %d\n", "LINUX_ELBIN", ELBIN);
#endif
#ifdef EDOTDOT
    if (!check_has(EDOTDOT))
        printf("#define %-32.32s %d\n", "LINUX_EDOTDOT", EDOTDOT);
#endif
#ifdef EBADMSG
    if (!check_has(EBADMSG))
        printf("#define %-32.32s %d\n", "LINUX_EBADMSG", EBADMSG);
#endif
#ifdef EFTYPE
    if (!check_has(EFTYPE))
        printf("#define %-32.32s %d\n", "LINUX_EFTYPE", EFTYPE);
#endif
#ifdef ENOTUNIQ
    if (!check_has(ENOTUNIQ))
        printf("#define %-32.32s %d\n", "LINUX_ENOTUNIQ", ENOTUNIQ);
#endif
#ifdef EBADFD
    if (!check_has(EBADFD))
        printf("#define %-32.32s %d\n", "LINUX_EBADFD", EBADFD);
#endif
#ifdef EREMCHG
    if (!check_has(EREMCHG))
        printf("#define %-32.32s %d\n", "LINUX_EREMCHG", EREMCHG);
#endif
#ifdef ELIBACC
    if (!check_has(ELIBACC))
        printf("#define %-32.32s %d\n", "LINUX_ELIBACC", ELIBACC);
#endif
#ifdef ELIBBAD
    if (!check_has(ELIBBAD))
        printf("#define %-32.32s %d\n", "LINUX_ELIBBAD", ELIBBAD);
#endif
#ifdef ELIBSCN
    if (!check_has(ELIBSCN))
        printf("#define %-32.32s %d\n", "LINUX_ELIBSCN", ELIBSCN);
#endif
#ifdef ELIBMAX
    if (!check_has(ELIBMAX))
        printf("#define %-32.32s %d\n", "LINUX_ELIBMAX", ELIBMAX);
#endif
#ifdef ELIBEXEC
    if (!check_has(ELIBEXEC))
        printf("#define %-32.32s %d\n", "LINUX_ELIBEXEC", ELIBEXEC);
#endif
#ifdef ENOSYS
    if (!check_has(ENOSYS))
        printf("#define %-32.32s %d\n", "LINUX_ENOSYS", ENOSYS);
#endif
#ifdef ENOTEMPTY
    if (!check_has(ENOTEMPTY))
        printf("#define %-32.32s %d\n", "LINUX_ENOTEMPTY", ENOTEMPTY);
#endif
#ifdef ENAMETOOLONG
    if (!check_has(ENAMETOOLONG))
        printf("#define %-32.32s %d\n", "LINUX_ENAMETOOLONG", ENAMETOOLONG);
#endif
#ifdef ELOOP
    if (!check_has(ELOOP))
        printf("#define %-32.32s %d\n", "LINUX_ELOOP", ELOOP);
#endif
#ifdef EOPNOTSUPP
    if (!check_has(EOPNOTSUPP))
        printf("#define %-32.32s %d\n", "LINUX_EOPNOTSUPP", EOPNOTSUPP);
#endif
#ifdef EPFNOSUPPORT
    if (!check_has(EPFNOSUPPORT))
        printf("#define %-32.32s %d\n", "LINUX_EPFNOSUPPORT", EPFNOSUPPORT);
#endif
#ifdef ECONNRESET
    if (!check_has(ECONNRESET))
        printf("#define %-32.32s %d\n", "LINUX_ECONNRESET", ECONNRESET);
#endif
#ifdef ENOBUFS
    if (!check_has(ENOBUFS))
        printf("#define %-32.32s %d\n", "LINUX_ENOBUFS", ENOBUFS);
#endif
#ifdef EAFNOSUPPORT
    if (!check_has(EAFNOSUPPORT))
        printf("#define %-32.32s %d\n", "LINUX_EAFNOSUPPORT", EAFNOSUPPORT);
#endif
#ifdef EPROTOTYPE
    if (!check_has(EPROTOTYPE))
        printf("#define %-32.32s %d\n", "LINUX_EPROTOTYPE", EPROTOTYPE);
#endif
#ifdef ENOTSOCK
    if (!check_has(ENOTSOCK))
        printf("#define %-32.32s %d\n", "LINUX_ENOTSOCK", ENOTSOCK);
#endif
#ifdef ENOPROTOOPT
    if (!check_has(ENOPROTOOPT))
        printf("#define %-32.32s %d\n", "LINUX_ENOPROTOOPT", ENOPROTOOPT);
#endif
#ifdef ESHUTDOWN
    if (!check_has(ESHUTDOWN))
        printf("#define %-32.32s %d\n", "LINUX_ESHUTDOWN", ESHUTDOWN);
#endif
#ifdef ECONNREFUSED
    if (!check_has(ECONNREFUSED))
        printf("#define %-32.32s %d\n", "LINUX_ECONNREFUSED", ECONNREFUSED);
#endif
#ifdef EADDRINUSE
    if (!check_has(EADDRINUSE))
        printf("#define %-32.32s %d\n", "LINUX_EADDRINUSE", EADDRINUSE);
#endif
#ifdef ECONNABORTED
    if (!check_has(ECONNABORTED))
        printf("#define %-32.32s %d\n", "LINUX_ECONNABORTED", ECONNABORTED);
#endif
#ifdef ENETUNREACH
    if (!check_has(ENETUNREACH))
        printf("#define %-32.32s %d\n", "LINUX_ENETUNREACH", ENETUNREACH);
#endif
#ifdef ENETDOWN
    if (!check_has(ENETDOWN))
        printf("#define %-32.32s %d\n", "LINUX_ENETDOWN", ENETDOWN);
#endif
#ifdef ETIMEDOUT
    if (!check_has(ETIMEDOUT))
        printf("#define %-32.32s %d\n", "LINUX_ETIMEDOUT", ETIMEDOUT);
#endif
#ifdef EHOSTDOWN
    if (!check_has(EHOSTDOWN))
        printf("#define %-32.32s %d\n", "LINUX_EHOSTDOWN", EHOSTDOWN);
#endif
#ifdef EHOSTUNREACH
    if (!check_has(EHOSTUNREACH))
        printf("#define %-32.32s %d\n", "LINUX_EHOSTUNREACH", EHOSTUNREACH);
#endif
#ifdef EINPROGRESS
    if (!check_has(EINPROGRESS))
        printf("#define %-32.32s %d\n", "LINUX_EINPROGRESS", EINPROGRESS);
#endif
#ifdef EALREADY
    if (!check_has(EALREADY))
        printf("#define %-32.32s %d\n", "LINUX_EALREADY", EALREADY);
#endif
#ifdef EDESTADDRREQ
    if (!check_has(EDESTADDRREQ))
        printf("#define %-32.32s %d\n", "LINUX_EDESTADDRREQ", EDESTADDRREQ);
#endif
#ifdef EMSGSIZE
    if (!check_has(EMSGSIZE))
        printf("#define %-32.32s %d\n", "LINUX_EMSGSIZE", EMSGSIZE);
#endif
#ifdef EPROTONOSUPPORT
    if (!check_has(EPROTONOSUPPORT))
        printf("#define %-32.32s %d\n", "LINUX_EPROTONOSUPPORT", EPROTONOSUPPORT);
#endif
#ifdef ESOCKTNOSUPPORT
    if (!check_has(ESOCKTNOSUPPORT))
        printf("#define %-32.32s %d\n", "LINUX_ESOCKTNOSUPPORT", ESOCKTNOSUPPORT);
#endif
#ifdef EADDRNOTAVAIL
    if (!check_has(EADDRNOTAVAIL))
        printf("#define %-32.32s %d\n", "LINUX_EADDRNOTAVAIL", EADDRNOTAVAIL);
#endif
#ifdef ENETRESET
    if (!check_has(ENETRESET))
        printf("#define %-32.32s %d\n", "LINUX_ENETRESET", ENETRESET);
#endif
#ifdef EISCONN
    if (!check_has(EISCONN))
        printf("#define %-32.32s %d\n", "LINUX_EISCONN", EISCONN);
#endif
#ifdef ENOTCONN
    if (!check_has(ENOTCONN))
        printf("#define %-32.32s %d\n", "LINUX_ENOTCONN", ENOTCONN);
#endif
#ifdef ETOOMANYREFS
    if (!check_has(ETOOMANYREFS))
        printf("#define %-32.32s %d\n", "LINUX_ETOOMANYREFS", ETOOMANYREFS);
#endif
#ifdef EPROCLIM
    if (!check_has(EPROCLIM))
        printf("#define %-32.32s %d\n", "LINUX_EPROCLIM", EPROCLIM);
#endif
#ifdef EUSERS
    if (!check_has(EUSERS))
        printf("#define %-32.32s %d\n", "LINUX_EUSERS", EUSERS);
#endif
#ifdef EDQUOT
    if (!check_has(EDQUOT))
        printf("#define %-32.32s %d\n", "LINUX_EDQUOT", EDQUOT);
#endif
#ifdef ESTALE
    if (!check_has(ESTALE))
        printf("#define %-32.32s %d\n", "LINUX_ESTALE", ESTALE);
#endif
#ifdef ENOTSUP
    if (!check_has(ENOTSUP))
        printf("#define %-32.32s %d\n", "LINUX_ENOTSUP", ENOTSUP);
#endif
#ifdef ENOMEDIUM
    if (!check_has(ENOMEDIUM))
        printf("#define %-32.32s %d\n", "LINUX_ENOMEDIUM", ENOMEDIUM);
#endif
#ifdef EILSEQ
    if (!check_has(EILSEQ))
        printf("#define %-32.32s %d\n", "LINUX_EILSEQ", EILSEQ);
#endif
#ifdef EOVERFLOW
    if (!check_has(EOVERFLOW))
        printf("#define %-32.32s %d\n", "LINUX_EOVERFLOW", EOVERFLOW);
#endif
#ifdef ECANCELED
    if (!check_has(ECANCELED))
        printf("#define %-32.32s %d\n", "LINUX_ECANCELED", ECANCELED);
#endif
#ifdef ENOTRECOVERABLE
    if (!check_has(ENOTRECOVERABLE))
        printf("#define %-32.32s %d\n", "LINUX_ENOTRECOVERABLE", ENOTRECOVERABLE);
#endif
#ifdef EOWNERDEAD
    if (!check_has(EOWNERDEAD))
        printf("#define %-32.32s %d\n", "LINUX_EOWNERDEAD", EOWNERDEAD);
#endif
#ifdef ESTRPIPE
    if (!check_has(ESTRPIPE))
        printf("#define %-32.32s %d\n", "LINUX_ESTRPIPE", ESTRPIPE);
#endif
#ifdef EHWPOISON
    if (!check_has(EHWPOISON))
        printf("#define %-32.32s %d\n", "LINUX_EHWPOISON", EHWPOISON);
#endif
#ifdef EISNAM
    if (!check_has(EISNAM))
        printf("#define %-32.32s %d\n", "LINUX_EISNAM", EISNAM);
#endif
#ifdef EKEYEXPIRED
    if (!check_has(EKEYEXPIRED))
        printf("#define %-32.32s %d\n", "LINUX_EKEYEXPIRED", EKEYEXPIRED);
#endif
#ifdef EKEYREJECTED
    if (!check_has(EKEYREJECTED))
        printf("#define %-32.32s %d\n", "LINUX_EKEYREJECTED", EKEYREJECTED);
#endif
#ifdef EKEYREVOKED
    if (!check_has(EKEYREVOKED))
        printf("#define %-32.32s %d\n", "LINUX_EKEYREVOKED", EKEYREVOKED);
#endif
#ifdef EWOULDBLOCK
    if (!check_has(EWOULDBLOCK))
        printf("#define %-32.32s %d\n", "LINUX_EWOULDBLOCK", EWOULDBLOCK);
#endif

    printf("\n");
    reset_has();

    printf("enum __errno {\n");
#ifdef EPERM
    if (!check_has(EPERM))
        printf("    __%s = %s,\n", "EPERM", "EPERM");
#endif
#ifdef ENOENT
    if (!check_has(ENOENT))
        printf("    __%s = %s,\n", "ENOENT", "ENOENT");
#endif
#ifdef ESRCH
    if (!check_has(ESRCH))
        printf("    __%s = %s,\n", "ESRCH", "ESRCH");
#endif
#ifdef EINTR
    if (!check_has(EINTR))
        printf("    __%s = %s,\n", "EINTR", "EINTR");
#endif
#ifdef EIO
    if (!check_has(EIO))
        printf("    __%s = %s,\n", "EIO", "EIO");
#endif
#ifdef ENXIO
    if (!check_has(ENXIO))
        printf("    __%s = %s,\n", "ENXIO", "ENXIO");
#endif
#ifdef E2BIG
    if (!check_has(E2BIG))
        printf("    __%s = %s,\n", "E2BIG", "E2BIG");
#endif
#ifdef ENOEXEC
    if (!check_has(ENOEXEC))
        printf("    __%s = %s,\n", "ENOEXEC", "ENOEXEC");
#endif
#ifdef EBADF
    if (!check_has(EBADF))
        printf("    __%s = %s,\n", "EBADF", "EBADF");
#endif
#ifdef ECHILD
    if (!check_has(ECHILD))
        printf("    __%s = %s,\n", "ECHILD", "ECHILD");
#endif
#ifdef EAGAIN
    if (!check_has(EAGAIN))
        printf("    __%s = %s,\n", "EAGAIN", "EAGAIN");
#endif
#ifdef ENOMEM
    if (!check_has(ENOMEM))
        printf("    __%s = %s,\n", "ENOMEM", "ENOMEM");
#endif
#ifdef EACCES
    if (!check_has(EACCES))
        printf("    __%s = %s,\n", "EACCES", "EACCES");
#endif
#ifdef EFAULT
    if (!check_has(EFAULT))
        printf("    __%s = %s,\n", "EFAULT", "EFAULT");
#endif
#ifdef ENOTBLK
    if (!check_has(ENOTBLK))
        printf("    __%s = %s,\n", "ENOTBLK", "ENOTBLK");
#endif
#ifdef EBUSY
    if (!check_has(EBUSY))
        printf("    __%s = %s,\n", "EBUSY", "EBUSY");
#endif
#ifdef EEXIST
    if (!check_has(EEXIST))
        printf("    __%s = %s,\n", "EEXIST", "EEXIST");
#endif
#ifdef EXDEV
    if (!check_has(EXDEV))
        printf("    __%s = %s,\n", "EXDEV", "EXDEV");
#endif
#ifdef ENODEV
    if (!check_has(ENODEV))
        printf("    __%s = %s,\n", "ENODEV", "ENODEV");
#endif
#ifdef ENOTDIR
    if (!check_has(ENOTDIR))
        printf("    __%s = %s,\n", "ENOTDIR", "ENOTDIR");
#endif
#ifdef EISDIR
    if (!check_has(EISDIR))
        printf("    __%s = %s,\n", "EISDIR", "EISDIR");
#endif
#ifdef EINVAL
    if (!check_has(EINVAL))
        printf("    __%s = %s,\n", "EINVAL", "EINVAL");
#endif
#ifdef ENFILE
    if (!check_has(ENFILE))
        printf("    __%s = %s,\n", "ENFILE", "ENFILE");
#endif
#ifdef EMFILE
    if (!check_has(EMFILE))
        printf("    __%s = %s,\n", "EMFILE", "EMFILE");
#endif
#ifdef ENOTTY
    if (!check_has(ENOTTY))
        printf("    __%s = %s,\n", "ENOTTY", "ENOTTY");
#endif
#ifdef ETXTBSY
    if (!check_has(ETXTBSY))
        printf("    __%s = %s,\n", "ETXTBSY", "ETXTBSY");
#endif
#ifdef EFBIG
    if (!check_has(EFBIG))
        printf("    __%s = %s,\n", "EFBIG", "EFBIG");
#endif
#ifdef ENOSPC
    if (!check_has(ENOSPC))
        printf("    __%s = %s,\n", "ENOSPC", "ENOSPC");
#endif
#ifdef ESPIPE
    if (!check_has(ESPIPE))
        printf("    __%s = %s,\n", "ESPIPE", "ESPIPE");
#endif
#ifdef EROFS
    if (!check_has(EROFS))
        printf("    __%s = %s,\n", "EROFS", "EROFS");
#endif
#ifdef EMLINK
    if (!check_has(EMLINK))
        printf("    __%s = %s,\n", "EMLINK", "EMLINK");
#endif
#ifdef EPIPE
    if (!check_has(EPIPE))
        printf("    __%s = %s,\n", "EPIPE", "EPIPE");
#endif
#ifdef EDOM
    if (!check_has(EDOM))
        printf("    __%s = %s,\n", "EDOM", "EDOM");
#endif
#ifdef ERANGE
    if (!check_has(ERANGE))
        printf("    __%s = %s,\n", "ERANGE", "ERANGE");
#endif
#ifdef ENOMSG
    if (!check_has(ENOMSG))
        printf("    __%s = %s,\n", "ENOMSG", "ENOMSG");
#endif
#ifdef EIDRM
    if (!check_has(EIDRM))
        printf("    __%s = %s,\n", "EIDRM", "EIDRM");
#endif
#ifdef ECHRNG
    if (!check_has(ECHRNG))
        printf("    __%s = %s,\n", "ECHRNG", "ECHRNG");
#endif
#ifdef EL2NSYNC
    if (!check_has(EL2NSYNC))
        printf("    __%s = %s,\n", "EL2NSYNC", "EL2NSYNC");
#endif
#ifdef EL3HLT
    if (!check_has(EL3HLT))
        printf("    __%s = %s,\n", "EL3HLT", "EL3HLT");
#endif
#ifdef EL3RST
    if (!check_has(EL3RST))
        printf("    __%s = %s,\n", "EL3RST", "EL3RST");
#endif
#ifdef ELNRNG
    if (!check_has(ELNRNG))
        printf("    __%s = %s,\n", "ELNRNG", "ELNRNG");
#endif
#ifdef EUNATCH
    if (!check_has(EUNATCH))
        printf("    __%s = %s,\n", "EUNATCH", "EUNATCH");
#endif
#ifdef ENOCSI
    if (!check_has(ENOCSI))
        printf("    __%s = %s,\n", "ENOCSI", "ENOCSI");
#endif
#ifdef EL2HLT
    if (!check_has(EL2HLT))
        printf("    __%s = %s,\n", "EL2HLT", "EL2HLT");
#endif
#ifdef EDEADLK
    if (!check_has(EDEADLK))
        printf("    __%s = %s,\n", "EDEADLK", "EDEADLK");
#endif
#ifdef ENOLCK
    if (!check_has(ENOLCK))
        printf("    __%s = %s,\n", "ENOLCK", "ENOLCK");
#endif
#ifdef EBADE
    if (!check_has(EBADE))
        printf("    __%s = %s,\n", "EBADE", "EBADE");
#endif
#ifdef EBADR
    if (!check_has(EBADR))
        printf("    __%s = %s,\n", "EBADR", "EBADR");
#endif
#ifdef EXFULL
    if (!check_has(EXFULL))
        printf("    __%s = %s,\n", "EXFULL", "EXFULL");
#endif
#ifdef ENOANO
    if (!check_has(ENOANO))
        printf("    __%s = %s,\n", "ENOANO", "ENOANO");
#endif
#ifdef EBADRQC
    if (!check_has(EBADRQC))
        printf("    __%s = %s,\n", "EBADRQC", "EBADRQC");
#endif
#ifdef EBADSLT
    if (!check_has(EBADSLT))
        printf("    __%s = %s,\n", "EBADSLT", "EBADSLT");
#endif
#ifdef EDEADLOCK
    if (!check_has(EDEADLOCK))
        printf("    __%s = %s,\n", "EDEADLOCK", "EDEADLOCK");
#endif
#ifdef EBFONT
    if (!check_has(EBFONT))
        printf("    __%s = %s,\n", "EBFONT", "EBFONT");
#endif
#ifdef ENOSTR
    if (!check_has(ENOSTR))
        printf("    __%s = %s,\n", "ENOSTR", "ENOSTR");
#endif
#ifdef ENODATA
    if (!check_has(ENODATA))
        printf("    __%s = %s,\n", "ENODATA", "ENODATA");
#endif
#ifdef ETIME
    if (!check_has(ETIME))
        printf("    __%s = %s,\n", "ETIME", "ETIME");
#endif
#ifdef ENOSR
    if (!check_has(ENOSR))
        printf("    __%s = %s,\n", "ENOSR", "ENOSR");
#endif
#ifdef ENONET
    if (!check_has(ENONET))
        printf("    __%s = %s,\n", "ENONET", "ENONET");
#endif
#ifdef ENOPKG
    if (!check_has(ENOPKG))
        printf("    __%s = %s,\n", "ENOPKG", "ENOPKG");
#endif
#ifdef EREMOTE
    if (!check_has(EREMOTE))
        printf("    __%s = %s,\n", "EREMOTE", "EREMOTE");
#endif
#ifdef ENOLINK
    if (!check_has(ENOLINK))
        printf("    __%s = %s,\n", "ENOLINK", "ENOLINK");
#endif
#ifdef EADV
    if (!check_has(EADV))
        printf("    __%s = %s,\n", "EADV", "EADV");
#endif
#ifdef ESRMNT
    if (!check_has(ESRMNT))
        printf("    __%s = %s,\n", "ESRMNT", "ESRMNT");
#endif
#ifdef ECOMM
    if (!check_has(ECOMM))
        printf("    __%s = %s,\n", "ECOMM", "ECOMM");
#endif
#ifdef EPROTO
    if (!check_has(EPROTO))
        printf("    __%s = %s,\n", "EPROTO", "EPROTO");
#endif
#ifdef EMULTIHOP
    if (!check_has(EMULTIHOP))
        printf("    __%s = %s,\n", "EMULTIHOP", "EMULTIHOP");
#endif
#ifdef ELBIN
    if (!check_has(ELBIN))
        printf("    __%s = %s,\n", "ELBIN", "ELBIN");
#endif
#ifdef EDOTDOT
    if (!check_has(EDOTDOT))
        printf("    __%s = %s,\n", "EDOTDOT", "EDOTDOT");
#endif
#ifdef EBADMSG
    if (!check_has(EBADMSG))
        printf("    __%s = %s,\n", "EBADMSG", "EBADMSG");
#endif
#ifdef EFTYPE
    if (!check_has(EFTYPE))
        printf("    __%s = %s,\n", "EFTYPE", "EFTYPE");
#endif
#ifdef ENOTUNIQ
    if (!check_has(ENOTUNIQ))
        printf("    __%s = %s,\n", "ENOTUNIQ", "ENOTUNIQ");
#endif
#ifdef EBADFD
    if (!check_has(EBADFD))
        printf("    __%s = %s,\n", "EBADFD", "EBADFD");
#endif
#ifdef EREMCHG
    if (!check_has(EREMCHG))
        printf("    __%s = %s,\n", "EREMCHG", "EREMCHG");
#endif
#ifdef ELIBACC
    if (!check_has(ELIBACC))
        printf("    __%s = %s,\n", "ELIBACC", "ELIBACC");
#endif
#ifdef ELIBBAD
    if (!check_has(ELIBBAD))
        printf("    __%s = %s,\n", "ELIBBAD", "ELIBBAD");
#endif
#ifdef ELIBSCN
    if (!check_has(ELIBSCN))
        printf("    __%s = %s,\n", "ELIBSCN", "ELIBSCN");
#endif
#ifdef ELIBMAX
    if (!check_has(ELIBMAX))
        printf("    __%s = %s,\n", "ELIBMAX", "ELIBMAX");
#endif
#ifdef ELIBEXEC
    if (!check_has(ELIBEXEC))
        printf("    __%s = %s,\n", "ELIBEXEC", "ELIBEXEC");
#endif
#ifdef ENOSYS
    if (!check_has(ENOSYS))
        printf("    __%s = %s,\n", "ENOSYS", "ENOSYS");
#endif
#ifdef ENOTEMPTY
    if (!check_has(ENOTEMPTY))
        printf("    __%s = %s,\n", "ENOTEMPTY", "ENOTEMPTY");
#endif
#ifdef ENAMETOOLONG
    if (!check_has(ENAMETOOLONG))
        printf("    __%s = %s,\n", "ENAMETOOLONG", "ENAMETOOLONG");
#endif
#ifdef ELOOP
    if (!check_has(ELOOP))
        printf("    __%s = %s,\n", "ELOOP", "ELOOP");
#endif
#ifdef EOPNOTSUPP
    if (!check_has(EOPNOTSUPP))
        printf("    __%s = %s,\n", "EOPNOTSUPP", "EOPNOTSUPP");
#endif
#ifdef EPFNOSUPPORT
    if (!check_has(EPFNOSUPPORT))
        printf("    __%s = %s,\n", "EPFNOSUPPORT", "EPFNOSUPPORT");
#endif
#ifdef ECONNRESET
    if (!check_has(ECONNRESET))
        printf("    __%s = %s,\n", "ECONNRESET", "ECONNRESET");
#endif
#ifdef ENOBUFS
    if (!check_has(ENOBUFS))
        printf("    __%s = %s,\n", "ENOBUFS", "ENOBUFS");
#endif
#ifdef EAFNOSUPPORT
    if (!check_has(EAFNOSUPPORT))
        printf("    __%s = %s,\n", "EAFNOSUPPORT", "EAFNOSUPPORT");
#endif
#ifdef EPROTOTYPE
    if (!check_has(EPROTOTYPE))
        printf("    __%s = %s,\n", "EPROTOTYPE", "EPROTOTYPE");
#endif
#ifdef ENOTSOCK
    if (!check_has(ENOTSOCK))
        printf("    __%s = %s,\n", "ENOTSOCK", "ENOTSOCK");
#endif
#ifdef ENOPROTOOPT
    if (!check_has(ENOPROTOOPT))
        printf("    __%s = %s,\n", "ENOPROTOOPT", "ENOPROTOOPT");
#endif
#ifdef ESHUTDOWN
    if (!check_has(ESHUTDOWN))
        printf("    __%s = %s,\n", "ESHUTDOWN", "ESHUTDOWN");
#endif
#ifdef ECONNREFUSED
    if (!check_has(ECONNREFUSED))
        printf("    __%s = %s,\n", "ECONNREFUSED", "ECONNREFUSED");
#endif
#ifdef EADDRINUSE
    if (!check_has(EADDRINUSE))
        printf("    __%s = %s,\n", "EADDRINUSE", "EADDRINUSE");
#endif
#ifdef ECONNABORTED
    if (!check_has(ECONNABORTED))
        printf("    __%s = %s,\n", "ECONNABORTED", "ECONNABORTED");
#endif
#ifdef ENETUNREACH
    if (!check_has(ENETUNREACH))
        printf("    __%s = %s,\n", "ENETUNREACH", "ENETUNREACH");
#endif
#ifdef ENETDOWN
    if (!check_has(ENETDOWN))
        printf("    __%s = %s,\n", "ENETDOWN", "ENETDOWN");
#endif
#ifdef ETIMEDOUT
    if (!check_has(ETIMEDOUT))
        printf("    __%s = %s,\n", "ETIMEDOUT", "ETIMEDOUT");
#endif
#ifdef EHOSTDOWN
    if (!check_has(EHOSTDOWN))
        printf("    __%s = %s,\n", "EHOSTDOWN", "EHOSTDOWN");
#endif
#ifdef EHOSTUNREACH
    if (!check_has(EHOSTUNREACH))
        printf("    __%s = %s,\n", "EHOSTUNREACH", "EHOSTUNREACH");
#endif
#ifdef EINPROGRESS
    if (!check_has(EINPROGRESS))
        printf("    __%s = %s,\n", "EINPROGRESS", "EINPROGRESS");
#endif
#ifdef EALREADY
    if (!check_has(EALREADY))
        printf("    __%s = %s,\n", "EALREADY", "EALREADY");
#endif
#ifdef EDESTADDRREQ
    if (!check_has(EDESTADDRREQ))
        printf("    __%s = %s,\n", "EDESTADDRREQ", "EDESTADDRREQ");
#endif
#ifdef EMSGSIZE
    if (!check_has(EMSGSIZE))
        printf("    __%s = %s,\n", "EMSGSIZE", "EMSGSIZE");
#endif
#ifdef EPROTONOSUPPORT
    if (!check_has(EPROTONOSUPPORT))
        printf("    __%s = %s,\n", "EPROTONOSUPPORT", "EPROTONOSUPPORT");
#endif
#ifdef ESOCKTNOSUPPORT
    if (!check_has(ESOCKTNOSUPPORT))
        printf("    __%s = %s,\n", "ESOCKTNOSUPPORT", "ESOCKTNOSUPPORT");
#endif
#ifdef EADDRNOTAVAIL
    if (!check_has(EADDRNOTAVAIL))
        printf("    __%s = %s,\n", "EADDRNOTAVAIL", "EADDRNOTAVAIL");
#endif
#ifdef ENETRESET
    if (!check_has(ENETRESET))
        printf("    __%s = %s,\n", "ENETRESET", "ENETRESET");
#endif
#ifdef EISCONN
    if (!check_has(EISCONN))
        printf("    __%s = %s,\n", "EISCONN", "EISCONN");
#endif
#ifdef ENOTCONN
    if (!check_has(ENOTCONN))
        printf("    __%s = %s,\n", "ENOTCONN", "ENOTCONN");
#endif
#ifdef ETOOMANYREFS
    if (!check_has(ETOOMANYREFS))
        printf("    __%s = %s,\n", "ETOOMANYREFS", "ETOOMANYREFS");
#endif
#ifdef EPROCLIM
    if (!check_has(EPROCLIM))
        printf("    __%s = %s,\n", "EPROCLIM", "EPROCLIM");
#endif
#ifdef EUSERS
    if (!check_has(EUSERS))
        printf("    __%s = %s,\n", "EUSERS", "EUSERS");
#endif
#ifdef EDQUOT
    if (!check_has(EDQUOT))
        printf("    __%s = %s,\n", "EDQUOT", "EDQUOT");
#endif
#ifdef ESTALE
    if (!check_has(ESTALE))
        printf("    __%s = %s,\n", "ESTALE", "ESTALE");
#endif
#ifdef ENOTSUP
    if (!check_has(ENOTSUP))
        printf("    __%s = %s,\n", "ENOTSUP", "ENOTSUP");
#endif
#ifdef ENOMEDIUM
    if (!check_has(ENOMEDIUM))
        printf("    __%s = %s,\n", "ENOMEDIUM", "ENOMEDIUM");
#endif
#ifdef EILSEQ
    if (!check_has(EILSEQ))
        printf("    __%s = %s,\n", "EILSEQ", "EILSEQ");
#endif
#ifdef EOVERFLOW
    if (!check_has(EOVERFLOW))
        printf("    __%s = %s,\n", "EOVERFLOW", "EOVERFLOW");
#endif
#ifdef ECANCELED
    if (!check_has(ECANCELED))
        printf("    __%s = %s,\n", "ECANCELED", "ECANCELED");
#endif
#ifdef ENOTRECOVERABLE
    if (!check_has(ENOTRECOVERABLE))
        printf("    __%s = %s,\n", "ENOTRECOVERABLE", "ENOTRECOVERABLE");
#endif
#ifdef EOWNERDEAD
    if (!check_has(EOWNERDEAD))
        printf("    __%s = %s,\n", "EOWNERDEAD", "EOWNERDEAD");
#endif
#ifdef ESTRPIPE
    if (!check_has(ESTRPIPE))
        printf("    __%s = %s,\n", "ESTRPIPE", "ESTRPIPE");
#endif
#ifdef EHWPOISON
    if (!check_has(EHWPOISON))
        printf("    __%s = %s,\n", "EHWPOISON", "EHWPOISON");
#endif
#ifdef EISNAM
    if (!check_has(EISNAM))
        printf("    __%s = %s,\n", "EISNAM", "EISNAM");
#endif
#ifdef EKEYEXPIRED
    if (!check_has(EKEYEXPIRED))
        printf("    __%s = %s,\n", "EKEYEXPIRED", "EKEYEXPIRED");
#endif
#ifdef EKEYREJECTED
    if (!check_has(EKEYREJECTED))
        printf("    __%s = %s,\n", "EKEYREJECTED", "EKEYREJECTED");
#endif
#ifdef EKEYREVOKED
    if (!check_has(EKEYREVOKED))
        printf("    __%s = %s,\n", "EKEYREVOKED", "EKEYREVOKED");
#endif
#ifdef EWOULDBLOCK
    if (!check_has(EWOULDBLOCK))
        printf("    __%s = %s,\n", "EWOULDBLOCK", "EWOULDBLOCK");
#endif
    printf("} __attribute__ ((packed));\n");
    printf("\n");

    reset_has();

    printf("static const enum __errno __errno_map[] = {\n");
#ifdef EPERM
    if (!check_has(EPERM))
        printf("    [%s] = __%s,\n", "LINUX_EPERM", "EPERM");
#endif
#ifdef ENOENT
    if (!check_has(ENOENT))
        printf("    [%s] = __%s,\n", "LINUX_ENOENT", "ENOENT");
#endif
#ifdef ESRCH
    if (!check_has(ESRCH))
        printf("    [%s] = __%s,\n", "LINUX_ESRCH", "ESRCH");
#endif
#ifdef EINTR
    if (!check_has(EINTR))
        printf("    [%s] = __%s,\n", "LINUX_EINTR", "EINTR");
#endif
#ifdef EIO
    if (!check_has(EIO))
        printf("    [%s] = __%s,\n", "LINUX_EIO", "EIO");
#endif
#ifdef ENXIO
    if (!check_has(ENXIO))
        printf("    [%s] = __%s,\n", "LINUX_ENXIO", "ENXIO");
#endif
#ifdef E2BIG
    if (!check_has(E2BIG))
        printf("    [%s] = __%s,\n", "LINUX_E2BIG", "E2BIG");
#endif
#ifdef ENOEXEC
    if (!check_has(ENOEXEC))
        printf("    [%s] = __%s,\n", "LINUX_ENOEXEC", "ENOEXEC");
#endif
#ifdef EBADF
    if (!check_has(EBADF))
        printf("    [%s] = __%s,\n", "LINUX_EBADF", "EBADF");
#endif
#ifdef ECHILD
    if (!check_has(ECHILD))
        printf("    [%s] = __%s,\n", "LINUX_ECHILD", "ECHILD");
#endif
#ifdef EAGAIN
    if (!check_has(EAGAIN))
        printf("    [%s] = __%s,\n", "LINUX_EAGAIN", "EAGAIN");
#endif
#ifdef ENOMEM
    if (!check_has(ENOMEM))
        printf("    [%s] = __%s,\n", "LINUX_ENOMEM", "ENOMEM");
#endif
#ifdef EACCES
    if (!check_has(EACCES))
        printf("    [%s] = __%s,\n", "LINUX_EACCES", "EACCES");
#endif
#ifdef EFAULT
    if (!check_has(EFAULT))
        printf("    [%s] = __%s,\n", "LINUX_EFAULT", "EFAULT");
#endif
#ifdef ENOTBLK
    if (!check_has(ENOTBLK))
        printf("    [%s] = __%s,\n", "LINUX_ENOTBLK", "ENOTBLK");
#endif
#ifdef EBUSY
    if (!check_has(EBUSY))
        printf("    [%s] = __%s,\n", "LINUX_EBUSY", "EBUSY");
#endif
#ifdef EEXIST
    if (!check_has(EEXIST))
        printf("    [%s] = __%s,\n", "LINUX_EEXIST", "EEXIST");
#endif
#ifdef EXDEV
    if (!check_has(EXDEV))
        printf("    [%s] = __%s,\n", "LINUX_EXDEV", "EXDEV");
#endif
#ifdef ENODEV
    if (!check_has(ENODEV))
        printf("    [%s] = __%s,\n", "LINUX_ENODEV", "ENODEV");
#endif
#ifdef ENOTDIR
    if (!check_has(ENOTDIR))
        printf("    [%s] = __%s,\n", "LINUX_ENOTDIR", "ENOTDIR");
#endif
#ifdef EISDIR
    if (!check_has(EISDIR))
        printf("    [%s] = __%s,\n", "LINUX_EISDIR", "EISDIR");
#endif
#ifdef EINVAL
    if (!check_has(EINVAL))
        printf("    [%s] = __%s,\n", "LINUX_EINVAL", "EINVAL");
#endif
#ifdef ENFILE
    if (!check_has(ENFILE))
        printf("    [%s] = __%s,\n", "LINUX_ENFILE", "ENFILE");
#endif
#ifdef EMFILE
    if (!check_has(EMFILE))
        printf("    [%s] = __%s,\n", "LINUX_EMFILE", "EMFILE");
#endif
#ifdef ENOTTY
    if (!check_has(ENOTTY))
        printf("    [%s] = __%s,\n", "LINUX_ENOTTY", "ENOTTY");
#endif
#ifdef ETXTBSY
    if (!check_has(ETXTBSY))
        printf("    [%s] = __%s,\n", "LINUX_ETXTBSY", "ETXTBSY");
#endif
#ifdef EFBIG
    if (!check_has(EFBIG))
        printf("    [%s] = __%s,\n", "LINUX_EFBIG", "EFBIG");
#endif
#ifdef ENOSPC
    if (!check_has(ENOSPC))
        printf("    [%s] = __%s,\n", "LINUX_ENOSPC", "ENOSPC");
#endif
#ifdef ESPIPE
    if (!check_has(ESPIPE))
        printf("    [%s] = __%s,\n", "LINUX_ESPIPE", "ESPIPE");
#endif
#ifdef EROFS
    if (!check_has(EROFS))
        printf("    [%s] = __%s,\n", "LINUX_EROFS", "EROFS");
#endif
#ifdef EMLINK
    if (!check_has(EMLINK))
        printf("    [%s] = __%s,\n", "LINUX_EMLINK", "EMLINK");
#endif
#ifdef EPIPE
    if (!check_has(EPIPE))
        printf("    [%s] = __%s,\n", "LINUX_EPIPE", "EPIPE");
#endif
#ifdef EDOM
    if (!check_has(EDOM))
        printf("    [%s] = __%s,\n", "LINUX_EDOM", "EDOM");
#endif
#ifdef ERANGE
    if (!check_has(ERANGE))
        printf("    [%s] = __%s,\n", "LINUX_ERANGE", "ERANGE");
#endif
#ifdef ENOMSG
    if (!check_has(ENOMSG))
        printf("    [%s] = __%s,\n", "LINUX_ENOMSG", "ENOMSG");
#endif
#ifdef EIDRM
    if (!check_has(EIDRM))
        printf("    [%s] = __%s,\n", "LINUX_EIDRM", "EIDRM");
#endif
#ifdef ECHRNG
    if (!check_has(ECHRNG))
        printf("    [%s] = __%s,\n", "LINUX_ECHRNG", "ECHRNG");
#endif
#ifdef EL2NSYNC
    if (!check_has(EL2NSYNC))
        printf("    [%s] = __%s,\n", "LINUX_EL2NSYNC", "EL2NSYNC");
#endif
#ifdef EL3HLT
    if (!check_has(EL3HLT))
        printf("    [%s] = __%s,\n", "LINUX_EL3HLT", "EL3HLT");
#endif
#ifdef EL3RST
    if (!check_has(EL3RST))
        printf("    [%s] = __%s,\n", "LINUX_EL3RST", "EL3RST");
#endif
#ifdef ELNRNG
    if (!check_has(ELNRNG))
        printf("    [%s] = __%s,\n", "LINUX_ELNRNG", "ELNRNG");
#endif
#ifdef EUNATCH
    if (!check_has(EUNATCH))
        printf("    [%s] = __%s,\n", "LINUX_EUNATCH", "EUNATCH");
#endif
#ifdef ENOCSI
    if (!check_has(ENOCSI))
        printf("    [%s] = __%s,\n", "LINUX_ENOCSI", "ENOCSI");
#endif
#ifdef EL2HLT
    if (!check_has(EL2HLT))
        printf("    [%s] = __%s,\n", "LINUX_EL2HLT", "EL2HLT");
#endif
#ifdef EDEADLK
    if (!check_has(EDEADLK))
        printf("    [%s] = __%s,\n", "LINUX_EDEADLK", "EDEADLK");
#endif
#ifdef ENOLCK
    if (!check_has(ENOLCK))
        printf("    [%s] = __%s,\n", "LINUX_ENOLCK", "ENOLCK");
#endif
#ifdef EBADE
    if (!check_has(EBADE))
        printf("    [%s] = __%s,\n", "LINUX_EBADE", "EBADE");
#endif
#ifdef EBADR
    if (!check_has(EBADR))
        printf("    [%s] = __%s,\n", "LINUX_EBADR", "EBADR");
#endif
#ifdef EXFULL
    if (!check_has(EXFULL))
        printf("    [%s] = __%s,\n", "LINUX_EXFULL", "EXFULL");
#endif
#ifdef ENOANO
    if (!check_has(ENOANO))
        printf("    [%s] = __%s,\n", "LINUX_ENOANO", "ENOANO");
#endif
#ifdef EBADRQC
    if (!check_has(EBADRQC))
        printf("    [%s] = __%s,\n", "LINUX_EBADRQC", "EBADRQC");
#endif
#ifdef EBADSLT
    if (!check_has(EBADSLT))
        printf("    [%s] = __%s,\n", "LINUX_EBADSLT", "EBADSLT");
#endif
#ifdef EDEADLOCK
    if (!check_has(EDEADLOCK))
        printf("    [%s] = __%s,\n", "LINUX_EDEADLOCK", "EDEADLOCK");
#endif
#ifdef EBFONT
    if (!check_has(EBFONT))
        printf("    [%s] = __%s,\n", "LINUX_EBFONT", "EBFONT");
#endif
#ifdef ENOSTR
    if (!check_has(ENOSTR))
        printf("    [%s] = __%s,\n", "LINUX_ENOSTR", "ENOSTR");
#endif
#ifdef ENODATA
    if (!check_has(ENODATA))
        printf("    [%s] = __%s,\n", "LINUX_ENODATA", "ENODATA");
#endif
#ifdef ETIME
    if (!check_has(ETIME))
        printf("    [%s] = __%s,\n", "LINUX_ETIME", "ETIME");
#endif
#ifdef ENOSR
    if (!check_has(ENOSR))
        printf("    [%s] = __%s,\n", "LINUX_ENOSR", "ENOSR");
#endif
#ifdef ENONET
    if (!check_has(ENONET))
        printf("    [%s] = __%s,\n", "LINUX_ENONET", "ENONET");
#endif
#ifdef ENOPKG
    if (!check_has(ENOPKG))
        printf("    [%s] = __%s,\n", "LINUX_ENOPKG", "ENOPKG");
#endif
#ifdef EREMOTE
    if (!check_has(EREMOTE))
        printf("    [%s] = __%s,\n", "LINUX_EREMOTE", "EREMOTE");
#endif
#ifdef ENOLINK
    if (!check_has(ENOLINK))
        printf("    [%s] = __%s,\n", "LINUX_ENOLINK", "ENOLINK");
#endif
#ifdef EADV
    if (!check_has(EADV))
        printf("    [%s] = __%s,\n", "LINUX_EADV", "EADV");
#endif
#ifdef ESRMNT
    if (!check_has(ESRMNT))
        printf("    [%s] = __%s,\n", "LINUX_ESRMNT", "ESRMNT");
#endif
#ifdef ECOMM
    if (!check_has(ECOMM))
        printf("    [%s] = __%s,\n", "LINUX_ECOMM", "ECOMM");
#endif
#ifdef EPROTO
    if (!check_has(EPROTO))
        printf("    [%s] = __%s,\n", "LINUX_EPROTO", "EPROTO");
#endif
#ifdef EMULTIHOP
    if (!check_has(EMULTIHOP))
        printf("    [%s] = __%s,\n", "LINUX_EMULTIHOP", "EMULTIHOP");
#endif
#ifdef ELBIN
    if (!check_has(ELBIN))
        printf("    [%s] = __%s,\n", "LINUX_ELBIN", "ELBIN");
#endif
#ifdef EDOTDOT
    if (!check_has(EDOTDOT))
        printf("    [%s] = __%s,\n", "LINUX_EDOTDOT", "EDOTDOT");
#endif
#ifdef EBADMSG
    if (!check_has(EBADMSG))
        printf("    [%s] = __%s,\n", "LINUX_EBADMSG", "EBADMSG");
#endif
#ifdef EFTYPE
    if (!check_has(EFTYPE))
        printf("    [%s] = __%s,\n", "LINUX_EFTYPE", "EFTYPE");
#endif
#ifdef ENOTUNIQ
    if (!check_has(ENOTUNIQ))
        printf("    [%s] = __%s,\n", "LINUX_ENOTUNIQ", "ENOTUNIQ");
#endif
#ifdef EBADFD
    if (!check_has(EBADFD))
        printf("    [%s] = __%s,\n", "LINUX_EBADFD", "EBADFD");
#endif
#ifdef EREMCHG
    if (!check_has(EREMCHG))
        printf("    [%s] = __%s,\n", "LINUX_EREMCHG", "EREMCHG");
#endif
#ifdef ELIBACC
    if (!check_has(ELIBACC))
        printf("    [%s] = __%s,\n", "LINUX_ELIBACC", "ELIBACC");
#endif
#ifdef ELIBBAD
    if (!check_has(ELIBBAD))
        printf("    [%s] = __%s,\n", "LINUX_ELIBBAD", "ELIBBAD");
#endif
#ifdef ELIBSCN
    if (!check_has(ELIBSCN))
        printf("    [%s] = __%s,\n", "LINUX_ELIBSCN", "ELIBSCN");
#endif
#ifdef ELIBMAX
    if (!check_has(ELIBMAX))
        printf("    [%s] = __%s,\n", "LINUX_ELIBMAX", "ELIBMAX");
#endif
#ifdef ELIBEXEC
    if (!check_has(ELIBEXEC))
        printf("    [%s] = __%s,\n", "LINUX_ELIBEXEC", "ELIBEXEC");
#endif
#ifdef ENOSYS
    if (!check_has(ENOSYS))
        printf("    [%s] = __%s,\n", "LINUX_ENOSYS", "ENOSYS");
#endif
#ifdef ENOTEMPTY
    if (!check_has(ENOTEMPTY))
        printf("    [%s] = __%s,\n", "LINUX_ENOTEMPTY", "ENOTEMPTY");
#endif
#ifdef ENAMETOOLONG
    if (!check_has(ENAMETOOLONG))
        printf("    [%s] = __%s,\n", "LINUX_ENAMETOOLONG", "ENAMETOOLONG");
#endif
#ifdef ELOOP
    if (!check_has(ELOOP))
        printf("    [%s] = __%s,\n", "LINUX_ELOOP", "ELOOP");
#endif
#ifdef EOPNOTSUPP
    if (!check_has(EOPNOTSUPP))
        printf("    [%s] = __%s,\n", "LINUX_EOPNOTSUPP", "EOPNOTSUPP");
#endif
#ifdef EPFNOSUPPORT
    if (!check_has(EPFNOSUPPORT))
        printf("    [%s] = __%s,\n", "LINUX_EPFNOSUPPORT", "EPFNOSUPPORT");
#endif
#ifdef ECONNRESET
    if (!check_has(ECONNRESET))
        printf("    [%s] = __%s,\n", "LINUX_ECONNRESET", "ECONNRESET");
#endif
#ifdef ENOBUFS
    if (!check_has(ENOBUFS))
        printf("    [%s] = __%s,\n", "LINUX_ENOBUFS", "ENOBUFS");
#endif
#ifdef EAFNOSUPPORT
    if (!check_has(EAFNOSUPPORT))
        printf("    [%s] = __%s,\n", "LINUX_EAFNOSUPPORT", "EAFNOSUPPORT");
#endif
#ifdef EPROTOTYPE
    if (!check_has(EPROTOTYPE))
        printf("    [%s] = __%s,\n", "LINUX_EPROTOTYPE", "EPROTOTYPE");
#endif
#ifdef ENOTSOCK
    if (!check_has(ENOTSOCK))
        printf("    [%s] = __%s,\n", "LINUX_ENOTSOCK", "ENOTSOCK");
#endif
#ifdef ENOPROTOOPT
    if (!check_has(ENOPROTOOPT))
        printf("    [%s] = __%s,\n", "LINUX_ENOPROTOOPT", "ENOPROTOOPT");
#endif
#ifdef ESHUTDOWN
    if (!check_has(ESHUTDOWN))
        printf("    [%s] = __%s,\n", "LINUX_ESHUTDOWN", "ESHUTDOWN");
#endif
#ifdef ECONNREFUSED
    if (!check_has(ECONNREFUSED))
        printf("    [%s] = __%s,\n", "LINUX_ECONNREFUSED", "ECONNREFUSED");
#endif
#ifdef EADDRINUSE
    if (!check_has(EADDRINUSE))
        printf("    [%s] = __%s,\n", "LINUX_EADDRINUSE", "EADDRINUSE");
#endif
#ifdef ECONNABORTED
    if (!check_has(ECONNABORTED))
        printf("    [%s] = __%s,\n", "LINUX_ECONNABORTED", "ECONNABORTED");
#endif
#ifdef ENETUNREACH
    if (!check_has(ENETUNREACH))
        printf("    [%s] = __%s,\n", "LINUX_ENETUNREACH", "ENETUNREACH");
#endif
#ifdef ENETDOWN
    if (!check_has(ENETDOWN))
        printf("    [%s] = __%s,\n", "LINUX_ENETDOWN", "ENETDOWN");
#endif
#ifdef ETIMEDOUT
    if (!check_has(ETIMEDOUT))
        printf("    [%s] = __%s,\n", "LINUX_ETIMEDOUT", "ETIMEDOUT");
#endif
#ifdef EHOSTDOWN
    if (!check_has(EHOSTDOWN))
        printf("    [%s] = __%s,\n", "LINUX_EHOSTDOWN", "EHOSTDOWN");
#endif
#ifdef EHOSTUNREACH
    if (!check_has(EHOSTUNREACH))
        printf("    [%s] = __%s,\n", "LINUX_EHOSTUNREACH", "EHOSTUNREACH");
#endif
#ifdef EINPROGRESS
    if (!check_has(EINPROGRESS))
        printf("    [%s] = __%s,\n", "LINUX_EINPROGRESS", "EINPROGRESS");
#endif
#ifdef EALREADY
    if (!check_has(EALREADY))
        printf("    [%s] = __%s,\n", "LINUX_EALREADY", "EALREADY");
#endif
#ifdef EDESTADDRREQ
    if (!check_has(EDESTADDRREQ))
        printf("    [%s] = __%s,\n", "LINUX_EDESTADDRREQ", "EDESTADDRREQ");
#endif
#ifdef EMSGSIZE
    if (!check_has(EMSGSIZE))
        printf("    [%s] = __%s,\n", "LINUX_EMSGSIZE", "EMSGSIZE");
#endif
#ifdef EPROTONOSUPPORT
    if (!check_has(EPROTONOSUPPORT))
        printf("    [%s] = __%s,\n", "LINUX_EPROTONOSUPPORT", "EPROTONOSUPPORT");
#endif
#ifdef ESOCKTNOSUPPORT
    if (!check_has(ESOCKTNOSUPPORT))
        printf("    [%s] = __%s,\n", "LINUX_ESOCKTNOSUPPORT", "ESOCKTNOSUPPORT");
#endif
#ifdef EADDRNOTAVAIL
    if (!check_has(EADDRNOTAVAIL))
        printf("    [%s] = __%s,\n", "LINUX_EADDRNOTAVAIL", "EADDRNOTAVAIL");
#endif
#ifdef ENETRESET
    if (!check_has(ENETRESET))
        printf("    [%s] = __%s,\n", "LINUX_ENETRESET", "ENETRESET");
#endif
#ifdef EISCONN
    if (!check_has(EISCONN))
        printf("    [%s] = __%s,\n", "LINUX_EISCONN", "EISCONN");
#endif
#ifdef ENOTCONN
    if (!check_has(ENOTCONN))
        printf("    [%s] = __%s,\n", "LINUX_ENOTCONN", "ENOTCONN");
#endif
#ifdef ETOOMANYREFS
    if (!check_has(ETOOMANYREFS))
        printf("    [%s] = __%s,\n", "LINUX_ETOOMANYREFS", "ETOOMANYREFS");
#endif
#ifdef EPROCLIM
    if (!check_has(EPROCLIM))
        printf("    [%s] = __%s,\n", "LINUX_EPROCLIM", "EPROCLIM");
#endif
#ifdef EUSERS
    if (!check_has(EUSERS))
        printf("    [%s] = __%s,\n", "LINUX_EUSERS", "EUSERS");
#endif
#ifdef EDQUOT
    if (!check_has(EDQUOT))
        printf("    [%s] = __%s,\n", "LINUX_EDQUOT", "EDQUOT");
#endif
#ifdef ESTALE
    if (!check_has(ESTALE))
        printf("    [%s] = __%s,\n", "LINUX_ESTALE", "ESTALE");
#endif
#ifdef ENOTSUP
    if (!check_has(ENOTSUP))
        printf("    [%s] = __%s,\n", "LINUX_ENOTSUP", "ENOTSUP");
#endif
#ifdef ENOMEDIUM
    if (!check_has(ENOMEDIUM))
        printf("    [%s] = __%s,\n", "LINUX_ENOMEDIUM", "ENOMEDIUM");
#endif
#ifdef EILSEQ
    if (!check_has(EILSEQ))
        printf("    [%s] = __%s,\n", "LINUX_EILSEQ", "EILSEQ");
#endif
#ifdef EOVERFLOW
    if (!check_has(EOVERFLOW))
        printf("    [%s] = __%s,\n", "LINUX_EOVERFLOW", "EOVERFLOW");
#endif
#ifdef ECANCELED
    if (!check_has(ECANCELED))
        printf("    [%s] = __%s,\n", "LINUX_ECANCELED", "ECANCELED");
#endif
#ifdef ENOTRECOVERABLE
    if (!check_has(ENOTRECOVERABLE))
        printf("    [%s] = __%s,\n", "LINUX_ENOTRECOVERABLE", "ENOTRECOVERABLE");
#endif
#ifdef EOWNERDEAD
    if (!check_has(EOWNERDEAD))
        printf("    [%s] = __%s,\n", "LINUX_EOWNERDEAD", "EOWNERDEAD");
#endif
#ifdef ESTRPIPE
    if (!check_has(ESTRPIPE))
        printf("    [%s] = __%s,\n", "LINUX_ESTRPIPE", "ESTRPIPE");
#endif
#ifdef EHWPOISON
    if (!check_has(EHWPOISON))
        printf("    [%s] = __%s,\n", "LINUX_EHWPOISON", "EHWPOISON");
#endif
#ifdef EISNAM
    if (!check_has(EISNAM))
        printf("    [%s] = __%s,\n", "LINUX_EISNAM", "EISNAM");
#endif
#ifdef EKEYEXPIRED
    if (!check_has(EKEYEXPIRED))
        printf("    [%s] = __%s,\n", "LINUX_EKEYEXPIRED", "EKEYEXPIRED");
#endif
#ifdef EKEYREJECTED
    if (!check_has(EKEYREJECTED))
        printf("    [%s] = __%s,\n", "LINUX_EKEYREJECTED", "EKEYREJECTED");
#endif
#ifdef EKEYREVOKED
    if (!check_has(EKEYREVOKED))
        printf("    [%s] = __%s,\n", "LINUX_EKEYREVOKED", "EKEYREVOKED");
#endif
#ifdef EWOULDBLOCK
    if (!check_has(EWOULDBLOCK))
        printf("    [%s] = __%s,\n", "LINUX_EWOULDBLOCK", "EWOULDBLOCK");
#endif
    printf("};\n");
    printf("static inline int __map_errno(unsigned int linux_errno) {\n");
    printf("    if (linux_errno < sizeof(__errno_map)/sizeof(__errno_map[0]))\n");
    printf("        return __errno_map[linux_errno];\n");
    printf("    return EINVAL;\n");
    printf("}\n");
    return 0;
}

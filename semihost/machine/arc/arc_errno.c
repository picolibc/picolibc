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

#include "arc_semihost.h"

void
arc_semihost_errno(int def)
{
    int err = arc_semihost0(SYS_SEMIHOST_errno);

    switch (err) {
    case TARGET_ERRNO_EPERM: errno = EPERM; break;
    case TARGET_ERRNO_ENOENT: errno = ENOENT; break;
    case TARGET_ERRNO_ESRCH: errno = ESRCH; break;
    case TARGET_ERRNO_EINTR: errno = EINTR; break;
    case TARGET_ERRNO_EIO: errno = EIO; break;
    case TARGET_ERRNO_ENXIO: errno = ENXIO; break;
    case TARGET_ERRNO_E2BIG: errno = E2BIG; break;
    case TARGET_ERRNO_ENOEXEC: errno = ENOEXEC; break;
    case TARGET_ERRNO_EBADF: errno = EBADF; break;
    case TARGET_ERRNO_ECHILD: errno = ECHILD; break;
    case TARGET_ERRNO_EAGAIN: errno = EAGAIN; break;
    case TARGET_ERRNO_ENOMEM: errno = ENOMEM; break;
    case TARGET_ERRNO_EACCES: errno = EACCES; break;
    case TARGET_ERRNO_EFAULT: errno = EFAULT; break;
//    case TARGET_ERRNO_ENOTBLK: errno = ENOTBLK; break;
    case TARGET_ERRNO_EBUSY: errno = EBUSY; break;
    case TARGET_ERRNO_EEXIST: errno = EEXIST; break;
    case TARGET_ERRNO_EXDEV: errno = EXDEV; break;
    case TARGET_ERRNO_ENODEV: errno = ENODEV; break;
    case TARGET_ERRNO_ENOTDIR: errno = ENOTDIR; break;
    case TARGET_ERRNO_EISDIR: errno = EISDIR; break;
    case TARGET_ERRNO_EINVAL: errno = EINVAL; break;
    case TARGET_ERRNO_ENFILE: errno = ENFILE; break;
    case TARGET_ERRNO_EMFILE: errno = EMFILE; break;
    case TARGET_ERRNO_ENOTTY: errno = ENOTTY; break;
    case TARGET_ERRNO_ETXTBSY: errno = ETXTBSY; break;
    case TARGET_ERRNO_EFBIG: errno = EFBIG; break;
    case TARGET_ERRNO_ENOSPC: errno = ENOSPC; break;
    case TARGET_ERRNO_ESPIPE: errno = ESPIPE; break;
    case TARGET_ERRNO_EROFS: errno = EROFS; break;
    case TARGET_ERRNO_EMLINK: errno = EMLINK; break;
    case TARGET_ERRNO_EPIPE: errno = EPIPE; break;
    case TARGET_ERRNO_EDOM: errno = EDOM; break;
    case TARGET_ERRNO_ERANGE: errno = ERANGE; break;
    case TARGET_ERRNO_ENOMSG: errno = ENOMSG; break;
    case TARGET_ERRNO_EIDRM: errno = EIDRM; break;
//    case TARGET_ERRNO_ECHRNG: errno = ECHRNG; break;
//    case TARGET_ERRNO_EL2NSYNC: errno = EL2NSYNC; break;
//    case TARGET_ERRNO_EL3HLT: errno = EL3HLT; break;
//    case TARGET_ERRNO_EL3RST: errno = EL3RST; break;
//    case TARGET_ERRNO_ELNRNG: errno = ELNRNG; break;
//    case TARGET_ERRNO_EUNATCH: errno = EUNATCH; break;
//    case TARGET_ERRNO_ENOCSI: errno = ENOCSI; break;
//    case TARGET_ERRNO_EL2HLT: errno = EL2HLT; break;
    case TARGET_ERRNO_EDEADLK: errno = EDEADLK; break;
    case TARGET_ERRNO_ENOLCK: errno = ENOLCK; break;
//    case TARGET_ERRNO_EBADE: errno = EBADE; break;
//    case TARGET_ERRNO_EBADR: errno = EBADR; break;
//    case TARGET_ERRNO_EXFULL: errno = EXFULL; break;
//    case TARGET_ERRNO_ENOANO: errno = ENOANO; break;
//    case TARGET_ERRNO_EBADRQC: errno = EBADRQC; break;
//    case TARGET_ERRNO_EBADSLT: errno = EBADSLT; break;
//    case TARGET_ERRNO_EDEADLOCK: errno = EDEADLOCK; break;
//    case TARGET_ERRNO_EBFONT: errno = EBFONT; break;
    case TARGET_ERRNO_ENOSTR: errno = ENOSTR; break;
    case TARGET_ERRNO_ENODATA: errno = ENODATA; break;
    case TARGET_ERRNO_ETIME: errno = ETIME; break;
    case TARGET_ERRNO_ENOSR: errno = ENOSR; break;
//    case TARGET_ERRNO_ENONET: errno = ENONET; break;
//    case TARGET_ERRNO_ENOPKG: errno = ENOPKG; break;
//    case TARGET_ERRNO_EREMOTE: errno = EREMOTE; break;
    case TARGET_ERRNO_ENOLINK: errno = ENOLINK; break;
//    case TARGET_ERRNO_EADV: errno = EADV; break;
//    case TARGET_ERRNO_ESRMNT: errno = ESRMNT; break;
//    case TARGET_ERRNO_ECOMM: errno = ECOMM; break;
    case TARGET_ERRNO_EPROTO: errno = EPROTO; break;
    case TARGET_ERRNO_EMULTIHOP: errno = EMULTIHOP; break;
//    case TARGET_ERRNO_ELBIN: errno = ELBIN; break;
//    case TARGET_ERRNO_EDOTDOT: errno = EDOTDOT; break;
    case TARGET_ERRNO_EBADMSG: errno = EBADMSG; break;
//    case TARGET_ERRNO_EFTYPE: errno = EFTYPE; break;
//    case TARGET_ERRNO_ENOTUNIQ: errno = ENOTUNIQ; break;
//    case TARGET_ERRNO_EBADFD: errno = EBADFD; break;
//    case TARGET_ERRNO_EREMCHG: errno = EREMCHG; break;
//    case TARGET_ERRNO_ELIBACC: errno = ELIBACC; break;
//    case TARGET_ERRNO_ELIBBAD: errno = ELIBBAD; break;
//    case TARGET_ERRNO_ELIBSCN: errno = ELIBSCN; break;
//    case TARGET_ERRNO_ELIBMAX: errno = ELIBMAX; break;
//    case TARGET_ERRNO_ELIBEXEC: errno = ELIBEXEC; break;
    case TARGET_ERRNO_ENOSYS: errno = ENOSYS; break;
//    case TARGET_ERRNO_ENMFILE: errno = ENMFILE; break;
    case TARGET_ERRNO_ENOTEMPTY: errno = ENOTEMPTY; break;
    case TARGET_ERRNO_ENAMETOOLONG: errno = ENAMETOOLONG; break;
    case TARGET_ERRNO_ELOOP: errno = ELOOP; break;
    case TARGET_ERRNO_EOPNOTSUPP: errno = EOPNOTSUPP; break;
//    case TARGET_ERRNO_EPFNOSUPPORT: errno = EPFNOSUPPORT; break;
    case TARGET_ERRNO_ECONNRESET: errno = ECONNRESET; break;
    case TARGET_ERRNO_ENOBUFS: errno = ENOBUFS; break;
//    case TARGET_ERRNO_EAFNOSUPPORT: errno = EAFNOSUPPORT; break;
    case TARGET_ERRNO_EPROTOTYPE: errno = EPROTOTYPE; break;
    case TARGET_ERRNO_ENOTSOCK: errno = ENOTSOCK; break;
    case TARGET_ERRNO_ENOPROTOOPT: errno = ENOPROTOOPT; break;
//    case TARGET_ERRNO_ESHUTDOWN: errno = ESHUTDOWN; break;
    case TARGET_ERRNO_ECONNREFUSED: errno = ECONNREFUSED; break;
    case TARGET_ERRNO_EADDRINUSE: errno = EADDRINUSE; break;
    case TARGET_ERRNO_ECONNABORTED: errno = ECONNABORTED; break;
    case TARGET_ERRNO_ENETUNREACH: errno = ENETUNREACH; break;
    case TARGET_ERRNO_ENETDOWN: errno = ENETDOWN; break;
    case TARGET_ERRNO_ETIMEDOUT: errno = ETIMEDOUT; break;
//    case TARGET_ERRNO_EHOSTDOWN: errno = EHOSTDOWN; break;
    case TARGET_ERRNO_EHOSTUNREACH: errno = EHOSTUNREACH; break;
    case TARGET_ERRNO_EINPROGRESS: errno = EINPROGRESS; break;
    case TARGET_ERRNO_EALREADY: errno = EALREADY; break;
    case TARGET_ERRNO_EDESTADDRREQ: errno = EDESTADDRREQ; break;
    case TARGET_ERRNO_EMSGSIZE: errno = EMSGSIZE; break;
    case TARGET_ERRNO_EPROTONOSUPPORT: errno = EPROTONOSUPPORT; break;
//    case TARGET_ERRNO_ESOCKTNOSUPPORT: errno = ESOCKTNOSUPPORT; break;
    case TARGET_ERRNO_EADDRNOTAVAIL: errno = EADDRNOTAVAIL; break;
    case TARGET_ERRNO_ENETRESET: errno = ENETRESET; break;
    case TARGET_ERRNO_EISCONN: errno = EISCONN; break;
    case TARGET_ERRNO_ENOTCONN: errno = ENOTCONN; break;
//    case TARGET_ERRNO_ETOOMANYREFS: errno = ETOOMANYREFS; break;
//    case TARGET_ERRNO_EPROCLIM: errno = EPROCLIM; break;
//    case TARGET_ERRNO_EUSERS: errno = EUSERS; break;
    case TARGET_ERRNO_EDQUOT: errno = EDQUOT; break;
    case TARGET_ERRNO_ESTALE: errno = ESTALE; break;
    case TARGET_ERRNO_ENOTSUP: errno = ENOTSUP; break;
//    case TARGET_ERRNO_ENOMEDIUM: errno = ENOMEDIUM; break;
//    case TARGET_ERRNO_ENOSHARE: errno = ENOSHARE; break;
//    case TARGET_ERRNO_ECASECLASH: errno = ECASECLASH; break;
    case TARGET_ERRNO_EILSEQ: errno = EILSEQ; break;
    case TARGET_ERRNO_EOVERFLOW: errno = EOVERFLOW; break;
    case TARGET_ERRNO_ECANCELED: errno = ECANCELED; break;
    case TARGET_ERRNO_ENOTRECOVERABLE: errno = ENOTRECOVERABLE; break;
    case TARGET_ERRNO_EOWNERDEAD: errno = EOWNERDEAD; break;
//    case TARGET_ERRNO_ESTRPIPE: errno = ESTRPIPE; break;
//    case TARGET_ERRNO_EHWPOISON: errno = EHWPOISON; break;
//    case TARGET_ERRNO_EISNAM: errno = EISNAM; break;
//    case TARGET_ERRNO_EKEYEXPIRED: errno = EKEYEXPIRED; break;
//    case TARGET_ERRNO_EKEYREJECTED: errno = EKEYREJECTED; break;
//    case TARGET_ERRNO_EKEYREVOKED: errno = EKEYREVOKED; break;
    default: errno = def; break;
    }
}

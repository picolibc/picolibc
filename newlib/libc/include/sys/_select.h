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

#ifndef _SYS__SELECT_H
#define _SYS__SELECT_H

_BEGIN_STD_C

#  define _SYS_TYPES_FD_SET
/*
 * Select uses bit masks of file descriptors in longs.
 * These macros manipulate such bit fields (the filesystem macros use chars).
 * FD_SETSIZE may be defined by the user, but the default here
 * should be enough for most uses.
 */
#ifndef FD_SETSIZE
# if   defined(__rtems__)
#  define FD_SETSIZE	256
# else
#  define FD_SETSIZE	64
# endif
#endif

typedef unsigned long	__fd_mask;
#if __BSD_VISIBLE
typedef __fd_mask	fd_mask;
#endif

#define __NFDBITS	((int)sizeof(__fd_mask) * 8) /* bits per mask */
#define __FD_ARRAY_SIZE ((FD_SETSIZE + __NFDBITS - 1) / __NFDBITS)
#if __BSD_VISIBLE
#define NFDBITS		__NFDBITS
#endif

typedef	struct fd_set {
        __fd_mask	__fds_bits[__FD_ARRAY_SIZE];
} fd_set;

#define __fdset_mask(n)	((__fd_mask)1 << ((n) % __NFDBITS))
#define FD_CLR(n, p)	((p)->__fds_bits[(n)/__NFDBITS] &= ~__fdset_mask(n))
#if __BSD_VISIBLE
#define FD_COPY(f, t)	(void)(*(t) = *(f))
#endif
#define FD_ISSET(n, p)	(((p)->__fds_bits[(n)/__NFDBITS] & __fdset_mask(n)) != 0)
#define FD_SET(n, p)	((p)->__fds_bits[(n)/__NFDBITS] |= __fdset_mask(n))
#define FD_ZERO(p) do {                                                 \
                fd_set *__p = (p);                                      \
                __size_t __i;                                           \
                for (__i = 0; __i < __FD_ARRAY_SIZE; __i++)             \
                        __p->__fds_bits[__i] = 0;                       \
        } while (0)

int select (int __n, fd_set *__readfds, fd_set *__writefds,
            fd_set *__exceptfds, struct timeval *__timeout);

_END_STD_C

#endif /* sys/_select.h */

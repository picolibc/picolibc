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

#ifndef _SYS_SELECT_H
#define _SYS_SELECT_H

/* We don't define fd_set and friends if we are compiling POSIX
   source, or if we have included (or may include as indicated
   by __USE_W32_SOCKETS) the W32api winsock[2].h header which
   defines Windows versions of them.   Note that a program which
   includes the W32api winsock[2].h header must know what it is doing;
   it must not call the Cygwin select function.
*/
# if !(defined (_WINSOCK_H) || defined (_WINSOCKAPI_) || defined (__USE_W32_SOCKETS))

#include <sys/cdefs.h>
#include <sys/_sigset.h>
#include <sys/_timeval.h>
#include <sys/_timespec.h>

#if !defined(_SIGSET_T_DECLARED)
#define	_SIGSET_T_DECLARED
typedef	__sigset_t	sigset_t;
#endif

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

#define _NFDBITS	((int)sizeof(__fd_mask) * 8) /* bits per mask */
#if __BSD_VISIBLE
#define NFDBITS		_NFDBITS
#endif

#ifndef	_howmany
#define	_howmany(x,y)	(((x) + ((y) - 1)) / (y))
#endif

typedef	struct fd_set {
	__fd_mask	__fds_bits[_howmany(FD_SETSIZE, _NFDBITS)];
} fd_set;
#if __BSD_VISIBLE
#define fds_bits	__fds_bits
#endif

#define __fdset_mask(n)	((__fd_mask)1 << ((n) % _NFDBITS))
#define FD_CLR(n, p)	((p)->__fds_bits[(n)/_NFDBITS] &= ~__fdset_mask(n))
#if __BSD_VISIBLE
#define FD_COPY(f, t)	(void)(*(t) = *(f))
#endif
#define FD_ISSET(n, p)	(((p)->__fds_bits[(n)/_NFDBITS] & __fdset_mask(n)) != 0)
#define FD_SET(n, p)	((p)->__fds_bits[(n)/_NFDBITS] |= __fdset_mask(n))
#define FD_ZERO(p) do {					\
        fd_set *_p;					\
        __size_t _n;					\
							\
        _p = (p);					\
        _n = _howmany(FD_SETSIZE, _NFDBITS);		\
        while (_n > 0)					\
                _p->__fds_bits[--_n] = 0;		\
} while (0)

#if !defined (__INSIDE_CYGWIN_NET__)

__BEGIN_DECLS

int select __P ((int __n, fd_set *__readfds, fd_set *__writefds,
		 fd_set *__exceptfds, struct timeval *__timeout));
#if __POSIX_VISIBLE >= 200112
int pselect __P ((int __n, fd_set *__readfds, fd_set *__writefds,
		  fd_set *__exceptfds, const struct timespec *__timeout,
		  const sigset_t *__set));
#endif

__END_DECLS

#endif /* !__INSIDE_CYGWIN_NET__ */

#endif /* !(_WINSOCK_H || _WINSOCKAPI_ || __USE_W32_SOCKETS) */

#endif /* sys/select.h */

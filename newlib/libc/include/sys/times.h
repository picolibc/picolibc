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
#ifndef	_SYS_TIMES_H
#ifdef __cplusplus
extern "C" {
#endif
#define	_SYS_TIMES_H

#include <_ansi.h>
#include <sys/_types.h>

#if !defined(__clock_t_defined) && !defined(_CLOCK_T_DECLARED)
typedef	_CLOCK_T_	clock_t;
#define	__clock_t_defined
#define	_CLOCK_T_DECLARED
#endif

/*  Get Process Times, P1003.1b-1993, p. 92 */
struct tms {
	clock_t	tms_utime;		/* user time */
	clock_t	tms_stime;		/* system time */
	clock_t	tms_cutime;		/* user time, children */
	clock_t	tms_cstime;		/* system time, children */
};

clock_t times (struct tms *);
#ifdef _LIBC
clock_t _times (struct tms *);
#endif

#ifdef __cplusplus
}
#endif
#endif	/* !_SYS_TIMES_H */

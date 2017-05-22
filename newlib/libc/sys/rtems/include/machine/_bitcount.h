/*-
 * Copyright (c) 2015 John Baldwin <jhb@FreeBSD.org>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _MACHINE__BITCOUNT_H
#define	_MACHINE__BITCOUNT_H

#ifdef __POPCNT__
#define	__bitcount64(x)	__builtin_popcountll((__uint64_t)(x))
#define	__bitcount32(x)	__builtin_popcountl((__uint32_t)(x))
#define	__bitcount16(x)	__builtin_popcount((__uint16_t)(x))
#define	__bitcountl(x)	__builtin_popcountl((unsigned long)(x))
#define	__bitcount(x)	__builtin_popcount((unsigned int)(x))
#else /* !__POPCNT__ */
static __inline __uint16_t
__bitcount16(__uint16_t _x)
{
	_x = (_x & 0x5555u) + ((_x & 0xaaaau) >> 1);
	_x = (_x & 0x3333u) + ((_x & 0xccccu) >> 2);
	_x = (_x + (_x >> 4)) & 0x0f0fu;
	_x = (_x + (_x >> 8)) & 0x00ffu;
	return (_x);
}

static __inline __uint32_t
__bitcount32(__uint32_t _x)
{
	_x = (_x & 0x55555555u) + ((_x & 0xaaaaaaaau) >> 1);
	_x = (_x & 0x33333333u) + ((_x & 0xccccccccu) >> 2);
	_x = (_x + (_x >> 4)) & 0x0f0f0f0fu;
	_x = (_x + (_x >> 8));
	_x = (_x + (_x >> 16)) & 0x000000ffu;
	return (_x);
}

static __inline __uint64_t
__bitcount64(__uint64_t _x)
{
#if __SIZEOF_LONG__ == 8
	_x = (_x & 0x5555555555555555u) + ((_x & 0xaaaaaaaaaaaaaaaau) >> 1);
	_x = (_x & 0x3333333333333333u) + ((_x & 0xccccccccccccccccu) >> 2);
	_x = (_x + (_x >> 4)) & 0x0f0f0f0f0f0f0f0fu;
	_x = (_x + (_x >> 8));
	_x = (_x + (_x >> 16));
	_x = (_x + (_x >> 32)) & 0x000000ffu;
	return (_x);
#else
	return (__bitcount32(_x >> 32) + __bitcount32(_x));
#endif
}

#if __SIZEOF_LONG__ == 8
#define	__bitcountl(x)	__bitcount64((unsigned long)(x))
#else
#define	__bitcountl(x)	__bitcount32((unsigned long)(x))
#endif

#if __SIZEOF_INT__ == 8
#define	__bitcount(x)	__bitcount64((unsigned int)(x))
#else
#define	__bitcount(x)	__bitcount32((unsigned int)(x))
#endif
#endif /* __POPCNT__ */

#endif /* _MACHINE__BITCOUNT_H */

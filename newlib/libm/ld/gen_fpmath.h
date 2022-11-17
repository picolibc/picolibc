/*-
 * Copyright (c) 2002, 2003 David Schultz <das@FreeBSD.ORG>
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: src/lib/libc/i386/_fpmath.h,v 1.6 2008/01/17 16:39:06 bde Exp $
 */

#include <stdint.h>

/* Guess long double layout based on compiler defines */

#if __LDBL_MANT_DIG__ == 64 && 16383 <= __LDBL_MAX_EXP__ && __LDBL_MAX_EXP__ <= 16384

union IEEEl2bits {
	long double	e;
	struct {
#if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
		uint64_t	sign	:1;
		uint64_t	exp	:15;
		uint64_t	manh	:32;
		uint64_t	manl	:32;
		uint64_t	junk	:16;
#else
		uint64_t	manl	:32;
		uint64_t	manh	:32;
		uint64_t	exp	:15;
		uint64_t	sign	:1;
		uint64_t	junk	:16;
#endif
	} bits;
	struct {
#if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
		uint64_t 	expsign	:16;
		uint64_t        man	:64;
		uint64_t	junk	:16;
#else
		uint64_t        man	:64;
		uint64_t 	expsign	:16;
		uint64_t	junk	:16;
#endif
	} xbits;
};

#define	LDBL_NBIT	0x80000000
#define	mask_nbit_l(u)	((u).bits.manh &= ~LDBL_NBIT)

#define	LDBL_MANH_SIZE	32
#define	LDBL_MANL_SIZE	32

#define	LDBL_TO_ARRAY32(u, a) do {			\
	(a)[0] = (uint32_t)(u).bits.manl;		\
	(a)[1] = (uint32_t)(u).bits.manh;		\
} while (0)

#endif

#if __LDBL_MANT_DIG__ == 113 && 16383 <= __LDBL_MAX_EXP__ && __LDBL_MAX_EXP__ <= 16384

/* 128-bit long double */

union IEEEl2bits {
	long double	e;
	struct {
#if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
		uint64_t	manl	:64;
		uint64_t	manh	:48;
		uint64_t	exp	:15;
		uint64_t	sign	:1;
#else
		uint64_t	sign	:1;
		uint64_t	exp	:15;
		uint64_t	manh	:48;
		uint64_t	manl	:64;
#endif
	} bits;
	/* TODO andrew: Check the packing here */
	struct {
#if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
		uint64_t	manl	:64;
		uint64_t	manh	:48;
		uint64_t	expsign	:16;
#else
		uint64_t	expsign	:16;
		uint64_t	manh	:48;
		uint64_t	manl	:64;
#endif
	} xbits;
};

#define	LDBL_NBIT	0
#define	LDBL_IMPLICIT_NBIT
#define	mask_nbit_l(u)	((void)0)

#define	LDBL_MANH_SIZE	48
#define	LDBL_MANL_SIZE	64

#define	LDBL_TO_ARRAY32(u, a) do {			\
	(a)[0] = (uint32_t)(u).bits.manl;		\
	(a)[1] = (uint32_t)((u).bits.manl >> 32);	\
	(a)[2] = (uint32_t)(u).bits.manh;		\
	(a)[3] = (uint32_t)((u).bits.manh >> 32);	\
} while(0)

#endif /* __LDBL_MANT_DIG__ == 113 */

/*-
 * Copyright (c) 2003 Mike Barcroft <mike@FreeBSD.org>
 * Copyright (c) 2002 David Schultz <das@FreeBSD.ORG>
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
 * $FreeBSD: src/lib/libc/include/fpmath.h,v 1.4 2008/12/23 22:20:59 marcel Exp $
 */
#ifndef _FPMATH_H_
#define _FPMATH_H_

/* Guess long double layout based on compiler defines */

#if __LDBL_MANT_DIG__ == 64 && 16383 <= __LDBL_MAX_EXP__ && __LDBL_MAX_EXP__ <= 16384
#define _INTEL80_FLOAT
#ifdef __LP64__
#define _INTEL80_FLOAT_PAD
#endif
#endif

#if __LDBL_MANT_DIG__ == 113 && 16383 <= __LDBL_MAX_EXP__ && __LDBL_MAX_EXP__ <= 16384
#define _IEEE128_FLOAT
#endif

#if __LDBL_MANT_DIG__ == 106 && __LDBL_MAX_EXP__ == 1024
#define _DOUBLE_DOUBLE_FLOAT
#endif

#ifndef __FLOAT_WORD_ORDER__
#define __FLOAT_WORD_ORDER__     __BYTE_ORDER__
#endif

#endif /* _FPMATH_H_ */

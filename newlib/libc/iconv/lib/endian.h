/*-
 * Copyright (c) 2000
 *	Konstantin Chuguev.  All rights reserved.
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
 *	iconv (Charset Conversion Library) v2.0
 */
#ifndef __LOCAL_ENDIAN_H__
#define __LOCAL_ENDIAN_H__

#include <sys/param.h>
#include <sys/types.h>

#define ICONV_CCT_BE 0
#define ICONV_CCT_LE 1

#if (BYTE_ORDER == LITTLE_ENDIAN)

#define ICONV_ORDER ICONV_CCT_LE 
#define _1l(b0, b1, b2, b3)	b3, b2, b1, b0
#define _2s(b0, b1, b2, b3)	b1, b0, b3, b2

#elif (BYTE_ORDER == BIG_ENDIAN)

#define ICONV_ORDER ICONV_CCT_BE
#define _1l(b0, b1, b2, b3)	b0, b1, b2, b3
#define _2s(b0, b1, b2, b3)	b0, b1, b2, b3

#else

#error "Unknown byte order."

#endif

#endif /* #ifndef __LOCAL_ENDIAN_H__ */


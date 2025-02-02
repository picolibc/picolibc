/*
 * Copyright (c) 1989 The Regents of the University of California.
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
 * 3. Neither the name of the University nor the names of its contributors
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

#define _DEFAULT_SOURCE
/* Make sure we're using fast ctype */
#define _PICOLIBC_CTYPE_SMALL 0
#include "ctype_.h"
#include "locale_private.h"
#include "../stdlib/local.h"

#if defined(ALLOW_NEGATIVE_CTYPE_INDEX)
/* No static const on Cygwin since it's referenced and potentially overwritten
   for compatibility with older applications. */
const
char _ctype_b[128 + 256] = {
	_CTYPE_DATA_128_255,
	_CTYPE_DATA_0_127,
	_CTYPE_DATA_128_255
};

#else	/* !ALLOW_NEGATIVE_CTYPE_INDEX */

const char _ctype_[1 + 256] = {
	0,
	_CTYPE_DATA_0_127,
	_CTYPE_DATA_128_255
};

#endif	/* !ALLOW_NEGATIVE_CTYPE_INDEX */

#ifdef _MB_EXTENDED_CHARSETS_NON_UNICODE

#include "ctype_extended.h"

#if defined(ALLOW_NEGATIVE_CTYPE_INDEX)

#define __CTYPE(base) [locale_ ## base - locale_EXTENDED_BASE] = { \
        _CTYPE_ ## base ## _128_254,        \
        0,                                  \
        _CTYPE_DATA_0_127,                  \
        _CTYPE_ ## base ## _128_254,        \
        _CTYPE_ ## base ## _255             \
        }

#else

#define __CTYPE(base) [locale_ ## base - locale_EXTENDED_BASE] = { \
        0,                                  \
        _CTYPE_DATA_0_127,                  \
        _CTYPE_ ## base ## _128_254,        \
        _CTYPE_ ## base ## _255             \
        }

#endif

const char __ctype[locale_END - locale_EXTENDED_BASE][CTYPE_OFFSET + 1 + 256] = {
#ifdef _MB_EXTENDED_CHARSETS_ISO
    __CTYPE(ISO_8859_1),
    __CTYPE(ISO_8859_2),
    __CTYPE(ISO_8859_3),
    __CTYPE(ISO_8859_4),
    __CTYPE(ISO_8859_5),
    __CTYPE(ISO_8859_6),
    __CTYPE(ISO_8859_7),
    __CTYPE(ISO_8859_8),
    __CTYPE(ISO_8859_9),
    __CTYPE(ISO_8859_10),
    __CTYPE(ISO_8859_11),
    __CTYPE(ISO_8859_13),
    __CTYPE(ISO_8859_14),
    __CTYPE(ISO_8859_15),
    __CTYPE(ISO_8859_16),
#endif
#ifdef _MB_EXTENDED_CHARSETS_WINDOWS
    __CTYPE(CP437),
    __CTYPE(CP720),
    __CTYPE(CP737),
    __CTYPE(CP775),
    __CTYPE(CP850),
    __CTYPE(CP852),
    __CTYPE(CP855),
    __CTYPE(CP857),
    __CTYPE(CP858),
    __CTYPE(CP862),
    __CTYPE(CP866),
    __CTYPE(CP874),
    __CTYPE(CP1125),
    __CTYPE(CP1250),
    __CTYPE(CP1251),
    __CTYPE(CP1252),
    __CTYPE(CP1253),
    __CTYPE(CP1254),
    __CTYPE(CP1255),
    __CTYPE(CP1256),
    __CTYPE(CP1257),
    __CTYPE(CP1258),
    __CTYPE(KOI8_R),
    __CTYPE(KOI8_U),
    __CTYPE(GEORGIAN_PS),
    __CTYPE(PT154),
    __CTYPE(KOI8_T),
#endif
#ifdef _MB_EXTENDED_CHARSETS_JIS
    __CTYPE(EUCJP),
    __CTYPE(SJIS),
#endif
};

#endif /* _MB_EXTENDED_CHARSETS_NON_UNICODE */

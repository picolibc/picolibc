/*
 * Copyright (c) 2003, Artem B. Bityuckiy, SoftMine Corporation.
 * Rights transferred to Franklin Electronic Publishers.
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
 */
#ifndef __ICONV_DEPS_H__
#define __ICONV_DEPS_H__

#include <newlib.h>

/*
 * EUC-JP requires us_ascii, jis_x0208_1983, jis_x0201, jis_x0212_1990
 */
#ifdef _ICONV_CONVERTER_EUC_JP
#  ifndef _ICONV_CONVERTER_US_ASCII
#    define _ICONV_CONVERTER_US_ASCII
#  endif
#  ifndef _ICONV_CONVERTER_JIS_X0208_1983
#    define _ICONV_CONVERTER_JIS_X0208_1983
#  endif
#  ifndef _ICONV_CONVERTER_JIS_X0201
#    define _ICONV_CONVERTER_JIS_X0201
#  endif
#  ifndef _ICONV_CONVERTER_JIS_X0212_1990
#    define _ICONV_CONVERTER_JIS_X0212_1990
#  endif
#endif /* #ifdef _ICONV_CONVERTER_EUC_JP */

/*
 * EUC-KR requires us_ascii, ksx1001
 */
#ifdef _ICONV_CONVERTER_EUC_KR
#  ifndef _ICONV_CONVERTER_US_ASCII
#    define _ICONV_CONVERTER_US_ASCII
#  endif
#  ifndef _ICONV_CONVERTER_KSX_1001
#    define _ICONV_CONVERTER_KSX_1001
#  endif
#endif /* #ifdef _ICONV_CONVERTER_EUC_KR */

/*
 * EUC-TW requires us_ascii, cns11643_plane1, cns11643_plane2, cns11643_plane14
 */
#ifdef _ICONV_CONVERTER_EUC_TW
#  ifndef _ICONV_CONVERTER_US_ASCII
#    define _ICONV_CONVERTER_US_ASCII
#  endif
#  ifndef _ICONV_CONVERTER_CNS11643_PLANE1
#    define _ICONV_CONVERTER_CNS11643_PLANE1
#  endif
#  ifndef _ICONV_CONVERTER_CNS11643_PLANE2
#    define _ICONV_CONVERTER_CNS11643_PLANE2
#  endif
#  ifndef _ICONV_CONVERTER_CNS11643_PLANE14
#    define _ICONV_CONVERTER_CNS11643_PLANE14
#  endif
#endif /* #ifdef _ICONV_CONVERTER_EUC_TW */

/*
 * GB2380 CES requires us_ascii, gb-2312-80 CCS
 */
#ifdef _ICONV_CONVERTER_GB2312
#  ifndef _ICONV_CONVERTER_US_ASCII
#    define _ICONV_CONVERTER_US_ASCII
#  endif
#  ifndef _ICONV_CONVERTER_GB_2312_80
#    define _ICONV_CONVERTER_GB_2312_80
#  endif
#endif /* #ifdef _ICONV_CONVERTER_GB2312 */

#endif /* #ifndef __ICONV_DEPS_H__ */


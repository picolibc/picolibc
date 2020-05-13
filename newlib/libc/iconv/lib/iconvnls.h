/*
Copyright (c) 2003-2004, Artem B. Bityuckiy

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
 */
#ifndef __ICONV_ICONVNLS_H__
#define __ICONV_ICONVNLS_H__

#include <newlib.h>

/*
 * Include ucs-2-internal or ucs-4-internal if Newlib is configured as
 * "multibyte-capable".
 * ============================================================================
 */
#ifdef _MB_CAPABLE
/*
 * Determine size of wchar_t. If size of wchar_t is 2, UCS-2-INTERNAL is used
 * as widechar's encoding. If size of wchar_t is 4, UCS-4-INTERNAL is used as
 * widechar's encoding.
 */
# if WCHAR_MAX > 0xFFFF
#  ifndef _ICONV_FROM_ENCODING_UCS_4_INTERNAL
#   define _ICONV_FROM_ENCODING_UCS_4_INTERNAL
#  endif
#  ifndef _ICONV_TO_ENCODING_UCS_4_INTERNAL
#   define _ICONV_TO_ENCODING_UCS_4_INTERNAL
#  endif
# elif WCHAR_MAX > 0xFF
#  ifndef _ICONV_FROM_ENCODING_UCS_2_INTERNAL
#   define _ICONV_FROM_ENCODING_UCS_2_INTERNAL
#  endif
#  ifndef _ICONV_TO_ENCODING_UCS_2_INTERNAL
#   define _ICONV_TO_ENCODING_UCS_2_INTERNAL
#  endif
# else
#  error Do not know how to work with 1 byte widechars.
# endif
#endif /* _MB_CAPABLE */

#endif /* !__ICONV_ICONVNLS_H__ */


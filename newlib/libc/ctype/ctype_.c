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
#include "setlocale.h"
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

#ifdef _MB_EXTENDED_CHARSETS_ANY

#include "ctype_extended.h"

#if defined(_MB_EXTENDED_CHARSETS_ISO)
#include "ctype_iso.h"
#endif
#if defined(_MB_EXTENDED_CHARSETS_WINDOWS)
#include "ctype_cp.h"
#endif
#if defined(_MB_EXTENDED_CHARSETS_JIS)
#include "ctype_jis.h"
#endif

#ifdef _TINY_LOCALE
#if defined(ALLOW_NEGATIVE_CTYPE_INDEX)
#define __ctype_ascii   _ctype_b
#define CTYPE_OFFSET    127
#else
#define __ctype_ascii   _ctype_
#define CTYPE_OFFSET    0
#endif

void
__set_ctype(enum locale_id id,
            const char ** ctype)
{
    switch (id) {
    default:
    case locale_C:
    case locale_UTF_8:
        *ctype = __ctype_ascii + CTYPE_OFFSET;
        break;
#ifdef _MB_EXTENDED_CHARSETS_ISO
    case locale_ISO_8859_1:
    case locale_ISO_8859_2:
    case locale_ISO_8859_3:
    case locale_ISO_8859_4:
    case locale_ISO_8859_5:
    case locale_ISO_8859_6:
    case locale_ISO_8859_7:
    case locale_ISO_8859_8:
    case locale_ISO_8859_9:
    case locale_ISO_8859_10:
    case locale_ISO_8859_11:
    case locale_ISO_8859_13:
    case locale_ISO_8859_14:
    case locale_ISO_8859_15:
    case locale_ISO_8859_16:
        *ctype = __ctype_iso[id - locale_ISO_8859_1] + CTYPE_OFFSET;
        break;
#endif
#ifdef _MB_EXTENDED_CHARSETS_WINDOWS
    case locale_GEORGIAN_PS:
    case locale_PT154:
    case locale_KOI8_T:
    case locale_CP437:
    case locale_CP720:
    case locale_CP737:
    case locale_CP775:
    case locale_CP850:
    case locale_CP852:
    case locale_CP855:
    case locale_CP857:
    case locale_CP858:
    case locale_CP862:
    case locale_CP866:
    case locale_CP874:
    case locale_CP1125:
    case locale_CP1250:
    case locale_CP1251:
    case locale_CP1252:
    case locale_CP1253:
    case locale_CP1254:
    case locale_CP1255:
    case locale_CP1256:
    case locale_CP1257:
    case locale_CP1258:
    case locale_KOI8_R:
    case locale_KOI8_U:
        *ctype = __ctype_cp[id - locale_CP437] + CTYPE_OFFSET;
        break;
#endif
#ifdef _MB_EXTENDED_CHARSETS_JIS
    case locale_JIS:
        *ctype = __ctype_ascii + CTYPE_OFFSET;
        break;
    case locale_EUCJP:
        *ctype = __ctype_eucjp + CTYPE_OFFSET;
        break;
    case locale_SJIS:
        *ctype = __ctype_sjis + CTYPE_OFFSET;
        break;
#endif
    }
}
#else
void
__set_ctype (struct __locale_t *loc, const char *charset)
{
#if defined(_MB_EXTENDED_CHARSETS_ISO) || defined(_MB_EXTENDED_CHARSETS_WINDOWS)
  int idx;
#endif
  const char *ctype_ptr = NULL;

  switch (*charset)
    {
#if defined(_MB_EXTENDED_CHARSETS_ISO)
    case 'I':
      idx = __iso_8859_index (charset + 9);
      /* The ctype table has a leading ISO-8859-1 element so we have to add
	 1 to the index returned by __iso_8859_index.  If __iso_8859_index
	 returns < 0, it's ISO-8859-1. */
      if (idx < 0)
        idx = 0;
      else
        ++idx;
      ctype_ptr = __ctype_iso[idx];
      break;
#endif
#if defined(_MB_EXTENDED_CHARSETS_WINDOWS)
    case 'C':
      idx = __cp_index (charset + 2);
      if (idx < 0)
        break;
      ctype_ptr = __ctype_cp[idx];
      break;
#endif
#if defined(_MB_EXTENDED_CHARSETS_JIS)
    case 'E':
      ctype_ptr = __ctype_eucjp;
      break;
    case 'S':
      ctype_ptr = __ctype_sjis;
      break;
#endif
    default:
      break;
    }
  if (!ctype_ptr)
    {
#  if defined(ALLOW_NEGATIVE_CTYPE_INDEX)
     ctype_ptr = _ctype_b;
#  else
     ctype_ptr = _ctype_;
#  endif
    }
#  if defined(ALLOW_NEGATIVE_CTYPE_INDEX)
  loc->ctype_ptr = ctype_ptr + 127;
#  else
  loc->ctype_ptr = ctype_ptr;
#  endif
}
#endif
#endif /* _MB_EXTENDED_CHARSETS_ANY */

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

/* Make sure we're using fast ctype */
#define _PICOLIBC_CTYPE_SMALL 0
#include "ctype_.h"
#include "../locale/setlocale.h"

#define _CTYPE_DATA_0_127 \
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C, \
	_C,	_C|_S, _C|_S, _C|_S,	_C|_S,	_C|_S,	_C,	_C, \
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C, \
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C, \
	_S|_B,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P, \
	_N,	_N,	_N,	_N,	_N,	_N,	_N,	_N, \
	_N,	_N,	_P,	_P,	_P,	_P,	_P,	_P, \
	_P,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U, \
	_U,	_U,	_U,	_P,	_P,	_P,	_P,	_P, \
	_P,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L, \
	_L,	_L,	_L,	_P,	_P,	_P,	_P,	_C

#define _CTYPE_DATA_128_255 \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0, \
	0,	0,	0,	0,	0,	0,	0,	0

#if defined(_MB_CAPABLE)
#if defined(_MB_EXTENDED_CHARSETS_ISO)
#include "ctype_iso.h"
#endif
#if defined(_MB_EXTENDED_CHARSETS_WINDOWS)
#include "ctype_cp.h"
#endif
#endif

#if defined(ALLOW_NEGATIVE_CTYPE_INDEX)
/* No static const on Cygwin since it's referenced and potentially overwritten
   for compatibility with older applications. */
#ifndef __CYGWIN__
const
#endif
char _ctype_b[128 + 256] = {
	_CTYPE_DATA_128_255,
	_CTYPE_DATA_0_127,
	_CTYPE_DATA_128_255
};

#  ifdef __CYGWIN__
/* For backward compatibility */
char __EXPORT *__ctype_ptr__ = DEFAULT_CTYPE_PTR;

#    ifdef __x86_64__
__asm__ ("					\n\
        .data					\n\
	.globl  _ctype_				\n\
	.set    _ctype_,_ctype_b+127		\n\
	.text                                   \n\
");
#    else
__asm__ ("					\n\
        .data					\n\
	.globl  __ctype_			\n\
	.set    __ctype_,__ctype_b+127		\n\
	.text                                   \n\
");
#    endif
#  endif /* !__CYGWIN__ */

#else	/* !ALLOW_NEGATIVE_CTYPE_INDEX */

const char _ctype_[1 + 256] = {
	0,
	_CTYPE_DATA_0_127,
	_CTYPE_DATA_128_255
};

#endif	/* !ALLOW_NEGATIVE_CTYPE_INDEX */

#if defined(_MB_CAPABLE)
/* Cygwin has its own implementation which additionally maintains backward
   compatibility with applications built under older Cygwin releases. */
#ifndef __CYGWIN__
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
#endif /* !__CYGWIN__ */
#endif /* _MB_CAPABLE */

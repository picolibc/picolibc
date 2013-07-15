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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)ctype_.c	5.6 (Berkeley) 6/1/90";
#endif /* LIBC_SCCS and not lint */

#include <ctype.h>

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

#if (defined(__GNUC__) && !defined(__CHAR_UNSIGNED__) && !defined(COMPACT_CTYPE)) || defined (__CYGWIN__)
#define ALLOW_NEGATIVE_CTYPE_INDEX
#endif

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
static _CONST
#endif
char _ctype_b[128 + 256] = {
	_CTYPE_DATA_128_255,
	_CTYPE_DATA_0_127,
	_CTYPE_DATA_128_255
};

#ifdef _NEED_OLD_CTYPE_PTR_DEFINITION
#ifndef _MB_CAPABLE
_CONST
#endif
char __EXPORT *__ctype_ptr = (char *) _ctype_b + 128;
#endif

#ifndef _MB_CAPABLE
_CONST
#endif
char __EXPORT *__ctype_ptr__ = (char *) _ctype_b + 127;

#  ifdef __CYGWIN__
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
#  else /* !__CYGWIN__ */

_CONST char _ctype_[1 + 256] = {
	0,
	_CTYPE_DATA_0_127,
	_CTYPE_DATA_128_255
};
#  endif /* !__CYGWIN__ */

#else	/* !defined(ALLOW_NEGATIVE_CTYPE_INDEX) */

_CONST char _ctype_[1 + 256] = {
	0,
	_CTYPE_DATA_0_127,
	_CTYPE_DATA_128_255
};

#ifdef _NEED_OLD_CTYPE_PTR_DEFINITION
#ifndef _MB_CAPABLE
_CONST
#endif
char *__ctype_ptr = (char *) _ctype_ + 1;
#endif

#ifndef _MB_CAPABLE
_CONST
#endif
char *__ctype_ptr__ = (char *) _ctype_;

#endif

#if defined(_MB_CAPABLE)
/* Cygwin has its own implementation which additionally maintains backward
   compatibility with applications built under older Cygwin releases. */
#ifndef __CYGWIN__
void
__set_ctype (const char *charset)
{
#if defined(_MB_EXTENDED_CHARSETS_ISO) || defined(_MB_EXTENDED_CHARSETS_WINDOWS)
  int idx;
#endif

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
#  if defined(ALLOW_NEGATIVE_CTYPE_INDEX)
#ifdef _NEED_OLD_CTYPE_PTR_DEFINITION
      __ctype_ptr = (char *) (__ctype_iso[idx] + 128);
#endif
      __ctype_ptr__ = (char *) (__ctype_iso[idx] + 127);
#  else
#ifdef _NEED_OLD_CTYPE_PTR_DEFINITION
      __ctype_ptr = (char *) __ctype_iso[idx] + 1;
#endif
      __ctype_ptr__ = (char *) __ctype_iso[idx];
#  endif
      return;
#endif
#if defined(_MB_EXTENDED_CHARSETS_WINDOWS)
    case 'C':
      idx = __cp_index (charset + 2);
      if (idx < 0)
        break;
#  if defined(ALLOW_NEGATIVE_CTYPE_INDEX)
#ifdef _NEED_OLD_CTYPE_PTR_DEFINITION
      __ctype_ptr = (char *) (__ctype_cp[idx] + 128);
#endif
      __ctype_ptr__ = (char *) (__ctype_cp[idx] + 127);
#  else
#ifdef _NEED_OLD_CTYPE_PTR_DEFINITION
      __ctype_ptr = (char *) __ctype_cp[idx] + 1;
#endif
      __ctype_ptr__ = (char *) __ctype_cp[idx];
#  endif
      return;
#endif
    default:
      break;
    }
#  if defined(ALLOW_NEGATIVE_CTYPE_INDEX)
#ifdef _NEED_OLD_CTYPE_PTR_DEFINITION
  __ctype_ptr = (char *) _ctype_b + 128;
#endif
  __ctype_ptr__ = (char *) _ctype_b + 127;
#  else
#ifdef _NEED_OLD_CTYPE_PTR_DEFINITION
  __ctype_ptr = (char *) _ctype_ + 1;
#endif
  __ctype_ptr__ = (char *) _ctype_;
#  endif
}
#endif /* !__CYGWIN__ */
#endif /* _MB_CAPABLE */

/*-
 * Copyright (C) 1997 by Andrey A. Chernov, Moscow, Russia.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND
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
 *
 * $FreeBSD: src/lib/libc/locale/setlocale.h,v 1.4 2001/12/20 18:28:52 phantom Exp $
 */

#ifndef _SETLOCALE_H_
#define	_SETLOCALE_H_

#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include "lctype.h"
#include "lmessages.h"
#include "lnumeric.h"
#include "timelocal.h"
#include "lmonetary.h"

#define ENCODING_LEN 31
#define CATEGORY_LEN 11
#define _LC_LAST      7

#ifdef __CYGWIN__
struct lc_collate_T;
#endif

struct _thr_locale_t
{
  char			 categories[_LC_LAST][ENCODING_LEN + 1];
  int			(*__wctomb) (struct _reent *, char *, wchar_t,
				     const char *, mbstate_t *);
  int			(*__mbtowc) (struct _reent *, wchar_t *, const char *,
				     size_t, const char *, mbstate_t *);
  char			*ctype_ptr; /* Unused in __global_locale */
  int			 cjk_lang;
#ifndef __HAVE_LOCALE_INFO__
  char			 mb_cur_max[2];
  char			 ctype_codeset[ENCODING_LEN + 1];
  char			 message_codeset[ENCODING_LEN + 1];
#else
  struct lc_ctype_T	*ctype;
  char			*ctype_buf;
  struct lc_monetary_T	*monetary;
  char			*monetary_buf;
  struct lc_numeric_T	*numeric;
  char			*numeric_buf;
  struct lc_time_T	*time;
  char			*time_buf;
  struct lc_messages_T	*messages;
  char			*messages_buf;
#ifdef __CYGWIN__
  struct lc_collate_T	*collate;
#endif
  /* Append more categories here. */
#endif
};

#ifdef __cplusplus
extern "C" {
#endif

extern struct _thr_locale_t __global_locale;

/* In POSIX terms the global locale is the process-wide locale.  Use this
   function to always refer to the global locale. */
_ELIDABLE_INLINE struct _thr_locale_t *
__get_global_locale ()
{
  return &__global_locale;
}

/* Per REENT locale.  This is newlib-internal. */
_ELIDABLE_INLINE struct _thr_locale_t *
__get_locale_r (struct _reent *r)
{
  return r->_locale;
}

/* In POSIX terms the current locale is the locale used by all functions
   using locale info without providing a locale as parameter (*_l functions).
   The current locale is either the locale of the current thread, if the
   thread called uselocale, or the global locale if not. */
_ELIDABLE_INLINE struct _thr_locale_t *
__get_current_locale ()
{
  return _REENT->_locale ?: &__global_locale;
}

#ifdef __cplusplus
}
#endif

extern char *_PathLocale;

#endif /* !_SETLOCALE_H_ */

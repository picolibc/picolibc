/*
 * Copyright (c) 2000, 2001 Alexey Zelkin <phantom@FreeBSD.org>
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
 */

#include "setlocale.h"

#define LCNUMERIC_SIZE (sizeof(struct lc_numeric_T) / sizeof(char *))

static const char	numempty[] = { CHAR_MAX, '\0' };

const struct lc_numeric_T _C_numeric_locale = {
	".",     			/* decimal_point */
	"",     			/* thousands_sep */
	numempty			/* grouping */
#ifdef __HAVE_LOCALE_INFO_EXTENDED__
	, "ASCII",			/* codeset */
	L".",				/* wdecimal_point */
	L"",				/* wthousands_sep */
#endif
};

#ifdef __HAVE_LOCALE_INFO__
#ifdef __CYGWIN__
int
__numeric_load_locale (struct __locale_t *locale, const char *name ,
		       void *f_wctomb, const char *charset)
{
  int ret= -1;
  struct lc_numeric_T nm;
  char *bufp = NULL;

  extern int __set_lc_numeric_from_win (const char *,
					const struct lc_numeric_T *,
					struct lc_numeric_T *, char **,
					void *, const char *);
  ret = __set_lc_numeric_from_win (name, &_C_numeric_locale, &nm, &bufp,
				   f_wctomb, charset);
  /* ret == -1: error, ret == 0: C/POSIX, ret > 0: valid */
  if (ret >= 0)
    {
      struct lc_numeric_T *nmp = NULL;

      if (ret > 0)
	{
	  nmp = (struct lc_numeric_T *) calloc (1, sizeof *nmp);
	  if (!nmp)
	    {
	      free (bufp);
	      return -1;
	    }
	  *nmp = nm;
	}
      struct __lc_cats tmp = locale->lc_cat[LC_NUMERIC];
      locale->lc_cat[LC_NUMERIC].ptr = ret == 0 ? &_C_numeric_locale : nmp;
      locale->lc_cat[LC_NUMERIC].buf = bufp;
      /* If buf is not NULL, both pointers have been alloc'ed */
      if (tmp.buf)
	{
	  free ((void *) tmp.ptr);
	  free (tmp.buf);
	}
      ret = 0;
    }
  return ret;
}
#endif
#endif

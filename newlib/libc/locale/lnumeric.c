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
#include "ldpart.h"

extern const char *__fix_locale_grouping_str(const char *);

#define LCNUMERIC_SIZE (sizeof(struct lc_numeric_T) / sizeof(char *))

static char	numempty[] = { CHAR_MAX, '\0' };

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

#ifndef __CYGWIN__
static struct lc_numeric_T _numeric_locale;
static int	_numeric_using_locale;
static char	*_numeric_locale_buf;
#endif

int
__numeric_load_locale (struct __locale_t *locale, const char *name ,
		       void *f_wctomb, const char *charset)
{
  int ret;
  struct lc_numeric_T nm;
  char *bufp = NULL;

#ifdef __CYGWIN__
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
	    return -1;
	  memcpy (nmp, &nm, sizeof *nmp);
	}
      locale->numeric = ret == 0 ? &_C_numeric_locale : nmp;
      if (locale->numeric_buf)
	free (locale->numeric_buf);
      locale->numeric_buf = bufp;
      ret = 0;
    }
#else
  ret = __part_load_locale(name, &_numeric_using_locale,
			   _numeric_locale_buf, "LC_NUMERIC",
			   LCNUMERIC_SIZE, LCNUMERIC_SIZE,
			   (const char **)&_numeric_locale);
  if (ret == 0 && _numeric_using_locale)
      _numeric_locale.grouping =
	      __fix_locale_grouping_str(_numeric_locale.grouping);
#endif
  return ret;
}

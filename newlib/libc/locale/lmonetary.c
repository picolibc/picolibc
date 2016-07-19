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

#include <sys/cdefs.h>

#include <limits.h>
#include <stdlib.h>
#include "setlocale.h"
#include "ldpart.h"

extern const char * __fix_locale_grouping_str(const char *);

#define LCMONETARY_SIZE (sizeof(struct lc_monetary_T) / sizeof(char *))

static char	empty[] = "";
static char	numempty[] = { CHAR_MAX, '\0'};
#ifdef __HAVE_LOCALE_INFO_EXTENDED__
static wchar_t	wempty[] = L"";
#endif

const struct lc_monetary_T _C_monetary_locale = {
	empty,		/* int_curr_symbol */
	empty,		/* currency_symbol */
	empty,		/* mon_decimal_point */
	empty,		/* mon_thousands_sep */
	numempty,	/* mon_grouping */
	empty,		/* positive_sign */
	empty,		/* negative_sign */
	numempty,	/* int_frac_digits */
	numempty,	/* frac_digits */
	numempty,	/* p_cs_precedes */
	numempty,	/* p_sep_by_space */
	numempty,	/* n_cs_precedes */
	numempty,	/* n_sep_by_space */
	numempty,	/* p_sign_posn */
	numempty	/* n_sign_posn */
#ifdef __HAVE_LOCALE_INFO_EXTENDED__
	, numempty,	/* int_p_cs_precedes */
	numempty,	/* int_p_sep_by_space */
	numempty,	/* int_n_cs_precedes */
	numempty,	/* int_n_sep_by_space */
	numempty,	/* int_p_sign_posn */
	numempty,	/* int_n_sign_posn */
	"ASCII",	/* codeset */
	wempty,		/* wint_curr_symbol */
	wempty,		/* wcurrency_symbol */
	wempty,		/* wmon_decimal_point */
	wempty,		/* wmon_thousands_sep */
	wempty,		/* wpositive_sign */
	wempty		/* wnegative_sign */
#endif
};

#ifndef __CYGWIN__
static struct lc_monetary_T _monetary_locale;
static int	_monetary_using_locale;
static char	*_monetary_locale_buf;

static char
cnv(const char *str) {
	int i = strtol(str, NULL, 10);
	if (i == -1)
		i = CHAR_MAX;
	return (char)i;
}
#endif

int
__monetary_load_locale (struct _thr_locale_t *locale, const char *name ,
			void *f_wctomb, const char *charset)
{
  int ret;
  struct lc_monetary_T mo;
  char *bufp = NULL;

#ifdef __CYGWIN__
  extern int __set_lc_monetary_from_win (const char *,
					 const struct lc_monetary_T *,
					 struct lc_monetary_T *, char **,
					 void *, const char *);
  ret = __set_lc_monetary_from_win (name, &_C_monetary_locale, &mo, &bufp,
				    f_wctomb, charset);
  /* ret == -1: error, ret == 0: C/POSIX, ret > 0: valid */
  if (ret >= 0)
    {
      struct lc_monetary_T *mop = NULL;

      if (ret > 0)
	{
	  mop = (struct lc_monetary_T *) calloc (1, sizeof *mop);
	  if (!mop)
	    return -1;
	  memcpy (mop, &mo, sizeof *mop);
	}
      locale->monetary = ret == 0 ? &_C_monetary_locale : mop;
      if (locale->monetary_buf)
	free (locale->monetary_buf);
      locale->monetary_buf = bufp;
      ret = 0;
    }
#else
  ret = __part_load_locale(name, &_monetary_using_locale,
			   _monetary_locale_buf, "LC_MONETARY",
			   LCMONETARY_SIZE, LCMONETARY_SIZE,
			   (const char **)&_monetary_locale);
  if (ret == 0 && _monetary_using_locale) {
    _monetary_locale.mon_grouping =
	 __fix_locale_grouping_str(_monetary_locale.mon_grouping);

#define M_ASSIGN_CHAR(NAME) (((char *)_monetary_locale.NAME)[0] = \
		       cnv(_monetary_locale.NAME))

    M_ASSIGN_CHAR(int_frac_digits);
    M_ASSIGN_CHAR(frac_digits);
    M_ASSIGN_CHAR(p_cs_precedes);
    M_ASSIGN_CHAR(p_sep_by_space);
    M_ASSIGN_CHAR(n_cs_precedes);
    M_ASSIGN_CHAR(n_sep_by_space);
    M_ASSIGN_CHAR(p_sign_posn);
    M_ASSIGN_CHAR(n_sign_posn);
  }
#endif
  return ret;
}

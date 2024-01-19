/*-
 * Copyright (c) 2001 Alexey Zelkin <phantom@FreeBSD.org>
 * Copyright (c) 1997 FreeBSD Inc.
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

#include <stddef.h>
#include "setlocale.h"

#define LCTIME_SIZE (sizeof(struct lc_time_T) / sizeof(char *))

const struct lc_time_T	_C_time_locale = {
	{
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	}, {
		"January", "February", "March", "April", "May", "June",
		"July", "August", "September", "October", "November", "December"
	}, {
		"Sun", "Mon", "Tue", "Wed",
		"Thu", "Fri", "Sat"
	}, {
		"Sunday", "Monday", "Tuesday", "Wednesday",
		"Thursday", "Friday", "Saturday"
	},

	/* X_fmt */
	"%H:%M:%S",

	/*
	 * x_fmt
	 * Since the C language standard calls for
	 * "date, using locale's date format," anything goes.
	 * Using just numbers (as here) makes Quakers happier;
	 * it's also compatible with SVR4.
	 */
	"%m/%d/%y",

	/*
	 * c_fmt
	 */
	"%a %b %e %H:%M:%S %Y",

	/* am pm */
	{ "AM", "PM" },

	/* date_fmt */
	"%a %b %e %H:%M:%S %Z %Y",
	
	/* alt_month
	 * Standalone months forms for %OB
	 */
	{
		"January", "February", "March", "April", "May", "June",
		"July", "August", "September", "October", "November", "December"
	},

	/* md_order
	 * Month / day order in dates
	 */
	"md",

	/* ampm_fmt
	 * To determine 12-hour clock format time (empty, if N/A)
	 */
	"%I:%M:%S %p",

	/* era
	 * Era.  This and the following entries are used if the alternative
	 * date format is specified in strftime
	 */
	"",

	/* era_d_fmt
	 * Era date format used with the %Ex
	 */
	"",

	/* era_d_t_fmt
	 * Era date/time format (%Ec)
	 */
	"",

	/* era_t_fmt
	 * Era time format (%EX)
	 */
	"",

	/* alt_digits
	 * Alternate digits used if %O format prefix is specified
	 */
	""
#ifdef __HAVE_LOCALE_INFO_EXTENDED__
	, "ASCII",	/* codeset */
	{
		L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun",
		L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec"
	}, {
		L"January", L"February", L"March", L"April", L"May", L"June",
		L"July", L"August", L"September", L"October", L"November",
		L"December"
	}, {
		L"Sun", L"Mon", L"Tue", L"Wed",
		L"Thu", L"Fri", L"Sat"
	}, {
		L"Sunday", L"Monday", L"Tuesday", L"Wednesday",
		L"Thursday", L"Friday", L"Saturday"
	},
	L"%H:%M:%S",
	L"%m/%d/%y",
	L"%a %b %e %H:%M:%S %Y",
	{ L"AM", L"PM" },
	L"%a %b %e %H:%M:%S %Z %Y",
	L"%I:%M:%S %p",
	L"",
	L"",
	L"",
	L"",
	L""
#endif
};

#ifdef __CYGWIN__
int
__time_load_locale (struct __locale_t *locale, const char *name,
		    void *f_wctomb, const char *charset)
{
  int	ret = 0;
  struct lc_time_T ti;
  char *bufp = NULL;

#ifdef __HAVE_LOCALE_INFO__
#ifdef __CYGWIN__
  extern int __set_lc_time_from_win (const char *, const struct lc_time_T *,
				     struct lc_time_T *, char **, void *,
				     const char *);
  ret = __set_lc_time_from_win (name, &_C_time_locale, &ti, &bufp,
				f_wctomb, charset);
  /* ret == -1: error, ret == 0: C/POSIX, ret > 0: valid */
  if (ret >= 0)
    {
      struct lc_time_T *tip = NULL;

      if (ret > 0)
	{
	  tip = (struct lc_time_T *) calloc (1, sizeof *tip);
	  if (!tip)
	    {
	      free (bufp);
	      return -1;
	    }
	  *tip = ti;
	}
      struct __lc_cats tmp = locale->lc_cat[LC_TIME];
      locale->lc_cat[LC_TIME].ptr = ret == 0 ? &_C_time_locale : tip;
      locale->lc_cat[LC_TIME].buf = bufp;
      /* If buf is not NULL, both pointers have been alloc'ed */
      if (tmp.buf)
	{
	  free ((void *) tmp.ptr);
	  free (tmp.buf);
	}
      ret = 0;
    }
#else
  /* TODO */
#endif
#endif /* __HAVE_LOCALE_INFO__ */
  return (ret);
}
#endif

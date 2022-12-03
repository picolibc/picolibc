/*
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

#define LCCTYPE_SIZE (sizeof(struct lc_ctype_T) / sizeof(char *))

static char	numone[] = { '\1', '\0'};

const struct lc_ctype_T _C_ctype_locale = {
	"ASCII",			/* codeset */
	numone				/* mb_cur_max */
#ifdef __HAVE_LOCALE_INFO_EXTENDED__
	,
	{ "0", "1", "2", "3", "4",	/* outdigits */
	  "5", "6", "7", "8", "9" },
	{ L"0", L"1", L"2", L"3", L"4",	/* woutdigits */
	  L"5", L"6", L"7", L"8", L"9" }
#endif
};

#ifdef __CYGWIN__
static char	numsix[] = { '\6', '\0'};

const struct lc_ctype_T _C_utf8_ctype_locale = {
	"UTF-8",			/* codeset */
	numsix				/* mb_cur_max */
#ifdef __HAVE_LOCALE_INFO_EXTENDED__
	,
	{ "0", "1", "2", "3", "4",	/* outdigits */
	  "5", "6", "7", "8", "9" },
	{ L"0", L"1", L"2", L"3", L"4",	/* woutdigits */
	  L"5", L"6", L"7", L"8", L"9" }
#endif
};
#endif

/* NULL locale indicates global locale (called from setlocale) */
int
__ctype_load_locale (struct __locale_t *locale, const char *name,
		     void *f_wctomb, const char *charset, int mb_cur_max)
{
  int ret;
  struct lc_ctype_T ct;
  char *bufp = NULL;

#ifdef __CYGWIN__
  extern int __set_lc_ctype_from_win (const char *, const struct lc_ctype_T *,
				      struct lc_ctype_T *, char **, void *,
				      const char *, int);
  ret = __set_lc_ctype_from_win (name, &_C_ctype_locale, &ct, &bufp,
				 f_wctomb, charset, mb_cur_max);
  /* ret == -1: error, ret == 0: C/POSIX, ret > 0: valid */
  if (ret >= 0)
    {
      struct lc_ctype_T *ctp = NULL;

      if (ret > 0)
	{
	  ctp = (struct lc_ctype_T *) calloc (1, sizeof *ctp);
	  if (!ctp)
	    {
	      free (bufp);
	      return -1;
	    }
	  *ctp = ct;
	}
      struct __lc_cats tmp = locale->lc_cat[LC_CTYPE];
      locale->lc_cat[LC_CTYPE].ptr = ret == 0 ? &_C_ctype_locale : ctp;
      locale->lc_cat[LC_CTYPE].buf = bufp;
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
  return ret;
}

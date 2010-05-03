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

#include <limits.h>
#include <string.h>
#include "lctype.h"
#include "ldpart.h"
#include "setlocale.h"

#define LCCTYPE_SIZE (sizeof(struct lc_ctype_T) / sizeof(char *))

static char	numone[] = { '\1', '\0'};

static const struct lc_ctype_T _C_ctype_locale = {
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

static struct lc_ctype_T _ctype_locale;
static int	_ctype_using_locale;
#ifdef __HAVE_LOCALE_INFO_EXTENDED__
static char	*_ctype_locale_buf;
#else
/* Max encoding_len + NUL byte + 1 byte mb_cur_max plus trailing NUL byte */
#define _CTYPE_BUF_SIZE	(ENCODING_LEN + 3)
static char _ctype_locale_buf[_CTYPE_BUF_SIZE];
#endif

int
__ctype_load_locale(const char *name, void *f_wctomb, const char *charset,
		    int mb_cur_max)
{
	int ret;

#ifdef __CYGWIN__
	extern int __set_lc_ctype_from_win (const char *,
					    const struct lc_ctype_T *,
					    struct lc_ctype_T *, char **,
					    void *, const char *, int);
	int old_ctype_using_locale = _ctype_using_locale;
	_ctype_using_locale = 0;
	ret = __set_lc_ctype_from_win (name, &_C_ctype_locale, &_ctype_locale,
				       &_ctype_locale_buf, f_wctomb, charset,
				       mb_cur_max);
	/* ret == -1: error, ret == 0: C/POSIX, ret > 0: valid */
	if (ret < 0)
	  _ctype_using_locale = old_ctype_using_locale;
	else
	  {
	    _ctype_using_locale = ret;
	    ret = 0;
	  }
#elif !defined (__HAVE_LOCALE_INFO_EXTENDED__)
	if (!strcmp (name, "C"))
	  _ctype_using_locale = 0;
	else
	  {
	    _ctype_locale.codeset = strcpy (_ctype_locale_buf, charset);
	    char *mbc = _ctype_locale_buf + _CTYPE_BUF_SIZE - 2;
	    mbc[0] = mb_cur_max;
	    mbc[1] = '\0';
	    _ctype_locale.mb_cur_max = mbc;
	    _ctype_using_locale = 1;
	  }
	ret = 0;
#else
	ret = __part_load_locale(name, &_ctype_using_locale,
		_ctype_locale_buf, "LC_CTYPE",
		LCCTYPE_SIZE, LCCTYPE_SIZE,
		(const char **)&_ctype_locale);
	if (ret == 0 && _ctype_using_locale)
		_ctype_locale.grouping =
			__fix_locale_grouping_str(_ctype_locale.grouping);
#endif
	return ret;
}

struct lc_ctype_T *
__get_current_ctype_locale(void) {

	return (_ctype_using_locale
		? &_ctype_locale
		: (struct lc_ctype_T *)&_C_ctype_locale);
}

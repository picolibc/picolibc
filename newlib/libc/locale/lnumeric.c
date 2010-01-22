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
#include "lnumeric.h"
#include "ldpart.h"

extern int __nlocale_changed;
extern const char *__fix_locale_grouping_str(const char *);

#define LCNUMERIC_SIZE (sizeof(struct lc_numeric_T) / sizeof(char *))

static char	numempty[] = { CHAR_MAX, '\0' };

static const struct lc_numeric_T _C_numeric_locale = {
	".",     	/* decimal_point */
	"",     	/* thousands_sep */
	numempty	/* grouping */
};

static struct lc_numeric_T _numeric_locale;
static int	_numeric_using_locale;
static char	*_numeric_locale_buf;

int
__numeric_load_locale(const char *name , void *f_wctomb, const char *charset)
{
	int ret;

#ifdef __CYGWIN__
	extern int __set_lc_numeric_from_win (const char *,
					      struct lc_numeric_T *,
					      void *, const char *);
	int old_numeric_using_locale = _numeric_using_locale;
	_numeric_using_locale = 0;
	ret = __set_lc_numeric_from_win (name, &_numeric_locale,
					 f_wctomb, charset);
	/* ret == -1: error, ret == 0: C/POSIX, ret > 0: valid */
	if (ret < 0)
	  _numeric_using_locale = old_numeric_using_locale;
	else
	  {
	    _numeric_using_locale = ret;
	    __nlocale_changed = 1;
	    ret = 0;
	  }
#else
	__nlocale_changed = 1;
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

struct lc_numeric_T *
__get_current_numeric_locale(void) {

	return (_numeric_using_locale
		? &_numeric_locale
		: (struct lc_numeric_T *)&_C_numeric_locale);
}

#ifdef LOCALE_DEBUG
void
numericdebug(void) {
printf(	"decimal_point = %s\n"
	"thousands_sep = %s\n"
	"grouping = %s\n",
	_numeric_locale.decimal_point,
	_numeric_locale.thousands_sep,
	_numeric_locale.grouping
);
}
#endif /* LOCALE_DEBUG */

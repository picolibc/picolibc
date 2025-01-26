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

#define _GNU_SOURCE
#include "setlocale.h"

#define LCMONETARY_SIZE (sizeof(struct lc_monetary_T) / sizeof(char *))

static const char	empty[] = "";
static const char	numempty[] = { CHAR_MAX, '\0'};
#ifdef __HAVE_LOCALE_INFO_EXTENDED__
static const wchar_t	wempty[] = L"";
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

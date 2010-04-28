/*-
 * Copyright (c) 1997-2002 FreeBSD Project.
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
 *
 * $FreeBSD: src/lib/libc/stdtime/timelocal.h,v 1.11 2002/01/24 15:07:44 phantom Exp $
 */

#ifndef _TIMELOCAL_H_
#define	_TIMELOCAL_H_

#include <_ansi.h>
#include <sys/cdefs.h>
#include <wchar.h>

__BEGIN_DECLS

/*
 * Private header file for the strftime and strptime localization
 * stuff.
 */
struct lc_time_T {
	const char	*mon[12];
	const char	*month[12];
	const char	*wday[7];
	const char	*weekday[7];
	const char	*X_fmt;
	const char	*x_fmt;
	const char	*c_fmt;
	const char	*am_pm[2];
	const char	*date_fmt;
	const char	*alt_month[12];	/* unused */
	const char	*md_order;
	const char	*ampm_fmt;
	const char	*era;
	const char	*era_d_fmt;
	const char	*era_d_t_fmt;
	const char	*era_t_fmt;
	const char	*alt_digits;
#ifdef __HAVE_LOCALE_INFO_EXTENDED__
	const char	*codeset;	 /* codeset for mbtowc conversion */
	const wchar_t	*wmon[12];
	const wchar_t	*wmonth[12];
	const wchar_t	*wwday[7];
	const wchar_t	*wweekday[7];
	const wchar_t	*wX_fmt;
	const wchar_t	*wx_fmt;
	const wchar_t	*wc_fmt;
	const wchar_t	*wam_pm[2];
	const wchar_t	*wdate_fmt;
	const wchar_t	*wampm_fmt;
	const wchar_t	*wera;
	const wchar_t	*wera_d_fmt;
	const wchar_t	*wera_d_t_fmt;
	const wchar_t	*wera_t_fmt;
	const wchar_t	*walt_digits;
#endif
};

struct lc_time_T *__get_current_time_locale(void);
int	__time_load_locale(const char *, void *, const char *);

__END_DECLS

#endif /* !_TIMELOCAL_H_ */

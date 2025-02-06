/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2025 Keith Packard
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _LOCALE_PRIVATE_H_
#define _LOCALE_PRIVATE_H_

#include <locale.h>
#include <sys/_locale.h>
#include <stdbool.h>
#include <wchar.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>

#define _HAVE_POSIX_LOCALE_API

#define NUMCAT  ((_LC_MESSAGES - LC_ALL) + 1)

/*
 * This operates like _tolower on upper case letters, but also works
 * correctly on lower case letters.
 */
#define LOCALE_ISUPPER(c)      ('A' <= (c) && (c) <= 'Z')
#define LOCALE_TOLOWER(c)      ((c) | ('a' - 'A'))

enum locale_id {
    locale_INVALID,
    locale_BASE,
    locale_C = locale_BASE,
#ifdef _MB_CAPABLE
    locale_UTF_8,
    locale_UCS_2,
    locale_UCS_2LE,
    locale_UCS_2BE,
    locale_UCS_4,
    locale_UCS_4LE,
    locale_UCS_4BE,
#ifdef _MB_EXTENDED_CHARSETS_ISO
    locale_ISO_BASE,
    locale_EXTENDED_BASE = locale_ISO_BASE,
    locale_ISO_8859_1 = locale_ISO_BASE,
    locale_ISO_8859_2,
    locale_ISO_8859_3,
    locale_ISO_8859_4,
    locale_ISO_8859_5,
    locale_ISO_8859_6,
    locale_ISO_8859_7,
    locale_ISO_8859_8,
    locale_ISO_8859_9,
    locale_ISO_8859_10,
    locale_ISO_8859_11,
    locale_ISO_8859_13,
    locale_ISO_8859_14,
    locale_ISO_8859_15,
    locale_ISO_8859_16,
#endif
#ifdef _MB_EXTENDED_CHARSETS_WINDOWS
    locale_WINDOWS_BASE,
#ifndef _MB_EXTENDED_CHARSETS_ISO
    locale_EXTENDED_BASE = locale_WINDOWS_BASE,
#endif
    locale_CP437 = locale_WINDOWS_BASE,
    locale_CP720,
    locale_CP737,
    locale_CP775,
    locale_CP850,
    locale_CP852,
    locale_CP855,
    locale_CP857,
    locale_CP858,
    locale_CP862,
    locale_CP866,
    locale_CP874,
    locale_CP1125,
    locale_CP1250,
    locale_CP1251,
    locale_CP1252,
    locale_CP1253,
    locale_CP1254,
    locale_CP1255,
    locale_CP1256,
    locale_CP1257,
    locale_CP1258,
    locale_KOI8_R,
    locale_KOI8_U,
    locale_GEORGIAN_PS,
    locale_PT154,
    locale_KOI8_T,
#endif
#ifdef _MB_EXTENDED_CHARSETS_JIS
    locale_JIS_BASE,
#if !defined(_MB_EXTENDED_CHARSETS_ISO) && !defined(_MB_EXTENDED_CHARSETS_WINDOWS)
    locale_EXTENDED_BASE = locale_JIS_BASE,
#endif
    locale_JIS = locale_JIS_BASE,
    locale_EUCJP,
    locale_SJIS,
#endif
#endif
    locale_END,
};

typedef int             wctomb_f (char *, wchar_t, mbstate_t *);
typedef wctomb_f        *wctomb_p;

typedef int             mbtowc_f (wchar_t *, const char *, size_t,
                                  mbstate_t *);
typedef mbtowc_f        *mbtowc_p;

#ifndef _DEFAULT_LOCALE
#define _DEFAULT_LOCALE  locale_C
#endif

extern const char       * const __locale_names[];
extern locale_t         __global_locale;

#ifdef _HAVE_POSIX_LOCALE_API
extern NEWLIB_THREAD_LOCAL locale_t    _locale;
#endif

#ifdef _MB_CAPABLE

extern const wctomb_p __wctomb[locale_END - locale_BASE];
extern const mbtowc_p __mbtowc[locale_END - locale_BASE];

#define __get_wctomb(id)        __wctomb[(id) - locale_BASE]
#define __get_mbtowc(id)        __mbtowc[(id) - locale_BASE]

#ifdef _HAVE_POSIX_LOCALE_API
#define __WCTOMB_L(l)   (__get_wctomb(l))
#define __MBTOWC_L(l)   (__get_mbtowc(l))
#define __WCTOMB        (__WCTOMB_L(__get_current_locale()))
#define __MBTOWC        (__MBTOWC_L(__get_current_locale()))
#else
#define __WCTOMB        (__get_wctomb(__global_locale))
#define __MBTOWC        (__get_mbtowc(__global_locale))
#endif

#else

#define __WCTOMB        __ascii_wctomb
#define __MBTOWC        __ascii_mbtowc
#define __WCTOMB_L(l)   ((void) (l), __ascii_wctomb)
#define __MBTOWC_L(l)   ((void) (l), __ascii_mbtowc)

#endif

static inline locale_t
__get_current_locale(void) {
#ifdef _HAVE_POSIX_LOCALE_API
    if (_locale)
        return _locale;
#endif
    return __global_locale;
}

static inline const char *
__locale_name(locale_t id) {
    return __locale_names[id - locale_BASE];
}

enum locale_id
__find_charset(const char *name);

enum locale_id
__find_locale(const char *name);

/* LC_NUMERIC data */
#define DECIMAL_POINT           "."
#define DECIMAL_POINT_L(l)      ((void) (l), DECIMAL_POINT)
#define THOUSANDS_SEP           ""
#define THOUSANDS_SEP_L(l)      ((void) (l), THOUSANDS_SEP)
#define NUMERIC_GROUPING        ""
#define NUMERIC_GROUPING_L(l)   ((void) (l), "")
#define WDECIMAL_POINT          L"."
#define WDECIMAL_POINT_L(l)     ((void) (l), DECIMAL_POINT)
#define WTHOUSANDS_SEP          L""
#define WTHOUSANDS_SEP_L(l)     ((void) (l), THOUSANDS_SEP)

/* LC_TIME data */
extern const char *const __time_wday[7];
extern const char *const __time_weekday[7];
extern const char *const __time_mon[12];
extern const char *const __time_month[12];
extern const char *const __time_am_pm[2];
extern const wchar_t *const __wtime_wday[7];
extern const wchar_t *const __wtime_weekday[7];
extern const wchar_t *const __wtime_mon[12];
extern const wchar_t *const __wtime_month[12];
extern const wchar_t *const __wtime_am_pm[2];

#define TIME_ERA                ""
#define TIME_ERA_D_T_FMT_L(l)   ((void) (l), "")
#define TIME_ERA_D_T_FMT        ""
#define TIME_ERA_D_FMT_L(l)     ((void) (l), "")
#define TIME_ERA_D_FMT          ""
#define TIME_ERA_T_FMT_L(l)     ((void) (l), "")
#define TIME_ERA_T_FMT          ""
#define TIME_ALT_DIGITS         ""
#define TIME_WDAY               __time_wday
#define TIME_WEEKDAY            __time_weekday
#define TIME_MON                __time_mon
#define TIME_MONTH              __time_month
#define TIME_AM_PM              __time_am_pm
#define TIME_C_FMT              "%a %b %e %H:%M:%S %Y"
#define TIME_X_FMT              "%m/%d/%y"
#define TIME_UX_FMT             "%M/%D/%Y"
#define TIME_AMPM_FMT           "%I:%M:%S %p"

#define WTIME_ERA               L""
#define WTIME_ERA_D_T_FMT_L(l)  ((void) (l), L"")
#define WTIME_ERA_D_T_FMT       L""
#define WTIME_ERA_D_FMT_L(l)    ((void) (l), L"")
#define WTIME_ERA_D_FMT         L""
#define WTIME_ERA_T_FMT_L(l)    ((void) (l), L"")
#define WTIME_ERA_T_FMT         L""
#define WTIME_ALT_DIGITS_L(l)   ((void) (l), L"")
#define WTIME_ALT_DIGITS        L""
#define WTIME_WDAY              __wtime_wday
#define WTIME_WEEKDAY           __wtime_weekday
#define WTIME_MON               __wtime_mon
#define WTIME_MONTH             __wtime_month
#define WTIME_AM_PM             __wtime_am_pm
#define WTIME_C_FMT             L"%a %b %e %H:%M:%S %Y"
#define WTIME_X_FMT             L"%m/%d/%y"
#define WTIME_UX_FMT            L"%M/%D/%Y"
#define WTIME_AMPM_FMT          L"%I:%M:%S %p"

static inline bool
__locale_is_C(locale_t locale)
{
#ifdef _MB_CAPABLE
    if (!locale)
        locale = __get_current_locale();
    return locale == locale_C;
#else
    (void) locale;
    return true;
#endif
}

static inline size_t
__locale_mb_cur_max_l(locale_t locale)
{
    switch (locale) {
#ifdef _MB_CAPABLE
    case locale_UTF_8:
        return 6;
#ifdef _MB_EXTENDED_CHARSETS_JIS
    case locale_JIS:
        return 8;
    case locale_EUCJP:
        return 3;
    case locale_SJIS:
        return 2;
#endif
#endif
    default:
        return 1;
    }
}

static inline int
__locale_cjk_lang(void)
{
#ifdef _MB_EXTENDED_CHARSETS_JIS
    switch (__get_current_locale()) {
    case locale_JIS:
    case locale_EUCJP:
    case locale_SJIS:
        return 1;
    default:
        break;
    }
#endif
    return 0;
}

#endif /* _LOCALE_PRIVATE_H_ */

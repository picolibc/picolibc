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

#include "locale_private.h"

const char * const __locale_names[] = {
    [locale_C] = "C",
#ifdef _MB_CAPABLE
    [locale_UTF_8] = "C.UTF-8",
    [locale_UCS_2] = "C.UCS-2",
    [locale_UCS_2LE] = "C.UCS-2LE",
    [locale_UCS_2BE] = "C.UCS-2BE",
    [locale_UCS_4] = "C.UCS-4",
    [locale_UCS_4LE] = "C.UCS-4LE",
    [locale_UCS_4BE] = "C.UCS-4BE",
#ifdef _MB_EXTENDED_CHARSETS_ISO
    [locale_ISO_8859_1] = "C.ISO-8859-1",
    [locale_ISO_8859_2] = "C.ISO-8859-2",
    [locale_ISO_8859_3] = "C.ISO-8859-3",
    [locale_ISO_8859_4] = "C.ISO-8859-4",
    [locale_ISO_8859_5] = "C.ISO-8859-5",
    [locale_ISO_8859_6] = "C.ISO-8859-6",
    [locale_ISO_8859_7] = "C.ISO-8859-7",
    [locale_ISO_8859_8] = "C.ISO-8859-8",
    [locale_ISO_8859_9] = "C.ISO-8859-9",
    [locale_ISO_8859_10] = "C.ISO-8859-10",
    [locale_ISO_8859_11] = "C.ISO-8859-11",
    [locale_ISO_8859_13] = "C.ISO-8859-13",
    [locale_ISO_8859_14] = "C.ISO-8859-14",
    [locale_ISO_8859_15] = "C.ISO-8859-15",
    [locale_ISO_8859_16] = "C.ISO-8859-16",
#endif
#ifdef _MB_EXTENDED_CHARSETS_WINDOWS
    [locale_CP437] = "C.CP437",
    [locale_CP720] = "C.CP720",
    [locale_CP737] = "C.CP737",
    [locale_CP775] = "C.CP775",
    [locale_CP850] = "C.CP850",
    [locale_CP852] = "C.CP852",
    [locale_CP855] = "C.CP855",
    [locale_CP857] = "C.CP857",
    [locale_CP858] = "C.CP858",
    [locale_CP862] = "C.CP862",
    [locale_CP866] = "C.CP866",
    [locale_CP874] = "C.CP874",
    [locale_CP1125] = "C.CP1125",
    [locale_CP1250] = "C.CP1250",
    [locale_CP1251] = "C.CP1251",
    [locale_CP1252] = "C.CP1252",
    [locale_CP1253] = "C.CP1253",
    [locale_CP1254] = "C.CP1254",
    [locale_CP1255] = "C.CP1255",
    [locale_CP1256] = "C.CP1256",
    [locale_CP1257] = "C.CP1257",
    [locale_CP1258] = "C.CP1258",
    [locale_KOI8_R] = "C.KOI8-R",
    [locale_KOI8_U] = "C.KOI8-U",
    [locale_GEORGIAN_PS] = "C.GEORGIAN-PS",
    [locale_PT154] = "C.PT154",
    [locale_KOI8_T] = "C.KOI8-T",
#endif
#ifdef _MB_EXTENDED_CHARSETS_JIS
    [locale_JIS] = "C.JIS",
    [locale_EUCJP] = "C.EUC-JP",
    [locale_SJIS] = "C.SHIFT-JIS",
#endif
#endif
};

static const char *
__find_locale_sep(const char *str)
{
    char c;
    for (;;) {
        c = *str;
        switch (c) {
        case '\0':
        case '.':
        case '-':
            return str;
        }
        ++str;
    }
}

static bool
__skip_char(char c)
{
    return c == '.' || c == '_' || c == '-';
}

/*
 * Match two charset strings, ignoring ., _ and - unless
 * those occur at the end of the input string.
 * This is a case-insensitive match
 */

static bool
__match_charset(const char *str, const char *name)
{
    for (;;) {
        char    sc, nc;

        while (__skip_char(nc = *name++));
        while (__skip_char(sc = *str++) && nc);
        if (LOCALE_ISUPPER(sc)) sc = LOCALE_TOLOWER(sc);
        if (LOCALE_ISUPPER(nc)) nc = LOCALE_TOLOWER(nc);
        if (sc != nc)
            return false;
        if (!sc)
            return true;
    }
}

/*
 * Map a charset name to one of our internal locale ids.
 * Returns locale_INVALID for an invalid charset
 */
enum locale_id
__find_charset(const char *charset)
{
    enum locale_id id;

    if (__match_charset(charset, "ascii") ||
        __match_charset(charset, "us_ascii"))
        return locale_C;

#ifdef _MB_CAPABLE
    for (id = locale_UTF_8; id < locale_END; id++) {
        if (__match_charset(charset, __locale_names[id] + 2))
            break;
    }
    if (id == locale_END)
        id = locale_INVALID;
#else
    id = locale_INVALID;
#endif
    return id;
}

#ifdef _MB_CAPABLE
#define LOCALE_DEFAULT  locale_UTF_8
#else
#define LOCALE_DEFAULT  locale_C
#endif

/*
 * Map a locale name to one of our internal locale ids.
 * Returns locale_INVALID for an invalid charset
 */
enum locale_id
__find_locale(const char *name)
{
    enum locale_id     id = LOCALE_DEFAULT;
    const char          *lang_end;

    if (!*name)
        return _DEFAULT_LOCALE;

    /* POSIX is an alias for C */
    if (!strcmp(name, "POSIX"))
        name = __locale_names[locale_C];

    lang_end = __find_locale_sep(name);

    /* Switch the default locale to C when the territory is 'C' */
    if (lang_end == name + 1 && LOCALE_TOLOWER(*name) == 'c')
        id = locale_C;

    if (*lang_end)
        id = __find_charset(lang_end + 1);

    return id;
}

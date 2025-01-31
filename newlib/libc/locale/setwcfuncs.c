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

#define _DEFAULT_SOURCE
#include "locale_private.h"
#include "../stdlib/local.h"

#ifdef _MB_CAPABLE
#ifdef _MB_EXTENDED_CHARSETS_ISO
static uint8_t iso_val[] = {
    [locale_ISO_8859_1- locale_ISO_8859_1] = 1,
    [locale_ISO_8859_2- locale_ISO_8859_1] = 2,
    [locale_ISO_8859_3- locale_ISO_8859_1] = 3,
    [locale_ISO_8859_4- locale_ISO_8859_1] = 4,
    [locale_ISO_8859_5- locale_ISO_8859_1] = 5,
    [locale_ISO_8859_6- locale_ISO_8859_1] = 6,
    [locale_ISO_8859_7- locale_ISO_8859_1] = 7,
    [locale_ISO_8859_8- locale_ISO_8859_1] = 8,
    [locale_ISO_8859_9- locale_ISO_8859_1] = 9,
    [locale_ISO_8859_10- locale_ISO_8859_1] = 10,
    [locale_ISO_8859_11- locale_ISO_8859_1] = 11,
    [locale_ISO_8859_13- locale_ISO_8859_1] = 13,
    [locale_ISO_8859_14- locale_ISO_8859_1] = 14,
    [locale_ISO_8859_15- locale_ISO_8859_1] = 15,
    [locale_ISO_8859_16- locale_ISO_8859_1] = 16,
};
#endif
#ifdef _MB_EXTENDED_CHARSETS_WINDOWS
static uint16_t cp_val[] = {
    [locale_CP437 - locale_CP437] = 437,
    [locale_CP720 - locale_CP437] = 720,
    [locale_CP737 - locale_CP437] = 737,
    [locale_CP775 - locale_CP437] = 775,
    [locale_CP850 - locale_CP437] = 850,
    [locale_CP852 - locale_CP437] = 852,
    [locale_CP855 - locale_CP437] = 855,
    [locale_CP857 - locale_CP437] = 857,
    [locale_CP858 - locale_CP437] = 858,
    [locale_CP862 - locale_CP437] = 862,
    [locale_CP866 - locale_CP437] = 866,
    [locale_CP874 - locale_CP437] = 874,
    [locale_CP1125 - locale_CP437] = 1125,
    [locale_CP1250 - locale_CP437] = 1250,
    [locale_CP1251 - locale_CP437] = 1251,
    [locale_CP1252 - locale_CP437] = 1252,
    [locale_CP1253 - locale_CP437] = 1253,
    [locale_CP1254 - locale_CP437] = 1254,
    [locale_CP1255 - locale_CP437] = 1255,
    [locale_CP1256 - locale_CP437] = 1256,
    [locale_CP1257 - locale_CP437] = 1257,
    [locale_CP1258 - locale_CP437] = 1258,
    [locale_KOI8_R - locale_CP437] = 20866,
    [locale_KOI8_U - locale_CP437] = 21866,
    [locale_GEORGIAN_PS - locale_CP437] = 101,
    [locale_PT154 - locale_CP437] = 102,
    [locale_KOI8_T - locale_CP437] = 103,
};
#endif

void
__set_wc_funcs(enum locale_id id,
               wctomb_t *wctomb,
               mbtowc_t *mbtowc)
{
    switch (id) {
    default:
        *wctomb = __ascii_wctomb;
        *mbtowc = __ascii_mbtowc;
        break;
    case locale_UTF_8:
        *wctomb = __utf8_wctomb;
        *mbtowc = __utf8_mbtowc;
        break;
#ifdef _MB_EXTENDED_CHARSETS_ISO
    case locale_ISO_8859_1:
    case locale_ISO_8859_2:
    case locale_ISO_8859_3:
    case locale_ISO_8859_4:
    case locale_ISO_8859_5:
    case locale_ISO_8859_6:
    case locale_ISO_8859_7:
    case locale_ISO_8859_8:
    case locale_ISO_8859_9:
    case locale_ISO_8859_10:
    case locale_ISO_8859_11:
    case locale_ISO_8859_13:
    case locale_ISO_8859_14:
    case locale_ISO_8859_15:
    case locale_ISO_8859_16:
        *wctomb = __iso_wctomb (iso_val[id - locale_ISO_8859_1]);
        *mbtowc = __iso_mbtowc (iso_val[id - locale_ISO_8859_1]);
        break;
#endif
#ifdef _MB_EXTENDED_CHARSETS_WINDOWS
    case locale_CP437:
    case locale_CP720:
    case locale_CP737:
    case locale_CP775:
    case locale_CP850:
    case locale_CP852:
    case locale_CP855:
    case locale_CP857:
    case locale_CP858:
    case locale_CP862:
    case locale_CP866:
    case locale_CP874:
    case locale_CP1125:
    case locale_CP1250:
    case locale_CP1251:
    case locale_CP1252:
    case locale_CP1253:
    case locale_CP1254:
    case locale_CP1255:
    case locale_CP1256:
    case locale_CP1257:
    case locale_CP1258:
    case locale_KOI8_R:
    case locale_KOI8_U:
    case locale_GEORGIAN_PS:
    case locale_PT154:
    case locale_KOI8_T:
        *wctomb = __cp_wctomb (cp_val[id - locale_CP437]);
        *mbtowc = __cp_mbtowc (cp_val[id - locale_CP437]);
        break;
#endif
#ifdef _MB_EXTENDED_CHARSETS_JIS
    case locale_JIS:
        *wctomb = __jis_wctomb;
        *mbtowc = __jis_mbtowc;
        break;
    case locale_EUCJP:
        *wctomb = __eucjp_wctomb;
        *mbtowc = __eucjp_mbtowc;
        break;
    case locale_SJIS:
        *wctomb = __sjis_wctomb;
        *mbtowc = __sjis_mbtowc;
        break;
#endif
    }
}

#endif /* _MB_CAPABLE */

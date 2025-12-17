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

#include <inttypes.h>
#include <wchar.h>

#ifdef __MB_EXTENDED_CHARSETS_JIS

#ifdef __AVR__
#define split_unicode
#endif

#define define_unicode
#define define_unicode_ff
#include "jis.h"

uint32_t
__unicode_to_euc_jp(wchar_t unicode)
{
#if __SIZEOF_WCHAR_T__ > 2
    if (unicode > 0xffff)
        return INVALID_JIS;
#endif
    uint8_t                   row = unicode >> 8;
    uint8_t                   col = unicode;
    uint8_t                   first_col, last_col;
    const uint16_t           *codes;
    uint16_t                  jis;
    const struct jis_charmap *map;

    /* ASCII */
    if (row == 0)
        if (col < 0xa0 && col != 0x8f && col != 0x8e)
            return unicode;

    if (row == 0xff) {
        codes = __euc_jp_unicode_ff_codes;
        first_col = __euc_jp_unicode_ff_first_col;
        last_col = __euc_jp_unicode_ff_last_col;
    } else {

#if __euc_jp_unicode_first_row != 0
        if (row < __euc_jp_unicode_first_row)
            return INVALID_JIS;
#endif
        if (row > __euc_jp_unicode_last_row)
            return INVALID_JIS;

        map = &__euc_jp_unicode_map[row - __euc_jp_unicode_first_row];

        if (map->offset == 0xffff)
            return INVALID_JIS;

#ifdef split_unicode
        if (map->offset >= __euc_jp_split_codes) {
            codes = __euc_jp_unicode_codes_second + (map->offset - __euc_jp_split_codes);
        } else
#endif
        {
            codes = __euc_jp_unicode_codes + map->offset;
        }
        first_col = map->first;
        last_col = map->last;
    }

    if (col < first_col || last_col < col)
        return INVALID_JIS;
    jis = codes[col - first_col];
    if (jis == __euc_jp_invalid_jis)
        return INVALID_JIS;

    /* Map JIS X213 values */
    if ((jis & 0x8000) == 0 && (jis & 0x80) != 0)
        return (uint32_t)jis | (uint32_t)0x8f8000U;

    return jis;
}

#endif /* __MB_EXTENDED_CHARSETS_JIS */

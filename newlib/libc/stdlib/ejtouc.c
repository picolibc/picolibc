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

#define define_jis_x0208
#define define_jis_x0213
#include "jis.h"

wint_t
__euc_jp_to_unicode(uint32_t euc_jp)
{
    uint8_t                   byte0 = euc_jp >> 16;
    uint8_t                   byte1 = euc_jp >> 8;
    uint8_t                   byte2 = euc_jp;
    const uint16_t           *codes;
    const struct jis_charmap *map;
    uint8_t                   first_row, last_row;
    uint16_t                  unicode;

    switch (byte0) {
    case 0x8f:
        codes = __euc_jp_jis_x0213_codes;
        map = __euc_jp_jis_x0213_map;
        first_row = __euc_jp_jis_x0213_first_row;
        last_row = __euc_jp_jis_x0213_last_row;
        break;
    case 0x00:
        switch (byte1) {
        case 0x00:
            /* ASCII */
            if (byte2 < 0xa0 && byte2 != 0x8f && byte2 != 0x8e)
                return byte2;
            return WEOF;
        case 0x8e:
            /* Half-width Katakana */
            if (0xa1 <= byte2 && byte2 <= 0xdf)
                return (uint16_t) byte2 - 0xa1U + 0xff61U;
            return WEOF;
        default:
            codes = __euc_jp_jis_x0208_codes;
            map = __euc_jp_jis_x0208_map;
            first_row = __euc_jp_jis_x0208_first_row;
            last_row = __euc_jp_jis_x0208_last_row;
            break;
        }
        break;
    default:
        return WEOF;
    }

    if (byte1 < first_row || last_row < byte1)
        return WEOF;

    map = &map[byte1 - first_row];

    if (map->offset == 0xffff || byte2 < map->first || map->last < byte2)
        return WEOF;

    codes = &codes[map->offset];
    unicode = codes[byte2 - map->first];

    if (unicode == __euc_jp_invalid_unicode)
        return WEOF;

    return unicode;
}

#endif /* __MB_EXTENDED_CHARSETS_JIS */

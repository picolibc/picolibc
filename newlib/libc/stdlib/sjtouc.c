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

#include "jis.h"

wint_t
__shift_jis_to_unicode(uint16_t shift_jis)
{
    uint8_t byte1, byte2, sjis1, sjis2;

    sjis1 = shift_jis >> 8;
    sjis2 = shift_jis & 0xff;

    switch (sjis1) {
    case 0:
        /* ASCII */
        switch (sjis2) {
        case 0x5c:
            return 0xa5; /* ¥ */
        case 0x7e:
            return 0x203e; /* ‾ */
        default:
            if (sjis2 < 0x80)
                return sjis2;
            if (0xa1 <= sjis2 && sjis2 <= 0xdf)
                return sjis2 + (0xff61 - 0xa1);
            return WEOF;
        }
        break;
    }

    if (0x81 <= sjis1 && sjis1 <= 0x9f)
        byte1 = (sjis1 - 112 + 64) << 1;
    else if (0xe0 <= sjis1 && sjis1 <= 0xef)
        byte1 = (sjis1 - 176 + 64) << 1;
    else
        return WEOF;

    if (0x40 <= sjis2 && sjis2 <= 0x9e && sjis2 != 0x7f) {
        byte1 -= 1;
        byte2 = sjis2 - 31 + 128;
        if (sjis2 >= 0x7f)
            byte2--;
    } else if (0x9f <= sjis2 && sjis2 <= 0xfc) {
        byte2 = sjis2 - 126 + 128;
    } else {
        return WEOF;
    }

    return __euc_jp_to_unicode(((uint16_t) byte1 << 8) | byte2);
}

#endif /* __MB_EXTENDED_CHARSETS_JIS */

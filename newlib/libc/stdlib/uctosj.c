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

uint16_t
__unicode_to_shift_jis(wchar_t unicode)
{
    /* Deviations from EUC-JP */
    switch (unicode) {
    case 0xa5: /* ¥ */
        return 0x5c;
    case 0x203e: /* ‾ */
        return 0x7e;
    case 0xffe0: /* ￠ -> ¢ */
        return 0x8191;
    case 0xffe1: /* ￡ -> £ */
        return 0x8192;
    case 0xffe2: /* ￢ -> ¬ */
        return 0x81ca;
    default:
        break;
    }

    /* ASCII */
    if (unicode <= 0x7f)
        return unicode;

    /* Shift-JIS doesn't include these control chars */
    if (unicode <= 0x9f)
        return INVALID_JIS;

    uint32_t euc_jp = __unicode_to_euc_jp(unicode);

    if (euc_jp == INVALID_JIS)
        return euc_jp;

    /* shift-jis doesn't handle 3-byte euc-jp values */
    if (euc_jp > 0xffff)
        return INVALID_JIS;

    /* Katakana */
    if (0x8ea1 <= euc_jp && euc_jp <= 0x8efe) {
        return euc_jp - 0x8e00;
    }

    /* Convert to JIS X 0208 values */
    uint8_t j1 = (euc_jp >> 8) & 0x7f;
    uint8_t j2 = euc_jp & 0x7f;
    uint8_t s1, s2;

    /* Compute Shift-JIS bytes */

    if (33 <= j1 && j1 <= 94)
        s1 = ((j1 + 1) >> 1) + 112;
    else
        s1 = ((j1 + 1) >> 1) + 176;

    if (j1 & 1)
        s2 = j2 + 31 + (j2 / 96);
    else
        s2 = j2 + 126;

    return ((uint16_t)s1 << 8) | s2;
}

#endif /* __MB_EXTENDED_CHARSETS_JIS */

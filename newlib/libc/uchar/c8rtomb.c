/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2024 Keith Packard
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

#include "uchar-local.h"

size_t
c8rtomb (char *s, char8_t c8, mbstate_t *ps)
{
    static mbstate_t _state;
    char32_t    c32;
    int count;

    if (ps == NULL)
        ps = &_state;

    if (s == NULL)
        c8 = 0;

    if (char8_is_one_byte(c8)) {
        if (ps->__count != 0) {
            errno = EILSEQ;
            return (size_t) -1;
        }
        c32 = c8;
    } else if (char8_is_first_byte(c8)) {
        /*
         * For a two-byte sequence, we need to have at least two bits
         * of contribution from the first byte or the sequence could
         * have been a single byte.
         */
        if (ps->__count != 0 || c8 <= 0xc1) {
            errno = EILSEQ;
            return (size_t) -1;
        }
        count = 1;
        while ((c8 & (1 << (6 - count))) != 0) {
            if (count > 4) {
                errno = EILSEQ;
                return (size_t) -1;
            }
            count++;
        }
        c32 = (char32_t) (c8 & ((1 << (6-count)) - 1)) << (6 * count);
        if (!char32_is_valid(c32))
        {
            errno = EILSEQ;
            return (size_t) -1;
        }
        ps->__value.__ucs = c32;
        ps->__count = -count;
        return 0;
    } else {
        count = -ps->__count;
        c8 &= 0x3f;
        if (count == 0 || (ps->__value.__ucs == 0 && c8 < (0x80 >> count)))
        {
            errno = EILSEQ;
            return (size_t) -1;
        }
        count--;
        c32 = ps->__value.__ucs | ((char32_t) c8 << (6 * count));
        if (!char32_is_valid(c32))
        {
            errno = EILSEQ;
            return (size_t) -1;
        }
        ps->__count = -count;
        if (count != 0) {
            ps->__value.__ucs = c32;
            return 0;
        }
    }

#if __SIZEOF_WCHAR_T__ == 2
    return c32rtomb (s, c32, ps);
#elif __SIZEOF_WCHAR_T__ == 4
    return wcrtomb(s, (wchar_t) c32, ps);
#else
#error wchar_t size unknown
#endif
}

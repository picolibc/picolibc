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
c16rtomb (char *s, char16_t c16, mbstate_t *ps)
{
    static mbstate_t local_state;

    if (ps == NULL)
        ps = &local_state;

#if __SIZEOF_WCHAR_T__ == 2
    return wcrtomb(s, (wchar_t) c16, ps);
#elif __SIZEOF_WCHAR_T__ == 4
    char32_t    c32;

    if (ps->__count == -1) {
        ps->__count = 0;
        /* Check for low surrogate */
        if (char16_is_low_surrogate(c16)) {
            c32 = ps->__value.__ucs | (c16 & 0x3ff);
        } else {
            errno = EILSEQ;
            return (size_t) -1;
        }
    } else if (char16_is_high_surrogate(c16)) {
        /* High surrogate */
        ps->__value.__ucs = ((char32_t) (c16 & 0x3ff) << 10) + 0x10000;
        ps->__count = -1;
        return 0;
    } else {
        c32 = c16;
    }
    return wcrtomb(s, (wchar_t) c32, ps);
#else
#error wchar_t size unknown
#endif
}

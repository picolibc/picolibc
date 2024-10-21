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
mbrtoc16(char16_t * __restrict pc16, const char * __restrict s, size_t n,
         _mbstate_t * __restrict ps)
{
    static NEWLIB_THREAD_LOCAL mbstate_t local_state;

    if (ps == NULL)
        ps = &local_state;

#if __SIZEOF_WCHAR_T__ == 2
    return mbrtowc((wchar_t *) pc16, s, n, ps);
#elif __SIZEOF_WCHAR_T__ == 4
    wchar_t wc;
    size_t ret;

    if (ps->__count == -1) {
        *pc16 = (ps->__value.__ucs & 0x3ff) + LOW_SURROGATE_FIRST;
        ps->__count = 0;
        ps->__value.__ucs = 0;
        return (size_t) -3;
    }
    ret = mbrtowc(&wc, s, n, ps);

    switch (ret) {
    case (size_t) -2:       /* wc not stored */
        return ret;
    case (size_t) -1:       /* error */
        return ret;
    default:
        break;
    }
    if (wc >= 0x10000) {
        ps->__value.__ucs = wc;
        ps->__count = -1;
        *pc16 = ((ps->__value.__ucs - 0x10000) >> 10) + HIGH_SURROGATE_FIRST;
    } else {
        *pc16 = wc;
    }
    return ret;
#else
#error wchar_t size unknown
#endif
}

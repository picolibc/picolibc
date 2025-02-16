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

size_t mbrtoc32(char32_t * __restrict pc32, const char * __restrict s, size_t n,
                _mbstate_t * __restrict ps)
{
    static mbstate_t local_state;

    if (ps == NULL)
        ps = &local_state;

    char32_t    c32;
    wchar_t     wc;
    size_t      ret;

#if __SIZEOF_WCHAR_T__ == 2

    /*
     * All of the complexity here deals with UTF-8 locales with 2-byte
     * wchar. In this case, for code points outside the BMP, we have
     * to wait until mbrtowc returns a low surrogate and then go
     * extract the rest of the character from the mbstate bits. This
     * uses a lot of knowledge about the internals of how
     * __utf8_mbtowc works.
     */
#endif

    ret = mbrtowc(&wc, s, n, ps);

    switch (ret) {
    case (size_t) -2:       /* wc not stored */
        return ret;
    case (size_t) -1:       /* error */
        return ret;
    default:
        break;
    }

#if __SIZEOF_WCHAR_T__ == 2

    s += ret;
    n -= ret;

    char16_t    c16 = (char16_t) wc;

    /* Check for high surrogate */
    if (char16_is_high_surrogate(c16)) {
        size_t r;
        /* See if the low surrogate is ready yet */
        r = mbrtowc(&wc, s, n, ps);
        switch (r) {
        case (size_t) -2:       /* wc not stored */
            return r;
        case (size_t) -1:       /* error */
            return r;
        default:
            break;
        }
        c16 = (char16_t) wc;
        ret += r;
    }

    /* Check for low surrogate */
    if (char16_is_low_surrogate(c16)) {
        /*
         * The first three bytes are left in the buffer, go fetch them
         * and add in the low six bits in the low surrogate
         */
        c32 = ((((char32_t)ps->__value.__wchb[0] & 0x07) << 18) |
               (((char32_t)ps->__value.__wchb[1] & 0x3f) << 12) |
               (((char32_t)ps->__value.__wchb[2] & 0x3f) << 6) |
               ((char32_t)c16 & 0x3f));
    } else {
        c32 = (char32_t) c16;
    }

#elif __SIZEOF_WCHAR_T__ == 4

    c32 = (char32_t) wc;

#else
#error wchar_t size unknown
#endif

    if (!char32_is_valid(c32)) {
        errno = EILSEQ;
        return (size_t) -1;
    }

    /* Ignore parameter pc32 if s is a null pointer */
    if(s != NULL) *pc32 = c32;

    return ret;

}

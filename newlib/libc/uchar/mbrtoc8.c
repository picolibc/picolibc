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
mbrtoc8(char8_t * __restrict pc8, const char * __restrict s, size_t n,
        mbstate_t * __restrict ps)
{
    static NEWLIB_THREAD_LOCAL mbstate_t local_state;

    if (ps == NULL)
        ps = &local_state;

    if (ps->__count < 0) {
        int count = -ps->__count;

        count--;
        *pc8 = 0x80 | ((ps->__value.__ucs >> (6*count)) & 0x3f);
        ps->__count = -count;
        return (size_t) -3;
    }

    char32_t    c32;
    size_t      ret;

    ret = mbrtoc32(&c32, s, n, ps);
    switch (ret) {
    case (size_t) -2:
    case (size_t) -1:
        return ret;
    default:
        break;
    }

    if (!char32_is_one_byte(c32)) {
        int count = 1;
        char8_t c8;

        c8 = 0xc0;
        while (c32 >= ((char32_t) 1 << (6*count + (6 - count)))) {
            c8 |= (1 << (6 - count));
            count++;
        }
        c8 |= (c32 >> (6 * count));
        ps->__count = -count;
        ps->__value.__ucs = c32;
        *pc8 = c8;
    } else {
        *pc8 = (char8_t) c32;
    }
    return ret;
}

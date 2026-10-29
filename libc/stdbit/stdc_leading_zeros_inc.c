/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Keith Packard
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

#define NAME stdc_leading_zeros

#include "local-stdbit.h"

#ifdef stdbit_t
unsigned int
NAME_SB(stdbit_t value)
{
#if __has_builtin(__builtin_stdc_leading_zeros)
    return __builtin_stdc_leading_zeros(value);
#elif __has_builtin(__builtin_clzg)
    return (unsigned int)__builtin_clzg(value, (int)sizeof(stdbit_t) * 8);
#elif __has_builtin(__builtin_clz)    && WANT_SIZE == __SIZEOF_INT__ * 8
    if (value == 0)
        return WANT_SIZE;

    return __builtin_clz(value);
#elif __has_builtin(__builtin_clzl)  && WANT_SIZE == __SIZEOF_LONG__ * 8
    if (value == 0)
        return WANT_SIZE;

    return __builtin_clzl(value);
#elif __has_builtin(__builtin_clzll) && WANT_SIZE == __SIZEOF_LONG_LONG__ * 8
    if (value == 0)
        return WANT_SIZE;

    return __builtin_clzll(value);
#else 
    if (value == 0)
        return WANT_SIZE;

#if (WANT_SIZE & (WANT_SIZE - 1)) == 0
    unsigned int ceil_p2 = WANT_SIZE;  
#else
    unsigned int ceil_p2 = 1;
    while (ceil_p2 < WANT_SIZE) {
            ceil_p2 <<= 1;
    }
#endif  
    unsigned int ret = 0;
    for (int i = ceil_p2 >> 1;  i > 0; i >>= 1 ) {
        if (value >> (WANT_SIZE - i) == 0) {
            value <<= i;
            ret += i;
        }
    }
    return ret;

#endif
}

ALIASES

#endif

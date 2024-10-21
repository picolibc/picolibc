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

#ifndef _UCHAR_LOCAL_H_
#define _UCHAR_LOCAL_H_

#define _DEFAULT_SOURCE
#include <stdbool.h>
#include <uchar.h>
#include <wchar.h>
#include <errno.h>
#include <stdint.h>

#define HIGH_SURROGATE_FIRST    0xd800
#define HIGH_SURROGATE_LAST     0xdbff
#define LOW_SURROGATE_FIRST     0xdc00
#define LOW_SURROGATE_LAST      0xdfff

static inline bool char8_is_one_byte(char8_t c)
{
    return c <= 0x7f;
}

static inline bool char8_is_first_byte(char8_t c)
{
    /* don't need to check for <= 0xff because of char8_t range */
    return 0xc0 <= c;
}

static inline bool char8_is_continuation_byte(char8_t c)
{
    return 0x80 <= c && c <= 0xbf;
}

static inline bool char16_is_surrogate(char16_t c)
{
    return HIGH_SURROGATE_FIRST <= c && c <= LOW_SURROGATE_LAST;
}

static inline bool char16_is_high_surrogate(char16_t c)
{
    return HIGH_SURROGATE_FIRST <= c && c <= HIGH_SURROGATE_LAST;
}

static inline bool char16_is_low_surrogate(char16_t c)
{
    return LOW_SURROGATE_FIRST <= c && c <= LOW_SURROGATE_LAST;
}

static inline bool char32_is_one_byte(char32_t c)
{
    return c <= 0x7f;
}

static inline bool char32_is_surrogate(char32_t c)
{
    return HIGH_SURROGATE_FIRST <= c && c <= LOW_SURROGATE_LAST;
}

static inline bool char32_needs_surrogates(char32_t c)
{
    return 0x10000 <= c;
}

static inline bool char32_is_high_surrogate(char32_t c)
{
    return HIGH_SURROGATE_FIRST <= c && c <= HIGH_SURROGATE_LAST;
}

static inline bool char32_is_low_surrogate(char32_t c)
{
    return LOW_SURROGATE_FIRST <= c && c <= LOW_SURROGATE_LAST;
}

static inline bool char32_is_valid(char32_t c)
{
    return c <= 0x10ffff && !char32_is_surrogate(c);
}

#endif /* _UCHAR_LOCAL_H_ */

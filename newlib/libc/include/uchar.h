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

#ifndef _UCHAR_H_
#define _UCHAR_H_

#include <sys/cdefs.h>
#include <machine/_default_types.h>
#include <sys/_types.h>
#define __need_size_t
#include <stddef.h>

#define __STDC_VERSION_UCHAR_H_ 202311L

_BEGIN_STD_C

#ifndef _MBSTATE_DECLARED
typedef _mbstate_t mbstate_t;
#define _MBSTATE_DECLARED
#endif

#ifndef __cpp_char8_t
typedef unsigned char char8_t;
#endif
#if !defined __cplusplus || __cplusplus < 201103L
typedef __uint_least16_t char16_t;
typedef __uint_least32_t char32_t;
#endif

size_t mbrtoc8(char8_t * __restrict pc8, const char * __restrict s, size_t n,
               _mbstate_t * __restrict ps);

size_t c8rtomb(char * __restrict s, char8_t c8, _mbstate_t * __restrict ps);

size_t mbrtoc16(char16_t * __restrict pc16, const char * __restrict s, size_t n,
                _mbstate_t * __restrict ps);

size_t c16rtomb(char * __restrict s, char16_t c16, _mbstate_t * __restrict ps);

size_t mbrtoc32(char32_t * __restrict pc32, const char * __restrict s, size_t n,
                _mbstate_t * __restrict ps);

size_t c32rtomb(char * __restrict s, char32_t c32, _mbstate_t * __restrict ps);

_END_STD_C

#endif /* _UCHAR_H_ */

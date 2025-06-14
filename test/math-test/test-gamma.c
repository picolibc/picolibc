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

#include "math_test.h"

#include "gamma_vec.h"

#ifdef INCLUDE_FLOAT_32
#define test test_gamma_32
#define array gamma_32
#define func name_32(tgamma)
#define float_t float_32_t
#define unpack unpack_32
#define format format_32
#include "test-onevar.h"
#endif

#ifdef INCLUDE_FLOAT_64
#define test test_gamma_64
#define array gamma_64
#define func name_64(tgamma)
#define float_t float_64_t
#define unpack unpack_64
#define format format_64
#include "test-onevar.h"
#endif

#ifdef INCLUDE_FLOAT_80
#define test test_gamma_80
#define array gamma_80
#define func name_80(tgamma)
#define float_t float_80_t
#define unpack unpack_80
#define format format_80
#include "test-onevar.h"
#endif

#ifdef INCLUDE_FLOAT_128
#define test test_gamma_128
#define array gamma_128
#define func name_128(tgamma)
#define float_t float_128_t
#define unpack unpack_128
#define format format_128
#include "test-onevar.h"
#endif

int main(void)
{
    int ret = 0;

#ifdef INCLUDE_FLOAT_32
    if (test_gamma_32() != 0)
        ret = 1;
#endif
#ifdef INCLUDE_FLOAT_64
    if (test_gamma_64() != 0)
        ret = 1;
#endif
#ifdef INCLUDE_FLOAT_80
    if (test_gamma_80() != 0)
        ret = 1;
#endif
#ifdef INCLUDE_FLOAT_128
    if (test_gamma_128() != 0)
        ret = 1;
#endif
    return ret;
}

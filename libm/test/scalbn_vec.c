/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2021 Keith Packard
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
#include "test.h"

one_line_type scalbn_vec[] = {
    { 64, 0, 123, __LINE__, 0x3ff00000, 0x00000000, 0x3ff00000, 0x00000000, 0x00000000, 0x00000000 }, /* 1 = f(1, 0); */
    { 64, 0, 123, __LINE__, 0x7ff00000, 0x00000000, 0x3ff00000, 0x00000000, 0x40c38800, 0x00000000 }, /* +inf = f(1, 10000); */
#if __SIZEOF_INT__ < 4
    { 64, 0, 123, __LINE__, 0x7ff00000, 0x00000000, 0x3ff00000, 0x00000000, 0x40dfffc0, 0x00000000 }, /* +inf = f(1, 0x7fff); */
#else
    { 64, 0, 123, __LINE__, 0x7ff00000, 0x00000000, 0x3ff00000, 0x00000000, 0x41dfffff, 0xffc00000 }, /* +inf = f(1, 0x7fffffff); */
#endif
    { 0 },
};

void test_scalbn(int m) { run_vector_1(m, scalbn_vec, (void*)scalbn,"scalbn","ddi"); }
void test_scalbnf(int m) { run_vector_1(m, scalbn_vec, (void*)scalbnf,"scalbnf","ffi"); }

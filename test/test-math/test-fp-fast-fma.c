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

#include <math.h>
#include <stdio.h>

/*
 * math.h must define FP_FAST_FMA* whenever machine/math.h says that
 * fma is fast, and that needs hardware floating point.
 */

int
main(void)
{
    int ret = 0;

#if __HAVE_FAST_FMA && !defined(FP_FAST_FMA)
    printf("__HAVE_FAST_FMA set but FP_FAST_FMA not defined\n");
    ret = 1;
#endif
#if __HAVE_FAST_FMAF && !defined(FP_FAST_FMAF)
    printf("__HAVE_FAST_FMAF set but FP_FAST_FMAF not defined\n");
    ret = 1;
#endif
#if __HAVE_FAST_FMAL && !defined(FP_FAST_FMAL)
    printf("__HAVE_FAST_FMAL set but FP_FAST_FMAL not defined\n");
    ret = 1;
#endif
#if defined(FP_FAST_FMAF) && defined(__PICOLIBC__) && !defined(__HAVE_HW_FLOAT)
    printf("FP_FAST_FMAF defined but __HAVE_HW_FLOAT not defined\n");
    ret = 1;
#endif
#if defined(FP_FAST_FMA) && defined(__PICOLIBC__) && !defined(__HAVE_HW_DOUBLE)
    printf("FP_FAST_FMA defined but __HAVE_HW_DOUBLE not defined\n");
    ret = 1;
#endif
#if defined(FP_FAST_FMAL) && defined(__PICOLIBC__) && !defined(__HAVE_HW_LONG_DOUBLE)
    printf("FP_FAST_FMAL defined but __HAVE_HW_LONG_DOUBLE not defined\n");
    ret = 1;
#endif
    if (ret == 0)
        printf("success\n");
    return ret;
}

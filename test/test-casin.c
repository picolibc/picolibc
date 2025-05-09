/*
* SPDX-License-Identifier: BSD-3-Clause
* 
* Copyright © 2025, Synopsys Inc.
* Copyright © 2025, Solid Sands B.V.
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

#include <stdio.h>
#include <complex.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

typedef union {
    double d;
    uint64_t u;
} hexdouble;

/* Compute ULP difference between two doubles */
int ulp_diff(double a, double b) {
    int64_t ia, ib;

    memcpy(&ia, &a, sizeof(double));
    memcpy(&ib, &b, sizeof(double));

    /* Make lexicographically ordered as a twos-complement int */
    if (ia < 0) ia = 0x8000000000000000LL - ia;
    if (ib < 0) ib = 0x8000000000000000LL - ib;

    int64_t diff = ia - ib;
    if (diff < 0) diff = -diff;
    return (int)diff;
}

int main(void) {

    /* === Input === */
    hexdouble real = { .u = 0xc01a1427b72bc63c };  /* -0x1.a1427b72bc63cp+2 */
    hexdouble imag = { .u = 0x4020af5e93c31492 };  /*  0x1.0af5e93c31492p+3 */

    /* === Reference === */
    hexdouble ref_real = { .u = 0xbfe52888f112ceb8 };  /* -0x1.52888f112ceb8p-1 */
    hexdouble ref_imag = { .u = 0x40086d5fee20e1c0 };  /*  0x1.86d5fee20e1cp+1  */

    double complex z = CMPLX(real.d, imag.d);
    double complex result = casin(z);

    printf("Input:       casin(%a + %ai)\n", real.d, imag.d);
    printf("Computed:    real = %a, imag = %a\n", creal(result), cimag(result));
    printf("Reference:   real = %a, imag = %a\n", ref_real.d, ref_imag.d);

    int ulp_re = ulp_diff(creal(result), ref_real.d);
    int ulp_im = ulp_diff(cimag(result), ref_imag.d);

    printf("ULP diff:    real = %d, imag = %d\n", ulp_re, ulp_im);

    if (ulp_re <= 4 && ulp_im <= 4) {
        printf("PASS\n");
        return 0;
    } else {
        printf("FAIL\n");
        return 1;
    }   
}

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

#include "fdlibm.h"

/* pi/2 * 2**63 */
#define PI_62 ((uint64_t)0xc90fdaa22168c235)

/* 2/pi * 2**63 */
#define HPI_INV_63_HI ((uint32_t)0x517cc1b7)
#define HPI_INV_63_LO ((uint32_t)0x27220a95)

/* Return the upper 64-bits of a * b */
static inline uint64_t
mul64hi(uint64_t a, uint64_t b)
{
#ifdef __SIZEOF_INT128__
    return ((__uint128_t)a * b) >> 64;
#else
    uint32_t ah = a >> 32, al = a;
    uint32_t bh = b >> 32, bl = b;

    uint64_t hh = (uint64_t)ah * (uint64_t)bh;
    uint64_t hl = (uint64_t)ah * (uint64_t)bl;
    uint64_t lh = (uint64_t)al * (uint64_t)bh;
    uint64_t ll = (uint64_t)al * (uint64_t)bl;
    uint64_t carry = ((uint64_t)(uint32_t)hl + (uint64_t)(uint32_t)lh + (ll >> 32)) >> 32;

    return hh + (hl >> 32) + (lh >> 32) + carry;
#endif
}

/* Table with 4/PI to 192 bit precision.  To avoid unaligned accesses
   only 8 new bits are added per entry, making the table 4 times larger.  */
static const uint32_t __inv_pio4[24]
    = { 0xa2,       0xa2f9,     0xa2f983,   0xa2f9836e, 0xf9836e4e, 0x836e4e44,
        0x6e4e4415, 0x4e441529, 0x441529fc, 0x1529fc27, 0x29fc2757, 0xfc2757d1,
        0x2757d1f5, 0x57d1f534, 0xd1f534dd, 0xf534ddc0, 0x34ddc0db, 0xddc0db62,
        0xc0db6295, 0xdb629599, 0x6295993c, 0x95993c43, 0x993c4390, 0x3c439041 };

int
__rem_pio2f(float x, float *y)
{
    int32_t  xs;
    uint32_t xi, xa;
    uint64_t rem;

    GET_FLOAT_WORD(xi, x);
    xa = xi & 0x7fffffff;

    /* < pi/4 */
    if (xa < 0x3f490fdb) {
        y[0] = x;
        y[1] = 0;
        return 0;
    }

    xs = (int32_t)xi;

    int      xexp = xa >> 23;
    uint32_t xsig = (xa & 0x7fffff) | 0x800000;

    if (xa <= 0x42f00000) {
        /* <= 120 */

        /*
         * Convert to integer between 0xe0952b (pi/4 * 2**24) and
         * 0x78000000 (120 * 2**24)
         */
        xsig = xsig << (xexp - 126);

        /*
         * Multiply by 2/pi * 2**63, keeping only the upper 64-bits of the
         * rounded 128-bit product.
         */
        uint64_t hh = (uint64_t)xsig * HPI_INV_63_HI;
        uint64_t hl = (uint64_t)xsig * HPI_INV_63_LO;

        rem = (hh + ((hl + 0x8000000) >> 32)) << 7;
    } else {
        /* > 120 */

        /* Convert to integer value */
        xsig = xsig << (xexp & 7);

        /*
         * Multiply by 96 bits out of 4/pi above, keeping the middle
         * 64 bits
         */
        const uint32_t *arr = &__inv_pio4[(xexp >> 3) & 15];
        uint64_t        h = xsig * arr[0];
        uint64_t        m = (uint64_t)xsig * arr[4];
        uint64_t        l = (uint64_t)xsig * arr[8];

        rem = (h << 32) + m + (l >> 32);
    }

    /* Flip sign if negative */
    if (xs < 0)
        rem = -rem;

    /* Compute the rounded integer value */
    uint64_t n = (rem + (1ULL << 61)) >> 62;

    /*
     * Subtract out the integer component leaving only
     * the fraction
     */
    rem -= n << 62;

    /* Convert to a signed value after the subtraction */
    int64_t irem = (int64_t)rem;

    /* Multiply the fraction by pi/2 ** 2**63 */
    if (irem < 0)
        irem = -(int64_t)mul64hi((uint64_t)-irem, PI_62);
    else
        irem = mul64hi(irem, PI_62);

    /* Convert to floating point */
    float   y0 = irem;

    int64_t iy0;

#if defined(__RX__)
    /* float to 128-bit int conversions are broken on this target */
    int32_t y0i;
    GET_FLOAT_WORD(y0i, y0);
    uint32_t sig = (y0i & 0xffffff) | 0x800000;
    int      exp = ((y0i >> 23) & 0xff) - (127 + 23);

    iy0 = (int64_t)sig;
    if (exp >= 0)
        iy0 <<= exp;
    else
        iy0 >>= -exp;
    if (y0i < 0)
        iy0 = -iy0;
#else
    iy0 = (int64_t)y0;
#endif

    float y1 = irem - iy0;

    /* Scale back to the final range */
    y[0] = y0 * 0x1p-61f;
    y[1] = y1 * 0x1p-61f;
    return n & 3;
}

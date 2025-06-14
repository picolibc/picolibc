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

#include "double-math.h"

#define DD_SPLIT_BITS   ((__DBL_MANT_DIG__ + 1) >> 1)
#define DD_SPLIT        ((double) ((1UL << DD_SPLIT_BITS) + 1))

dd_t __dd_split(double a)
{
    double      t;
    dd_t        r;

    t = a * DD_SPLIT;
    r.h = t - (t - a);
    r.l = a - r.h;
    return r;
}

dd_t __dd_quick_two_sum(double a, double b)
{
    dd_t        r;

    r.h = a + b;
    r.l = (b - (r.h - a));
    return r;
}

dd_t __dd_two_sum(double a, double b)
{
    dd_t        r;
    double      v;

    r.h = a + b;
    v = r.h - a;
    r.l = (a - (r.h - v)) + (b - v);
    return r;
}

dd_t __dd_two_prod(double a, double b)
{
    dd_t        r;
    dd_t        ad, bd;

    r.h = a * b;
    ad = __dd_split(a);
    bd = __dd_split(b);
    r.l = ((ad.h * bd.h - r.h) + (ad.h * bd.l) + (ad.l * bd.h)) + (ad.l * bd.l);
    return l;
}

dd_t __dd_three_sum(double x, double y, double z)
{
    dd_t        uv = __dd_two_sum(x, y);
    dd_t        rw = __dd_two_sum(u.h, z);
    dd_t        r;

    r.h = rw.h;
    r.l = uv.l + rw.l;
    return r;
}

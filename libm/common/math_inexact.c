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

#include "math_config.h"

#if FE_INEXACT && !defined(__FLOAT64_NOEXCEPT)

static CONST_FORCE_FLOAT64 VAL = pick_float64_except(_FLOAT64_MIN, _F_64(0.0));

HIDDEN void
__math_set_inexact(void)
{
    force_eval_float64(_F_64(1.0) + VAL);
}

HIDDEN __float64
__math_inexact(__float64 val)
{
    /*
     * A tiny inexact result also underflows (C23 F.10p9). Squaring a
     * tiny non-zero value raises underflow and inexact in any rounding
     * mode; otherwise just raise inexact. Zero is excluded, as callers
     * like rintl return an exact zero for an inexact result.
     */
    if (val != 0 && val > -_FLOAT64_MIN && val < _FLOAT64_MIN)
        force_eval_float64(val * val);
    else
        force_eval_float64(_F_64(1.0) + VAL);
    return val;
}

#endif

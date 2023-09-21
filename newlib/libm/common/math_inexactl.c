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

#if defined(FE_INEXACT) && !defined(PICOLIBC_LONG_DOUBLE_NOEXECPT) && defined(_NEED_FLOAT_HUGE)

#ifdef _DOUBLE_DOUBLE_FLOAT
static CONST_FORCE_LONG_DOUBLE VAL = pick_long_double_except(LDBL_MIN, 0.0L);
static CONST_FORCE_LONG_DOUBLE VAL1 = pick_long_double_except(LDBL_MAX, 0.0L);
#define eqn (1.0L + VAL + VAL1)
#else
static CONST_FORCE_LONG_DOUBLE VAL = pick_long_double_except(LDBL_MIN, 0.0L);
#define eqn (1.0L + VAL)
#endif

HIDDEN void
__math_set_inexactl(void)
{
    force_eval_long_double(eqn);
}

HIDDEN long double
__math_inexactl(long double val)
{
    force_eval_long_double(eqn);
    return val;
}

#endif

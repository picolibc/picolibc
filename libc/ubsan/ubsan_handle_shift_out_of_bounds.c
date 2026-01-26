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

#include "ubsan.h"

void
__ubsan_handle_shift_out_of_bounds(void *_data, void *lhs, void *rhs)
{
    struct shift_out_of_bounds_data *data = _data;
#if 0
    /*
     * Allow implementation defined behavior where LHS is signed but
     * the shift is in-range. This is common when we want an arithmetic
     * right shift with negative values. This means we're also silencing
     * left shift with negative values, which is UB.
     */

    if (data->lhs_type->type_kind == type_kind_int)
    {
        unsigned int width = type_int_width(data->lhs_type);
        if (type_is_signed(data->rhs_type)) {
            intmax_t i = __ubsan_val_to_imax(data->rhs_type, rhs, 1024);
            if (i >= 0 && width > i)
                return;
        } else {
            if (width > __ubsan_val_to_umax(data->rhs_type, rhs, 1024))
                return;
        }
    }
#endif

    char lhs_str[VAL_STR_LEN];
    char rhs_str[VAL_STR_LEN];
    __ubsan_val_to_string(lhs_str, data->lhs_type, lhs);
    __ubsan_val_to_string(rhs_str, data->rhs_type, rhs);
    __ubsan_error(&data->location, "shift_out_of_bounds", "%s <> %s\n", lhs_str, rhs_str);
}

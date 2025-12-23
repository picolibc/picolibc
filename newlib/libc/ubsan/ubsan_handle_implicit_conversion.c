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

static const char * const implicit_conversion_kinds[] = {
    [implicit_conversion_integer_truncation] = "integer truncation",
    [implicit_conversion_unsigned_integer_truncation] = "unsigned integer truncation",
    [implicit_conversion_signed_integer_truncation] = "signed integer truncation",
    [implicit_conversion_integer_sign_change] = "integer sign change",
    [implicit_conversion_signed_integer_truncation_or_sign_change]
    = "signed integer truncation or sign change",
};

void
__ubsan_handle_implicit_conversion(void *_data, void *src, void *dst)
{
    struct implicit_conversion_data *data = _data;
    char                             src_str[VAL_STR_LEN];
    char                             dst_str[VAL_STR_LEN];
    const char                      *kind;
    __ubsan_val_to_string(src_str, data->from_type, src);
    __ubsan_val_to_string(dst_str, data->to_type, dst);
    if (data->kind < sizeof(implicit_conversion_kinds) / sizeof(implicit_conversion_kinds[0]))
        kind = implicit_conversion_kinds[data->kind];
    else
        kind = "unknown conversion kind";
    __ubsan_error(&data->location, "implicit_conversion", "%s: (%s) %s -> (%s) %s\n", kind,
                  data->from_type->type_name, src_str, data->to_type->type_name, dst_str);
}

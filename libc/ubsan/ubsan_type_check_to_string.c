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

static const char * const type_check_kinds[] = {
    [type_check_load_of] = "load_of",
    [type_check_store_to] = "store_to",
    [type_check_reference_binding_to] = "reference_binding_to",
    [type_check_member_access_within] = "member_access_within",
    [type_check_member_call_on] = "member_call_on",
    [type_check_constructor_call_on] = "constructor_call_on",
    [type_check_downcast_of_0] = "downcast_of_0",
    [type_check_downcast_of_1] = "downcast_of_1",
    [type_check_upcast_of] = "upcast_of",
    [type_check_cast_to_virtual_base_of] = "cast_to_virtual_base_of",
    [type_check_nonnull_binding_to] = "nonnull_binding_to",
    [type_check_dynamic_operation_on] = "dynamic_operation_on",
};

const char *
__ubsan_type_check_to_string(unsigned char type_check_kind)
{
    if (type_check_kind < sizeof(type_check_kinds) / sizeof(type_check_kinds[0]))
        return type_check_kinds[type_check_kind];
    return "unknown";
}

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

static const char * const cfi_type_check_kinds[] = {
    [cfi_type_check_v_call] = "cfi_type_check_v_call",
    [cfi_type_check_nv_call] = "cfi_type_check_nv_call",
    [cfi_type_check_derived_cast] = "cfi_type_check_derived_cast",
    [cfi_type_check_unrelated_cast] = "cfi_type_check_unrelated_cast",
    [cfi_type_check_i_call] = "cfi_type_check_i_call",
    [cfi_type_check_nvmf_call] = "cfi_type_check_nvmf_call",
    [cfi_type_check_vmf_call] = "cfi_type_check_vmf_call",
};

const char *
__ubsan_cfi_type_check_to_string(unsigned char cfi_type_check_kind)
{
    if (cfi_type_check_kind < sizeof(cfi_type_check_kinds) / sizeof(cfi_type_check_kinds[0]))
        return cfi_type_check_kinds[cfi_type_check_kind];
    return "unknown";
}

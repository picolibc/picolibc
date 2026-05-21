/*
Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
SPDX-License-Identifier: BSD-3-Clause-Clear

Redistribution and use in source and binary forms, with or without
modification, are permitted (subject to the limitations in the
disclaimer below) provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials provided
    with the distribution.

  * Neither the name of Qualcomm Technologies, Inc. nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * __tls_get_addr for the Hexagon libdl port.
 *
 * A shared object built with -ftls-model=global-dynamic accesses each
 * thread-local variable by calling __tls_get_addr with a pointer to the
 * variable's GOT TLS descriptor:
 *
 *     struct { uint32_t ti_module; uint32_t ti_offset; };
 *
 * The loader fills these two words while applying relocations:
 *   - R_HEX_DTPMOD_32 sets ti_module (see dl_relocate.c; ignored here).
 *   - R_HEX_DTPREL_32 (or a link-time constant) sets ti_offset, the byte
 *     offset of the variable within the module's TLS block.
 *
 * dl_arch_set_tls() (dl_relocate.c) points the Hexagon thread pointer (UGP)
 * at the base of the per-module TLS block that dlopen() allocated.  This
 * routine therefore returns UGP + ti_offset.  The three pieces —
 * dl_arch_set_tls (UGP = block base), the R_HEX_DTPREL_32 handler
 * (base-relative offset) and this function — must stay consistent.
 */

#include <stdint.h>

struct hex_tls_index {
    uint32_t ti_module; /* module ID (unused, see FIXME below) */
    uint32_t ti_offset; /* byte offset within the module's TLS block */
};

void *__tls_get_addr(struct hex_tls_index *ti);

void *
__tls_get_addr(struct hex_tls_index *ti)
{
    uintptr_t ugp;

    /* UGP is the base of the TLS block set by dl_arch_set_tls(). */
    __asm__ volatile("%0 = ugp" : "=r"(ugp));

    /*
     * FIXME: ti_module is ignored.  This supports only a single active TLS
     * block (one dlopen'd module using TLS at a time), matching the current
     * one-UGP-at-a-time design in dl_arch_set_tls().  A full multi-module
     * RTLD would index a per-thread DTV by ti_module here.
     */
    return (void *)(ugp + ti->ti_offset);
}

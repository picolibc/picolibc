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

#include "dl_arch.h"
#include "../../dl_impl.h"
#include <stdint.h>

/*
 * Apply a single RELA section's relocations.
 * Returns 0 on success, -1 if an unknown relocation type is encountered.
 */
static int
apply_relas(dl_handle_t *h, Elf32_Rela *relas, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        Elf32_Rela *r     = &relas[i];
        uint32_t   *addr  = (uint32_t *)(h->load_bias + r->r_offset);
        uint32_t    sym_idx = ELF32_R_SYM(r->r_info);
        uint32_t    type    = ELF32_R_TYPE(r->r_info);
        uintptr_t   sym_val = 0;

        if (sym_idx != 0) {
            Elf32_Sym *sym = &h->dynsym[sym_idx];
            if (sym->st_value != 0) {
                /* Symbol defined inside the shared library */
                sym_val = h->load_bias + sym->st_value;
            } else {
                /* Undefined symbol — resolve from built-in table.
                 * FIXME: on Linux, resolve via the OS dynamic linker's global
                 * symbol namespace (searching the main exe + all loaded
                 * libraries) instead of a hardcoded built-in table.
                 */
                sym_val = (uintptr_t)dl_builtin_lookup(
                    h->dynstr + sym->st_name);
            }
        }

        switch (type) {
        case R_HEX_RELATIVE:
            /* Base-relative: *addr = load_bias + addend */
            *addr = (uint32_t)(h->load_bias + (uintptr_t)(uint32_t)r->r_addend);
            break;

        case R_HEX_JMP_SLOT:
            /* PLT slot: *addr = sym_val (no addend) */
            /* FIXME: with RTLD_LAZY, set *addr to a _dl_runtime_resolve trampoline instead */
            *addr = (uint32_t)sym_val;
            break;

        case R_HEX_GLOB_DAT:
            /* GOT entry: *addr = sym_val + addend */
            *addr = (uint32_t)(sym_val + (uintptr_t)(uint32_t)r->r_addend);
            break;

        case R_HEX_32:
            /* Absolute 32-bit: *addr = sym_val + addend */
            *addr = (uint32_t)(sym_val + (uintptr_t)(uint32_t)r->r_addend);
            break;

        case R_HEX_DTPMOD_32:
            /* TLS descriptor module ID.
             * The bare-metal __tls_get_addr ignores this field; set it to 1
             * (a valid non-zero module ID) for any full RTLD that may be used.
             */
            *addr = 1;
            break;

        case R_HEX_DTPREL_32:
            /* TLS descriptor offset within the module's TLS block.
             * The compiler loads this value into r0 before calling
             * __tls_get_addr, which returns UGP + r0.
             * The addend is the byte offset of the variable in the TLS block.
             */
            *addr = (uint32_t)r->r_addend;
            break;

        default:
            /* Unknown relocation type — cannot safely continue */
            h->last_reloc_type = type;
            return -1;
        }
    }
    return 0;
}

/*
 * dl_arch_relocate - apply all relocations for a loaded Hexagon shared lib.
 * Called by dl.c after segments are mapped and the handle is populated.
 * Returns 0 on success, -1 on error.
 */
int
dl_arch_relocate(dl_handle_t *h)
{
    if (apply_relas(h, h->rela_dyn, h->rela_dyn_count) != 0)
        return -1;
    if (apply_relas(h, h->rela_plt, h->rela_plt_count) != 0)
        return -1;
    return 0;
}

/*
 * dl_arch_set_tls - set the Hexagon UGP register to tls_block.
 *
 * UGP is the Hexagon thread pointer.  The bare-metal __tls_get_addr
 * computes: result = UGP + ti_tlsoffset, so UGP must point to the base
 * of the TLS block before any TLS variable in the loaded .so is accessed.
 *
 * Returns the previous UGP value so the caller can restore it in dlclose().
 */
uintptr_t
dl_arch_set_tls(void *tls_block)
{
    uintptr_t old_ugp;
    __asm__ volatile("r0 = ugp; %0 = r0" : "=r"(old_ugp) : : "r0");
    __asm__ volatile("ugp = %0" : : "r"(tls_block));
    return old_ugp;
}

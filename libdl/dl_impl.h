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

#ifndef _DL_IMPL_H_
#define _DL_IMPL_H_

#include <stdint.h>
#include <stddef.h>
#include "elf32.h"

/*
 * Internal handle representing a loaded shared library.
 * All pointer fields point into load_buf (already relocated by load_bias).
 */
typedef struct dl_handle {
    uint8_t    *load_buf;  /* malloc'd + aligned segment memory        */
    uintptr_t   load_bias; /* = (uintptr_t)load_buf - min_vaddr        */

    Elf32_Sym  *dynsym;  /* .dynsym base                             */
    const char *dynstr;  /* .dynstr base                             */
    uint32_t    nsyms;   /* number of .dynsym entries                */
    uint32_t   *hashtab; /* .hash base: [nbucket][nchain][b...][c...]*/

    Elf32_Rela *rela_dyn;       /* .rela.dyn base                           */
    size_t      rela_dyn_count; /* number of .rela.dyn entries              */
    Elf32_Rela *rela_plt;       /* .rela.plt base                           */
    size_t      rela_plt_count; /* number of .rela.plt entries              */

    void      **init_array;   /* DT_INIT_ARRAY base                       */
    size_t      init_arraysz; /* DT_INIT_ARRAYSZ in bytes                 */
    void      **fini_array;   /* DT_FINI_ARRAY base                       */
    size_t      fini_arraysz; /* DT_FINI_ARRAYSZ in bytes                 */

    uint8_t    *tls_block; /* malloc'd TLS block (PT_TLS)              */
    uint32_t    tls_memsz; /* PT_TLS p_memsz                           */
    uintptr_t   saved_ugp; /* UGP value saved before dlopen            */

    uint32_t    last_reloc_type; /* set by dl_arch_relocate on unknown type  */
} dl_handle_t;

/*
 * Arch-specific: apply all relocations in h->rela_dyn and h->rela_plt.
 * Returns 0 on success, -1 on error (unknown relocation type encountered).
 * Implemented in machine/<arch>/dl_relocate.c
 */
int       dl_arch_relocate(dl_handle_t *h);

/*
 * Arch-specific: set the thread pointer to tls_block, return the previous
 * thread pointer value so it can be restored in dlclose().
 * Implemented in machine/<arch>/dl_relocate.c
 */
uintptr_t dl_arch_set_tls(void *tls_block);

/*
 * Arch-specific: look up a symbol name in the built-in symbol table.
 * Returns the function/data pointer, or NULL if not found.
 * Implemented in machine/<arch>/dl_syms.c
 */
void     *dl_builtin_lookup(const char *name);

#endif /* _DL_IMPL_H_ */

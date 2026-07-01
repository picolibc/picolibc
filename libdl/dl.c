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
 * dl.c — minimal dlopen/dlsym/dlclose/dlerror for picolibc.
 *
 * Reads a shared library using standard POSIX file I/O (open/lseek/read/close),
 * maps its LOAD segments into malloc'd memory, applies all RELA relocations
 * via the arch-specific dl_arch_relocate(), then runs DT_INIT_ARRAY.
 *
 * Only ET_DYN ELF32 files with RELA relocations are supported.
 * The arch-specific relocation handler and built-in symbol table live
 * in machine/<arch>/dl_relocate.c and machine/<arch>/dl_syms.c.
 */

#include <dlfcn.h>
#include "elf32.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "dl_impl.h"

/* ------------------------------------------------------------------ */
/* Error handling                                                       */
/* ------------------------------------------------------------------ */

static char dl_errbuf[128];
static int  dl_has_error;

static void
dl_set_error(const char *msg)
{
    strncpy(dl_errbuf, msg, sizeof(dl_errbuf) - 1);
    dl_errbuf[sizeof(dl_errbuf) - 1] = '\0';
    dl_has_error = 1;
}

char *
dlerror(void)
{
    if (!dl_has_error)
        return NULL;
    dl_has_error = 0;
    return dl_errbuf;
}

/* ------------------------------------------------------------------ */
/* ELF hash (SysV)                                                      */
/* ------------------------------------------------------------------ */

static uint32_t
elf_hash(const char *name)
{
    uint32_t h = 0, g;
    while (*name) {
        h = (h << 4) + (uint8_t)*name++;
        if ((g = h & 0xf0000000u))
            h ^= g >> 24;
        h &= ~g;
    }
    return h;
}

/* ------------------------------------------------------------------ */
/* dlopen                                                               */
/* ------------------------------------------------------------------ */

void *
dlopen(const char *path, int mode)
{
    (void)mode; /* always RTLD_NOW — eager relocation */
    /* FIXME: honour RTLD_LAZY via a _dl_runtime_resolve PLT trampoline */

    /* 1. Read the .so file via POSIX file I/O */
    /* FIXME: on Linux use mmap(fd, ...) instead of malloc+read for zero-copy file loading */
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        dl_set_error("dlopen: cannot open file");
        return NULL;
    }

    /* Get file size with lseek — works on any POSIX OS or semihosting */
    off_t file_size_off = lseek(fd, 0, SEEK_END);
    if (file_size_off <= 0) {
        close(fd);
        dl_set_error("dlopen: cannot determine file size");
        return NULL;
    }
    if (lseek(fd, 0, SEEK_SET) != 0) {
        close(fd);
        dl_set_error("dlopen: cannot seek to start of file");
        return NULL;
    }
    size_t file_size = (size_t)file_size_off;

    uint8_t *file_buf = malloc(file_size);
    if (!file_buf) {
        close(fd);
        dl_set_error("dlopen: out of memory reading file");
        return NULL;
    }

    ssize_t nread = read(fd, file_buf, file_size);
    close(fd);
    if (nread < 0 || (size_t)nread != file_size) {
        free(file_buf);
        dl_set_error("dlopen: short read");
        return NULL;
    }

    /* 2. Validate ELF header */
    if (file_size < sizeof(Elf32_Ehdr)) {
        free(file_buf);
        dl_set_error("dlopen: file too small for ELF header");
        return NULL;
    }

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)file_buf;

    if (ehdr->e_ident[EI_MAG0] != ELFMAG0 ||
        ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
        ehdr->e_ident[EI_MAG2] != ELFMAG2 ||
        ehdr->e_ident[EI_MAG3] != ELFMAG3) {
        free(file_buf);
        dl_set_error("dlopen: not an ELF file");
        return NULL;
    }

    if (ehdr->e_type != ET_DYN) {
        free(file_buf);
        dl_set_error("dlopen: not a shared library (ET_DYN)");
        return NULL;
    }

    /* 3. Walk PT_LOAD segments: determine total span */
    Elf32_Phdr *phdrs = (Elf32_Phdr *)(file_buf + ehdr->e_phoff);
    uint32_t min_vaddr = UINT32_MAX, max_vaddr = 0;

    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdrs[i].p_type != PT_LOAD)
            continue;
        if (phdrs[i].p_vaddr < min_vaddr)
            min_vaddr = phdrs[i].p_vaddr;
        uint32_t end = phdrs[i].p_vaddr + phdrs[i].p_memsz;
        if (end > max_vaddr)
            max_vaddr = end;
    }

    if (min_vaddr == UINT32_MAX || max_vaddr == 0) {
        free(file_buf);
        dl_set_error("dlopen: no PT_LOAD segments");
        return NULL;
    }

    /*
     * FIXME: on Linux use per-segment mmap(MAP_PRIVATE|MAP_FIXED, fd, p_offset)
     * with correct PROT_READ|PROT_EXEC / PROT_READ|PROT_WRITE flags instead of
     * malloc+memcpy.  This gives zero-copy, demand-paged, W^X-correct loading.
     */
    size_t load_size = (size_t)(max_vaddr - min_vaddr);
    uint8_t *raw_buf = malloc(load_size + 0x1000);
    if (!raw_buf) {
        free(file_buf);
        dl_set_error("dlopen: out of memory for segments");
        return NULL;
    }

    /* Align load_buf to 0x1000 */
    uint8_t *load_buf = (uint8_t *)(((uintptr_t)raw_buf + 0xfff) & ~(uintptr_t)0xfff);
    uintptr_t load_bias = (uintptr_t)load_buf - (uintptr_t)min_vaddr;

    /* Zero the entire region first (handles BSS) */
    memset(load_buf, 0, load_size);

    /* Copy each PT_LOAD segment */
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdrs[i].p_type != PT_LOAD)
            continue;
        memcpy(load_buf + (phdrs[i].p_vaddr - min_vaddr),
               file_buf + phdrs[i].p_offset,
               phdrs[i].p_filesz);
    }

    /* 4. Allocate and populate the handle */
    dl_handle_t *h = calloc(1, sizeof(dl_handle_t));
    if (!h) {
        free(raw_buf);
        free(file_buf);
        dl_set_error("dlopen: out of memory for handle");
        return NULL;
    }

    h->load_buf  = raw_buf;   /* keep raw_buf for free() */
    h->load_bias = load_bias;

    /* 4b. Parse PT_TLS: allocate and initialise the TLS block.
     *
     * The .tdata section (p_filesz bytes) holds initialised TLS data and
     * is already mapped inside the PT_LOAD region.  We allocate a fresh
     * block, copy .tdata into it, and zero the .tbss tail.
     * UGP is set to the base of this block after relocation (step 6b).
     */
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdrs[i].p_type != PT_TLS)
            continue;

        h->tls_memsz = phdrs[i].p_memsz;
        h->tls_block = malloc(h->tls_memsz);
        if (!h->tls_block) {
            free(raw_buf);
            free(file_buf);
            free(h);
            dl_set_error("dlopen: out of memory for TLS block");
            return NULL;
        }
        /* Zero the entire block first (covers .tbss and any padding).
         * Use explicit memset rather than calloc so that MALLOC_PERTURB_
         * (which fills malloc'd memory with a non-zero pattern) does not
         * corrupt zero-initialised TLS variables.
         */
        memset(h->tls_block, 0, h->tls_memsz);
        /* Copy initialised TLS data (.tdata) */
        if (phdrs[i].p_filesz > 0)
            memcpy(h->tls_block,
                   load_buf + (phdrs[i].p_vaddr - min_vaddr),
                   phdrs[i].p_filesz);
        break; /* only one PT_TLS */
    }

    /* 5. Parse PT_DYNAMIC to locate all needed sections */
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdrs[i].p_type != PT_DYNAMIC)
            continue;

        Elf32_Dyn *dyn = (Elf32_Dyn *)(load_bias + phdrs[i].p_vaddr);
        uintptr_t rela_dyn_addr = 0, rela_plt_addr = 0;
        size_t    rela_dyn_sz   = 0, rela_plt_sz   = 0;
        uintptr_t init_array_addr = 0, fini_array_addr = 0;

        for (; dyn->d_tag != DT_NULL; dyn++) {
            switch (dyn->d_tag) {
            case DT_SYMTAB:
                h->dynsym = (Elf32_Sym *)(load_bias + dyn->d_un.d_ptr);
                break;
            case DT_STRTAB:
                h->dynstr = (const char *)(load_bias + dyn->d_un.d_ptr);
                break;
            case DT_HASH:
                h->hashtab = (uint32_t *)(load_bias + dyn->d_un.d_ptr);
                break;
            case DT_RELA:
                rela_dyn_addr = load_bias + dyn->d_un.d_ptr;
                break;
            case DT_RELASZ:
                rela_dyn_sz = dyn->d_un.d_val;
                break;
            case DT_JMPREL:
                rela_plt_addr = load_bias + dyn->d_un.d_ptr;
                break;
            case DT_PLTRELSZ:
                rela_plt_sz = dyn->d_un.d_val;
                break;
            case DT_INIT_ARRAY:
                init_array_addr = load_bias + dyn->d_un.d_ptr;
                break;
            case DT_INIT_ARRAYSZ:
                h->init_arraysz = dyn->d_un.d_val;
                break;
            case DT_FINI_ARRAY:
                fini_array_addr = load_bias + dyn->d_un.d_ptr;
                break;
            case DT_FINI_ARRAYSZ:
                h->fini_arraysz = dyn->d_un.d_val;
                break;
            default:
                break;
            }
        }

        if (rela_dyn_addr) {
            h->rela_dyn       = (Elf32_Rela *)rela_dyn_addr;
            h->rela_dyn_count = rela_dyn_sz / sizeof(Elf32_Rela);
        }
        if (rela_plt_addr) {
            h->rela_plt       = (Elf32_Rela *)rela_plt_addr;
            h->rela_plt_count = rela_plt_sz / sizeof(Elf32_Rela);
        }
        if (init_array_addr)
            h->init_array = (void **)init_array_addr;
        if (fini_array_addr)
            h->fini_array = (void **)fini_array_addr;

        /* Derive nsyms from .hash nchain (= number of dynsym entries) */
        if (h->hashtab)
            h->nsyms = h->hashtab[1]; /* nchain */

        break; /* only one PT_DYNAMIC */
    }

    /* 6. Apply all relocations (arch-specific) */
    if (dl_arch_relocate(h) != 0) {
        /* Format error message with the unknown relocation type number */
        char reloc_errbuf[64];
        snprintf(reloc_errbuf, sizeof(reloc_errbuf),
                 "dlopen: unsupported relocation type %u", h->last_reloc_type);
        dl_set_error(reloc_errbuf);
        free(h->tls_block);
        free(file_buf);
        free(raw_buf);
        free(h);
        return NULL;
    }

    /* 6b. Set thread pointer (UGP) to the TLS block, saving the old value
     *     so dlclose() can restore it.  Must happen after relocation so
     *     that the DTPREL GOT entries are already filled in.
     */
    if (h->tls_block)
        h->saved_ugp = dl_arch_set_tls(h->tls_block);

    /* 7. Run DT_INIT_ARRAY constructors */
    if (h->init_array) {
        size_t n = h->init_arraysz / sizeof(void *);
        for (size_t i = 0; i < n; i++) {
            void (*fn)(void) = (void (*)(void))h->init_array[i];
            if (fn)
                fn();
        }
    }

    free(file_buf);
    return h;
}

/* ------------------------------------------------------------------ */
/* dlsym                                                                */
/* ------------------------------------------------------------------ */

void *
dlsym(void *handle, const char *symbol)
{
    if (!handle || !symbol)
        return NULL;

    dl_handle_t *h = (dl_handle_t *)handle;

    if (!h->hashtab || !h->dynsym || !h->dynstr)
        return NULL;

    uint32_t nbucket = h->hashtab[0];
    uint32_t *bucket = &h->hashtab[2];
    uint32_t *chain  = &h->hashtab[2 + nbucket];

    uint32_t idx = bucket[elf_hash(symbol) % nbucket];
    while (idx != 0 && idx < h->nsyms) {
        Elf32_Sym *sym = &h->dynsym[idx];
        if (strcmp(h->dynstr + sym->st_name, symbol) == 0 &&
            sym->st_value != 0) {
            return (void *)(h->load_bias + sym->st_value);
        }
        idx = chain[idx];
    }

    dl_set_error("dlsym: symbol not found");
    return NULL;
}

/* ------------------------------------------------------------------ */
/* dlclose                                                              */
/* ------------------------------------------------------------------ */

int
dlclose(void *handle)
{
    if (!handle)
        return 0;

    dl_handle_t *h = (dl_handle_t *)handle;

    /* Run DT_FINI_ARRAY destructors in reverse order */
    if (h->fini_array) {
        size_t n = h->fini_arraysz / sizeof(void *);
        for (size_t i = n; i > 0; i--) {
            void (*fn)(void) = (void (*)(void))h->fini_array[i - 1];
            if (fn)
                fn();
        }
    }

    free(h->load_buf);

    /* Restore the thread pointer and free the TLS block */
    if (h->tls_block) {
        dl_arch_set_tls((void *)h->saved_ugp);
        free(h->tls_block);
    }

    free(h);
    return 0;
}

/* ------------------------------------------------------------------ */
/* dladdr — stub (not needed for testing)                               */
/* ------------------------------------------------------------------ */

int
dladdr(const void *addr, Dl_info *info)
{
    (void)addr; (void)info;
    return 0;
}

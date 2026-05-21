/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
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

#ifndef _DLFCN_H
#define _DLFCN_H

#include <sys/cdefs.h>
#include <stddef.h>

/*
 * Flags for the 'mode' argument of dlopen().
 *
 * RTLD_LAZY and RTLD_NOW are accepted for API compatibility; this
 * implementation always performs eager (immediate) relocation, so
 * both flags have the same effect.
 */
#define RTLD_LAZY   0x0001 /* Relocations performed lazily (treated as NOW) */
#define RTLD_NOW    0x0002 /* Relocations performed immediately at load time */
#define RTLD_GLOBAL 0x0100 /* Symbols exported to subsequently loaded libraries */
#define RTLD_LOCAL  0x0000 /* Symbols not exported (default) */
#define RTLD_NOLOAD 0x0004 /* Do not load; return handle if already loaded */

/*
 * Special pseudo-handle values for dlsym().
 *
 * RTLD_DEFAULT: search all currently loaded libraries in load order.
 * RTLD_NEXT:    search libraries loaded after the caller's library.
 */
#define RTLD_DEFAULT ((void *)0)
#define RTLD_NEXT    ((void *)-1)

/*
 * Information structure filled by dladdr().
 */
typedef struct {
    const char *dli_fname; /* Pathname (logical name) of the shared object */
    void       *dli_fbase; /* Load base address of the shared object */
    const char *dli_sname; /* Name of the nearest symbol at or before addr */
    void       *dli_saddr; /* Exact address of that symbol */
} Dl_info;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * dlopen() - Load a shared library.
 *
 * 'path' is a filename on the semihosting filesystem (e.g. "libc.so").
 * The file is read into a malloc'd buffer via semihosting I/O, then
 * parsed and relocated in place.  No filesystem abstraction layer is
 * required; the semihosting calls are made directly inside dlopen().
 *
 * Returns an opaque handle on success, or NULL on failure.
 * Call dlerror() to retrieve the error message after a NULL return.
 */
void *dlopen(const char *path, int mode);

/*
 * dlsym() - Look up a symbol in a loaded library.
 *
 * 'handle' must be a value returned by dlopen(), or one of the
 * special pseudo-handles RTLD_DEFAULT or RTLD_NEXT.
 *
 * Returns the symbol's address, or NULL if not found.
 */
void *dlsym(void *handle, const char *symbol);

/*
 * dlclose() - Unload a shared library.
 *
 * Calls all DT_FINI_ARRAY functions in reverse order, then frees the
 * memory allocated for the loaded segments.
 *
 * Returns 0 on success, non-zero on error.
 */
int   dlclose(void *handle);

/*
 * dlerror() - Return the last dynamic linking error message.
 *
 * Returns a pointer to a static string describing the most recent
 * error, or NULL if no error has occurred since the last call.
 * The error state is cleared after each call (POSIX semantics).
 *
 * Note: this implementation uses a plain global buffer and is not
 * thread-safe.
 */
char *dlerror(void);

/*
 * dladdr() - Reverse-map an address to a symbol.
 *
 * Searches all loaded libraries for the symbol whose address is
 * closest to (but not greater than) 'addr', and fills *info.
 *
 * Returns non-zero on success, 0 if no matching symbol was found.
 */
int   dladdr(const void *addr, Dl_info *info);

#ifdef __cplusplus
}
#endif

#endif /* _DLFCN_H */

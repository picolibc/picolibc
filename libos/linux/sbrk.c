/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Keith Packard
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

#include "local-linux.h"
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/mman.h>

static void *real_brk;
static void *reported_brk;
static bool  use_mmap;

#define BRK_CHUNK (1024 * 1024)
#define BRK_MASK  (BRK_CHUNK - 1)

void * __disable_sanitizer
sbrk(intptr_t increment)
{
    if (real_brk == NULL) {
        real_brk = (void *)_syscall(LINUX_SYS_brk, 0);
        reported_brk = real_brk;
    }

    void *new_brk = reported_brk + increment;
    void *old_brk = reported_brk;

    if ((uintptr_t)new_brk < (uintptr_t)old_brk)
        return (void *)(uintptr_t)-1;

    if ((uintptr_t)new_brk > (uintptr_t)real_brk) {
        void *old_real_brk = real_brk;
        void *new_real_brk = (void *)(((uintptr_t)new_brk + BRK_MASK) & ~(uintptr_t)BRK_MASK);
        void *ret = old_real_brk;

        if (!use_mmap)
            ret = (void *)_syscall(LINUX_SYS_brk, new_real_brk);
        if (ret == old_real_brk) {
            /*
             * Something mapped after the break stops it from growing;
             * take memory from an anonymous mapping instead. malloc
             * copes with the gap.
             */
            size_t size = ((size_t)increment + BRK_MASK) & ~(size_t)BRK_MASK;
            void  *map
                = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (map == MAP_FAILED)
                return (void *)(uintptr_t)-1;
            use_mmap = true;
            old_brk = map;
            new_brk = map + increment;
            ret = map + size;
        }
        real_brk = (void *)ret;
    }
    reported_brk = new_brk;
    return old_brk;
}

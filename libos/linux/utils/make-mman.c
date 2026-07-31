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

#define _GNU_SOURCE
#include <sys/mman.h>
#include <stdio.h>

int
main(void)
{
    printf("#define LINUX_PROT_READ 0x%x\n", PROT_READ);
    printf("#define LINUX_PROT_WRITE 0x%x\n", PROT_WRITE);
    printf("#define LINUX_PROT_EXEC 0x%x\n", PROT_EXEC);
    printf("#define LINUX_PROT_NONE 0x%x\n", PROT_NONE);
    printf("#define LINUX_MAP_SHARED 0x%x\n", MAP_SHARED);
    printf("#define LINUX_MAP_PRIVATE 0x%x\n", MAP_PRIVATE);
    printf("#define LINUX_MAP_FIXED 0x%x\n", MAP_FIXED);
#ifdef MAP_ANONYMOUS
    printf("#define LINUX_MAP_ANONYMOUS 0x%x\n", MAP_ANONYMOUS);
#endif
    printf("#define LINUX_MS_ASYNC 0x%x\n", MS_ASYNC);
    printf("#define LINUX_MS_SYNC 0x%x\n", MS_SYNC);
    printf("#define LINUX_MS_INVALIDATE 0x%x\n", MS_INVALIDATE);
    printf("#define LINUX_MCL_CURRENT 0x%x\n", MCL_CURRENT);
    printf("#define LINUX_MCL_FUTURE 0x%x\n", MCL_FUTURE);
#ifdef MCL_ONFAULT
    printf("#define LINUX_MCL_ONFAULT 0x%x\n", MCL_ONFAULT);
#endif
    printf("#define LINUX_POSIX_MADV_NORMAL 0x%x\n", POSIX_MADV_NORMAL);
    printf("#define LINUX_POSIX_MADV_SEQUENTIAL 0x%x\n", POSIX_MADV_SEQUENTIAL);
    printf("#define LINUX_POSIX_MADV_RANDOM 0x%x\n", POSIX_MADV_RANDOM);
    printf("#define LINUX_POSIX_MADV_WILLNEED 0x%x\n", POSIX_MADV_WILLNEED);
    printf("#define LINUX_POSIX_MADV_DONTNEED 0x%x\n", POSIX_MADV_DONTNEED);
#ifdef POSIX_TYPED_MEM_ALLOCATE
    printf("#define LINUX_POSIX_TYPED_MEM_ALLOCATE 0x%x\n", POSIX_TYPED_MEM_ALLOCATE);
#endif
#ifdef POSIX_TYPED_MEM_ALLOCATE_CONTIG
    printf("#define LINUX_POSIX_TYPED_MEM_ALLOCATE_CONTIG 0x%x\n", POSIX_TYPED_MEM_ALLOCATE_CONTIG);
#endif
#ifdef POSIX_TYPED_MEM_MAP_ALLOCATABLE
    printf("#define LINUX_POSIX_TYPED_MEM_MAP_ALLOCATABLE 0x%x\n", POSIX_TYPED_MEM_MAP_ALLOCATABLE);
#endif
#ifdef MADV_NORMAL
    printf("#define LINUX_MADV_NORMAL 0x%x\n", MADV_NORMAL);
#endif
#ifdef MADV_RANDOM
    printf("#define LINUX_MADV_RANDOM 0x%x\n", MADV_RANDOM);
#endif
#ifdef MADV_SEQUENTIAL
    printf("#define LINUX_MADV_SEQUENTIAL 0x%x\n", MADV_SEQUENTIAL);
#endif
#ifdef MADV_WILLNEED
    printf("#define LINUX_MADV_WILLNEED 0x%x\n", MADV_WILLNEED);
#endif
#ifdef MADV_DONTNEED
    printf("#define LINUX_MADV_DONTNEED 0x%x\n", MADV_DONTNEED);
#endif
#ifdef MADV_FREE
    printf("#define LINUX_MADV_FREE 0x%x\n", MADV_FREE);
#endif
#ifdef MADV_REMOVE
    printf("#define LINUX_MADV_REMOVE 0x%x\n", MADV_REMOVE);
#endif
#ifdef MADV_DONTFORK
    printf("#define LINUX_MADV_DONTFORK 0x%x\n", MADV_DONTFORK);
#endif
#ifdef MADV_DOFORK
    printf("#define LINUX_MADV_DOFORK 0x%x\n", MADV_DOFORK);
#endif
#ifdef MADV_MERGEABLE
    printf("#define LINUX_MADV_MERGEABLE 0x%x\n", MADV_MERGEABLE);
#endif
#ifdef MADV_UNMERGEABLE
    printf("#define LINUX_MADV_UNMERGEABLE 0x%x\n", MADV_UNMERGEABLE);
#endif
#ifdef MADV_HUGEPAGE
    printf("#define LINUX_MADV_HUGEPAGE 0x%x\n", MADV_HUGEPAGE);
#endif
#ifdef MADV_NOHUGEPAGE
    printf("#define LINUX_MADV_NOHUGEPAGE 0x%x\n", MADV_NOHUGEPAGE);
#endif
#ifdef MADV_DONTDUMP
    printf("#define LINUX_MADV_DONTDUMP 0x%x\n", MADV_DONTDUMP);
#endif
#ifdef MADV_DODUMP
    printf("#define LINUX_MADV_DODUMP 0x%x\n", MADV_DODUMP);
#endif
#ifdef MADV_WIPEONFORK
    printf("#define LINUX_MADV_WIPEONFORK 0x%x\n", MADV_WIPEONFORK);
#endif
#ifdef MADV_KEEPONFORK
    printf("#define LINUX_MADV_KEEPONFORK 0x%x\n", MADV_KEEPONFORK);
#endif
#ifdef MADV_COLD
    printf("#define LINUX_MADV_COLD 0x%x\n", MADV_COLD);
#endif
#ifdef MADV_PAGEOUT
    printf("#define LINUX_MADV_PAGEOUT 0x%x\n", MADV_PAGEOUT);
#endif
#ifdef MADV_POPULATE_READ
    printf("#define LINUX_MADV_POPULATE_READ 0x%x\n", MADV_POPULATE_READ);
#endif
#ifdef MADV_POPULATE_WRITE
    printf("#define LINUX_MADV_POPULATE_WRITE 0x%x\n", MADV_POPULATE_WRITE);
#endif
    return 0;
}

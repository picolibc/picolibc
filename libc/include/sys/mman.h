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

#ifndef _SYS_MMAN_H_
#define _SYS_MMAN_H_

#include <sys/cdefs.h>
#include <sys/_types.h>

_BEGIN_STD_C

#ifndef _SIZE_T_DECLARED
typedef __size_t size_t;
#define _SIZE_T_DECLARED
#endif

#ifndef _OFF_T_DECLARED
typedef __off_t off_t;
#define _OFF_T_DECLARED
#endif

/*
 * Protection flags (prot argument to mmap / mprotect).
 */
#define PROT_NONE  0x0 /* no access */
#define PROT_READ  0x1 /* pages can be read */
#define PROT_WRITE 0x2 /* pages can be written */
#define PROT_EXEC  0x4 /* pages can be executed */

/*
 * Mapping flags (flags argument to mmap).
 */
#define MAP_SHARED    0x001 /* share changes */
#define MAP_PRIVATE   0x002 /* changes are private */
#define MAP_FIXED     0x010 /* interpret addr exactly */
#define MAP_ANONYMOUS 0x020 /* not backed by any file */
#if __GNU_VISIBLE
#define MAP_ANON MAP_ANONYMOUS
#endif

/*
 * Error return value from mmap().
 */
#define MAP_FAILED ((void *)-1)

/*
 * Flags for msync().
 */
#define MS_ASYNC      1 /* perform asynchronous writes */
#define MS_SYNC       4 /* perform synchronous writes */
#define MS_INVALIDATE 2 /* invalidate cached data */

/*
 * Advice values for posix_madvise() (POSIX names).
 */
#define POSIX_MADV_NORMAL     0 /* no special treatment */
#define POSIX_MADV_RANDOM     1 /* expect random page references */
#define POSIX_MADV_SEQUENTIAL 2 /* expect sequential page references */
#define POSIX_MADV_WILLNEED   3 /* will need these pages */
#define POSIX_MADV_DONTNEED   4 /* will not need these pages */

/*
 * GNU madvise() advice values (superset of POSIX_MADV_*).
 */
#if __GNU_VISIBLE
#define MADV_NORMAL         POSIX_MADV_NORMAL     /* no special treatment */
#define MADV_RANDOM         POSIX_MADV_RANDOM     /* expect random page references */
#define MADV_SEQUENTIAL     POSIX_MADV_SEQUENTIAL /* expect sequential page references */
#define MADV_WILLNEED       POSIX_MADV_WILLNEED   /* will need these pages */
#define MADV_DONTNEED       POSIX_MADV_DONTNEED   /* will not need these pages */
#define MADV_FREE           8                     /* free pages, but keep mapping */
#define MADV_REMOVE         9                     /* remove these pages & resources */
#define MADV_DONTFORK       10                    /* don't inherit across fork */
#define MADV_DOFORK         11                    /* do inherit across fork */
#define MADV_MERGEABLE      12                    /* KSM may merge identical pages */
#define MADV_UNMERGEABLE    13                    /* KSM may not merge identical pages */
#define MADV_HUGEPAGE       14                    /* worth backing with hugepages */
#define MADV_NOHUGEPAGE     15                    /* not worth backing with hugepages */
#define MADV_DONTDUMP       16                    /* exclude from core dump */
#define MADV_DODUMP         17                    /* clear MADV_DONTDUMP flag */
#define MADV_WIPEONFORK     18                    /* zero memory on fork (child only) */
#define MADV_KEEPONFORK     19                    /* undo MADV_WIPEONFORK */
#define MADV_COLD           20                    /* deactivate these pages */
#define MADV_PAGEOUT        21                    /* reclaim these pages */
#define MADV_POPULATE_READ  22                    /* prefault page tables readable */
#define MADV_POPULATE_WRITE 23                    /* prefault page tables writable */
#endif

/*
 * Flags for mlockall().
 */
#define MCL_CURRENT 1 /* lock all currently mapped pages */
#define MCL_FUTURE  2 /* lock all pages mapped in the future */
#if __GNU_VISIBLE
#define MCL_ONFAULT   4    /* lock pages when they are faulted in */
#define MLOCK_ONFAULT 0x01 /* lock pages when they are faulted in (mlock2) */
#endif

/*
 * Function declarations.
 */
void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset);
int   munmap(void *addr, size_t len);
int   mprotect(void *addr, size_t len, int prot);
int   msync(void *addr, size_t len, int flags);
int   mlock(const void *addr, size_t len);
int   munlock(const void *addr, size_t len);
int   mlockall(int flags);
int   munlockall(void);
int   posix_madvise(void *addr, size_t len, int advice);
#if __GNU_VISIBLE
int madvise(void *addr, size_t len, int advice);
#endif

_END_STD_C

#endif /* _SYS_MMAN_H_ */

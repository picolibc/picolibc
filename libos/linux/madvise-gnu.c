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
 * madvise: GNU extension.
 *
 * Placed in a separate file with #define _GNU_SOURCE so that the
 * madvise() declaration from <sys/mman.h> is visible before the
 * definition.  Translates picolibc MADV_* values to LINUX_MADV_*
 * kernel values via an explicit switch.
 */

#define _GNU_SOURCE
#include "local-linux.h"
#include <sys/mman.h>

static int
_madv_to_linux(int advice)
{
    switch (advice) {
#ifdef MADV_NORMAL
    case MADV_NORMAL:
        return LINUX_MADV_NORMAL;
#endif
#ifdef MADV_RANDOM
    case MADV_RANDOM:
        return LINUX_MADV_RANDOM;
#endif
#ifdef MADV_SEQUENTIAL
    case MADV_SEQUENTIAL:
        return LINUX_MADV_SEQUENTIAL;
#endif
#ifdef MADV_WILLNEED
    case MADV_WILLNEED:
        return LINUX_MADV_WILLNEED;
#endif
#ifdef MADV_DONTNEED
    case MADV_DONTNEED:
        return LINUX_MADV_DONTNEED;
#endif
#ifdef MADV_FREE
    case MADV_FREE:
        return LINUX_MADV_FREE;
#endif
#ifdef MADV_REMOVE
    case MADV_REMOVE:
        return LINUX_MADV_REMOVE;
#endif
#ifdef MADV_DONTFORK
    case MADV_DONTFORK:
        return LINUX_MADV_DONTFORK;
#endif
#ifdef MADV_DOFORK
    case MADV_DOFORK:
        return LINUX_MADV_DOFORK;
#endif
#ifdef MADV_MERGEABLE
    case MADV_MERGEABLE:
        return LINUX_MADV_MERGEABLE;
#endif
#ifdef MADV_UNMERGEABLE
    case MADV_UNMERGEABLE:
        return LINUX_MADV_UNMERGEABLE;
#endif
#ifdef MADV_HUGEPAGE
    case MADV_HUGEPAGE:
        return LINUX_MADV_HUGEPAGE;
#endif
#ifdef MADV_NOHUGEPAGE
    case MADV_NOHUGEPAGE:
        return LINUX_MADV_NOHUGEPAGE;
#endif
#ifdef MADV_DONTDUMP
    case MADV_DONTDUMP:
        return LINUX_MADV_DONTDUMP;
#endif
#ifdef MADV_DODUMP
    case MADV_DODUMP:
        return LINUX_MADV_DODUMP;
#endif
#ifdef MADV_WIPEONFORK
    case MADV_WIPEONFORK:
        return LINUX_MADV_WIPEONFORK;
#endif
#ifdef MADV_KEEPONFORK
    case MADV_KEEPONFORK:
        return LINUX_MADV_KEEPONFORK;
#endif
#ifdef MADV_COLD
    case MADV_COLD:
        return LINUX_MADV_COLD;
#endif
#ifdef MADV_PAGEOUT
    case MADV_PAGEOUT:
        return LINUX_MADV_PAGEOUT;
#endif
#ifdef MADV_POPULATE_READ
    case MADV_POPULATE_READ:
        return LINUX_MADV_POPULATE_READ;
#endif
#ifdef MADV_POPULATE_WRITE
    case MADV_POPULATE_WRITE:
        return LINUX_MADV_POPULATE_WRITE;
#endif
    default:
        return -1;
    }
}

int
madvise(void *addr, size_t len, int advice)
{
    int linux_advice = _madv_to_linux(advice);
    if (linux_advice < 0) {
        errno = EINVAL;
        return -1;
    }
    return (int)syscall(LINUX_SYS_madvise, addr, len, linux_advice);
}

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

#include "local-linux.h"
#include <sys/mman.h>

void *
mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
    int linux_prot = 0;
    int linux_flags = 0;

    if (prot & PROT_READ)
        linux_prot |= LINUX_PROT_READ;
    if (prot & PROT_WRITE)
        linux_prot |= LINUX_PROT_WRITE;
    if (prot & PROT_EXEC)
        linux_prot |= LINUX_PROT_EXEC;

    if (flags & MAP_SHARED)
        linux_flags |= LINUX_MAP_SHARED;
    if (flags & MAP_PRIVATE)
        linux_flags |= LINUX_MAP_PRIVATE;
    if (flags & MAP_FIXED)
        linux_flags |= LINUX_MAP_FIXED;
    if (flags & MAP_ANONYMOUS)
        linux_flags |= LINUX_MAP_ANONYMOUS;

#ifdef LINUX_SYS_mmap2
    /*
     * 32-bit ARM and i686: the mmap2 syscall takes the file offset in
     * units of pages (4096 bytes) rather than bytes.  POSIX requires
     * that offset be a multiple of the page size; return EINVAL if not.
     */
    if (offset & 0xfff) {
        errno = EINVAL;
        return MAP_FAILED;
    }
    return (void *)syscall(LINUX_SYS_mmap2, addr, len, linux_prot, linux_flags, fd,
                           (long)(offset >> 12));
#else
    return (void *)syscall(LINUX_SYS_mmap, addr, len, linux_prot, linux_flags, fd, (long)offset);
#endif
}

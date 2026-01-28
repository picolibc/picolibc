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
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>

int
main(void)
{
#ifdef O_APPEND
    printf("#define %-32.32s 0x%08x\n", "LINUX_O_APPEND", O_APPEND);
#endif
#ifdef O_ASYNC
    printf("#define %-32.32s 0x%08x\n", "LINUX_O_ASYNC", O_ASYNC);
#endif
#ifdef O_CLOEXEC
    printf("#define %-32.32s 0x%08x\n", "LINUX_O_CLOEXEC", O_CLOEXEC);
#endif
#ifdef O_CREAT
    printf("#define %-32.32s 0x%08x\n", "LINUX_O_CREAT", O_CREAT);
#endif
#ifdef O_DIRECT
    printf("#define %-32.32s 0x%08x\n", "LINUX_O_DIRECT", O_DIRECT);
#endif
#ifdef O_DIRECTORY
    printf("#define %-32.32s 0x%08x\n", "LINUX_O_DIRECTORY", O_DIRECTORY);
#endif
#ifdef O_DSYNC
    printf("#define %-32.32s 0x%08x\n", "LINUX_O_DSYNC", O_DSYNC);
#endif
#ifdef O_EXCL
    printf("#define %-32.32s 0x%08x\n", "LINUX_O_EXCL", O_EXCL);
#endif
#ifdef O_LARGEFILE
    printf("#define %-32.32s 0x%08x\n", "LINUX_O_LARGEFILE", O_LARGEFILE);
#endif
#ifdef O_NOATIME
    printf("#define %-32.32s 0x%08x\n", "LINUX_O_NOATIME", O_NOATIME);
#endif
#ifdef O_NOCTTY
    printf("#define %-32.32s 0x%08x\n", "LINUX_O_NOCTTY", O_NOCTTY);
#endif
#ifdef O_NOFOLLOW
    printf("#define %-32.32s 0x%08x\n", "LINUX_O_NOFOLLOW", O_NOFOLLOW);
#endif
#ifdef O_NONBLOCK
    printf("#define %-32.32s 0x%08x\n", "LINUX_O_NONBLOCK", O_NONBLOCK);
#endif
#ifdef O_PATH
    printf("#define %-32.32s 0x%08x\n", "LINUX_O_PATH", O_PATH);
#endif
#ifdef O_SYNC
    printf("#define %-32.32s 0x%08x\n", "LINUX_O_SYNC", O_SYNC);
#endif
#ifdef O_TMPFILE
    printf("#define %-32.32s 0x%08x\n", "LINUX_O_TMPFILE", O_TMPFILE);
#endif
#ifdef O_TRUNC
    printf("#define %-32.32s 0x%08x\n", "LINUX_O_TRUNC", O_TRUNC);
#endif

#ifdef AT_FDCWD
    printf("#define %-32.32s 0x%08x\n", "LINUX_AT_FDCWD", AT_FDCWD);
#endif
#ifdef AT_EMPTY_PATH
    printf("#define %-32.32s 0x%08x\n", "LINUX_AT_EMPTY_PATH", AT_EMPTY_PATH);
#endif
#ifdef AT_NO_AUTOMOUNT
    printf("#define %-32.32s 0x%08x\n", "LINUX_AT_NO_AUTOMOUNT", AT_NO_AUTOMOUNT);
#endif
#ifdef AT_SYMLINK_NOFOLLOW
    printf("#define %-32.32s 0x%08x\n", "LINUX_AT_SYMLINK_NOFOLLOW", AT_SYMLINK_NOFOLLOW);
#endif
#ifdef AT_STATX_SYNC_AS_STAT
    printf("#define %-32.32s 0x%08x\n", "LINUX_AT_STATX_SYNC_AS_STAT", AT_STATX_SYNC_AS_STAT);
#endif
#ifdef AT_STATX_FORCE_SYNC
    printf("#define %-32.32s 0x%08x\n", "LINUX_AT_STATX_FORCE_SYNC", AT_STATX_FORCE_SYNC);
#endif
#ifdef AT_STATX_DONT_SYNC
    printf("#define %-32.32s 0x%08x\n", "LINUX_AT_STATX_DONT_SYNC", AT_STATX_DONT_SYNC);
#endif
#ifdef STATX_TYPE
    printf("#define %-32.32s 0x%08x\n", "LINUX_STATX_TYPE", STATX_TYPE);
#endif
#ifdef STATX_MODE
    printf("#define %-32.32s 0x%08x\n", "LINUX_STATX_MODE", STATX_MODE);
#endif
#ifdef STATX_NLINK
    printf("#define %-32.32s 0x%08x\n", "LINUX_STATX_NLINK", STATX_NLINK);
#endif
#ifdef STATX_UID
    printf("#define %-32.32s 0x%08x\n", "LINUX_STATX_UID", STATX_UID);
#endif
#ifdef STATX_GID
    printf("#define %-32.32s 0x%08x\n", "LINUX_STATX_GID", STATX_GID);
#endif
#ifdef STATX_ATIME
    printf("#define %-32.32s 0x%08x\n", "LINUX_STATX_ATIME", STATX_ATIME);
#endif
#ifdef STATX_MTIME
    printf("#define %-32.32s 0x%08x\n", "LINUX_STATX_MTIME", STATX_MTIME);
#endif
#ifdef STATX_CTIME
    printf("#define %-32.32s 0x%08x\n", "LINUX_STATX_CTIME", STATX_CTIME);
#endif
#ifdef STATX_INO
    printf("#define %-32.32s 0x%08x\n", "LINUX_STATX_INO", STATX_INO);
#endif
#ifdef STATX_SIZE
    printf("#define %-32.32s 0x%08x\n", "LINUX_STATX_SIZE", STATX_SIZE);
#endif
#ifdef STATX_BLOCKS
    printf("#define %-32.32s 0x%08x\n", "LINUX_STATX_BLOCKS", STATX_BLOCKS);
#endif
#ifdef STATX_BASIC_STATS
    printf("#define %-32.32s 0x%08x\n", "LINUX_STATX_BASIC_STATS", STATX_BASIC_STATS);
#endif
#ifdef STATX_BTIME
    printf("#define %-32.32s 0x%08x\n", "LINUX_STATX_BTIME", STATX_BTIME);
#endif
#ifdef STATX_ALL
    printf("#define %-32.32s 0x%08x\n", "LINUX_STATX_ALL", STATX_ALL);
#endif
#ifdef STATX_MNT_ID
    printf("#define %-32.32s 0x%08x\n", "LINUX_STATX_MNT_ID", STATX_MNT_ID);
#endif
#ifdef STATX_DIOALIGN
    printf("#define %-32.32s 0x%08x\n", "LINUX_STATX_DIOALIGN", STATX_DIOALIGN);
#endif
#ifdef STATX_MNT_ID_UNIQUE
    printf("#define %-32.32s 0x%08x\n", "LINUX_STATX_MNT_ID_UNIQUE", STATX_MNT_ID_UNIQUE);
#endif
#ifdef STATX_SUBVOL
    printf("#define %-32.32s 0x%08x\n", "LINUX_STATX_SUBVOL", STATX_SUBVOL);
#endif
#ifdef STATX_WRITE_ATOMIC
    printf("#define %-32.32s 0x%08x\n", "LINUX_STATX_WRITE_ATOMIC", STATX_WRITE_ATOMIC);
#endif
#ifdef STATX_DIO_READ_ALIGN
    printf("#define %-32.32s 0x%08x\n", "LINUX_STATX_DIO_READ_ALIGN", STATX_DIO_READ_ALIGN);
#endif
    return 0;
}

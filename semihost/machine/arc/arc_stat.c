/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2023 Keith Packard
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

#include "arc_semihost.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

struct arc_stat
{
    uint16_t my_dev;
    uint16_t my___pad1;
    uint32_t my_ino;
    uint16_t my_mode;
    uint16_t my_nlink;
    uint16_t my_uid;
    uint16_t my_gid;
    uint16_t my_rdev;
    uint16_t my___pad2;
    uint32_t my_size;
    uint32_t my_blksize;
    uint32_t my_blocks;
    uint32_t my_atime;
    uint32_t my___unused1;
    uint32_t my_mtime;
    uint32_t my___unused2;
    uint32_t my_ctime;
    uint32_t my___unused3;
    uint32_t my___unused4;
    uint32_t my___unused5;
};

int
stat(const char *pathname, struct stat *restrict statbuf)
{
    struct arc_stat arc_stat;

    int ret = arc_semihost2(SYS_SEMIHOST_stat, (uintptr_t) pathname, (uintptr_t) &arc_stat);
    if (ret < 0) {
        arc_semihost_errno(ENOENT);
    } else {
        statbuf->st_dev = arc_stat.my_dev;
        statbuf->st_ino = arc_stat.my_ino;
        statbuf->st_mode = arc_stat.my_mode;
        statbuf->st_nlink = arc_stat.my_nlink;
        statbuf->st_uid = arc_stat.my_uid;
        statbuf->st_gid = arc_stat.my_gid;
        statbuf->st_rdev = arc_stat.my_rdev;
        statbuf->st_size = arc_stat.my_size;
        statbuf->st_blksize = arc_stat.my_blksize;
        statbuf->st_blocks = arc_stat.my_blocks;
        statbuf->st_atime = arc_stat.my_atime;
        statbuf->st_mtime = arc_stat.my_mtime;
        statbuf->st_ctime = arc_stat.my_ctime;
    }
    return ret;
}

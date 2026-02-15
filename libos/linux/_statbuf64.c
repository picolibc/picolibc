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

#include "local-statx.h"

int
_statbuf64(struct stat64 *statbuf, const struct __kernel_statx *statxbuf)
{
    statbuf->st_dev = (statxbuf->stx_dev_major << 8) | (statxbuf->stx_dev_minor);
    statbuf->st_ino = (ino_t)statxbuf->stx_ino;
    statbuf->st_mode = statxbuf->stx_mode;
    statbuf->st_nlink = statxbuf->stx_nlink;
    statbuf->st_uid = statxbuf->stx_uid;
    statbuf->st_gid = statxbuf->stx_gid;
    statbuf->st_rdev = (statxbuf->stx_rdev_major << 8) | (statxbuf->stx_rdev_minor);
    statbuf->st_size = statxbuf->stx_size;
#ifdef _STAT_HAS_ST_BLKSIZE
    statbuf->st_blksize = statxbuf->stx_blksize;
    statbuf->st_blocks = statxbuf->stx_blocks;
#endif
    statbuf->st_atim.tv_sec = statxbuf->stx_atime.tv_sec;
    statbuf->st_atim.tv_nsec = statxbuf->stx_atime.tv_nsec;
    statbuf->st_mtim.tv_sec = statxbuf->stx_mtime.tv_sec;
    statbuf->st_mtim.tv_nsec = statxbuf->stx_mtime.tv_nsec;
    statbuf->st_ctim.tv_sec = statxbuf->stx_ctime.tv_sec;
    statbuf->st_ctim.tv_nsec = statxbuf->stx_ctime.tv_nsec;
    return 0;
}

/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Linumiz
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

#define _DEFAULT_SOURCE
#include <sys/stat.h>
#include <string.h>
#include "v850_semihost.h"

/* stat as written by the simulator */
struct v850_sim_stat {
    uint16_t st_dev;
    uint16_t st_ino;
    uint32_t st_mode;
    uint16_t st_nlink;
    uint16_t st_uid;
    uint16_t st_gid;
    uint16_t st_rdev;
    uint32_t st_size;
    uint32_t atime;
    uint32_t pad0;
    uint32_t mtime;
    uint32_t pad1;
    uint32_t ctime;
};

int
fstat(int fd, struct stat *sbuf)
{
    struct v850_sim_stat s;
    int                  ret;

    ret = v850_syscall(V850_SYS_fstat, (uint32_t)fd, (uint32_t)(uintptr_t)&s, 0);
    if (ret < 0)
        return ret;
    memset(sbuf, 0, sizeof(*sbuf));
    sbuf->st_dev = s.st_dev;
    sbuf->st_ino = s.st_ino;
    sbuf->st_mode = s.st_mode;
    sbuf->st_nlink = s.st_nlink;
    sbuf->st_uid = s.st_uid;
    sbuf->st_gid = s.st_gid;
    sbuf->st_rdev = s.st_rdev;
    sbuf->st_size = s.st_size;
    sbuf->st_atim.tv_sec = s.atime;
    sbuf->st_mtim.tv_sec = s.mtime;
    sbuf->st_ctim.tv_sec = s.ctime;
    return ret;
}

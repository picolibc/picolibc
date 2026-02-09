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

#include "local-dirent.h"

#include <stddef.h>
#include <string.h>
#include <stdint.h>

struct dirent *
readdir(DIR *dir)
{
    struct __kernel_dirent64 *k_dirent;
    struct dirent            *dirent;

    if (dir->offset == dir->count) {
        ssize_t ret = syscall(LINUX_SYS_getdents64, dir->fd, dir->buf, sizeof(dir->buf));
        if (ret <= 0)
            return NULL;
        dir->offset = 0;
        dir->count = (size_t)ret;
    }
    k_dirent = (struct __kernel_dirent64 *)(dir->buf + dir->offset);
    dir->offset += k_dirent->d_reclen;

    dirent = &dir->dirent;
    dirent->d_ino = (ino_t)k_dirent->d_ino;
    dirent->d_off = (__off_t)k_dirent->d_off;

    size_t name_len = strlen((char *)k_dirent->d_name);
    if (name_len >= sizeof(dirent->d_name))
        name_len = sizeof(dirent->d_name) - 1;
    memcpy(dirent->d_name, k_dirent->d_name, name_len);
    dirent->d_name[name_len] = '\0';
    return dirent;
}

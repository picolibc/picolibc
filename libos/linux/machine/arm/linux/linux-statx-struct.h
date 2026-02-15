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

#ifndef _LINUX_STATX_STRUCT_H_
#define _LINUX_STATX_STRUCT_H_

struct __kernel_statx {
    __uint8_t                       __adjust_0[4];
    __uint32_t                      stx_blksize;
    __uint8_t                       __adjust_8[8];
    __uint32_t                      stx_nlink;
    __uint32_t                      stx_uid;
    __uint32_t                      stx_gid;
    __uint16_t                      stx_mode;
    __uint8_t                       __adjust_30[2];
    __uint64_t                      stx_ino;
    __uint64_t                      stx_size;
    __uint64_t                      stx_blocks;
    __uint8_t                       __adjust_56[8];
    struct __kernel_statx_timestamp stx_atime;
    __uint8_t                       __adjust_80[16];
    struct __kernel_statx_timestamp stx_ctime;
    struct __kernel_statx_timestamp stx_mtime;
    __uint32_t                      stx_rdev_major;
    __uint32_t                      stx_rdev_minor;
    __uint32_t                      stx_dev_major;
    __uint32_t                      stx_dev_minor;
    __uint8_t                       __adjust_144[112];
};

#define SIMPLE_MAP_STATX(_t, _f)                     \
    do {                                             \
        (_t)->stx_dev_major = (_f)->stx_dev_major;   \
        (_t)->stx_dev_minor = (_f)->stx_dev_minor;   \
        (_t)->stx_ino = (_f)->stx_ino;               \
        (_t)->stx_mode = (_f)->stx_mode;             \
        (_t)->stx_nlink = (_f)->stx_nlink;           \
        (_t)->stx_uid = (_f)->stx_uid;               \
        (_t)->stx_gid = (_f)->stx_gid;               \
        (_t)->stx_rdev_major = (_f)->stx_rdev_major; \
        (_t)->stx_rdev_minor = (_f)->stx_rdev_minor; \
        (_t)->stx_size = (_f)->stx_size;             \
        (_t)->stx_blksize = (_f)->stx_blksize;       \
        (_t)->stx_blocks = (_f)->stx_blocks;         \
    } while (0)

#endif /* _LINUX_STATX_STRUCT_H_ */

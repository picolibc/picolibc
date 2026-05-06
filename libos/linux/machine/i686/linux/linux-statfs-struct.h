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

#ifndef _LINUX_STATFS_STRUCT_H_
#define _LINUX_STATFS_STRUCT_H_

struct __kernel_statfs {
    __int32_t         f_type;
    __int32_t         f_bsize;
    __uint32_t        f_blocks;
    __uint32_t        f_bfree;
    __uint32_t        f_bavail;
    __uint32_t        f_files;
    __uint32_t        f_ffree;
    __kernel___fsid_t f_fsid;
    __int32_t         f_namelen;
    __int32_t         f_frsize;
    __int32_t         f_flags;
    __uint8_t         __adjust_48[16];
};

#define SIMPLE_MAP_STATFS(_t, _f)          \
    do {                                   \
        (_t)->f_type = (_f)->f_type;       \
        (_t)->f_bsize = (_f)->f_bsize;     \
        (_t)->f_blocks = (_f)->f_blocks;   \
        (_t)->f_bfree = (_f)->f_bfree;     \
        (_t)->f_bavail = (_f)->f_bavail;   \
        (_t)->f_files = (_f)->f_files;     \
        (_t)->f_ffree = (_f)->f_ffree;     \
        (_t)->f_namelen = (_f)->f_namelen; \
        (_t)->f_frsize = (_f)->f_frsize;   \
        (_t)->f_flags = (_f)->f_flags;     \
    } while (0)

#endif /* _LINUX_STATFS_STRUCT_H_ */

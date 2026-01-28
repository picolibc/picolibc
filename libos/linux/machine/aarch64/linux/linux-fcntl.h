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
#ifndef _LINUX_FCNTL_H_
#define _LINUX_FCNTL_H_
#define LINUX_AT_EMPTY_PATH         0x00001000
#define LINUX_AT_FDCWD              0xffffff9c
#define LINUX_AT_NO_AUTOMOUNT       0x00000800
#define LINUX_AT_STATX_DONT_SYNC    0x00004000
#define LINUX_AT_STATX_FORCE_SYNC   0x00002000
#define LINUX_AT_STATX_SYNC_AS_STAT 0x00000000
#define LINUX_AT_SYMLINK_NOFOLLOW   0x00000100
#define LINUX_O_APPEND              0x00000400
#define LINUX_O_ASYNC               0x00002000
#define LINUX_O_CLOEXEC             0x00080000
#define LINUX_O_CREAT               0x00000040
#define LINUX_O_DIRECT              0x00010000
#define LINUX_O_DIRECTORY           0x00004000
#define LINUX_O_DSYNC               0x00001000
#define LINUX_O_EXCL                0x00000080
#define LINUX_O_LARGEFILE           0x00000000
#define LINUX_O_NOATIME             0x00040000
#define LINUX_O_NOCTTY              0x00000100
#define LINUX_O_NOFOLLOW            0x00008000
#define LINUX_O_NONBLOCK            0x00000800
#define LINUX_O_PATH                0x00200000
#define LINUX_O_SYNC                0x00101000
#define LINUX_O_TMPFILE             0x00404000
#define LINUX_O_TRUNC               0x00000200
#define LINUX_STATX_ALL             0x00000fff
#define LINUX_STATX_ATIME           0x00000020
#define LINUX_STATX_BASIC_STATS     0x000007ff
#define LINUX_STATX_BLOCKS          0x00000400
#define LINUX_STATX_BTIME           0x00000800
#define LINUX_STATX_CTIME           0x00000080
#define LINUX_STATX_DIOALIGN        0x00002000
#define LINUX_STATX_DIO_READ_ALIGN  0x00020000
#define LINUX_STATX_GID             0x00000010
#define LINUX_STATX_INO             0x00000100
#define LINUX_STATX_MNT_ID          0x00001000
#define LINUX_STATX_MNT_ID_UNIQUE   0x00004000
#define LINUX_STATX_MODE            0x00000002
#define LINUX_STATX_MTIME           0x00000040
#define LINUX_STATX_NLINK           0x00000004
#define LINUX_STATX_SIZE            0x00000200
#define LINUX_STATX_SUBVOL          0x00008000
#define LINUX_STATX_TYPE            0x00000001
#define LINUX_STATX_UID             0x00000008
#define LINUX_STATX_WRITE_ATOMIC    0x00010000
#endif /* _LINUX_FCNTL_H_ */

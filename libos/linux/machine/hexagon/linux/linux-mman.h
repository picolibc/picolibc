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
#ifndef _LINUX_MMAN_H_
#define _LINUX_MMAN_H_
#define LINUX_MADV_COLD             0x14
#define LINUX_MADV_DODUMP           0x11
#define LINUX_MADV_DOFORK           0xb
#define LINUX_MADV_DONTDUMP         0x10
#define LINUX_MADV_DONTFORK         0xa
#define LINUX_MADV_DONTNEED         0x4
#define LINUX_MADV_FREE             0x8
#define LINUX_MADV_HUGEPAGE         0xe
#define LINUX_MADV_KEEPONFORK       0x13
#define LINUX_MADV_MERGEABLE        0xc
#define LINUX_MADV_NOHUGEPAGE       0xf
#define LINUX_MADV_NORMAL           0x0
#define LINUX_MADV_PAGEOUT          0x15
#define LINUX_MADV_POPULATE_READ    0x16
#define LINUX_MADV_POPULATE_WRITE   0x17
#define LINUX_MADV_RANDOM           0x1
#define LINUX_MADV_REMOVE           0x9
#define LINUX_MADV_SEQUENTIAL       0x2
#define LINUX_MADV_UNMERGEABLE      0xd
#define LINUX_MADV_WILLNEED         0x3
#define LINUX_MADV_WIPEONFORK       0x12
#define LINUX_MAP_ANONYMOUS         0x20
#define LINUX_MAP_FIXED             0x10
#define LINUX_MAP_PRIVATE           0x2
#define LINUX_MAP_SHARED            0x1
#define LINUX_MCL_CURRENT           0x1
#define LINUX_MCL_FUTURE            0x2
#define LINUX_MCL_ONFAULT           0x4
#define LINUX_MS_ASYNC              0x1
#define LINUX_MS_INVALIDATE         0x2
#define LINUX_MS_SYNC               0x4
#define LINUX_POSIX_MADV_DONTNEED   0x4
#define LINUX_POSIX_MADV_NORMAL     0x0
#define LINUX_POSIX_MADV_RANDOM     0x1
#define LINUX_POSIX_MADV_SEQUENTIAL 0x2
#define LINUX_POSIX_MADV_WILLNEED   0x3
#define LINUX_PROT_EXEC             0x4
#define LINUX_PROT_NONE             0x0
#define LINUX_PROT_READ             0x1
#define LINUX_PROT_WRITE            0x2
#endif /* _LINUX_MMAN_H_ */

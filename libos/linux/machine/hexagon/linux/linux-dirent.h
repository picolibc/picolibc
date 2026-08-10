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
#ifndef _LINUX_DIRENT_H_
#define _LINUX_DIRENT_H_
#define LINUX_DT_BLK     6
#define LINUX_DT_CHR     2
#define LINUX_DT_DIR     4
#define LINUX_DT_FIFO    1
#define LINUX_DT_LNK     10
#define LINUX_DT_REG     8
#define LINUX_DT_SOCK    12
#define LINUX_DT_UNKNOWN 0
#define MAP_DT(p_dt, l_dt) \
    switch (l_dt) {        \
    case LINUX_DT_BLK:     \
        p_dt = DT_BLK;     \
        break;             \
    case LINUX_DT_CHR:     \
        p_dt = DT_CHR;     \
        break;             \
    case LINUX_DT_DIR:     \
        p_dt = DT_DIR;     \
        break;             \
    case LINUX_DT_FIFO:    \
        p_dt = DT_FIFO;    \
        break;             \
    case LINUX_DT_LNK:     \
        p_dt = DT_LNK;     \
        break;             \
    case LINUX_DT_REG:     \
        p_dt = DT_REG;     \
        break;             \
    case LINUX_DT_SOCK:    \
        p_dt = DT_SOCK;    \
        break;             \
    case LINUX_DT_UNKNOWN: \
        p_dt = DT_UNKNOWN; \
        break;             \
    }
#endif /* _LINUX_DIRENT_H_ */

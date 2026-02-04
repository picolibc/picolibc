/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2024 Keith Packard
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
#include <sys/cdefs.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>

#ifndef __weak_reference
#define __fallback_fpathconf fpathconf
#endif

long
__fallback_fpathconf(int fd, int name)
{
    (void)fd;
    switch (name) {
    case _PC_FILESIZEBITS:
        return sizeof(off_t) * 8;
    case _PC_LINK_MAX:
        return _POSIX_LINK_MAX;
    case _PC_MAX_CANON:
        return _POSIX_MAX_CANON;
    case _PC_MAX_INPUT:
        return _POSIX_MAX_INPUT;
    case _PC_NAME_MAX:
        return _POSIX_NAME_MAX;
    case _PC_PATH_MAX:
        return _POSIX_PATH_MAX;
    case _PC_PIPE_BUF:
        return _POSIX_PIPE_BUF;
    case _PC_2_SYMLINKS:
        return 0;
    case _PC_ALLOC_SIZE_MIN:
        return 0;
    case _PC_REC_INCR_XFER_SIZE:
        return 1;
    case _PC_REC_MAX_XFER_SIZE:
        return LONG_MAX;
    case _PC_REC_MIN_XFER_SIZE:
        return 0;
    case _PC_REC_XFER_ALIGN:
        return 1;
    case _PC_SYMLINK_MAX:
        return _POSIX_SYMLINK_MAX;
    case _PC_TEXTDOMAIN_MAX:
        return _POSIX_NAME_MAX - 3;
    case _PC_CHOWN_RESTRICTED:
        return 0;
    case _PC_NO_TRUNC:
        return 0;
    case _PC_VDISABLE:
        return 0;
    case _PC_ASYNC_IO:
        return 0;
    case _PC_FALLOC:
        return 0;
    case _PC_PRIO_IO:
        return 0;
    case _PC_SYNC_IO:
        return 0;
    case _PC_TIMESTAMP_RESOLUTION:
        return 1000000000L;
    default:
        errno = EINVAL;
        return -1;
    }
}

#ifdef __weak_reference
__weak_reference(__fallback_fpathconf, fpathconf);
#endif

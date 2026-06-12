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

static int
convert_open_flags(int flags)
{
    int nsim_flags = 0;

    nsim_flags |= (flags & O_RDONLY) ? ARC_LINUX_RDONLY : 0;
    nsim_flags |= (flags & O_WRONLY) ? ARC_LINUX_WRONLY : 0;
    nsim_flags |= (flags & O_RDWR) ? ARC_LINUX_RDWR : 0;
    nsim_flags |= (flags & O_CREAT) ? ARC_LINUX_CREAT : 0;
    nsim_flags |= (flags & O_APPEND) ? ARC_LINUX_APPEND : 0;
    nsim_flags |= (flags & O_TRUNC) ? ARC_LINUX_TRUNC : 0;
    nsim_flags |= (flags & O_EXCL) ? ARC_LINUX_EXCL : 0;

    return nsim_flags;
}

int
open(const char *pathname, int flags, ...)
{
    va_list ap;
    int     ret;
    int     nsim_flags = convert_open_flags(flags);

    va_start(ap, flags);
    uintptr_t mode = va_arg(ap, uintptr_t);
    va_end(ap);
    ret = arc_semihost3(SYS_SEMIHOST_open, (uintptr_t)pathname, nsim_flags, mode);

    if (ret < 0)
        arc_semihost_errno(EINVAL);
    return ret;
}

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

#include "local-linux.h"
#include <fcntl.h>
#include <stdarg.h>

int
fcntl(int fd, int op, ...)
{
    va_list ap;
    int     arg = 0;

    switch (op) {
    case F_DUPFD:
        op = LINUX_F_DUPFD;
        break;
    case F_DUPFD_CLOEXEC:
        op = LINUX_F_DUPFD_CLOEXEC;
        break;
    case F_GETFD:
        op = LINUX_F_GETFD;
        break;
    case F_SETFD:
        op = LINUX_F_SETFD;
        arg = 1;
        break;
    case F_GETFL:
        op = LINUX_F_GETFL;
        break;
    case F_SETFL:
        op = LINUX_F_SETFL;
        arg = 1;
        break;
    default:
        errno = EINVAL;
        return -1;
    }
    va_start(ap, op);
    if (arg)
        arg = va_arg(ap, int);
    va_end(ap);
    return syscall(LINUX_SYS_fcntl, fd, op, arg);
}

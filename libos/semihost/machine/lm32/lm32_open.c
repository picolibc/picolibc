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

#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include "lm32_semihost.h"

#define SIM_O_RDONLY 0
#define SIM_O_WRONLY 1
#define SIM_O_RDWR   2
#define SIM_O_APPEND 0x008
#define SIM_O_CREAT  0x200
#define SIM_O_TRUNC  0x400
#define SIM_O_EXCL   0x800

static int
sim_flags(int flags)
{
    int sim = 0;

    sim = flags & 3;
    if (flags & O_RDWR)
        sim |= SIM_O_RDWR;
    if (flags & O_APPEND)
        sim |= SIM_O_APPEND;
    if (flags & O_CREAT)
        sim |= SIM_O_CREAT;
    if (flags & O_TRUNC)
        sim |= SIM_O_TRUNC;
    if (flags & O_EXCL)
        sim |= SIM_O_EXCL;
    return sim;
}

int
open(const char *pathname, int flags, ...)
{
    va_list ap;
    va_start(ap, flags);
    uintptr_t mode = va_arg(ap, uintptr_t);
    va_end(ap);

    struct lm32_scall_args args = {
        .r8 = TARGET_NEWLIB_SYS_open,
        .r1 = (uint32_t)(uintptr_t)pathname,
        .r2 = (uint32_t)sim_flags(flags),
        .r3 = (uint32_t)mode,
    };
    struct lm32_scall_ret ret;

    lm32_scall(&args, &ret);
    return (int)ret.r1;
}

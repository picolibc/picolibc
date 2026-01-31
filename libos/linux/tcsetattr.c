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

#include "local-termios.h"

#define MAP_BIT(s, d, sbit, dbit) do {          \
        if ((s) & (sbit))                       \
            (d) |= (dbit);                      \
        else                                    \
            (d) &= ~(dbit);                     \
    } while(0)


#define MAP_4(s, d, sbits, dbits, s0, s1, s2, s3, d0, d1, d2, d3) do {  \
        (d) &= ~(dbits);                                                \
        switch ((s) & (sbits)) {                                        \
        case s0: (d) |= d0; break;                                      \
        case s1: (d) |= d1; break;                                      \
        case s2: (d) |= d2; break;                                      \
        case s3: (d) |= d3; break;                                      \
        }                                                               \
    } while(0)

#define MAP_CC(i) (ktermios.c_cc[LINUX_ ## i] = termios->c_cc[i])

int
tcsetattr(int fd, int actions, const struct termios *termios)
{
    struct __kernel_termios ktermios;
    int ret;
    long cmd;

    switch (actions) {
    case TCSANOW:
        cmd = LINUX_TCSETS2;
        break;
    case TCSADRAIN:
        cmd = LINUX_TCSETSW2;
        break;
    case TCSAFLUSH:
        cmd = LINUX_TCSETSF2;
        break;
    default:
        errno = EINVAL;
        return -1;
    }

    ret = syscall(LINUX_SYS_ioctl, fd, LINUX_TCGETS2, &ktermios);
    if (ret < 0)
        return ret;

    MAP_STRUCT;

    return syscall(LINUX_SYS_ioctl, fd, cmd, &ktermios);
}

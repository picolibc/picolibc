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

#ifndef _LOCAL_TERMIOS_H_
#define _LOCAL_TERMIOS_H_

#include "local-linux.h"
#include <linux/linux-statx_timestamp-struct.h>
#include <linux/linux-termios-struct.h>
#include <linux/linux-winsize-struct.h>
#include <linux/linux-termios.h>

#include <termios.h>

#define _CAT(a, b)                   a##b
#define CAT(a, b)                    _CAT(a, b)
#define MAP_FIELD(field, sbit, dbit) MAP_BIT(termios->field, ktermios.field, sbit, dbit)

#define MAP_FIELD(field, sbit, dbit) MAP_BIT(termios->field, ktermios.field, sbit, dbit)

#define MAP_FIELD4(field, sbit, dbit, s0, s1, s2, s3, d0, d1, d2, d3)                 \
    MAP_4(termios->field, ktermios.field, sbit, dbit, s0, s1, s2, s3, d0, d1, d2, d3)

#define MAP_I(bit) MAP_FIELD(c_iflag, bit, LINUX_##bit)

#define MAP_O(bit) MAP_FIELD(c_oflag, bit, LINUX_##bit)

#define MAP_O4(bit, b0, b1, b2, b3)                                                           \
    MAP_FIELD4(c_oflag, bit, LINUX_##bit, b0, b1, b2, b3, LINUX_##b0, LINUX_##b1, LINUX_##b2, \
               LINUX_##b3)

#define MAP_C(bit) MAP_FIELD(c_cflag, bit, LINUX_##bit)

#define MAP_C4(bit, b0, b1, b2, b3)                                                           \
    MAP_FIELD4(c_cflag, bit, LINUX_##bit, b0, b1, b2, b3, LINUX_##b0, LINUX_##b1, LINUX_##b2, \
               LINUX_##b3)

#define MAP_L(bit) MAP_FIELD(c_lflag, bit, LINUX_##bit)

#define MAP_STRUCT                              \
    do {                                        \
        MAP_I(BRKINT);                          \
        MAP_I(ICRNL);                           \
        MAP_I(IGNBRK);                          \
        MAP_I(IGNCR);                           \
        MAP_I(IGNPAR);                          \
        MAP_I(INLCR);                           \
        MAP_I(INPCK);                           \
        MAP_I(ISTRIP);                          \
        MAP_I(IXANY);                           \
        MAP_I(IXOFF);                           \
        MAP_I(IXON);                            \
        MAP_I(PARMRK);                          \
                                                \
        MAP_O(OPOST);                           \
        MAP_O(ONLCR);                           \
        MAP_O(OCRNL);                           \
        MAP_O(ONOCR);                           \
        MAP_O(ONLRET);                          \
        MAP_O(OFDEL);                           \
        MAP_O(OFILL);                           \
        MAP_O(NLDLY);                           \
        MAP_O4(CRDLY, CR0, CR1, CR2, CR3);      \
        MAP_O4(TABDLY, TAB0, TAB1, TAB2, TAB3); \
        MAP_O(BSDLY);                           \
        MAP_O(VTDLY);                           \
        MAP_O(FFDLY);                           \
                                                \
        MAP_C4(CSIZE, CS5, CS6, CS7, CS8);      \
        MAP_C(CSTOPB);                          \
        MAP_C(CREAD);                           \
        MAP_C(PARENB);                          \
        MAP_C(PARODD);                          \
        MAP_C(HUPCL);                           \
        MAP_C(CLOCAL);                          \
                                                \
        MAP_L(ECHO);                            \
        MAP_L(ECHOE);                           \
        MAP_L(ECHOK);                           \
        MAP_L(ECHONL);                          \
        MAP_L(ICANON);                          \
        MAP_L(IEXTEN);                          \
        MAP_L(ISIG);                            \
        MAP_L(NOFLSH);                          \
        MAP_L(TOSTOP);                          \
                                                \
        MAP_CC(VEOF);                           \
        MAP_CC(VEOL);                           \
        MAP_CC(VERASE);                         \
        MAP_CC(VINTR);                          \
        MAP_CC(VKILL);                          \
        MAP_CC(VMIN);                           \
        MAP_CC(VQUIT);                          \
        MAP_CC(VSTART);                         \
        MAP_CC(VSTOP);                          \
        MAP_CC(VSUSP);                          \
        MAP_CC(VTIME);                          \
                                                \
        MAP_SPEED(c_ispeed);                    \
        MAP_SPEED(c_ospeed);                    \
    } while (0)

#define MAP_WINSIZE(a, b)          \
    do {                           \
        (a)->ws_row = (b)->ws_row; \
        (a)->ws_col = (b)->ws_col; \
    } while (0)

/*
 * Linux and picolibc both use the actual baud rate
 * as the speed value
 */
static inline __kernel_speed_t
_speed_to_kernel(speed_t speed)
{
    return (__kernel_speed_t)speed;
}

static inline speed_t
_speed_from_kernel(__kernel_speed_t speed)
{
    return (speed_t)speed;
}

#endif /* _LOCAL_TERMIOS_H_ */

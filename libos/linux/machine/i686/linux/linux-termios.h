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
#ifndef _LINUX_TERMIOS_H_
#define _LINUX_TERMIOS_H_
#define LINUX_VEOF      4
#define LINUX_VMIN      6
#define LINUX_VEOL      11
#define LINUX_VTIME     5
#define LINUX_VERASE    2
#define LINUX_VINTR     0
#define LINUX_VKILL     3
#define LINUX_VQUIT     1
#define LINUX_VSTART    8
#define LINUX_VSTOP     9
#define LINUX_VSUSP     10

#define LINUX_BRKINT    2
#define LINUX_ICRNL     256
#define LINUX_IGNBRK    1
#define LINUX_IGNCR     128
#define LINUX_IGNPAR    4
#define LINUX_INLCR     64
#define LINUX_INPCK     16
#define LINUX_ISTRIP    32
#define LINUX_IXANY     2048
#define LINUX_IXOFF     4096
#define LINUX_IXON      1024
#define LINUX_PARMRK    8

#define LINUX_OPOST     1
#define LINUX_ONLCR     4
#define LINUX_OCRNL     8
#define LINUX_ONOCR     16
#define LINUX_ONLRET    32
#define LINUX_OFDEL     128
#define LINUX_OFILL     64
#define LINUX_NLDLY     256
#define LINUX_NL0       0
#define LINUX_NL1       256
#define LINUX_CRDLY     1536
#define LINUX_CR0       0
#define LINUX_CR1       512
#define LINUX_CR2       1024
#define LINUX_CR3       1536
#define LINUX_TABDLY    6144
#define LINUX_TAB0      0
#define LINUX_TAB1      2048
#define LINUX_TAB2      4096
#define LINUX_TAB3      6144
#define LINUX_BSDLY     8192
#define LINUX_BS0       0
#define LINUX_BS1       8192
#define LINUX_VTDLY     16384
#define LINUX_VT0       0
#define LINUX_VT1       16384
#define LINUX_FFDLY     32768
#define LINUX_FF0       0
#define LINUX_FF1       32768

#define LINUX_B0        0
#define LINUX_B50       1
#define LINUX_B75       2
#define LINUX_B110      3
#define LINUX_B134      4
#define LINUX_B150      5
#define LINUX_B200      6
#define LINUX_B300      7
#define LINUX_B600      8
#define LINUX_B1200     9
#define LINUX_B1800     10
#define LINUX_B2400     11
#define LINUX_B4800     12
#define LINUX_B9600     13
#define LINUX_B19200    14
#define LINUX_B38400    15

#define LINUX_CSIZE     48
#define LINUX_CS5       0
#define LINUX_CS6       16
#define LINUX_CS7       32
#define LINUX_CS8       48
#define LINUX_CSTOPB    64
#define LINUX_CREAD     128
#define LINUX_PARENB    256
#define LINUX_PARODD    512
#define LINUX_HUPCL     1024
#define LINUX_CLOCAL    2048

#define LINUX_ECHO      8
#define LINUX_ECHOE     16
#define LINUX_ECHOK     32
#define LINUX_ECHONL    64
#define LINUX_ICANON    2
#define LINUX_IEXTEN    32768
#define LINUX_ISIG      1
#define LINUX_NOFLSH    128
#define LINUX_TOSTOP    256

#define LINUX_TCSANOW   0
#define LINUX_TCSADRAIN 1
#define LINUX_TCSAFLUSH 2

#define LINUX_TCIFLUSH  0
#define LINUX_TCOFLUSH  1
#define LINUX_TCIOFLUSH 2

#define LINUX_TCIOFF    2
#define LINUX_TCION     3
#define LINUX_TCOOFF    0
#define LINUX_TCOON     1

#define LINUX_TCGETS    0x5401
#define LINUX_TCGETS2   0x802c542a
#define LINUX_TCSETS    0x5402
#define LINUX_TCSETSW   0x5403
#define LINUX_TCSETSF   0x5404
#define LINUX_TCSETS2   0x402c542b
#define LINUX_TCSETSW2  0x402c542c
#define LINUX_TCSETSF2  0x402c542d

#define LINUX_TCFLSH    0x0000540b

#include <linux/linux-termios-struct.h>
#endif /* _LINUX_TERMIOS_H_ */

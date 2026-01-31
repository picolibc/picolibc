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

#ifndef _SYS_TERMIOS_H_
#define _SYS_TERMIOS_H_

#include <sys/_types.h>

typedef unsigned char cc_t;
typedef unsigned int  speed_t;
typedef unsigned int  tcflag_t;

#define VEOF   0
#define VEOL   1
#define VERASE 2
#define VINTR  3
#define VKILL  4
#define VMIN   5
#define VQUIT  6
#define VSTART 7
#define VSTOP  8
#define VSUSP  9
#define VTIME  10
#define NCCS   11

struct termios {
    tcflag_t c_iflag;
    tcflag_t c_oflag;
    tcflag_t c_cflag;
    tcflag_t c_lflag;
    cc_t     c_cc[NCCS];
    speed_t  c_ispeed;
    speed_t  c_ospeed;
};

#define BRKINT    0x0001
#define ICRNL     0x0002
#define IGNBRK    0x0004
#define IGNCR     0x0008
#define IGNPAR    0x0010
#define INLCR     0x0020
#define INPCK     0x0040
#define ISTRIP    0x0080
#define IXANY     0x0100
#define IXOFF     0x0200
#define IXON      0x0400
#define PARMRK    0x0800

#define OPOST     0x0001
#define ONLCR     0x0002
#define OCRNL     0x0004
#define ONOCR     0x0008
#define ONLRET    0x0010
#define OFDEL     0x0020
#define OFILL     0x0040
#define NLDLY     0x0080
#define NL0       0x0000
#define NL1       0x0080
#define CRDLY     0x0300
#define CR0       0x0000
#define CR1       0x0100
#define CR2       0x0200
#define CR3       0x0300
#define TABDLY    0x0c00
#define TAB0      0x0000
#define TAB1      0x0400
#define TAB2      0x0800
#define TAB3      0x0c00
#define BSDLY     0x1000
#define BS0       0x0000
#define BS1       0x1000
#define VTDLY     0x2000
#define VT0       0x0000
#define VT1       0x2000
#define FFDLY     0x4000
#define FF0       0x0000
#define FF1       0x4000

#define B0        0U
#define B50       50U
#define B75       75U
#define B110      110U
#define B134      134U
#define B150      150U
#define B200      200U
#define B300      300U
#define B600      600U
#define B1200     1200U
#define B1800     1800U
#define B2400     2400U
#define B4800     4800U
#define B9600     9600U
#define B19200    19200U
#define B38400    38400U
#define B7200     7200U
#define B14400    14400U
#define B28800    28800U
#define B33600    33600U
#define B57600    57600U
#define B76800    76800U
#define B115200   115200U
#define B153600   153600U
#define B230400   230400U
#define B307200   307200U
#define B460800   460800U
#define B500000   500000U
#define B576000   576000U
#define B614400   614400U
#define B921600   921600U
#define B1000000  1000000U
#define B1152000  1152000U
#define B1500000  1500000U
#define B2000000  2000000U
#define B2500000  2500000U
#define B3000000  3000000U
#define B3500000  3500000U
#define B4000000  4000000U
#define B5000000  5000000U
#define B10000000 10000000U

#define CSIZE     0x000f
#define CS5       0x0005
#define CS6       0x0006
#define CS7       0x0007
#define CS8       0x0008
#define CSTOPB    0x0010
#define CREAD     0x0020
#define PARENB    0x0040
#define PARODD    0x0080
#define HUPCL     0x0100
#define CLOCAL    0x0200

#define ECHO      0x0001
#define ECHOE     0x0002
#define ECHOK     0x0004
#define ECHONL    0x0008
#define ICANON    0x0010
#define IEXTEN    0x0020
#define ISIG      0x0040
#define NOFLSH    0x0080
#define TOSTOP    0x0100

#define TCSANOW   0
#define TCSADRAIN 1
#define TCSAFLUSH 2

#define TCIFLUSH  1
#define TCOFLUSH  2
#define TCIOFLUSH 3

#define TCIOFF    0
#define TCION     1
#define TCOOFF    2
#define TCOON     3

speed_t cfgetispeed(const struct termios *);
speed_t cfgetospeed(const struct termios *);
int     cfsetispeed(struct termios *, speed_t);
int     cfsetospeed(struct termios *, speed_t);
int     tcdrain(int);
int     tcflow(int, int);
int     tcflush(int, int);
int     tcgetattr(int, struct termios *);
__pid_t tcgetsid(int);
int     tcsendbreak(int, int);
int     tcsetattr(int, int, const struct termios *);

#endif

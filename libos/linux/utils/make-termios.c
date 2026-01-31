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

#include <asm/termbits.h>
#include <sys/ioctl.h>
#include <stdio.h>

int
main(void)
{
    printf("#define LINUX_VEOF %d\n", VEOF);
    printf("#define LINUX_VMIN %d\n", VMIN);
    printf("#define LINUX_VEOL %d\n", VEOL);
    printf("#define LINUX_VTIME %d\n", VTIME);
    printf("#define LINUX_VERASE %d\n", VERASE);
    printf("#define LINUX_VINTR %d\n", VINTR);
    printf("#define LINUX_VKILL %d\n", VKILL);
    printf("#define LINUX_VQUIT %d\n", VQUIT);
    printf("#define LINUX_VSTART %d\n", VSTART);
    printf("#define LINUX_VSTOP %d\n", VSTOP);
    printf("#define LINUX_VSUSP %d\n", VSUSP);
    printf("\n");
    printf("#define LINUX_BRKINT %d\n", BRKINT);
    printf("#define LINUX_ICRNL %d\n", ICRNL);
    printf("#define LINUX_IGNBRK %d\n", IGNBRK);
    printf("#define LINUX_IGNCR %d\n", IGNCR);
    printf("#define LINUX_IGNPAR %d\n", IGNPAR);
    printf("#define LINUX_INLCR %d\n", INLCR);
    printf("#define LINUX_INPCK %d\n", INPCK);
    printf("#define LINUX_ISTRIP %d\n", ISTRIP);
    printf("#define LINUX_IXANY %d\n", IXANY);
    printf("#define LINUX_IXOFF %d\n", IXOFF);
    printf("#define LINUX_IXON %d\n", IXON);
    printf("#define LINUX_PARMRK %d\n", PARMRK);
    printf("\n");
    printf("#define LINUX_OPOST %d\n", OPOST);
    printf("#define LINUX_ONLCR %d\n", ONLCR);
    printf("#define LINUX_OCRNL %d\n", OCRNL);
    printf("#define LINUX_ONOCR %d\n", ONOCR);
    printf("#define LINUX_ONLRET %d\n", ONLRET);
    printf("#define LINUX_OFDEL %d\n", OFDEL);
    printf("#define LINUX_OFILL %d\n", OFILL);
    printf("#define LINUX_NLDLY %d\n", NLDLY);
    printf("#define LINUX_NL0 %d\n", NL0);
    printf("#define LINUX_NL1 %d\n", NL1);
    printf("#define LINUX_CRDLY %d\n", CRDLY);
    printf("#define LINUX_CR0 %d\n", CR0);
    printf("#define LINUX_CR1 %d\n", CR1);
    printf("#define LINUX_CR2 %d\n", CR2);
    printf("#define LINUX_CR3 %d\n", CR3);
    printf("#define LINUX_TABDLY %d\n", TABDLY);
    printf("#define LINUX_TAB0 %d\n", TAB0);
    printf("#define LINUX_TAB1 %d\n", TAB1);
    printf("#define LINUX_TAB2 %d\n", TAB2);
    printf("#define LINUX_TAB3 %d\n", TAB3);
    printf("#define LINUX_BSDLY %d\n", BSDLY);
    printf("#define LINUX_BS0 %d\n", BS0);
    printf("#define LINUX_BS1 %d\n", BS1);
    printf("#define LINUX_VTDLY %d\n", VTDLY);
    printf("#define LINUX_VT0 %d\n", VT0);
    printf("#define LINUX_VT1 %d\n", VT1);
    printf("#define LINUX_FFDLY %d\n", FFDLY);
    printf("#define LINUX_FF0 %d\n", FF0);
    printf("#define LINUX_FF1 %d\n", FF1);
    printf("\n");
    printf("#define LINUX_B0 %d\n", B0);
    printf("#define LINUX_B50 %d\n", B50);
    printf("#define LINUX_B75 %d\n", B75);
    printf("#define LINUX_B110 %d\n", B110);
    printf("#define LINUX_B134 %d\n", B134);
    printf("#define LINUX_B150 %d\n", B150);
    printf("#define LINUX_B200 %d\n", B200);
    printf("#define LINUX_B300 %d\n", B300);
    printf("#define LINUX_B600 %d\n", B600);
    printf("#define LINUX_B1200 %d\n", B1200);
    printf("#define LINUX_B1800 %d\n", B1800);
    printf("#define LINUX_B2400 %d\n", B2400);
    printf("#define LINUX_B4800 %d\n", B4800);
    printf("#define LINUX_B9600 %d\n", B9600);
    printf("#define LINUX_B19200 %d\n", B19200);
    printf("#define LINUX_B38400 %d\n", B38400);
    printf("\n");
    printf("#define LINUX_CSIZE %d\n", CSIZE);
    printf("#define LINUX_CS5 %d\n", CS5);
    printf("#define LINUX_CS6 %d\n", CS6);
    printf("#define LINUX_CS7 %d\n", CS7);
    printf("#define LINUX_CS8 %d\n", CS8);
    printf("#define LINUX_CSTOPB %d\n", CSTOPB);
    printf("#define LINUX_CREAD %d\n", CREAD);
    printf("#define LINUX_PARENB %d\n", PARENB);
    printf("#define LINUX_PARODD %d\n", PARODD);
    printf("#define LINUX_HUPCL %d\n", HUPCL);
    printf("#define LINUX_CLOCAL %d\n", CLOCAL);
    printf("\n");
    printf("#define LINUX_ECHO %d\n", ECHO);
    printf("#define LINUX_ECHOE %d\n", ECHOE);
    printf("#define LINUX_ECHOK %d\n", ECHOK);
    printf("#define LINUX_ECHONL %d\n", ECHONL);
    printf("#define LINUX_ICANON %d\n", ICANON);
    printf("#define LINUX_IEXTEN %d\n", IEXTEN);
    printf("#define LINUX_ISIG %d\n", ISIG);
    printf("#define LINUX_NOFLSH %d\n", NOFLSH);
    printf("#define LINUX_TOSTOP %d\n", TOSTOP);
    printf("\n");
    printf("#define LINUX_TCSANOW %d\n", TCSANOW);
    printf("#define LINUX_TCSADRAIN %d\n", TCSADRAIN);
    printf("#define LINUX_TCSAFLUSH %d\n", TCSAFLUSH);
    printf("\n");
    printf("#define LINUX_TCIFLUSH %d\n", TCIFLUSH);
    printf("#define LINUX_TCOFLUSH %d\n", TCOFLUSH);
    printf("#define LINUX_TCIOFLUSH %d\n", TCIOFLUSH);
    printf("\n");
    printf("#define LINUX_TCIOFF %d\n", TCIOFF);
    printf("#define LINUX_TCION %d\n", TCION);
    printf("#define LINUX_TCOOFF %d\n", TCOOFF);
    printf("#define LINUX_TCOON %d\n", TCOON);
    printf("\n");
    printf("#define LINUX_TCGETS 0x%x\n", TCGETS);
    printf("#define LINUX_TCGETS2 0x%x\n", TCGETS2);
    printf("#define LINUX_TCSETS 0x%x\n", TCSETS);
    printf("#define LINUX_TCSETSW 0x%x\n", TCSETSW);
    printf("#define LINUX_TCSETSF 0x%x\n", TCSETSF);
    printf("#define LINUX_TCSETS2 0x%08x\n", TCSETS2);
    printf("#define LINUX_TCSETSW2 0x%08x\n", TCSETSW2);
    printf("#define LINUX_TCSETSF2 0x%08x\n", TCSETSF2);
    printf("\n");
    printf("#define LINUX_TCFLSH 0x%08x\n", TCFLSH);
    printf("\n");
    printf("#include <linux/linux-termios-struct.h>\n");
    return 0;
}

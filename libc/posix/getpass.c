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

#define _DEFAULT_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

char *
getpass(const char *prompt)
{
    static char    line[PASS_MAX + 1];
    FILE          *tty;
    int            fd;
    struct termios termios;
    tcflag_t       c_lflag = 0;
    bool           set = false;
    char          *ret, *nl;

    tty = fopen("/dev/tty", "r+");
    if (!tty) {
        errno = ENXIO;
        return NULL;
    }
    fd = fileno(tty);
    if (fd != -1) {
        if (tcgetattr(fd, &termios) != -1) {
            c_lflag = termios.c_lflag;
            termios.c_lflag &= ~(ECHO | ISIG);
            set = (tcsetattr(fd, TCSAFLUSH, &termios) != -1);
        }
    }
    fputs(prompt, tty);
    ret = fgets(line, sizeof(line), tty);
    if (ret) {
        nl = strchr(ret, '\n');
        if (nl)
            *nl = '\0';
    }
    if (set) {
        putc('\n', tty);
        termios.c_lflag = c_lflag;
        (void)tcsetattr(fd, TCSAFLUSH, &termios);
    }
    fclose(tty);
    return ret;
}

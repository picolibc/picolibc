/*
Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
SPDX-License-Identifier: BSD-3-Clause-Clear

Redistribution and use in source and binary forms, with or without
modification, are permitted (subject to the limitations in the
disclaimer below) provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials provided
    with the distribution.

  * Neither the name of Qualcomm Technologies, Inc. nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _HEXAGON_SEMIHOST_H_
#define _HEXAGON_SEMIHOST_H_

#include <stdint.h>
#include <sys/stat.h>

/* System call codes */
enum hexagon_system_call_code {
    SYS_OPEN = 1,
    SYS_CLOSE = 2,
    SYS_READ = 6,
    SYS_WRITE = 5,
    SYS_ISTTY = 9,
    SYS_SEEK = 10,
    SYS_FLEN = 12,
    SYS_REMOVE = 14,
    SYS_GET_CMDLINE = 21,
    SYS_EXIT = 24,
    SYS_FTELL = 0x100,
};

/* Software interrupt */
#define SWI "trap0 (#0)"

/* Hexagon semihosting calls */
int  flen(int fd);
int  hexagon_ftell(int fd);
int  get_cmdline(char *buffer, int count);

int  hexagon_semihost(enum hexagon_system_call_code code, int *args);

void hexagon_semihost_errno(int err);
enum {
    HEX_EPERM = 1,
    HEX_ENOENT = 2,
    HEX_EINTR = 4,
    HEX_EIO = 5,
    HEX_ENXIO = 6,
    HEX_EBADF = 9,
    HEX_EAGAIN = 11,
    HEX_ENOMEM = 12,
    HEX_EACCES = 13,
    HEX_EFAULT = 14,
    HEX_EBUSY = 16,
    HEX_EEXIST = 17,
    HEX_EXDEV = 18,
    HEX_ENODEV = 19,
    HEX_ENOTDIR = 20,
    HEX_EISDIR = 21,
    HEX_EINVAL = 22,
    HEX_ENFILE = 23,
    HEX_EMFILE = 24,
    HEX_ENOTTY = 25,
    HEX_ETXTBSY = 26,
    HEX_EFBIG = 27,
    HEX_ENOSPC = 28,
    HEX_ESPIPE = 29,
    HEX_EROFS = 30,
    HEX_EMLINK = 31,
    HEX_EPIPE = 32,
    HEX_ERANGE = 34,
    HEX_ENAMETOOLONG = 36,
    HEX_ENOSYS = 38,
    HEX_ELOOP = 40,
    HEX_EOVERFLOW = 75,
};

#endif

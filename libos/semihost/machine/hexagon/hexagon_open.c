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

#include "fcntl.h"
#include "hexagon_semihost.h"
#include <string.h>

#define O_BINARY 0

enum {
    HEX_SEMIMODE_READONLY_TEXT = 0,
    HEX_SEMIMODE_READONLY_BINARY = 1,
    HEX_SEMIMODE_READ_WRITE_TEXT = 2,
    HEX_SEMIMODE_READ_WRITE_BINARY = 3,
    HEX_SEMIMODE_WRITEONLY_CREATE_TRUNC_TEXT = 4,
    HEX_SEMIMODE_WRITEONLY_CREATE_TRUNC_BINARY = 5,
    HEX_SEMIMODE_READWRITE_CREATE_TRUNC_TEXT = 6,
    HEX_SEMIMODE_READWRITE_CREATE_TRUNC_BINARY = 7,
    HEX_SEMIMODE_WRITEONLY_CREATE_APPEND_TEXT = 8,
    HEX_SEMIMODE_WRITEONLY_CREATE_APPEND_BINARY = 9,
    HEX_SEMIMODE_READWRITE_CREATE_APPEND_TEXT = 10,
    HEX_SEMIMODE_READWRITE_CREATE_APPEND_BINARY = 11,
    HEX_SEMIMODE_READWRITE_CREATE_TEXT = 12,
    HEX_SEMIMODE_READWRITE_CREATE_EXCL_TEXT = 13,
    HEX_SEMIMODE_MODEMAP_LENGTH = 14,
};

static const int modemap[HEX_SEMIMODE_MODEMAP_LENGTH] = {
    [HEX_SEMIMODE_READONLY_BINARY] = O_RDONLY,
    [HEX_SEMIMODE_READONLY_TEXT] = O_RDONLY | O_BINARY,
    [HEX_SEMIMODE_READ_WRITE_TEXT] = O_RDWR,
    O_RDWR | O_BINARY,

    [HEX_SEMIMODE_WRITEONLY_CREATE_TRUNC_TEXT] = O_WRONLY | O_CREAT | O_TRUNC,
    [HEX_SEMIMODE_WRITEONLY_CREATE_TRUNC_BINARY] = O_WRONLY | O_CREAT | O_TRUNC | O_BINARY,
    [HEX_SEMIMODE_READWRITE_CREATE_TRUNC_TEXT] = O_RDWR | O_CREAT | O_TRUNC,
    [HEX_SEMIMODE_READWRITE_CREATE_TRUNC_BINARY] = O_RDWR | O_CREAT | O_TRUNC | O_BINARY,

    [HEX_SEMIMODE_WRITEONLY_CREATE_APPEND_TEXT] = O_WRONLY | O_CREAT | O_APPEND,
    [HEX_SEMIMODE_WRITEONLY_CREATE_APPEND_BINARY] = O_WRONLY | O_CREAT | O_APPEND | O_BINARY,
    [HEX_SEMIMODE_READWRITE_CREATE_APPEND_TEXT] = O_RDWR | O_CREAT | O_APPEND,
    [HEX_SEMIMODE_READWRITE_CREATE_APPEND_BINARY] = O_RDWR | O_CREAT | O_APPEND | O_BINARY,

    [HEX_SEMIMODE_READWRITE_CREATE_TEXT] = O_RDWR | O_CREAT,
    [HEX_SEMIMODE_READWRITE_CREATE_EXCL_TEXT] = O_RDWR | O_CREAT | O_EXCL,
};

int
open(const char *name, int mode, ...)
{
    mode &= O_RDONLY | O_WRONLY | O_RDWR | O_CREAT | O_TRUNC | O_APPEND | O_EXCL | O_BINARY;
    int semimode = 0;
    for (size_t i = 0; i < HEX_SEMIMODE_MODEMAP_LENGTH; ++i) {
        if (modemap[i] == mode) {
            semimode = i;
            break;
        }
    }

    size_t length = strlen(name);
    int    args[] = { (int)name, semimode, length };
    int    retval = hexagon_semihost(SYS_OPEN, args);
    return retval;
}

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

#define _DEFAULT_SOURCE
#include "hexagon_semihost.h"
#include <dirent.h>
#include <string.h>
#include <stdint.h>

/*
 * Semihosting dirent as defined by the Hexagon semihosting specification.
 * d_name size matches __NAME_MAX + 1 from sys/dirent.h so names are not
 * silently truncated by the host.
 */
struct __semihost_dirent {
    long d_ino;
    char d_name[256];
};

struct dirent *
readdir(DIR *dir)
{
    struct __semihost_dirent shd;
    register uintptr_t       r0 __asm__("r0") = SYS_READDIR;
    register uintptr_t       r1 __asm__("r1") = (uintptr_t)dir->fd;
    register uintptr_t       r2 __asm__("r2") = (uintptr_t)&shd;
    __asm__ __volatile__(SWI : "=r"(r0), "=r"(r1) : "r"(r0), "r"(r1), "r"(r2));
    if (r0 == 0) {
        /* r1==0 means end-of-directory; r1!=0 means error */
        if (r1 != 0)
            hexagon_semihost_errno((int)r1);
        return 0;
    }
    dir->dirent.d_ino = (ino_t)shd.d_ino;
    dir->dirent.d_type = DT_UNKNOWN;
    strncpy(dir->dirent.d_name, shd.d_name, sizeof(dir->dirent.d_name) - 1);
    dir->dirent.d_name[sizeof(dir->dirent.d_name) - 1] = '\0';
    return &dir->dirent;
}

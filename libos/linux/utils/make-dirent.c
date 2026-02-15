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

#include <dirent.h>
#include <stdio.h>

int
main(void)
{
    printf("#define LINUX_DT_BLK %d\n", DT_BLK);
    printf("#define LINUX_DT_CHR %d\n", DT_CHR);
    printf("#define LINUX_DT_DIR %d\n", DT_DIR);
    printf("#define LINUX_DT_FIFO %d\n", DT_FIFO);
    printf("#define LINUX_DT_LNK %d\n", DT_LNK);
    printf("#define LINUX_DT_REG %d\n", DT_REG);
    printf("#define LINUX_DT_SOCK %d\n", DT_SOCK);
    printf("#define LINUX_DT_UNKNOWN %d\n", DT_UNKNOWN);

    printf("#define MAP_DT(p_dt, l_dt) switch(l_dt) {\\\n");
    printf("    case LINUX_DT_BLK: p_dt = DT_BLK; break;\\\n ");
    printf("    case LINUX_DT_CHR: p_dt = DT_CHR; break;\\\n");
    printf("    case LINUX_DT_DIR: p_dt = DT_DIR; break;\\\n");
    printf("    case LINUX_DT_FIFO: p_dt = DT_FIFO; break;\\\n");
    printf("    case LINUX_DT_LNK: p_dt = DT_LNK; break;\\\n");
    printf("    case LINUX_DT_REG: p_dt = DT_REG; break;\\\n");
    printf("    case LINUX_DT_SOCK: p_dt = DT_SOCK; break;\\\n");
    printf("    case LINUX_DT_UNKNOWN: p_dt = DT_UNKNOWN; break;\\\n");
    printf("    }\n");

    return 0;
}

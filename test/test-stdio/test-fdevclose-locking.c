/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Artem Kulyk
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

/*
 * fdevopen's close hook calls fflush after __flockfile_close has
 * destroyed the FILE lock. With real __STDIO_LOCKING locks this must
 * not touch the destroyed lock.
 */

#include <stdio.h>

#ifndef __STDIO_LOCKING
int
main(void)
{
    return 77;
}
#else
static int
dev_put(char c, FILE *f)
{
    (void)f;
    return (unsigned char)c;
}

static int
dev_flush(FILE *f)
{
    (void)f;
    return 0;
}

int
main(void)
{
    FILE *f = fdevopen(dev_put, NULL, dev_flush);
    int   ret;

    if (!f) {
        perror("fdevopen");
        return 2;
    }
    if (fputs("hi", f) < 0) {
        printf("fputs failed\n");
        return 1;
    }
    ret = fclose(f);
    printf("fclose=%d\n", ret);
    if (ret != 0) {
        printf("fdevclose failed\n");
        return 1;
    }
    return 0;
}
#endif

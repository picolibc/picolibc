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
 * Under __STDIO_LOCKING, fclose must still report the first flush
 * failure even though the close callback flushes an empty buffer again.
 */

#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <string.h>

#ifndef __STDIO_LOCKING
int
main(void)
{
    return 77;
}
#else
static ssize_t
cookie_write(void *cookie, const char *buf, size_t n)
{
    (void)cookie;
    (void)buf;
    (void)n;
    errno = EIO;
    return -1;
}

static int
cookie_close(void *cookie)
{
    (void)cookie;
    return 0;
}

static cookie_io_functions_t funcs = {
    .read = 0,
    .write = cookie_write,
    .seek = 0,
    .close = cookie_close,
};

int
main(void)
{
    char          file_buf[8];
    unsigned char data[3] = { 'a', 'b', 'c' };
    FILE         *f = fopencookie(NULL, "w", funcs);
    int           ret;

    if (!f) {
        perror("fopencookie");
        return 2;
    }
    if (setvbuf(f, file_buf, _IOFBF, sizeof(file_buf)) != 0) {
        perror("setvbuf");
        return 2;
    }

    if (fwrite(data, 1, sizeof(data), f) != sizeof(data)) {
        printf("fwrite failed\n");
        return 1;
    }
    ret = fclose(f);
    printf("fclose=%d\n", ret);
    if (ret != EOF) {
        printf("fclose hid the first flush failure\n");
        return 1;
    }
    return 0;
}
#endif

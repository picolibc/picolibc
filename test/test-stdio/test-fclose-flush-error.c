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
 * fclose must report a final flush failure. The backend write fails,
 * but the backend close succeeds, so the close result must not mask
 * the lost buffered data.
 */

#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <string.h>

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
    unsigned char data[4] = { 'a', 'b', 'c', 'd' };
    FILE         *f = fopencookie(NULL, "w", funcs);
    size_t        written;
    int           ret;

    if (!f) {
        perror("fopencookie");
        return 2;
    }
    if (setvbuf(f, file_buf, _IOFBF, sizeof(file_buf)) != 0) {
        perror("setvbuf");
        return 2;
    }

    written = fwrite(data, 1, sizeof(data), f);
    ret = fclose(f);

    printf("fwrite=%zu fclose=%d\n", written, ret);
    if (written != sizeof(data) || ret != EOF) {
        printf("fclose hid the failed final flush\n");
        return 1;
    }
    return 0;
}

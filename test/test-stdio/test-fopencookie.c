/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Alexey Lapshin
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

#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define IO_T   ssize_t
#define BUF_T  char
#define SEEK_T off_t

static char   test_buf[1024];
static SEEK_T test_pos;
static SEEK_T test_end;

#define min(a, b) ((SEEK_T)(a) < (SEEK_T)(b) ? (SEEK_T)(a) : (SEEK_T)(b))
#define max(a, b) ((SEEK_T)(a) > (SEEK_T)(b) ? (SEEK_T)(a) : (SEEK_T)(b))

static IO_T
test_read(void *cookie, BUF_T *buf, size_t n)
{
    (void)cookie;
    assert(test_pos <= test_end);
    n = min(n, test_end - test_pos);
    memcpy(buf, test_buf + test_pos, n);
    test_pos += n;
    return n;
}

static IO_T
test_write(void *cookie, const BUF_T *buf, size_t n)
{
    (void)cookie;
    assert((size_t)test_pos <= sizeof(test_buf));
    n = min(n, sizeof(test_buf) - test_pos);
    memcpy(test_buf + test_pos, buf, n);
    test_pos += n;
    test_end = max(test_end, test_pos);
    return n;
}

static int
test_seek(void *cookie, SEEK_T *pos, int whence)
{
    SEEK_T new_pos = test_pos;

    (void)cookie;
    switch (whence) {
    case SEEK_CUR:
        new_pos = test_pos + *pos;
        break;
    case SEEK_SET:
        new_pos = *pos;
        break;
    case SEEK_END:
        new_pos = test_end + *pos;
        break;
    default:
        return -1;
    }
    if (new_pos < 0 || new_pos > test_end)
        return -1;
    test_pos = new_pos;
    *pos = new_pos;
    return 0;
}

static int
test_close(void *cookie)
{
    (void)cookie;
    return 0;
}

#define check(condition, message)                    \
    do {                                             \
        if (!(condition)) {                          \
            printf("%s: %s\n", message, #condition); \
            exit(1);                                 \
        }                                            \
    } while (0)

#define MESSAGE "hello, world"

int
main(void)
{
    cookie_io_functions_t io = {
        .read = test_read,
        .write = test_write,
        .seek = test_seek,
        .close = test_close,
    };
    FILE  *fp = fopencookie(NULL, "w+", io);
    char   buf[sizeof(MESSAGE)];
    size_t ret;

    check(fp != NULL, "fopencookie");
    fprintf(fp, "%s", MESSAGE);
    fflush(fp);
    check(memcmp(test_buf, MESSAGE, strlen(MESSAGE)) == 0, "printf");
    check(fseek(fp, 0L, SEEK_END) == 0, "fseek end");
    check(ftell(fp) == (long)strlen(MESSAGE), "ftell end");
    check(fseek(fp, -5L, SEEK_CUR) == 0, "fseek cur");
    check(ftell(fp) == (long)(strlen(MESSAGE) - 5), "ftell cur");
    check(fseek(fp, 0L, SEEK_SET) == 0, "fseek set");
    ret = fread(buf, 1, sizeof(buf), fp);
    check(ret == strlen(MESSAGE), "fread size");
    check(memcmp(buf, MESSAGE, strlen(MESSAGE)) == 0, "fread contents");
    fclose(fp);

    // dummy write and read test
    io.read = NULL;
    io.write = NULL;
    io.seek = NULL;
    io.close = NULL;
    fp = fopencookie(NULL, "w+", io);
    check(fp != NULL, "fopencookie");
    check(setvbuf(fp, NULL, _IONBF, 0) == 0, "setvbuf");
    check(fwrite(MESSAGE, 1, strlen(MESSAGE), fp) == 0, "fwrite size");
    check(fseek(fp, 0L, SEEK_END) == -1, "fseek end");
    check(ftell(fp) == -1, "ftell end");
    check(fseek(fp, -5L, SEEK_CUR) == -1, "fseek cur");
    check(ftell(fp) == -1, "ftell cur");
    check(fseek(fp, 0L, SEEK_SET) == -1, "fseek set");
    check(fread(buf, 1, sizeof(buf), fp) == 0, "fread size");
    check(memcmp(buf, MESSAGE, strlen(MESSAGE)) == 0, "fread contents");
    fclose(fp);
    return 0;
}
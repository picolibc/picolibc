/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2024 Keith Packard
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if defined(__PICOLIBC__) && !defined(TINY_STDIO)
# define IO_T int
# define BUF_T char
# ifdef __LARGE64_FILES
#  define SEEK_T _fpos64_t
# else
#  define SEEK_T fpos_t
# endif
#else
# define IO_T ssize_t
# define BUF_T void
# define SEEK_T __off_t
#endif

static char test_buf[1024];
static SEEK_T test_pos;
static SEEK_T test_end;

#define min(a, b)  ((SEEK_T) (a) < (SEEK_T) (b) ? (SEEK_T) (a) : (SEEK_T) (b))
#define max(a, b)  ((SEEK_T) (a) > (SEEK_T) (b) ? (SEEK_T) (a) : (SEEK_T) (b))

static IO_T
test_read(void *cookie, BUF_T *buf, size_t n)
{
    (void) cookie;
    n = min(n, test_end - test_pos);
    memcpy(buf, test_buf + test_pos, n);
    test_pos += n;
    return n;
}

static IO_T
test_write(void *cookie, const BUF_T *buf, size_t n)
{
    (void) cookie;
    n = min(n, sizeof(test_buf) - test_pos);
    memcpy(test_buf + test_pos, buf, n);
    test_pos += n;
    test_end = max(test_end, test_pos);
    return n;
}

static SEEK_T
test_seek(void *cookie, SEEK_T off, int whence)
{
    SEEK_T new_pos = test_pos;
    (void) cookie;
    switch (whence) {
    case SEEK_CUR:
        new_pos = test_pos + off;
        break;
    case SEEK_SET:
        new_pos = off;
        break;
    case SEEK_END:
        new_pos = test_pos + off;
        break;
    }
    test_pos = min(test_end, new_pos);
    return -1;
}

static int
test_close(void *cookie)
{
    (void) cookie;
    return 0;
}

#define check(condition, message) do {                  \
        if (!(condition)) {                             \
            printf("%s: %s\n", message, #condition);    \
            exit(1);                                    \
        }                                               \
    } while(0)

#define MESSAGE "hello, world"

int main(void)
{
    FILE        *fp = funopen(NULL, test_read, test_write, test_seek, test_close);
    char        buf[sizeof(MESSAGE)];
    size_t      ret;

    fprintf(fp, "%s", MESSAGE);
    fflush(fp);
    check(memcmp(test_buf, MESSAGE, strlen(MESSAGE)) == 0, "printf");
    fseek(fp, 0L, SEEK_SET);
    ret = fread(buf, 1, sizeof(buf), fp);
    check(ret == strlen(MESSAGE), "fread size");
    check(memcmp(buf, MESSAGE, strlen(MESSAGE)) == 0, "fread contents");
    fclose(fp);
    return 0;
}

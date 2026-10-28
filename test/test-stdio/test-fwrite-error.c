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
 * A failed fwrite must set only the stream error indicator. It must
 * not fabricate EOF or wide orientation, and it must not report bytes
 * from the chunk whose flush failed as successfully written.
 */

#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#ifdef __PICOLIBC__

static int fail_writes;

static ssize_t
cookie_write(void *cookie, const char *buf, size_t n)
{
    (void)cookie;
    (void)buf;
    (void)n;
    if (fail_writes) {
        errno = EIO;
        return -1;
    }
    return (ssize_t)n;
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

static int
check_error(FILE *f, const char *what, size_t count, size_t want_max)
{
    int errors = 0;

    if (count > want_max) {
        printf("%s: count %zu > %zu\n", what, count, want_max);
        errors++;
    }
    if (ferror(f) == 0) {
        printf("%s: ferror is clear\n", what);
        errors++;
    }
    if (feof(f)) {
        printf("%s: false EOF\n", what);
        errors++;
    }
    if (fwide(f, 0) > 0) {
        printf("%s: false wide orientation\n", what);
        errors++;
    }
    return errors;
}

static int
test_buffered_error(void)
{
    char          file_buf[8];
    unsigned char first[4] = { 'a', 'b', 'c', 'd' };
    unsigned char second[4] = { 'e', 'f', 'g', 'h' };
    FILE         *f = fopencookie(NULL, "w", funcs);
    size_t        r1, r2;
    int           errors;

    if (!f)
        return 1;
    if (setvbuf(f, file_buf, _IOFBF, sizeof(file_buf)) != 0) {
        fclose(f);
        return 1;
    }

    fail_writes = 0;
    r1 = fwrite(first, 1, sizeof(first), f);
    fail_writes = 1;
    r2 = fwrite(second, 1, sizeof(second), f);

    errors = 0;
    if (r1 != sizeof(first)) {
        printf("buffered: first fwrite=%zu\n", r1);
        errors++;
    }
    errors += check_error(f, "buffered", r2, sizeof(second) - 1);
    fclose(f);
    return errors;
}

static int
test_direct_preflush_error(void)
{
#ifdef __FAST_BUFIO
    char          file_buf[8];
    unsigned char first[4] = { 'a', 'b', 'c', 'd' };
    unsigned char large[8] = { 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l' };
    FILE         *f = fopencookie(NULL, "w", funcs);
    size_t        r1, r2;
    int           errors;

    if (!f)
        return 1;
    if (setvbuf(f, file_buf, _IOFBF, sizeof(file_buf)) != 0) {
        fclose(f);
        return 1;
    }

    fail_writes = 0;
    r1 = fwrite(first, 1, sizeof(first), f);
    fail_writes = 1;
    r2 = fwrite(large, 1, sizeof(large), f);

    errors = 0;
    if (r1 != sizeof(first)) {
        printf("direct: first fwrite=%zu\n", r1);
        errors++;
    }
    errors += check_error(f, "direct", r2, sizeof(large) - 1);
    fclose(f);
    return errors;
#else
    return 0;
#endif
}

static int
test_char_path_error(void)
{
    char   storage[4] = { 0 };
    char   data[8] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h' };
    FILE  *f = fmemopen(storage, sizeof(storage), "w");
    size_t written;
    int    errors = 0;

    if (!f)
        return 1;
    written = fwrite(data, 1, sizeof(data), f);
    if (written >= sizeof(data)) {
        printf("char: fwrite claimed %zu\n", written);
        errors++;
    }
    if (!ferror(f)) {
        printf("char: ferror is clear\n");
        errors++;
    }
    fclose(f);
    return errors;
}

int
main(void)
{
    int errors = 0;

    errors += test_buffered_error();
    errors += test_direct_preflush_error();
    errors += test_char_path_error();

    if (errors) {
        printf("%d fwrite error-check failures\n", errors);
        return 1;
    }
    return 0;
}

#else

int
main(void)
{
    printf("Skipping test-fwrite-error, not running picolibc\n");
    return 77;
}

#endif

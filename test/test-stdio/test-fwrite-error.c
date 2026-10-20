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

static int    write_calls;
static int    fail_on_call;
static int    partial_on_call;
static size_t partial_count;
static size_t backend_bytes;

static void
reset_backend(int fail_call, int partial_call, size_t partial)
{
    write_calls = 0;
    fail_on_call = fail_call;
    partial_on_call = partial_call;
    partial_count = partial;
    backend_bytes = 0;
}

static ssize_t
cookie_write(void *cookie, const char *buf, size_t n)
{
    (void)cookie;
    (void)buf;
    (void)n;
    write_calls++;
    if (write_calls == fail_on_call) {
        errno = EIO;
        return -1;
    }
    if (write_calls == partial_on_call && n > partial_count)
        n = partial_count;
    backend_bytes += n;
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
check_error(FILE *f, const char *what, size_t count, size_t want)
{
    int errors = 0;
    int saved_errno = errno;

    if (count != want) {
        printf("%s: count %zu != %zu\n", what, count, want);
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
    if (fwide(f, 0) >= 0) {
        printf("%s: stream is not byte-oriented\n", what);
        errors++;
    }
    if (saved_errno != EIO) {
        printf("%s: errno=%d instead of EIO\n", what, saved_errno);
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

    reset_backend(0, 0, 0);
    r1 = fwrite(first, 1, sizeof(first), f);
    reset_backend(1, 0, 0);
    errno = 0;
    r2 = fwrite(second, 1, sizeof(second), f);

    errors = 0;
    if (r1 != sizeof(first)) {
        printf("buffered: first fwrite=%zu\n", r1);
        errors++;
    }
    errors += check_error(f, "buffered", r2, sizeof(second) - 1);
    reset_backend(0, 0, 0);
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

    reset_backend(0, 0, 0);
    r1 = fwrite(first, 1, sizeof(first), f);
    reset_backend(1, 0, 0);
    errno = 0;
    r2 = fwrite(large, 1, sizeof(large), f);

    errors = 0;
    if (r1 != sizeof(first)) {
        printf("direct: first fwrite=%zu\n", r1);
        errors++;
    }
    errors += check_error(f, "direct", r2, 0);
    reset_backend(0, 0, 0);
    fclose(f);
    return errors;
#else
    return 0;
#endif
}

static int
test_partial_element_error(void)
{
    char          file_buf[8];
    unsigned char first[2] = { 'a', 'b' };
    unsigned char second[6] = { 'c', 'd', 'e', 'f', 'g', 'h' };
    FILE         *f = fopencookie(NULL, "w", funcs);
    size_t        r1, r2;
    int           errors;

    if (!f)
        return 1;
    if (setvbuf(f, file_buf, _IOFBF, sizeof(file_buf)) != 0) {
        fclose(f);
        return 1;
    }

    reset_backend(0, 0, 0);
    r1 = fwrite(first, 2, 1, f);
    reset_backend(1, 0, 0);
    errno = 0;
    r2 = fwrite(second, 2, 3, f);

    errors = 0;
    if (r1 != 1) {
        printf("partial: first fwrite=%zu\n", r1);
        errors++;
    }
    errors += check_error(f, "partial", r2, 2);
    reset_backend(0, 0, 0);
    fclose(f);
    return errors;
}

static int
test_later_flush_error(size_t size)
{
#ifndef __FAST_BUFIO
    char          file_buf[4];
    unsigned char data[8] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h' };
    FILE         *f = fopencookie(NULL, "w", funcs);
    size_t        count;
    int           errors = 0;

    if (!f)
        return 1;
    if (setvbuf(f, file_buf, _IOFBF, sizeof(file_buf)) != 0) {
        fclose(f);
        return 1;
    }

    reset_backend(2, 0, 0);
    errno = 0;
    count = fwrite(data, size, sizeof(data) / size, f);
    errors += check_error(f, size == 1 ? "later-flush-byte" : "later-flush-element", count,
                          size == 1 ? 7 : 3);
    if (backend_bytes != 4) {
        printf("later flush: backend bytes %zu != 4\n", backend_bytes);
        errors++;
    }
    reset_backend(0, 0, 0);
    fclose(f);
    return errors;
#else
    (void)size;
    return 0;
#endif
}

static int
test_partial_backend_error(void)
{
#ifndef __FAST_BUFIO
    char          file_buf[4];
    unsigned char data[4] = { 'a', 'b', 'c', 'd' };
    FILE         *f = fopencookie(NULL, "w", funcs);
    size_t        count;
    int           errors = 0;

    if (!f)
        return 1;
    if (setvbuf(f, file_buf, _IOFBF, sizeof(file_buf)) != 0) {
        fclose(f);
        return 1;
    }

    reset_backend(2, 1, 2);
    errno = 0;
    count = fwrite(data, 1, sizeof(data), f);
    errors += check_error(f, "partial-backend", count, 3);
    if (backend_bytes != 2) {
        printf("partial backend: bytes %zu != 2\n", backend_bytes);
        errors++;
    }
    reset_backend(0, 0, 0);
    fclose(f);
    return errors;
#else
    return 0;
#endif
}

static int
test_direct_partial_error(size_t size)
{
#ifdef __FAST_BUFIO
    char          file_buf[8];
    unsigned char prefix[2] = { 'x', 'y' };
    unsigned char data[8] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h' };
    FILE         *f = fopencookie(NULL, "w", funcs);
    size_t        first, count;
    int           errors = 0;

    if (!f)
        return 1;
    if (setvbuf(f, file_buf, _IOFBF, sizeof(file_buf)) != 0) {
        fclose(f);
        return 1;
    }

    reset_backend(0, 0, 0);
    first = fwrite(prefix, 1, sizeof(prefix), f);
    reset_backend(3, 2, 3);
    errno = 0;
    count = fwrite(data, size, sizeof(data) / size, f);
    if (first != sizeof(prefix)) {
        printf("direct partial: prefix=%zu\n", first);
        errors++;
    }
    errors += check_error(f, size == 1 ? "direct-partial-byte" : "direct-partial-element", count,
                          3 / size);
    if (backend_bytes != sizeof(prefix) + 3) {
        printf("direct partial: backend bytes %zu != %zu\n", backend_bytes, sizeof(prefix) + 3);
        errors++;
    }
    reset_backend(0, 0, 0);
    fclose(f);
    return errors;
#else
    (void)size;
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
    if (written != sizeof(storage)) {
        printf("char: fwrite returned %zu instead of %zu\n", written, sizeof(storage));
        errors++;
    }
    if (!ferror(f)) {
        printf("char: ferror is clear\n");
        errors++;
    }
    fclose(f);
    return errors;
}

static int
test_zero_length(void)
{
    unsigned char data[1] = { 'a' };
    FILE         *f = fopencookie(NULL, "w", funcs);
    int           errors = 0;

    if (!f)
        return 1;
    reset_backend(1, 0, 0);
    errno = EDOM;
    if (fwrite(data, 0, 1, f) != 0 || fwrite(data, 1, 0, f) != 0 || errno != EDOM || ferror(f) != 0
        || feof(f) != 0 || fwide(f, 0) >= 0) {
        printf("zero-length fwrite changed stream state\n");
        errors++;
    }
    reset_backend(0, 0, 0);
    fclose(f);
    return errors;
}

int
main(void)
{
    int errors = 0;

    errors += test_buffered_error();
    errors += test_partial_element_error();
    errors += test_later_flush_error(1);
    errors += test_later_flush_error(2);
    errors += test_partial_backend_error();
    errors += test_direct_preflush_error();
    errors += test_direct_partial_error(1);
    errors += test_direct_partial_error(2);
    errors += test_char_path_error();
    errors += test_zero_length();

    if (errors) {
        printf("%d fwrite error-check failures\n", errors);
        return 1;
    }
    return 0;
}

/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 Artem Kulyk
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
 * fmemopen update streams must not corrupt the buffer when an input
 * fflush follows a read or a pending ungetc.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>

static int
flush_after_read(void)
{
    char  data[4] = { 'A', 'B', 'C', 'D' };
    FILE *f = fmemopen(data, sizeof(data), "r+");
    int   first, next, flush_ret;
    int   errors = 0;

    if (!f) {
        perror("fmemopen");
        return 1;
    }

    first = fgetc(f);
    flush_ret = fflush(f);
    next = fgetc(f);

    if (first != 'A' || flush_ret != 0 || next != 'B') {
        printf("read+fflush: first=%c fflush=%d next=%c\n", first, flush_ret, next);
        errors++;
    }
    if (memcmp(data, "ABCD", sizeof(data)) != 0) {
        printf("read+fflush corrupted buffer: %.4s\n", data);
        errors++;
    }
    fclose(f);
    return errors;
}

static int
flush_after_ungetc(void)
{
    char  data[4] = { 'A', 'B', 'C', 'D' };
    FILE *f = fmemopen(data, sizeof(data), "r+");
    int   first, next, flush_ret;
    int   errors = 0;

    if (!f) {
        perror("fmemopen");
        return 1;
    }

    first = fgetc(f);
    if (ungetc(first, f) == EOF) {
        printf("ungetc failed\n");
        fclose(f);
        return 1;
    }
    flush_ret = fflush(f);
    next = fgetc(f);

    if (first != 'A' || flush_ret != 0 || next != 'A') {
        printf("ungetc+fflush: first=%c fflush=%d next=%c\n", first, flush_ret, next);
        errors++;
    }
    if (memcmp(data, "ABCD", sizeof(data)) != 0) {
        printf("ungetc+fflush corrupted buffer: %.4s\n", data);
        errors++;
    }
    fclose(f);
    return errors;
}

static int
write_after_flush(void)
{
    char  data[4] = { 'A', 'B', 'C', 'D' };
    FILE *f = fmemopen(data, sizeof(data), "r+");
    int   errors = 0;

    if (!f) {
        perror("fmemopen");
        return 1;
    }

    if (fgetc(f) != 'A' || fflush(f) != 0 || fputc('X', f) != 'X') {
        printf("read+flush+write failed\n");
        errors++;
    }
    if (memcmp(data, "AXCD", sizeof(data)) != 0) {
        printf("read+flush+write corrupted buffer: %.4s\n", data);
        errors++;
    }
    fclose(f);
    return errors;
}

int
main(void)
{
    int errors = 0;

    errors += flush_after_read();
    errors += flush_after_ungetc();
    errors += write_after_flush();

    if (errors) {
        printf("%d fmemopen r+ failures\n", errors);
        return 1;
    }
    return 0;
}

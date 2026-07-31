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

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#define check(condition, message)                    \
    do {                                             \
        if (!(condition)) {                          \
            printf("%s: %s\n", message, #condition); \
            exit(1);                                 \
        }                                            \
    } while (0)

#define MESSAGE "Hello picolibc!"

int
main(void)
{
    FILE  *stream = NULL;
    char  *buf = NULL;
    size_t len = 0;
    int    i;
    char   zero_buf[32] = { 0 };

    stream = open_memstream(&buf, &len);
    check(stream != NULL, "open_memstream");
    fprintf(stream, MESSAGE);
    fflush(stream);
    check(strcmp(buf, MESSAGE) == 0, "fprintf contents");
    check(len == strlen(MESSAGE), "fprintf length");
    fseek(stream, 0, SEEK_SET);
    fprintf(stream, "HELLO");
    fflush(stream);
    check(strcmp(buf, "HELLO picolibc!") == 0, "SEEK_SET 0");
    fseek(stream, sizeof(zero_buf), SEEK_CUR);
    fprintf(stream, MESSAGE);
    int zeroed_len = sizeof(zero_buf) - (strlen(MESSAGE) - strlen("HELLO"));
    check(memcmp(&buf[strlen(MESSAGE)], zero_buf, zeroed_len) == 0, "seek zero fill");
    check(strcmp(&buf[strlen(MESSAGE) + zeroed_len], MESSAGE) == 0, "write after SEEK_CUR");
    check(fseek(stream, -1000, SEEK_SET) != 0, "fseek beyond start of stream");
    fclose(stream);
    check(strcmp(buf, "HELLO picolibc!") == 0, "fseek patch");
    check(len == strlen(MESSAGE) * 2 + zeroed_len, "fseek patch length");
    free(buf);

    stream = open_memstream(&buf, &len);
    check(stream != NULL, "open_memstream SEEK_END");
    fprintf(stream, MESSAGE);
    fflush(stream);
    check(len == strlen(MESSAGE), "len changed after fflush");
    fseek(stream, sizeof(zero_buf), SEEK_END);
    fprintf(stream, MESSAGE);
    fclose(stream);
    check(strcmp(buf, MESSAGE) == 0, "keep data before SEEK_END");
    check(memcmp(&buf[strlen(MESSAGE)], zero_buf, zeroed_len) == 0, "seek zero fill");
    check(strcmp(&buf[strlen(MESSAGE) + sizeof(zero_buf)], MESSAGE) == 0, "write after SEEK_END");
    check(len == strlen(MESSAGE) * 2 + sizeof(zero_buf), "fseek SEEK_END length");
    free(buf);

    stream = open_memstream(&buf, &len);
    check(stream != NULL, "open_memstream unflushed SEEK_END");
    fprintf(stream, MESSAGE);
    fseek(stream, 0, SEEK_END);
    fprintf(stream, MESSAGE);
    check(fseek(stream, 0, 12345) != 0, "invalid whence");
    fclose(stream);
    check(strcmp(buf, MESSAGE MESSAGE) == 0, "SEEK_END uses unflushed stream size");
    check(len == strlen(MESSAGE) * 2, "unflushed SEEK_END length");
    free(buf);

    stream = open_memstream(&buf, &len);
    check(stream != NULL, "open_memstream unflushed SEEK_END");
    fprintf(stream, MESSAGE);
    fseek(stream, sizeof(zero_buf), SEEK_END);
    fprintf(stream, MESSAGE);
    fflush(stream);
    /*
     * Buffer is now:
     *
     *     MESSAGE\0...\0MESSAGE\0
     *                           ^
     */
    fseek(stream, strlen(MESSAGE) + 1, SEEK_SET);
    fprintf(stream, MESSAGE);
    fflush(stream);
    /*
     * Buffer is now:
     *
     *     MESSAGE\0MESSAGE\0...\0MESSAGE\0
     *             ^
     */
    check(len == strlen(MESSAGE) * 2 + 1, "SEEK_END length");
    check(fseek(stream, -strlen(MESSAGE) * 2, SEEK_END) == 0, "fseek SEEK_END");
    fflush(stream);
    check(len == 1, "SEEK_END length");
    check(fseek(stream, strlen(MESSAGE), SEEK_END) == 0, "fseek SEEK_END twice in row");
    fflush(stream);
    check(len == strlen(MESSAGE) + 1, "SEEK_END length");
    fprintf(stream, "TEST");
    fflush(stream);
    /*
     * Buffer is now:
     *
     *     MESSAGE\0TEST${MESSAGE[4:]}\0...\0MESSAGE\0
     *                 ^
     */
    check(len == strlen(MESSAGE) + 1 + strlen("TEST"), "SEEK_END length");
    check(buf[strlen(MESSAGE) + 1 + strlen("TEST") + 1] != 0,
          "fwrite does override data with null terminator");
    fclose(stream);
    free(buf);

#ifdef __PICOLIBC__ // we know what memory allocated on the start and how it grows
    stream = open_memstream(&buf, &len);
    check(stream != NULL, "open_memstream sparse flush terminator");
    check(fseek(stream, BUFSIZ + 32, SEEK_SET) == 0, "sparse SEEK_SET");
    fclose(stream);
    for (i = 0; i < BUFSIZ + BUFSIZ / 2; i++)
        if (buf[i] != 0)
            check(0, "sparse flush zero fill");
    free(buf);
#endif

    stream = open_memstream(&buf, &len);
    check(stream != NULL, "open_memstream realloc");
    fprintf(stream, MESSAGE);
    fflush(stream);
    check(malloc_usable_size(buf) <= BUFSIZ * 4, "malloc_usable_size <= BUFSIZ * 4");
    for (i = 0; i < BUFSIZ * 8; i++)
        fputc('A' + (i % ('Z' - 'A')), stream);
    fclose(stream);
    check(malloc_usable_size(buf) >= BUFSIZ * 8, "malloc_usable_size >= BUFSIZ * 8");
    free(buf);

    return 0;
}

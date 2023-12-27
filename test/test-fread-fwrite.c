/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2023 Keith Packard
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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifndef TEST_FILE_NAME
#define TEST_FILE_NAME "FIO.TXT"
#endif

#define check(condition, message) do {                  \
        if (!(condition)) {                             \
            printf("%s: %s\n", message, #condition);    \
            exit(1);                                    \
        }                                               \
    } while(0)

#define RAND_MUL        1664525
#define RAND_ADD        1013904223

static inline unsigned long
my_permute(long input)
{
    return input * RAND_MUL + RAND_ADD;
}

static inline uint8_t
test_value(long offset, int generation)
{
    return (uint8_t) my_permute(offset * (generation + 1));
}

static void
fill_buf(long offset, int generation, void *b, size_t len)
{
    uint8_t *bp = b;
    while (len--)
        *bp++ = test_value(offset++, generation);
}

static long
check_buf(long offset, int generation, void *b, size_t len)
{
    uint8_t *bp = b;
    while (len--) {
        if (test_value(offset, generation) != *bp++) {
            return offset;
        }
        offset++;
    }
    return -1L;
}

#define BUF_SIZE        2048

static uint8_t out[BUF_SIZE];
static uint8_t in[BUF_SIZE];

static long offsets[BUF_SIZE];

static void
random_offsets(int generation, int num_ops, size_t per_op)
{
    int i, j;

    for (i = 0; i < num_ops; i++)
        offsets[i] = per_op * i;

    for (i = 0; i < num_ops - 1; i++) {
        j = my_permute(i * num_ops * per_op * generation) % (num_ops - i) + i;
        long tmp = offsets[i];
        offsets[i] = offsets[j];
        offsets[j] = tmp;
    }
}

int
main(int argc, char **argv)
{
    FILE *f;
    int generation = 0;
    size_t ret;
    size_t size;
    size_t nmemb, per_op;
    off_t offset;
    char *file_name = TEST_FILE_NAME;
    int order;
    int num_ops;
    int op;
    int status = 0;
    int repeat = 10000;
    long failed;

    if (argc > 1) {
        repeat = atoi(argv[1]);
        if (!repeat)
            repeat = 10000;
    }

    printf("test file %s repeat %d\n", file_name, repeat);

    /* Make sure we can create a file, write contents and read them back */

    for (size = 1; size < 16; size <<= 1) {
        nmemb = BUF_SIZE / size;
        for (order = 0; order < 2; order++) {
            for (per_op = nmemb; per_op >= 1; per_op /= 2) {

                num_ops = nmemb / per_op;

                f = fopen(file_name, "w");
                check(f != NULL, "fopen w");

                fill_buf(0, generation, out, BUF_SIZE);

                if (order == 0) {
                    for (op = 0; op < num_ops; op++) {
                        ret = fwrite(out + op * size * per_op, size, per_op, f);
                        check(ret == per_op, "fwrite ordered");
                    }
                } else {
                    random_offsets(generation, num_ops, per_op);
                    offset = 0;
                    for (op = 0; op < num_ops; op++) {
                        fseek(f, size * offsets[op] - offset, SEEK_CUR);
                        ret = fwrite(out + size * offsets[op], size, per_op, f);
                        offset = size * offsets[op] + size * per_op;
                        check(ret == per_op, "fwrite random");
                    }
                }
                fclose(f);

                memset(in, 0xcd, BUF_SIZE);

                f = fopen(file_name, "r");
                check(f != NULL, "fopen r");
                if (order == 0) {
                    for (op = 0; op < num_ops; op++) {
                        ret = fread(in + op * size * per_op, size, per_op, f);
                        check(ret == per_op, "fread ordered");
                    }
                } else {
                    random_offsets(generation * 1146533039, num_ops, per_op);
                    offset = 0;
                    for (op = 0; op < num_ops; op++) {
                        fseek(f, size * offsets[op] - offset, SEEK_CUR);
                        ret = fread(in + size * offsets[op], size, per_op, f);
                        offset = size * offsets[op] + size * per_op;
                        check(ret == per_op, "fread random");
                    }
                }

                fclose(f);
                failed = check_buf(0, generation, in, BUF_SIZE);
                if (failed != -1L) {
                    printf("check failed at offset %ld size %zd order %d per_op %zd\n",
                           failed, size, order, per_op);
                    status = 1;
                }
                generation++;
            }
        }
    }

    /* Now do a mix of read/write operations */

    f = fopen(file_name, "w+");
    check(f != NULL, "fopen w+");

    memset(out, '\0', BUF_SIZE);

    ret = fwrite(out, 1, BUF_SIZE, f);
    check(ret == BUF_SIZE, "fwrite mixed setup");

    for (int generation = 0; generation < repeat; generation++) {

        if (random() & 0x8) {
            long write_offset, write_len;

            write_offset = random() % BUF_SIZE;
            write_len = random() % (BUF_SIZE - write_offset);

            fill_buf(write_offset, generation, out + write_offset, write_len);

            fseek(f, write_offset, SEEK_SET);

            ret = fwrite(out + write_offset, 1, write_len, f);
            check(ret == (size_t) write_len, "fwrite mixed");
        } else {
            long read_offset, read_len, i;
            read_offset = random() % BUF_SIZE;
            read_len = random() % (BUF_SIZE - read_offset);

            fseek(f, read_offset, SEEK_SET);

            /* Make sure we can unget a char and have it appear on the input */
            if (generation % 10 == 9) {
                int c = getc(f);
                ungetc(c + 1, f);
            }

            ret = fread(in + read_offset, 1, read_len, f);
            check(ret == (size_t) read_len, "fread mixed");

            for (i = 0; i < read_len; i++) {
                uint8_t expect = out[read_offset + i];
                if (generation % 10 == 9 && i == 0)
                    expect++;
                if (in[read_offset + i] != expect) {
                    printf("mixed check failed at offset %ld generation %d\n",
                           read_offset + i, generation);
                    status = 1;
                }
            }
        }
    }

    (void) remove(file_name);

    exit(status);
}

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
 * A pending ungetc character is part of the logical file position.
 * fseek(SEEK_CUR) must account for it before clearing the pushback.
 */

#define _GNU_SOURCE
#include <stdio.h>

static int
run_case(FILE *f, int delta, long want_pos, int want_next)
{
    int  first, next, seek_ret;
    long before, after;

    first = fgetc(f);
    before = ftell(f);
    if (ungetc(first, f) == EOF) {
        perror("ungetc");
        return 2;
    }

    seek_ret = fseek(f, delta, SEEK_CUR);
    after = ftell(f);
    next = fgetc(f);

    printf("first=%c before=%ld delta=%d seek=%d after=%ld next=%c\n", first, before, delta,
           seek_ret, after, next);

    if (first != 'A' || before != 1 || seek_ret != 0 || after != want_pos || next != want_next) {
        printf("ungetc/reposition failed\n");
        return 1;
    }
    return 0;
}

int
main(void)
{
    char  data[4] = { 'A', 'B', 'C', 'D' };
    FILE *f = fmemopen(data, sizeof(data), "r");
    int   errors = 0;

    if (!f) {
        perror("fmemopen");
        return 2;
    }

    errors += run_case(f, 0, 0, 'A');

    if (fseek(f, 0, SEEK_SET) != 0) {
        printf("reset failed\n");
        errors++;
    } else {
        errors += run_case(f, 1, 1, 'B');
    }

    fclose(f);
    return errors ? 1 : 0;
}

/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Keith Packard
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

#include <stdio.h>
#include <assert.h>
#include <string.h>

#ifdef __PICOLIBC__

static FILE  *fdevfile;
static char   fdevbuf[BUFSIZ];
static size_t fdevoff;

static int __nonnull((2))
test_put(char c, FILE *f)
{
    assert(f == fdevfile);
    assert(fdevoff < BUFSIZ);
    fdevbuf[fdevoff++] = c;
    return 0;
}

static int __nonnull((1))
test_get(FILE *f)
{
    assert(f == fdevfile);
    assert(fdevoff < BUFSIZ);
    return fdevbuf[fdevoff++];
}

static int __nonnull((1))
test_flush(FILE *f)
{
    assert(f == fdevfile);
    assert(fdevoff < BUFSIZ);
    fdevbuf[fdevoff] = '\0';
    return 0;
}

#define TEST_STRING "hello, world\n"

int
main(void)
{
    static char mybuf[BUFSIZ];
    char       *ret;

    /* Open file for writing */
    fdevfile = fdevopen(test_put, NULL, test_flush);

    /* Make sure fdevopen succeeded */
    assert(fdevfile);

    /* Write a string to the buffer */
    fputs(TEST_STRING, fdevfile);

    /* Close the file */
    fclose(fdevfile);

    /* Verify that the expected string was written */
    assert(strcmp(fdevbuf, TEST_STRING) == 0);

    /* Reset the file position */
    fdevoff = 0;

    /* Open file for reading */
    fdevfile = fdevopen(NULL, test_get, NULL);

    /* Make sure fdevopen succeeded */
    assert(fdevfile);

    /* Read a string from the buffer */
    ret = fgets(mybuf, sizeof(mybuf), fdevfile);

    /* Make sure fgets succeeded */
    assert(ret == mybuf);

    /* Verify that the expected string was read */
    assert(strcmp(mybuf, TEST_STRING) == 0);

    /* Close the file */
    fclose(fdevfile);

    return 0;
}

#else

int
main(void)
{
    printf("fdevopen not supported, test skipped\n");
    return 77;
}
#endif

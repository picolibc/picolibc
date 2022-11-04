/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2019 Keith Packard
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char file_name[] = "posix-io-test-file";
static const char test_string[] = "hello, world\n";

static void test_cleanup(void)
{
	remove(file_name);
}

static int
test_cmp(FILE *f, const char *t)
{
        int c;
	while ((c = getc(f)) != EOF) {
		if ((char) c != *t) {
			printf("read incorrect byte %c != %c\n", c, *t);
			return 1;
		}
		t++;
	}
        return 0;
}
int
main(void)
{
	FILE		*f;

	atexit(test_cleanup);
	f = fopen(file_name, "w+");
	if (!f) {
		printf("failed to open \"%s\" for writing\n", file_name);
		return 1;
	}

	if ((size_t) fprintf(f, "%s", test_string) != strlen(test_string)) {
		printf("failed to fprintf test string %s\n", test_string);
		return 1;
	}

#if defined(TINY_STDIO) || !defined(_HAVE_SEMIHOST)
        if (fseeko(f, -3, SEEK_CUR) < 0) {
                printf("failed to seek back 3 bytes\n");
                return 1;
        }
        if (test_cmp(f, test_string + sizeof(test_string) - 4))
                return 1;

        if (fseek(f, 2, SEEK_SET)) {
                printf("failed to seek to 2 bytes\n");
                return 1;
        }
        if (test_cmp(f, test_string + 2))
                return 1;
#endif

	if (fclose(f) != 0) {
		printf("fclose failed\n");
		return 1;
	}

	f = fopen(file_name, "r");
	if (!f) {
		printf("failed to open \"%s\" for reading\n", file_name);
		return 1;
	}

        if (test_cmp(f, test_string))
                return 1;

	printf("success\n");
        exit(0);
}

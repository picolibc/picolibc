/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2020 Keith Packard
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
#include <errno.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdint.h>

#ifndef __clang__
#pragma GCC diagnostic ignored "-Walloc-size-larger-than="
#endif

int
main(void)
{
	void *r;
	int result = 0;
	int pow;

	errno = 0;
	r = malloc(0);
        printf("malloc(0): %p\n", r);
	if (errno != 0) {
		printf("malloc(0) failed: %s. got %p\n", strerror(errno), r);
		result++;
	}
	free(r);

	r = memalign(128, 237);
        printf("memalign(128, 237): %p\n", r);
	if ((uintptr_t) r & 127) {
		printf("memalign(128, 237) unaligned (%p)\n", r);
		result++;
	}
	free(r);

	errno = 0;
	r = malloc(PTRDIFF_MAX);
        printf("malloc(PTRDIFF_MAX): %p\n", r);
	if (r || errno != ENOMEM) {
                printf("malloc(PTRDIFF_MAX) should have failed. got %p error %s\n", r, strerror(errno));
		result++;
	}

	errno = 0;
	r = malloc(SIZE_MAX);
        printf("malloc(SIZE_MAX): %p\n", r);
	if (r || errno != ENOMEM) {
		printf("malloc(SIZE_MAX) should have failed. got %p error %s\n", r, strerror(errno));
		result++;
	}

	errno = 0;
	r = calloc(1, SIZE_MAX);
        printf("calloc(1, SIZE_MAX): %p\n", r);
	if (r || errno != ENOMEM) {
		printf("calloc(1, SIZE_MAX) should have failed. got %p error %s\n", r, strerror(errno));
		result++;
	}

	errno = 0;
	r = reallocarray(NULL, 1, SIZE_MAX);
        printf("reallocarray(NULL, 1, SIZE_MAX): %p\n", r);
	if (r || errno != ENOMEM) {
		printf("reallocarray(NULL, 1, SIZE_MAX) should have failed. got %p error %s\n", r, strerror(errno));
		result++;
	}

	for (pow = 0; pow < 4; pow++) {
		errno = 0;
		r = calloc(SIZE_MAX >> pow, SIZE_MAX >> pow);
                printf("calloc(SIZE_MAX >> %d, SIZE_MAX >> %d): %p\n", pow, pow, r);
		if (r || errno != ENOMEM) {
			printf("calloc(SIZE_MAX >> %d, SIZE_MAX >> %d) should have failed. got %p error %s\n", pow, pow, r, strerror(errno));
			result++;
		}
		r = reallocarray(NULL, SIZE_MAX >> pow, SIZE_MAX >> pow);
                printf("reallocarray(SIZE_MAX >> %d, SIZE_MAX >> %d): %p\n", pow, pow, r);
		if (r || errno != ENOMEM) {
			printf("reallocarray(NULL, SIZE_MAX >> %d, SIZE_MAX >> %d) should have failed. got %p error %s\n", pow, pow, r, strerror(errno));
			result++;
		}
	}

	/* make sure realloc doesn't read past the source */

	void *big = malloc(1024);
	memset(big, '1', 1024);
        printf("big %p\n", big);
	if (big) {
		void *small = malloc(128);
		memset(small, '2', 128);
                printf("small %p\n", small);
		if (small) {
                        (void) atoi(small);
			free(big);
			char *med = realloc(small, 1024);
			if (med) {
                                printf("med %p\n", med);
#ifdef _NANO_MALLOC
				int i;
				for (i = 128; i < 1024; i++)
					if (med[i] != 0) {
						printf("looks like realloc read past old at %d (saw %d)\n", i, med[i]);
						++result;
					}
#endif
				free(med);
				small = NULL;
			}
			free (small);
		}
	}

	malloc_stats();

	return result;
}

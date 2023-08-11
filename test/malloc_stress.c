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
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#define NUM_MALLOC	128
#define MAX_ALLOC	1024

static uint8_t *blocks[NUM_MALLOC];
static size_t block_size[NUM_MALLOC];
static uint8_t block_data[NUM_MALLOC];

static int order[NUM_MALLOC];

static int
randint(int max)
{
	/* not evenly distributed, but it's good enough */
	return random() % max;
}

static void
shuffle_order(void)
{
	int i;
	for (i = 0; i < NUM_MALLOC; i++)
		order[i] = i;
	for (i = 0; i < NUM_MALLOC - 1; i++) {
		int j = randint(NUM_MALLOC - 1 - i) + i + 1;

		int t = order[i];
		order[i] = order[j];
		order[j] = t;
	}
}

static void
fill_block(int i)
{
	block_data[i] = randint(256);
	if (blocks[i])
		memset(blocks[i], block_data[i], block_size[i]);
}

static void
reset_block(int i)
{
	blocks[i] = NULL;
	block_data[i] = 0;
	block_size[i] = 0;
}

static void
reset_blocks(void)
{
	int i;

	for (i = 0; i < NUM_MALLOC; i++)
		reset_block(i);
}

static int
check_block(char *which, int i)
{
	size_t size = block_size[i];
	uint8_t data = block_data[i];
	uint8_t *block = blocks[i];
	size_t j;

	if (!block)
		return 0;
	for (j = 0; j < size; j++)
		if (block[j] != data) {
			printf("%s: wrong data in block %d at %u\n",
			       which, i, (unsigned) j);
			return 1;
		}
	return 0;
}

static int
check_blocks(char *which)
{
	int i;
	int result = 0;

	for (i = 0; i < NUM_MALLOC; i++)
		result += check_block(which, i);
	return result;
}

static int
check_malloc(size_t in_use)
{
	int result = 0;

	struct mallinfo info = mallinfo();
#ifdef _NANO_MALLOC
	if (info.arena < info.fordblks + in_use) {
		printf("non-free bytes in arena %zu free %zu\n", info.arena, info.fordblks);
		result++;
	}
	if (in_use == 0 && info.ordblks != 1) {
		printf("%zd blocks free\n", info.ordblks);
		result++;
	}
	if (info.uordblks < in_use) {
		printf("expected at least %zu in use (%zu)\n", in_use, info.uordblks);
		result++;
	}
        if (in_use == 0 && info.uordblks != 0) {
                printf("expected all free but %zu still reported in use\n", info.uordblks);
                result++;
        }
#endif
	return result;
}

#ifdef _NANO_MALLOC
extern size_t __malloc_minsize, __malloc_align, __malloc_head;
#endif

int
main(void)
{
	int loops;
	int result = 0;

	for (loops = 0; loops < 10; loops++)
	{
		long i;
		size_t in_use;

		reset_blocks();
		in_use = 0;

		/* Test slowly increasing size of a block using realloc */

		for (i = 0; i < NUM_MALLOC; i++) {
			result += check_block("grow", 0);
                        size_t new_size = block_size[0] + randint(MAX_ALLOC);
                        uint8_t *new_block = realloc(blocks[0], new_size);
                        if (new_block || new_size == 0) {
                            blocks[0] = new_block;
                            block_size[0] = new_size;
                            in_use = block_size[0];
                            fill_block(0);
                        }
		}

		result += check_block("grow done", 0);
		result += check_malloc(in_use);

		free(blocks[0]);
		in_use = 0;

		/* Make sure everything is free */
		result += check_malloc(in_use);

		reset_blocks();

#pragma GCC diagnostic push
#ifndef __clang__
#pragma GCC diagnostic ignored "-Walloc-size-larger-than=PTRDIFF_MAX"
#endif
		/* Test huge malloc sizes */
		for (i = sizeof(size_t) * 8 - 2; i < (long) (sizeof(size_t) * 8); i++) {
			blocks[0] = malloc((size_t) 1 << i);
			if (blocks[0])
				free(blocks[0]);
		}

		in_use = 0;

		/* Make sure everything is free */
		result += check_malloc(in_use);

		reset_blocks();

		/* Test allocating negative amounts */
		for (i = -1; i >= -128; i--) {
			blocks[0] = malloc((size_t) i);
#pragma GCC diagnostic pop
			if (blocks[0]) {
				printf("malloc size %ld succeeded\n", i);
				result++;
				free(blocks[0]);
			}
		}

		in_use = 0;

		/* Make sure everything is free */
		result += check_malloc(in_use);

		reset_blocks();

		/* Test allocating random chunks */
		for (i = 0; i < NUM_MALLOC; i++) {
			size_t size = randint(MAX_ALLOC);
                        uint8_t *new_block = malloc(size);
                        if (new_block || size == 0) {
                            block_size[i] = size;
                            blocks[i] = new_block;
                            in_use += size;
                            fill_block(i);
                        }
		}

		result += check_blocks("malloc");
		result += check_malloc(in_use);

		/* Realloc them to new random sizes in a random order
		 */
		shuffle_order();
		for (i = 0; i < NUM_MALLOC; i++) {
			size_t size = randint(MAX_ALLOC);
			int j = order[i];
                        uint8_t *new_block = realloc(blocks[j], size);
                        if (new_block || size == 0) {
                            blocks[j] = new_block;
                            in_use -= block_size[j];
                            block_size[j] = size;
                            in_use += size;
                            fill_block(j);
                        }
		}

		result += check_blocks("realloc");
		result += check_malloc(in_use);

		shuffle_order();
		for (i = 0; i < NUM_MALLOC; i++) {
			int j = order[i];
			check_block("realloc block", j);
			free(blocks[j]);
			in_use -= block_size[j];
		}

		result += check_malloc(in_use);
		if (in_use != 0) {
			printf("malloc stress accounting error\n");
			result++;
		}

		reset_blocks();

		for (i = 0; i < NUM_MALLOC; i++) {
			size_t size = randint(MAX_ALLOC);
			size_t align = 1 << (3 + randint(7));

                        uint8_t *new_block = memalign(align, size);
                        if (new_block || size == 0) {
                            block_size[i] = size;
                            blocks[i] = new_block;

                            if ((uintptr_t) blocks[i] & (align - 1)) {
                                printf("unaligned block returned %p align %zu size %zu\n",
                                       blocks[i], align, size);
                                result++;
                            }
                            fill_block(i);
                            in_use += size;
                        }
		}
		result += check_blocks("memalign");
		result += check_malloc(in_use);

		shuffle_order();
		for (i = 0; i < NUM_MALLOC; i++) {
			int j = order[i];
			check_block("free", j);
			free(blocks[j]);
			in_use -= block_size[j];
		}

		result += check_malloc(in_use);

		if (in_use != 0) {
			printf("malloc stress accounting error\n");
			result++;
		}
	}

	return result;
}

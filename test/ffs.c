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
#include <strings.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

long long int
rand_long_long(void)
{
	unsigned long long int r = 0;
	size_t x;

	for (x = 0; x < sizeof(r) / 4; x++)
		r = (r << 32) | (lrand48() & 0xffffffffL);
	return r;
}

int
slow_ffs(int i)
{
	int p;
	if (!i)
		return 0;
	for (p = 1; (i & (1 << (p - 1))) == 0; p++)
		;
	return p;
}

int
slow_ffsl(long int i)
{
	int p;
	if (!i)
		return 0;
	for (p = 1; (i & (1L << (p - 1))) == 0; p++)
		;
	return p;
}

int
slow_ffsll(long long int i)
{
	int p;
	if (!i)
		return 0;
	for (p = 1; (i & (1LL << (p - 1))) == 0; p++)
		;
	return p;
}

static int
check(long long int x, int got, int expect, char *which)
{
	long int low = x & 0xffffffffl;
	long int high = ((x >> 16) >> 16) & 0xffffffffl;
	if (got != expect) {
		printf("failed %s(%08lx%08lx): got %d expect %d\n", which, high, low, got, expect);
		return 1;
	}
	return 0;
}

int
main(void)
{
	int ret = 0;

	/* Test zero */
	ret += check(0, ffs(0), slow_ffs(0), "ffs");
	ret += check(0, ffsl(0), slow_ffsl(0), "ffsl");
	ret += check(0, ffsll(0), slow_ffsll(0), "ffsll");

	/* Test every bit position in a long long */
	for (size_t i = 0; i < sizeof(long long) * 8; i++) {
		long long int xll = 1LL << i;
		long xl = (long) xll;
		int x = (int) xll;

		ret += check(x, ffs(x), slow_ffs(x), "ffs");
		ret += check(xl, ffsl(xl), slow_ffsl(xl), "ffsl");
		ret += check(xll, ffsll(xll), slow_ffsll(xll), "ffsll");
	}

	/* Test another few random values */
#define N 100000
	for (int32_t i = 0; i < N; i++) {
		long long int xll = rand_long_long();
		long xl = (long) xll;
		int x = (int) xll;

		ret += check(x, ffs(x), slow_ffs(x), "ffs");
		ret += check(xl, ffsl(xl), slow_ffsl(xl), "ffsl");
		ret += check(xll, ffsll(xll), slow_ffsll(xll), "ffsll");
	}
	return ret;
}

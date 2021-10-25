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

#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>

#ifdef TESTS_ENABLE_STACK_PROTECTOR
static volatile bool expect_smash;

void __attribute__((noinline))
#ifdef _HAVE_CC_INHIBIT_LOOP_TO_LIBCALL
__attribute((__optimize__("-fno-tree-loop-distribute-patterns")))
#endif
    my_strcpy(char *d, char *s)
{
    while ((*d++ = *s++));
}

void __attribute__((noinline)) smash_array(char *source, char *dest)
{
	char	local[16];

	my_strcpy(local, source);
	local[0]++;
	my_strcpy(dest, local);
}

void
__stack_chk_fail (void)
{
	if (expect_smash) {
#ifdef TINY_STDIO
		puts("caught expected stack smash");
#endif
		_exit(0);
	} else {
#ifdef TINY_STDIO
		puts("caught unexpected stack smash");
#endif
		_exit(1);
	}
}

int main(void)
{
	char	source[64];
	char	dest[64];

	memset(source, 'x', 15);
	source[15] = '\0';
	expect_smash = false;
	smash_array(source, dest);
	expect_smash = false;
	printf("short source %s dest %s\n", source, dest);

	memset(source, 'x', 63);
	source[63] = '\0';
	expect_smash = true;
	smash_array(source, dest);
	expect_smash = false;
	printf("missed expected stack smash\n");
	return 1;
}
#else
int main(void)
{
	printf("stack protector disabled for tests\n");
	return 0;
}
#endif

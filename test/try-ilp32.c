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

#include "try-ilp32.h"

int
main(void)
{
	char	copy[128] = {"hello"};
	int	len;
	char	*a;

	asprintf(&a,"string %p", main);

	/*
	 * get_str returns a struct with two values, a char * and an
	 * int.  on ILP32 machines (32-bit ints and pointers, but
	 * 64-bit registers), this will be returned in a single
	 * register. We then want to turn around and pass that
	 * register, unmodified, to a function expecting a pointer.
	 * The compiler will 'optimize' this by leaving the whole
	 * return value in the register, including the 'int' field in
	 * the upper 32 bits. An incorrect strcpy implementation will
	 * not clear those bits before using the register causing
	 * an invalid memory access.
	 */
	strcpy(get_str(copy).base, a);
	len = strlen(copy);
	printf("copy %s len %d\n", copy, len);
	exit(0);
}

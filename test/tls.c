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

#include <stdio.h>
#include <picotls.h>
#include <stdlib.h>
#include <stdbool.h>

#define DATA_VAL	0x600df00d

NEWLIB_THREAD_LOCAL volatile int data_var = DATA_VAL;
NEWLIB_THREAD_LOCAL volatile int bss_var;

volatile int *volatile data_addr;
volatile int *volatile bss_addr;

int
check_tls(char *where, bool check_addr)
{
	int result = 0;

	printf("tls check %s %p %p\n", where, &data_var, &bss_var);
	if (data_var != DATA_VAL) {
		printf("%s: initialized thread var has wrong value (0x%x instead of 0x%x)\n",
		       where, data_var, DATA_VAL);
		result++;
	}

	if (bss_var != 0) {
		printf("%s: uninitialized thread var has wrong value (0x%x instead of 0x%x)\n",
		       where, bss_var, 0);
		result++;
	}

	data_var = ~data_var;

	if (data_var != ~DATA_VAL) {
		printf("%s: initialized thread var set to wrong value (0x%x instead of 0x%x)\n",
		       where, data_var, ~DATA_VAL);
		result++;
	}

	bss_var = ~bss_var;

	if (bss_var != ~0) {
		printf("%s: uninitialized thread var has wrong value (0x%x instead of 0x%x)\n",
		       where, bss_var, ~0);
		result++;
	}

	if (check_addr) {
		if (data_addr == &data_var) {
			printf("_set_tls didn't affect initialized addr %p\n", data_addr);
			result++;
		}

		if (bss_addr == &bss_var) {
			printf("_set_tls didn't affect uninitialized addr %p\n", bss_addr);
			result++;
		}
	}

	return result;
}

int
main(void)
{
	int result;

	data_addr = &data_var;
	bss_addr = &bss_var;

	result = check_tls("pre-defined", false);

#ifdef _HAVE_PICOLIBC_TLS_API

	void *tls = malloc(_tls_size());

	_init_tls(tls);
	_set_tls(tls);

	result += check_tls("allocated", true);
#endif

	printf("tls test result %d\n", result);
	return result;
}

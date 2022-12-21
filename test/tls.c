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
#include <stdint.h>
#include <string.h>

#define DATA_VAL 0x600df00d
#define DATA_VAL2 0x0a0a0a0a

NEWLIB_THREAD_LOCAL volatile int data_var = DATA_VAL;
_Alignas(128) NEWLIB_THREAD_LOCAL volatile int overaligned_data_var = DATA_VAL2;
NEWLIB_THREAD_LOCAL volatile int bss_var;
_Alignas(256) NEWLIB_THREAD_LOCAL volatile int overaligned_bss_var;

volatile int *volatile data_addr;
volatile int *volatile bss_addr;
volatile int *volatile overaligned_data_addr;
volatile int *volatile overaligned_bss_addr;

#ifdef PICOLIBC_TLS
extern char __tdata_start, __tdata_end;
extern char __data_start, __data_source;
extern char __tls_base;

static bool
inside_tls_region(void *ptr, const void *tls)
{
	return (uintptr_t)ptr >= (uintptr_t)tls &&
	       (uintptr_t)ptr < (uintptr_t)tls + _tls_size();
}

#define check_inside_tls_region(ptr, tls_start)                                \
	if (!inside_tls_region(__DEVOLATILE(void *, ptr), tls_start)) {        \
		printf("%s (%p) is not inside TLS region [%p-%p)\n", #ptr,     \
		       ptr, tls_start, (char *)tls_start + _tls_size());       \
		result++;                                                      \
	}
#endif

int
check_tls(char *where, bool check_addr, void *tls_region)
{
	int result = 0;

	printf("tls check %s %p %p %p %p\n", where, &data_var,
	       &overaligned_data_var, &bss_var, &overaligned_bss_var);
	if (!__is_aligned(tls_region, 128)) {
		printf("TLS data region (%p) is not aligned\n", tls_region);
		result++;
	}
	if (!__is_aligned((uintptr_t)&overaligned_data_var, 128)) {
		printf("overaligned_data_var (%p) is not aligned\n",
		       &overaligned_data_var);
		result++;
	}
	if (!__is_aligned((uintptr_t)&overaligned_bss_var, 256)) {
		printf("overaligned_bss_var (%p) is not aligned\n",
		       &overaligned_bss_var);
		result++;
	}
	if (data_var != DATA_VAL) {
		printf("%s: initialized thread var has wrong value (0x%x instead of 0x%x)\n",
		       where, data_var, DATA_VAL);
		result++;
	}
	if (overaligned_data_var != DATA_VAL2) {
		printf("%s: initialized overaligned thread var has wrong value (0x%x instead of 0x%x)\n",
		       where, overaligned_data_var, DATA_VAL2);
		result++;
	}

	if (bss_var != 0) {
		printf("%s: uninitialized thread var has wrong value (0x%x instead of 0x%x)\n",
		       where, bss_var, 0);
		result++;
	}
	if (overaligned_bss_var != 0) {
		printf("%s: uninitialized thread var has wrong value (0x%x instead of 0x%x)\n",
		       where, overaligned_bss_var, 0);
		result++;
	}

	data_var = ~data_var;

	if (data_var != ~DATA_VAL) {
		printf("%s: initialized thread var set to wrong value (0x%x instead of 0x%x)\n",
		       where, data_var, ~DATA_VAL);
		result++;
	}

	overaligned_data_var = ~overaligned_data_var;

	if (overaligned_data_var != ~DATA_VAL2) {
		printf("%s: initialized thread var set to wrong value (0x%x instead of 0x%x)\n",
		       where, overaligned_data_var, ~DATA_VAL2);
		result++;
	}

	bss_var = ~bss_var;

	if (bss_var != ~0) {
		printf("%s: uninitialized thread var has wrong value (0x%x instead of 0x%x)\n",
		       where, bss_var, ~0);
		result++;
	}

	overaligned_bss_var = ~overaligned_bss_var;

	if (overaligned_bss_var != ~0) {
		printf("%s: uninitialized thread var has wrong value (0x%x instead of 0x%x)\n",
		       where, overaligned_bss_var, ~0);
		result++;
	}

	if (check_addr) {
		if (data_addr == &data_var) {
			printf("_set_tls didn't affect initialized addr %p\n", data_addr);
			result++;
		}

                if (overaligned_data_addr == &overaligned_data_var) {
			printf("_set_tls didn't affect initialized addr %p\n", overaligned_data_addr);
			result++;
		}

		if (bss_addr == &bss_var) {
			printf("_set_tls didn't affect uninitialized addr %p\n", bss_addr);
			result++;
		}

		if (overaligned_bss_addr == &overaligned_bss_var) {
			printf("_set_tls didn't affect uninitialized addr %p\n", overaligned_bss_addr);
			result++;
		}
	}
#ifdef PICOLIBC_TLS
	check_inside_tls_region(&data_var, tls_region);
	check_inside_tls_region(&overaligned_data_var, tls_region);
	check_inside_tls_region(&bss_var, tls_region);
	check_inside_tls_region(&overaligned_bss_var, tls_region);
#endif
	return result;
}

void
hexdump(const void *ptr, int length, const char *hdr)
{
	const unsigned char *cp = ptr;

	for (int i = 0; i < length; i += 16) {
		printf("%s%08zx  ", hdr, i + (size_t)ptr);
		for (int j = 0; j < 16; j++) {
			int offset = i + j;
			if (offset < length)
				printf(" %02x", cp[offset]);
			else
				printf("   ");
		}
		printf("\n");
	}
}

int
main(void)
{
	int result = 0;

	data_addr = &data_var;
	overaligned_data_addr = &overaligned_data_var;
	bss_addr = &bss_var;
	overaligned_bss_addr = &overaligned_bss_var;

#ifdef PICOLIBC_TLS
	size_t tdata_size = &__tdata_end - &__tdata_start;
	printf("TLS template region: %p-%p (%zd bytes)\n", &__tdata_start,
	       &__tdata_start + tdata_size, tdata_size);

	result += check_tls("pre-defined", false, &__tls_base);
#else
	result += check_tls("pre-defined", false, NULL);
#endif


#ifdef _HAVE_PICOLIBC_TLS_API

	void *tls = aligned_alloc(_tls_align(), _tls_size());

	_init_tls(tls);
	_set_tls(tls);

	if (memcmp(tls, &__tdata_start, tdata_size) != 0) {
		printf("New TLS data in RAM does not match ROM\n");
		hexdump(&__tdata_start, tdata_size, "ROM:");
		hexdump(tls, tdata_size, "RAM:");
		result++;
	}

	result += check_tls("allocated", true, tls);
#endif

	printf("tls test result %d\n", result);
	return result;
}

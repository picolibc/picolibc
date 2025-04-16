/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2025 TK Chia
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

#ifndef _MACHINE__TLS_H_
#define _MACHINE__TLS_H_

#include <stdint.h>
#include <sys/param.h>

#if !defined(__i386__) && !defined(__x86_64)
#error
#endif

typedef struct
{
	/* Self-pointer per ABI (https://akkadia.org/drepper/tls.pdf). */
	void *__tcb;
#ifdef __THREAD_LOCAL_STORAGE_STACK_GUARD
	/*
	 * If we are configured to use a TLS stack protection canary, then
	 * GCC expects the canary to be located at the same offset into the
	 * TCB as implemented in GNU libc's Native POSIX Thread Library
	 * (NPTL) component.  So declare some additional NPTL-compatible TCB
	 * fields here.
	 */
	void *__dtv;
	void *__self;
	int32_t __multiple_threads;
#ifdef __x86_64
	int32_t __gscope_flag;
#endif
	uintptr_t __sysinfo;
	uintptr_t __stack_guard;
#endif
} __tcb_head_t;

#endif

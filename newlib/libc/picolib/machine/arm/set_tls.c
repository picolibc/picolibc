/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2019 Keith Packard
 * Copyright © 2024 Stephen Street
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

#include <picotls.h>
#include <string.h>
#include <stdint.h>
#include "arm_tls.h"

/* This needs to be global so that __aeabi_read_tp can
 * refer to it in an asm statement
 */
#ifndef ARM_TLS_CP15
extern void *__tls[];
#endif

/* The size of the thread control block.
 * TLS relocations are generated relative to
 * a location this far *before* the first thread
 * variable (!)
 * NB: The actual size before tp also includes padding
 * to align up to the alignment of .tdata/.tbss.
 */
#define TCB_SIZE	8
extern char __arm32_tls_tcb_offset;
#define TP_OFFSET ((size_t)&__arm32_tls_tcb_offset)

void
_set_tls(void *tls)
{
        tls = (uint8_t *) tls - TP_OFFSET;
#ifdef ARM_TLS_CP15
	__asm__("mcr p15, 0, %0, cr13, cr0, 3" : : "r" (tls));
#else
#ifdef ARM_RP2040
        uint32_t cpuid = *(uint32_t *)0xd0000000;
        __tls[cpuid] = tls;
#else
	__tls[0] = tls;
#endif
#endif
}

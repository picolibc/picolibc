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

#include <picotls.h>
#include <string.h>
#include <stdint.h>

/*
 * The TLS block consists of initialized data immediately followed by
 * zero filled data
 *
 * These addresses must be defined by the loader configuration file
 */

extern char __tdata_source[];	/* Source of TLS initialization data (in ROM) */

extern char __tdata_start[];    /* Start of static tdata area */
extern char __tdata_end[];      /* End of static tdata area */
extern char __tbss_start[];     /* Start of static zero-initialized TLS data */
extern char __tbss_end[];       /* End of static zero-initialized TLS data */

#ifdef __PICOLIBC_CRT_RUNTIME_SIZE
#define __tdata_size (__tdata_end - __tdata_start)
#define __tbss_size (__tbss_end - __tbss_start)
#define __tbss_offset (__tbss_start - __tdata_start)
#else
extern char __tdata_size[];	/* Size of TLS initized data */
extern char __tbss_size[];	/* Size of TLS zero-filled data */
extern char __tbss_offset[];    /* Offset from tdata to tbss */
#endif

void
_init_tls(void *__tls)
{
	char *tls = __tls;

	/* Copy tls initialized data */
	memcpy(tls, __tdata_source, (uintptr_t) __tdata_size);

	/* Clear tls zero data */
	memset(tls + (uintptr_t) __tbss_offset, '\0', (uintptr_t) __tbss_size);
}

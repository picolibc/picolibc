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

#include <string.h>
#include <picotls.h>

extern char __data_source__[];
extern char __data_start__[];
extern char __data_end__[];
extern char __bss_start__[];
extern char __bss_end__[];

/* These two functions must be defined in the architecture-specific
 * code
 */

void
_set_tls(void  *tls);

int
_start(void);

/* This is the application entry point */
int
main(void);

#ifdef HAVE_INITFINI_ARRAY
extern void __libc_init_array(void);
extern void __libc_fini_array(void);
#endif

/* After the architecture-specific chip initialization is done, this
 * function initializes the data and bss segments. Note that a static
 * block of TLS data is carefully interleaved with the regular data
 * and bss segments in picolibc.ld so that this one operation
 * initializes both. Then it runs the application code, starting with
 * any initialization functions, followed by the main application
 * entry point and finally any cleanup functions
 */

static inline void
__start(void)
{
	memcpy(__data_start__, __data_source__,
	       __data_end__ - __data_start__);
	memset(__bss_start__, '\0',
	       __bss_end__ - __bss_start__);
	_set_tls(__tls_base__);
#ifdef HAVE_INITFINI_ARRAY
	__libc_init_array();
#endif
	main();
#ifdef HAVE_INITFINI_ARRAY
	__libc_fini_array();
#endif
}

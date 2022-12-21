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
#include <stdint.h>
#include <stdlib.h>

extern char __data_source[];
extern char __data_start[];
extern char __data_end[];
extern char __data_size[];
extern char __bss_start[];
extern char __bss_end[];
extern char __bss_size[];
#ifdef PICOLIBC_TLS
extern char __tls_base[];
#endif

#ifdef __PICOLIBC_CRT_RUNTIME_SIZE
#define __data_size (__data_end - __data_start)
#define __bss_size (__bss_end - __bss_start)
#endif

/* These two functions must be defined in the architecture-specific
 * code
 */

void
_start(void);

/* This is the application entry point */
int
main(int, char **);

#ifdef _HAVE_INITFINI_ARRAY
extern void __libc_init_array(void);
#endif

/*
 * After the architecture-specific chip initialization is done, this function
 * initializes the data and bss segments. If the system uses thread local
 * storage, the TLS area is initialized and the thread pointer register is set
 * up to point to this area. Then it runs the application code, starting with
 * any initialization functions, followed by the main application entry point
 * and finally any cleanup functions.
 */

#include <picotls.h>
#include <stdio.h>
#ifdef CRT0_SEMIHOST
#include <semihost.h>
#endif

#ifndef CONSTRUCTORS
#define CONSTRUCTORS 1
#endif

static inline void
__start(void)
{
	memcpy(__data_start, __data_source, (uintptr_t) __data_size);
	memset(__bss_start, '\0', (uintptr_t) __bss_size);
#ifdef PICOLIBC_TLS
	/* Initialize thread local variables for the initial thread */
	_init_tls(__tls_base);
	/* Set the TLS pointer for the initial thread */
	_set_tls(__tls_base);
#endif
#if defined(_HAVE_INITFINI_ARRAY) && CONSTRUCTORS
	__libc_init_array();
#endif

#ifdef CRT0_SEMIHOST
#define CMDLINE_LEN     1024
#define ARGV_LEN        64
        static char cmdline[CMDLINE_LEN];
        static char *argv[ARGV_LEN];
        int argc = 0;

        argv[argc++] = "program-name";
        if (sys_semihost_get_cmdline(cmdline, sizeof(cmdline)) == 0)
        {
            char *c = cmdline;

            while (*c && argc < ARGV_LEN - 1) {
                argv[argc++] = c;
                while (*c && *c != ' ')
                    c++;
                if (!*c)
                    break;
                *c = '\0';
                while (*++c == ' ')
                    ;
            }
        }
        argv[argc] = NULL;
#else
#define argv NULL
#define argc 0
#endif

	int ret = main(argc, argv);
#ifdef CRT0_EXIT
	exit(ret);
#else
	(void) ret;
	for(;;);
#endif
}

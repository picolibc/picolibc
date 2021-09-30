/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2021 Keith Packard
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
#include <string.h>
#include <sys/lock.h>
#include "pico-onexit.h"

struct on_exit {
    union on_exit_func  func;
    void                *arg;
    int                 kind;
};

static struct on_exit on_exits[ATEXIT_MAX];

int
_on_exit(int kind, union on_exit_func func, void *arg)
{
	int	ret = -1;
	int	o;
	__LIBC_LOCK();
	for (o = 0; o < ATEXIT_MAX; o++) {
		if (on_exits[o].kind == PICO_ONEXIT_EMPTY) {
			on_exits[o].func = func;
			on_exits[o].arg = arg;
                        on_exits[o].kind = kind;
			ret = 0;
			break;
		}
	}
	__LIBC_UNLOCK();
	return ret;
}

int
on_exit (void (*func)(int, void *),void *arg)
{
    union on_exit_func func_u = { .on_exit = func };
    return _on_exit(PICO_ONEXIT_ONEXIT, func_u, arg);
}

void
__call_exitprocs(int code, void *param)
{
        (void) param;
	for (;;) {
		int	                i;
                union on_exit_func      func = {0};
                int                     kind = PICO_ONEXIT_EMPTY;
		void	                *arg = 0;

		__LIBC_LOCK();
		for (i = ATEXIT_MAX - 1; i >= 0; i--) {
                        kind = on_exits[i].kind;
			if (kind != PICO_ONEXIT_EMPTY) {
                                func = on_exits[i].func;
                                arg = on_exits[i].arg;
                                memset(&on_exits[i], '\0', sizeof(struct on_exit));
				break;
			}
		}
		__LIBC_UNLOCK();
                switch (kind) {
                case PICO_ONEXIT_EMPTY:
                        return;
                case PICO_ONEXIT_ONEXIT:
                        func.on_exit(code, arg);
                        break;
                case PICO_ONEXIT_ATEXIT:
                        func.atexit();
                        break;
                case PICO_ONEXIT_CXA_ATEXIT:
                        func.cxa_atexit(arg);
                        break;
                }
	}
}

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
#include <sys/lock.h>

struct _on_exit {
	void	(*func)(int, void *);
	void	*arg;
};

static struct _on_exit on_exits[ATEXIT_MAX];

int
on_exit (void (*func)(int, void *),void *arg)
{
	int	ret = -1;
	int	o;
	__LIBC_LOCK();
	for (o = 0; o < ATEXIT_MAX; o++) {
		if (!on_exits[o].func) {
			on_exits[o].func = func;
			on_exits[o].arg = arg;
			ret = 0;
			break;
		}
	}
	__LIBC_UNLOCK();
	return ret;
}

void
__call_exitprocs(int code, void *param)
{
	for (;;) {
		int	i;
		void	(*func)(int, void *) = NULL;
		void	*arg;

		__LIBC_LOCK();
		for (i = ATEXIT_MAX - 1; i >= 0; i--) {
			if ((func = on_exits[i].func) != NULL) {
				arg = on_exits[i].arg;
				on_exits[i].func = NULL;
				on_exits[i].arg = NULL;
				break;
			}
		}
		__LIBC_UNLOCK();
		if (func == NULL)
			break;
		func(code, arg);
	}
}

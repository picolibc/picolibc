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

#include "semihost-private.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

extern struct timeval __semihost_creat_time _ATTRIBUTE((__weak__));
extern int gettimeofday(struct timeval *restrict tv, void *restrict tz) _ATTRIBUTE((__weak__));

/*
 * note: binary mode has been chosen below because otherwise
 *       files are treated on host side as text files which
 *       is most probably not intented.  This means that
 *       data transfer is transparent between target and host.
 */

int
open(const char *pathname, int flags, ...)
{
	int semiflags = 0;

	switch (flags & (O_RDONLY|O_WRONLY|O_RDWR)) {
	case O_RDONLY:
		semiflags = SH_OPEN_R_B;		/* 'rb' */
		break;
	case O_WRONLY:
		if (flags & O_TRUNC)
			semiflags = SH_OPEN_W_B;	/* 'wb' */
		else
			semiflags = SH_OPEN_A_B;	/* 'ab' */
		break;
	default:
		if (flags & O_TRUNC)
			semiflags = SH_OPEN_W_PLUS_B;	/* 'wb+' */
		else
			semiflags = SH_OPEN_A_PLUS_B;	/* 'ab+' */
		break;
	}

	int ret;
	do {
		ret = sys_semihost_open(pathname, semiflags);
	}
#ifdef TINY_STDIO
	while(0);
#else
	while (0 <= ret && ret <= 2);
#endif
	if (ret == -1)
		errno = sys_semihost_errno();
        else if (&__semihost_creat_time && gettimeofday)
                gettimeofday(&__semihost_creat_time, NULL);

	return ret;
}

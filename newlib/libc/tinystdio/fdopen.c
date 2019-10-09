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

#include "stdio_private.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

static inline int
has_mode(int flags, int mode)
{
	return (flags & mode) ? 1 : 0;
}

FILE *
fdopen(int fd, const char *mode)
{
	int stdio_flags;
	int open_flags;
	struct __file_posix *pf;
	char *buf;

	stdio_flags = __posix_sflags(mode, &open_flags);
	if (stdio_flags == 0)
		return NULL;

	/* Allocate file structure and necessary buffers */
	pf = calloc(1, sizeof(struct __file_posix) +
		    BUFSIZ * (has_mode(stdio_flags, __SRD) + has_mode(stdio_flags, __SWR)));
	if (pf == NULL) {
		close(fd);
		return NULL;
	}
	buf = (char *) pf + sizeof (*pf);

	pf->cfile.file.flags = stdio_flags | __SCLOSE;
	pf->cfile.close = __posix_close;
	pf->fd = fd;
	if (stdio_flags & __SWR) {
		pf->cfile.file.put = __posix_putc;
		pf->cfile.file.flush = __posix_flush;
		pf->write_buf = buf;
		buf += BUFSIZ;
	}
	if (stdio_flags & __SRD) {
		pf->cfile.file.get = __posix_getc;
		pf->read_buf = buf;
	}

	if (open_flags & O_APPEND)
		(void) lseek(fd, 0, SEEK_END);

	return &(pf->cfile.file);
}

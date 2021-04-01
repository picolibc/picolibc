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
#include <unistd.h>

/* Posix I/O routines for tiny stdio */

int
__posix_flush(FILE *f)
{
	struct __file_posix *pf = (struct __file_posix *) f;

	__posix_lock(f);
	if (pf->write_len) {
		int off = 0;

		/* Flush everything, drop contents if that doesn't work */
		while (pf->write_len) {
			int this = write(pf->fd, pf->write_buf + off, pf->write_len);
			if (this <= 0) {
				pf->write_len = 0;
				__posix_unlock(f);
				return -1;
			}
			off += this;
			pf->write_len -= this;
		}
	}
	__posix_unlock(f);
	return 0;
}

int
__posix_putc(char c, FILE *f)
{
	struct __file_posix *pf = (struct __file_posix *) f;
	bool need_flush;

	__posix_lock(f);
	pf->write_buf[pf->write_len++] = c;

	/* flush if full, or if sending newline to stdout/stderr */
	need_flush = (pf->write_len >= BUFSIZ || (c == '\n' && pf->fd <= 2));
	__posix_unlock(f);

	if (need_flush)
		return __posix_flush(f);

	return 0;
}

int
__posix_getc(FILE *f)
{
	struct __file_posix *pf = (struct __file_posix *) f;
	unsigned char c;

	__posix_lock(f);
	if (pf->read_off >= pf->read_len) {

		/* Flush stdout if reading from stdin */
		if (f == stdin) {
			__posix_unlock(f);
			fflush(stdout);
			return __posix_getc(f);
		}

		/* Reset read pointer, read some data */
		pf->read_off = 0;
		pf->read_len = read(pf->fd, pf->read_buf, BUFSIZ);

		if (pf->read_len <= 0) {
			pf->read_len = 0;
			__posix_unlock(f);
			return EOF;
		}
	}

	/*
	 * Cast to unsigned avoids sign-extending chars with high-bit
	 * set
	 */
	c = (unsigned char) pf->read_buf[pf->read_off++];
	__posix_unlock(f);
	return c;
}

int
__posix_close(FILE *f)
{
	struct __file_posix *pf = (struct __file_posix *) f;
	int ret = 0;

	if (f->flush)
		ret = f->flush(f);

	__posix_lock(f);
	__posix_lock_close(f);
	/* Don't close stdin/stdout/stderr fds */
	if (pf->fd > 2)
		close(pf->fd);
	free(f);
	return ret;
}

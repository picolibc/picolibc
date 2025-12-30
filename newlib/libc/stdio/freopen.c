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

FILE *
freopen(const char *pathname, const char *mode, FILE *stream)
{
	FILE *ret = NULL;
	struct __file_bufio *pf = (struct __file_bufio *) stream;
	char *old_buf_to_free = NULL;
	char *buf;
	int old_fd;
	int fd;
	int stdio_flags;
	int open_flags;
	int old_buf_size;
	int buf_size;
	uint8_t new_bflags;

	__flockfile(stream);
	/* Can't reopen FILEs which aren't buffered */
	if (!(stream->flags & __SBUF))
		goto exit;

	stdio_flags = __stdio_flags(mode, &open_flags);
	if (stdio_flags == 0)
		goto exit;

	fd = open(pathname, open_flags, 0666);
	if (fd < 0)
		goto exit;

        fflush(stream);

        old_fd = (int)(intptr_t) (pf->ptr);
        old_buf_size = __bufio_get_buf_size(old_fd);
        buf_size = __bufio_get_buf_size(fd);

        __bufio_lock(stream);

        /*
         * freopen() should operate as fclose() + fopen():
         *  - Any custom buffer set via setvbuf() must be released.
         *  - The I/O buffer must be reallocated to match the new
         *    file's optimal block size (st_blksize).
         */
        new_bflags = pf->bflags;
        if ((new_bflags & __BFALL) && (buf_size <= old_buf_size)) {
            /* Preallocated buffer fits, reuse it */
            if (new_bflags & __BALL)
                old_buf_to_free = pf->buf;
            buf = (char *) (pf + 1);
        } else {
            buf = realloc(new_bflags & __BALL ? pf->buf : NULL, buf_size);
        }

        /* Check if buffer allocation failed */
        if (buf == NULL) {
            close(fd);
            goto exit_bufio_lock;
        }

        /* Set proper __BALL flag for the new buffer */
        if (buf == (char *)(pf + 1))
            new_bflags &= ~__BALL;
        else
            new_bflags |= __BALL;

        if (old_buf_to_free)
            free(old_buf_to_free);

        (void) __atomic_exchange_ungetc(&stream->unget, 0);
        close(old_fd);

        *pf = (struct __file_bufio) FDEV_SETUP_POSIX(fd, buf, buf_size, stdio_flags, new_bflags);

        ret = stream;

exit_bufio_lock:
	__bufio_unlock(stream);
exit:
	__funlock_return(stream, ret);
}

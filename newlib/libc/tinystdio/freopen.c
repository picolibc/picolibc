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

FILE *
freopen(const char *pathname, const char *mode, FILE *stream)
{
	struct __file_bufio *pf = (struct __file_bufio *) stream;
	int fd;
	int stdio_flags;
	int open_flags;

        /* Can't reopen FILEs which aren't buffered */
        if (!(stream->flags & __SBUF))
            return NULL;

	stdio_flags = __posix_sflags(mode, &open_flags);
	if (stdio_flags == 0)
		return NULL;

	fd = open(pathname, open_flags, 0666);
	if (fd < 0)
		return NULL;

        fflush(stream);

        __bufio_lock(stream);
        close(pf->fd);
        stream->flags = (stream->flags & ~(__SRD|__SWR|__SERR|__SEOF)) | stdio_flags;
        pf->pos = 0;
        pf->fd = fd;

        /* Switch to POSIX backend */
        pf->read = read;
        pf->write = write;
        pf->lseek = lseek;
        pf->close = close;

        __bufio_unlock(stream);

        return stream;
}

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

#include <stdio-bufio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

/* Buffered I/O routines for tiny stdio */

static int
__bufio_flush_locked(FILE *f)
{
	struct __file_bufio *bf = (struct __file_bufio *) f;
        char *buf;
        int ret = 0;
        off_t backup;

        switch (bf->dir) {
        case __SWR:
		/* Flush everything, drop contents if that doesn't work */
                buf = bf->buf;
		while (bf->len) {
                        ssize_t this = (bf->write) (bf->fd, buf, bf->len);
			if (this <= 0) {
                                bf->len = 0;
                                ret = -1;
                                break;
			}
			bf->pos += this;
			bf->len -= this;
		}
                break;
        case __SRD:
                /* Move the FD back to the current read position */
                backup = bf->len - bf->off;
                if (backup) {
                        bf->pos -= backup;
                        if (bf->lseek)
                                (void) (bf->lseek)(bf->fd, bf->pos, SEEK_SET);
                }
                bf->len = 0;
                bf->off = 0;
                break;
        default:
                break;
	}
	return ret;
}

int
__bufio_flush(FILE *f)
{
        int ret;

	__bufio_lock(f);
        ret = __bufio_flush_locked(f);
	__bufio_unlock(f);
	return ret;
}

/* Set I/O direction, flushing when it changes */
static int
__bufio_setdir_locked(FILE *f, uint8_t dir)
{
	struct __file_bufio *bf = (struct __file_bufio *) f;
        int ret = 0;

        if (bf->dir != dir) {
                ret = __bufio_flush_locked(f);
                bf->dir = dir;
        }
        return ret;
}

int
__bufio_put(char c, FILE *f)
{
	struct __file_bufio *bf = (struct __file_bufio *) f;
        int ret = (unsigned char) c;

	__bufio_lock(f);
        if (__bufio_setdir_locked(f, __SWR) < 0) {
                ret = _FDEV_ERR;
                goto bail;
        }

	bf->buf[bf->len++] = c;

	/* flush if full, or if sending newline when linebuffered */
	if (bf->len >= bf->size || (c == '\n' && (bf->bflags & __BLBF)))
		if (__bufio_flush_locked(f) < 0)
                        ret = _FDEV_ERR;

bail:
	__bufio_unlock(f);
	return ret;
}

int
__bufio_get(FILE *f)
{
	struct __file_bufio *bf = (struct __file_bufio *) f;
        int ret;
        bool flushed = false;

again:
	__bufio_lock(f);
        if (__bufio_setdir_locked(f, __SRD) < 0) {
                ret = _FDEV_ERR;
                goto bail;
        }

	if (bf->off >= bf->len) {

		/* Flush stdout if reading from stdin */
		if (f == stdin && !flushed) {
                        flushed = true;
			__bufio_unlock(f);
			fflush(stdout);
                        goto again;
		}

		/* Reset read pointer, read some data */
		bf->off = 0;
		bf->len = (bf->read)(bf->fd, bf->buf, bf->size);

		if (bf->len <= 0) {
			bf->len = 0;
                        ret = _FDEV_EOF;
                        goto bail;
		}

                /* Update FD pos */
                bf->pos += bf->len;
	}

	/*
	 * Cast to unsigned avoids sign-extending chars with high-bit
	 * set
	 */
	ret = (unsigned char) bf->buf[bf->off++];
bail:
	__bufio_unlock(f);
	return ret;
}

off_t
__bufio_seek(FILE *f, off_t offset, int whence)
{
	struct __file_bufio *bf = (struct __file_bufio *) f;
	off_t ret;

	__bufio_lock(f);
        if (__bufio_setdir_locked(f, 0) < 0)
                return _FDEV_ERR;
        if (bf->lseek) {
                if (whence == SEEK_CUR) {
                        whence = SEEK_SET;
                        offset += bf->pos;
                }
                ret = (bf->lseek)(bf->fd, offset, whence);
        } else
                ret = _FDEV_ERR;
        if (ret >= 0)
                bf->pos = ret;
        __bufio_unlock(f);
        return ret;
}

int
__bufio_setvbuf(FILE *f, char *buf, int mode, size_t size)
{
	struct __file_bufio *bf = (struct __file_bufio *) f;
        int ret = 0;

	__bufio_lock(f);
        bf->bflags &= ~__BLBF;
        switch (mode) {
        case _IONBF:
                buf = NULL;
                size = 1;
                break;
        case _IOLBF:
                bf->bflags |= __BLBF;
                break;
        case _IOFBF:
                break;
        default:
                ret = -1;
                goto bail;
        }
        if (bf->bflags & __BALL) {
                /*
                 * Handling allocation failures here is a bit tricky;
                 * we don't want to lose the existing buffer. Instead,
                 * we try to reallocate it
                 */
                if (!buf) {
                        buf = realloc(bf->buf, size);
                        if (!buf)
                                ret = -1;
                        else
                                bf->size = size;
                        goto bail;
                }
                free(bf->buf);
                bf->bflags &= ~__BALL;
        }
        if (!buf) {
                buf = malloc(size);
                if (!buf) {
                        ret = -1;
                        goto bail;
                }
                bf->bflags |= __BALL;
        }
        bf->buf = buf;
        bf->size = size;
bail:
        __bufio_unlock(f);
        return ret;
}

int
__bufio_close(FILE *f)
{
	struct __file_bufio *bf = (struct __file_bufio *) f;
	int ret = 0;

	__bufio_lock(f);
        ret = __bufio_flush_locked(f);

        if (bf->bflags & __BALL)
                free(bf->buf);

	__bufio_lock_close(f);
	/* Don't close stdin/stdout/stderr fds */
	if (bf->fd > 2)
		(bf->close)(bf->fd);
	free(f);
	return ret;
}


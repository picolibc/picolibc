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

/* Buffered I/O routines for tiny stdio */

int
__bufio_flush_locked(FILE *f)
{
	struct __file_bufio *bf = (struct __file_bufio *) f;
        char *buf;
        off_t backup;

        switch (bf->dir) {
        case __SWR:
		/* Flush everything, drop contents if that doesn't work */
                buf = bf->buf;
		while (bf->len) {
                        ssize_t this = (bf->write) (bf->fd, buf, bf->len);
			if (this <= 0) {
                                bf->len = 0;
                                return _FDEV_ERR;
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
	return 0;
}


int __bufio_fill_locked(FILE *f)
{
	struct __file_bufio *bf = (struct __file_bufio *) f;
        ssize_t len;

        /* Reset read pointer, read some data */
        bf->off = 0;
        len = (bf->read)(bf->fd, bf->buf, bf->size);

        if (len <= 0) {
                bf->len = 0;
                if (len < 0)
                        return _FDEV_ERR;
                else
                        return _FDEV_EOF;
        }

        /* Update FD pos */
        bf->len = len;
        bf->pos += len;
        return 0;
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
int
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

extern FILE *const stdin _ATTRIBUTE((__weak__));
extern FILE *const stdout _ATTRIBUTE((__weak__));

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

		/*
                 * Flush stdout if reading from stdin.
                 *
                 * The odd-looking NULL address checks along with the
                 * weak attributes for stdin and stdout above avoids
                 * pulling in stdin/stdout definitions just for this
                 * check.
                 */
		if (&stdin != NULL && &stdout != NULL && f == stdin && !flushed) {
                        flushed = true;
			__bufio_unlock(f);
			fflush(stdout);
                        goto again;
		}

                ret = __bufio_fill_locked(f);
                if (ret)
                    goto bail;
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

        if (!bf->lseek)
            return _FDEV_ERR;

	__bufio_lock(f);
        if (__bufio_setdir_locked(f, __SRD) < 0) {
                ret = _FDEV_ERR;
        } else {
                /* compute offset of the first char in the buffer */
                __off_t buf_pos = bf->pos - bf->len;

                switch (whence) {
                case SEEK_CUR:
                        /* Map CUR -> SET, accounting for position within buffer */
                        whence = SEEK_SET;
                        offset += buf_pos + bf->off;
                        __PICOLIBC_FALLTHROUGH;
                case SEEK_SET:
                        /* Optimize for seeks within buffer or just past buffer */
                        if (buf_pos <= offset && offset <= buf_pos + bf->len) {
                                bf->off = offset - buf_pos;
                                ret = offset;
                                break;
                        }
                        __PICOLIBC_FALLTHROUGH;
                default:
                        ret = (bf->lseek)(bf->fd, offset, whence);
                        if (ret >= 0)
                                bf->pos = ret;
                        /* Flush any buffered data after a real seek */
                        bf->len = 0;
                        bf->off = 0;
                        break;
                }
        }
        __bufio_unlock(f);
        return ret;
}

int
__bufio_setvbuf(FILE *f, char *buf, int mode, size_t size)
{
	struct __file_bufio *bf = (struct __file_bufio *) f;
        int ret = -1;

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
                goto bail;
        }
        if (bf->bflags & __BALL) {
                if (buf) {
                        free(bf->buf);
                        bf->bflags &= ~__BALL;
                } else {
                        /*
                         * Handling allocation failures here is a bit tricky;
                         * we don't want to lose the existing buffer. Instead,
                         * we try to reallocate it
                         */
                        buf = realloc(bf->buf, size);
                        if (!buf)
                                goto bail;
                }
        } else if (!buf) {
                buf = malloc(size);
                if (!buf)
                        goto bail;
                bf->bflags |= __BALL;
        }
        bf->buf = buf;
        bf->size = size;
        ret = 0;
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

        /*
         * Don't close the fd or free the FILE for things not
         * generated by fopen or fdopen. These will usually be static
         * FILE structs defined for stdin/stdout/stderr.
         */
        if (bf->bflags & __BFALL) {
                (bf->close)(bf->fd);
                free(f);
        }
	return ret;
}


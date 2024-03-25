/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2022 Keith Packard
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

#ifndef _STDIO_BUFIO_H_
#define _STDIO_BUFIO_H_

#include <stdio.h>
#include <sys/types.h>
#include <sys/lock.h>

#define __BALL  0x0001          /* bufio buf is allocated by stdio */
#define __BLBF  0x0002          /* bufio is line buffered */
#define __BFALL 0x0004          /* FILE is allocated by stdio */

struct __file_bufio {
        struct __file_ext xfile;
	int	fd;
        uint8_t dir;
        uint8_t bflags;
        __off_t pos;    /* FD position */
	char	*buf;
        int     size;   /* sizeof buf */
	int	len;    /* valid data in buf */
	int	off;    /* offset of data in buf */
        ssize_t (*read)(int fd, void *buf, size_t count);
        ssize_t (*write)(int fd, const void *buf, size_t count);
        __off_t (*lseek)(int fd, __off_t offset, int whence);
        int     (*close)(int fd);
#ifndef __SINGLE_THREAD__
	_LOCK_T lock;
#endif
};

#define FDEV_SETUP_BUFIO(_fd, _buf, _size, _read, _write, _lseek, _close, _rwflag, _bflags) \
        {                                                               \
                .xfile = FDEV_SETUP_EXT(__bufio_put, __bufio_get,       \
                                        __bufio_flush, __bufio_close,   \
                                        __bufio_seek, __bufio_setvbuf,  \
                                        (_rwflag) | __SBUF),            \
                .fd = _fd,                                              \
                .dir = 0,                                               \
                .bflags = (_bflags),                                    \
                .pos = 0,                                               \
                .buf = _buf,                                            \
                .size = _size,                                          \
                .len = 0,                                               \
                .off = 0,                                               \
                .read = _read,                                          \
                .write = _write,                                        \
                .lseek = _lseek,                                        \
                .close = _close,                                        \
        }

static inline void __bufio_lock_init(FILE *f) {
	(void) f;
	__lock_init(((struct __file_bufio *) f)->lock);
}

static inline void __bufio_lock_close(FILE *f) {
	(void) f;
        __lock_release(((struct __file_bufio *) f)->lock);
	__lock_close(((struct __file_bufio *) f)->lock);
}

static inline void __bufio_lock(FILE *f) {
	(void) f;
	__lock_acquire(((struct __file_bufio *) f)->lock);
}

static inline void __bufio_unlock(FILE *f) {
	(void) f;
	__lock_release(((struct __file_bufio *) f)->lock);
}

int
__bufio_flush_locked(FILE *f);

int
__bufio_fill_locked(FILE *f);

int
__bufio_setdir_locked(FILE *f, uint8_t dir);

int
__bufio_flush(FILE *f);

int
__bufio_put(char c, FILE *f);

int
__bufio_get(FILE *f);

off_t
__bufio_seek(FILE *f, off_t offset, int whence);

int
__bufio_setvbuf(FILE *f, char *buf, int mode, size_t size);

int
__bufio_close(FILE *f);

#endif /* _STDIO_BUFIO_H_ */

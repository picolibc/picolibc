/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2024 Keith Packard
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

#define _DEFAULT_SOURCE
#include <stdlib.h>
#include <stdio-bufio.h>
#include "stdio_private.h"

struct __file_funopen {
    struct __file_ext   xfile;
    void                *cookie;
    ssize_t (*read)(void *cookie, void *buf, size_t n);
    ssize_t (*write)(void *cookie, const void *buf, size_t n);
    __off_t (*seek)(void *cookie, __off_t off, int whence);
    int (*close)(void *cookie);
};

static int
__funopen_put(char c, FILE *f)
{
    struct __file_funopen *ff = (struct __file_funopen *) f;
    ssize_t ret;

    ret = (ff->write)(ff->cookie, &c, 1);
    switch (ret) {
    case 1:
        return (unsigned char) c;
    default:
        return _FDEV_ERR;
    }
}

static int
__funopen_get(FILE *f)
{
    struct __file_funopen *ff = (struct __file_funopen *) f;
    char c;
    ssize_t ret;

    ret = (ff->read)(ff->cookie, &c, 1);

    switch (ret) {
    case 1:
        return c;
    case 0:
        return _FDEV_EOF;
    default:
        return _FDEV_ERR;
    }
}

static int
__funopen_close(FILE *f)
{
    struct __file_funopen *ff = (struct __file_funopen *) f;
    int ret = 0;

    if (ff->close)
        ret = (ff->close)(ff->cookie);
    free(f);
    return ret;
}

static off_t
__funopen_seek(FILE *f, off_t offset, int whence)
{
    struct __file_funopen *ff = (struct __file_funopen *) f;

    if (ff->seek)
        return (ff->seek)(ff->cookie, offset, whence);
    return (off_t) -1;
}

#define FDEV_SETUP_FUNOPEN(_cookie, _read, _write, _seek, _close, _rwflag) \
        {                                                               \
                .xfile = FDEV_SETUP_EXT(__funopen_put, __funopen_get,   \
                                        NULL, __funopen_close,          \
                                        __funopen_seek, NULL,           \
                                        (_rwflag)),                     \
                .cookie = (void *) _cookie,                             \
                .read = _read,                                          \
                .write = _write,                                        \
                .seek = _seek,                                        \
                .close = _close,                                        \
        }


FILE *
funopen (const void *cookie,
         ssize_t (*readfn)(void *cookie, void *buf, size_t n),
         ssize_t (*writefn)(void *cookie, const void *buf, size_t n),
         __off_t (*seekfn)(void *cookie, __off_t off, int whence),
         int (*closefn)(void *cookie))
{
	struct __file_funopen *ff;
        int open_flags = 0;

        if (readfn)
            open_flags |= __SRD;
        if (writefn)
            open_flags |= __SWR;

	/* Allocate file structure and necessary buffers */
	ff = calloc(1, sizeof(struct __file_funopen));

	if (ff == NULL)
            return NULL;

        *ff = (struct __file_funopen)
            FDEV_SETUP_FUNOPEN(cookie, readfn, writefn, seekfn, closefn, open_flags);

	return (FILE *) ff;
}

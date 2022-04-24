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

#include "stdio_private.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define __MALL          0x01

struct __file_mem {
        struct __file_ext xfile;
        char    *buf;
        size_t  size;
        size_t  pos;
        uint8_t mflags;
};

static int __fmem_put(char c, FILE *f)
{
        struct __file_mem *mf = (struct __file_mem *) f;
        if ((f->flags & __SWR) && mf->pos < mf->size) {
                mf->buf[mf->pos++] = c;
                return (unsigned char) c;
        }
        return _FDEV_ERR;
}

static int __fmem_get(FILE *f)
{
        struct __file_mem *mf = (struct __file_mem *) f;
        int c;
        if ((f->flags & __SRD) && mf->pos < mf->size) {
                c = (unsigned char) mf->buf[mf->pos++];
                if (c == '\0')
                        c = _FDEV_EOF;
        } else
                c = _FDEV_ERR;
        return c;
}

static int __fmem_flush(FILE *f)
{
        struct __file_mem *mf = (struct __file_mem *) f;
        if ((f->flags & __SWR) && mf->pos < mf->size)
                mf->buf[mf->pos] = '\0';
        return 0;
}

static off_t __fmem_seek(FILE *f, off_t pos, int whence)
{
        struct __file_mem *mf = (struct __file_mem *) f;

        switch (whence) {
        case SEEK_SET:
                break;
        case SEEK_CUR:
                pos += mf->pos;
                break;
        case SEEK_END:
                pos += mf->size;
                break;
        }
        if (pos < 0 || mf->size < (size_t) pos)
                return EOF;
        mf->pos = pos;
        return pos;
}

static int __fmem_close(FILE *f)
{
        struct __file_mem *mf = (struct __file_mem *) f;

        if (mf->mflags & __MALL)
                free (mf->buf);
        else
                __fmem_flush(f);
        free(f);
        return 0;
}

FILE *
fmemopen(void *buf, size_t size, const char *mode)
{
	int stdio_flags;
        uint8_t mflags = 0;
	int open_flags;
	struct __file_mem *mf;

	stdio_flags = __posix_sflags(mode, &open_flags);
	if (stdio_flags == 0)
		return NULL;

	/* Allocate file structure and necessary buffers */
	mf = calloc(1, sizeof(struct __file_mem));

	if (mf == NULL)
		return NULL;

        if (buf == NULL) {
                buf = malloc(size);
                if (!buf) {
                        free(mf);
                        return NULL;
                }
                mflags |= __MALL;
        }

        *mf = (struct __file_mem) {
                .xfile = FDEV_SETUP_EXT(__fmem_put, __fmem_get, __fmem_flush, __fmem_close,
                                        __fmem_seek, NULL, stdio_flags),
                .buf = buf,
                .size = size,
                .pos = 0,
                .mflags = mflags,
        };

	return &(mf->xfile.cfile.file);
}

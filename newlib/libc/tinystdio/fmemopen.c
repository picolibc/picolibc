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

#define _DEFAULT_SOURCE /* for strnlen */
#include "stdio_private.h"
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define __MALL 0x01

struct __file_mem {
    struct __file_ext xfile;
    char *buf;
    size_t size; /* Current size. */
    size_t bufsize; /* Upper limit on size. */
    size_t pos;
    uint8_t mflags;
};

static int
__fmem_put(char c, FILE *f)
{
    struct __file_mem *mf = (struct __file_mem *)f;
    if ((f->flags & __SWR) == 0) {
        return _FDEV_ERR;
    } else if (mf->pos < mf->bufsize) {
        mf->buf[mf->pos++] = c;
        if (mf->pos > mf->size) {
            mf->size = mf->pos;
        }
        return (unsigned char)c;
    } else {
        return _FDEV_EOF;
    }
}

static int
__fmem_get(FILE *f)
{
    struct __file_mem *mf = (struct __file_mem *)f;
    if ((f->flags & __SRD) == 0) {
        return _FDEV_ERR;
    } else if (mf->pos < mf->size) {
        return (unsigned char)mf->buf[mf->pos++];
    } else {
        return _FDEV_EOF;
    }
}

static int
__fmem_flush(FILE *f)
{
    struct __file_mem *mf = (struct __file_mem *)f;
    if ((f->flags & __SWR) && mf->pos < mf->bufsize) {
        mf->buf[mf->pos] = '\0';
        if (mf->pos > mf->size) {
            mf->size = mf->pos;
        }
    }
    return 0;
}

static off_t
__fmem_seek(FILE *f, off_t pos, int whence)
{
    struct __file_mem *mf = (struct __file_mem *)f;

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
    if (pos < 0 || mf->size < (size_t)pos)
        return EOF;
    mf->pos = pos;
    return pos;
}

static int
__fmem_close(FILE *f)
{
    struct __file_mem *mf = (struct __file_mem *)f;

    if (mf->mflags & __MALL)
        free(mf->buf);
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
    size_t initial_pos = 0;
    size_t initial_size;
    struct __file_mem *mf;

    stdio_flags = __stdio_sflags(mode);

    if (stdio_flags == 0 || size == 0) {
        errno = EINVAL;
        return NULL;
    }

    /* Allocate file structure and necessary buffers */
    mf = calloc(1, sizeof(struct __file_mem));

    if (mf == NULL)
        return NULL;

    if (buf == NULL) {
        /* POSIX says return EINVAL if: The buf argument is a null pointer and
         * the mode argument does not include a '+' character. */
        if ((stdio_flags & (__SRD | __SWR)) != (__SRD | __SWR)) {
            free(mf);
            errno = EINVAL;
            return NULL;
        }
        buf = malloc(size);
        if (!buf) {
            free(mf);
            errno = ENOMEM;
            return NULL;
        }
        *((char *)buf) = '\0';
        mflags |= __MALL;
    }

    /* Check mode directly to avoid _POSIX_IO dependency. */
    if (mode[0] == 'a') {
        /* For append the position is set to the first NUL byte or the end. */
        initial_pos = (mflags & __MALL) ? 0 : strnlen(buf, size);
        initial_size = initial_pos;
    } else if (mode[0] == 'w') {
        initial_size = 0;
        /* w+ mode truncates the buffer, writing NUL */
        if ((stdio_flags & (__SRD | __SWR)) == (__SRD | __SWR)) {
            *((char *)buf) = '\0';
        }
    } else {
        initial_size = size;
    }

    *mf = (struct __file_mem){
        .xfile = FDEV_SETUP_EXT(__fmem_put, __fmem_get, __fmem_flush,
                                __fmem_close, __fmem_seek, NULL, stdio_flags),
        .buf = buf,
        .size = initial_size,
        .bufsize = size,
        .pos = initial_pos,
        .mflags = mflags,
    };

    return (FILE *)mf;
}

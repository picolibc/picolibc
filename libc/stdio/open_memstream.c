/*
 * SPDX-FileCopyrightText: 2024-2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "local-stdio.h"

#define GET_BUF(mf)  (*(mf->pbuf))
#define GET_SIZE(mf) (*(mf->psize))

struct __file_open_mem {
    struct __file_ext xfile;
    char            **pbuf;
    size_t           *psize;
    size_t            bsize; /* allocated buffer size */
    size_t            fsize; /* file content size */
    size_t            pos;   /* current position in the buffer */
};

static int
__open_mem_grow(struct __file_open_mem *mf, size_t additional_size)
{
    size_t bsize = mf->bsize;
    char  *tmp;

    if (additional_size > SIZE_MAX - mf->pos) {
        errno = ENOMEM;
        return _FDEV_ERR;
    }

    size_t required_size = mf->pos + additional_size;
    if (required_size < bsize)
        return 0;

    /* Avoid geometric growth, increase the buffer size by 1.5x */
    bsize += bsize / 2;

    /* seek can move position beyond calculated size */
    if (bsize < required_size)
        bsize = required_size;

    /* +1 for the null terminator */
    bsize += 1;

    /* check for overflow */
    if (bsize < mf->bsize) {
        errno = ENOMEM;
        return _FDEV_ERR;
    }

    tmp = realloc(GET_BUF(mf), bsize);
    if (!tmp)
        return _FDEV_ERR;

    /* fill the new buffer with zeros */
    memset(tmp + mf->fsize, 0, bsize - mf->fsize);

    /* update structure */
    GET_BUF(mf) = tmp;
    mf->bsize = bsize;

    return 0;
}

static int
__open_mem_put(char c, FILE *f)
{
    struct __file_open_mem *mf = (struct __file_open_mem *)f;

    /* 2 = 1 for the new character + 1 for the null terminator */
    int                     ret = __open_mem_grow(mf, 2);
    if (ret != 0)
        return ret;

    GET_BUF(mf)[mf->pos++] = c;

    if (mf->pos > mf->fsize) {
        mf->fsize = mf->pos;
        GET_BUF(mf)[mf->pos] = '\0';
    }
    return (unsigned char)c;
}

static int
__open_mem_flush(FILE *f)
{
    struct __file_open_mem *mf = (struct __file_open_mem *)f;

    GET_SIZE(mf) = mf->fsize; /* update the size of the file content */
    return 0;
}

static off_t
__open_mem_seek(FILE *f, off_t pos, int whence)
{
    struct __file_open_mem *mf = (struct __file_open_mem *)f;

    switch (whence) {
    case SEEK_SET:
        break;
    case SEEK_CUR:
        pos += mf->pos;
        break;
    case SEEK_END:
        pos += mf->fsize;
        break;
    default:
        errno = EINVAL;
        return EOF;
    }

    if (pos < 0) {
        errno = EINVAL;
        return EOF;
    }

    _Static_assert(sizeof(off_t) >= sizeof(size_t), "must avoid truncation");
    size_t offset = pos;
    if (mf->fsize < offset) {
        if (__open_mem_grow(mf, offset - mf->pos) != 0)
            return EOF;

        mf->fsize = offset;
    }
    mf->pos = offset;
    return pos;
}

static int
__open_mem_close(FILE *f)
{
    __open_mem_flush(f);
    free(f);
    return 0;
}

FILE *
open_memstream(char **buf, size_t *size)
{
    struct __file_open_mem *mf;

    if (!buf || !size) {
        errno = EINVAL;
        return NULL;
    }

    mf = calloc(1, sizeof(struct __file_open_mem));
    if (mf == NULL) {
        return NULL;
    }

    *buf = calloc(1, BUFSIZ);
    if (*buf == NULL) {
        free(mf);
        return NULL;
    }

    *mf = (struct __file_open_mem) {
        .xfile = FDEV_SETUP_EXT(__open_mem_put, NULL, __open_mem_flush, __open_mem_close,
                                __open_mem_seek, NULL, __SWR),
        .pbuf = buf,
        .psize = size,
        .bsize = BUFSIZ,
        .fsize = 0,
        .pos = 0,
    };

    return (FILE *)mf;
}

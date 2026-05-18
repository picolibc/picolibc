/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Alexey Lapshin
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

#include "local-stdio.h"

struct fopencookie_info {
    void                 *cookie;
    cookie_io_functions_t io;
};

static ssize_t
fopencookie_read(void *ptr, void *buf, size_t count)
{
    struct fopencookie_info *info = ptr;

    if (!info->io.read)
        return -1;
    return info->io.read(info->cookie, buf, count);
}

static ssize_t
fopencookie_write(void *ptr, const void *buf, size_t count)
{
    struct fopencookie_info *info = ptr;
    ssize_t                  ret;

    if (!info->io.write)
        return -1;

    ret = info->io.write(info->cookie, buf, count);
    if (ret == 0)
        return -1;
    return ret;
}

static __off_t
fopencookie_seek(void *ptr, __off_t offset, int whence)
{
    struct fopencookie_info *info = ptr;

    if (!info->io.seek)
        return _FDEV_ERR;
    if (info->io.seek(info->cookie, &offset, whence) == -1)
        return _FDEV_ERR;
    if (offset == (__off_t)-1)
        return _FDEV_ERR;
    return offset;
}

static int
fopencookie_close(void *ptr)
{
    struct fopencookie_info *info = ptr;

    if (!info->io.close)
        return 0;
    return info->io.close(info->cookie);
}

FILE *
fopencookie(void *cookie, const char *mode, cookie_io_functions_t io_funcs)
{
    struct __file_bufio     *bf;
    struct fopencookie_info *info;
    char                    *buf;
    int                      open_flags;
    int                      o;

    open_flags = __stdio_flags(mode, &o);
    if (!open_flags)
        return NULL;

    bf = calloc(1, sizeof(struct __file_bufio) + sizeof(struct fopencookie_info) + BUFSIZ);
    if (bf == NULL)
        return NULL;

    info = (struct fopencookie_info *)(bf + 1);
    buf = (char *)(info + 1);

    info->cookie = cookie;
    info->io = io_funcs;

    *bf = (struct __file_bufio)FDEV_SETUP_BUFIO_PTR(info, buf, BUFSIZ, fopencookie_read,
                                                    fopencookie_write, fopencookie_seek,
                                                    fopencookie_close, open_flags, __BFALL);

    return (FILE *)bf;
}

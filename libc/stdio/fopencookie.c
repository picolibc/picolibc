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

struct __file_cookie {
    struct __file_bufio   bfile;
    void                 *cookie;
    cookie_io_functions_t io;
    char                  buf[BUFSIZ];
};

static ssize_t
fopencookie_read(void *ptr, void *buf, size_t count)
{
    struct __file_cookie *pf = ptr;

    if (!pf->io.read)
        return 0;

    return pf->io.read(pf->cookie, buf, count);
}

static ssize_t
fopencookie_write(void *ptr, const void *buf, size_t count)
{
    struct __file_cookie *pf = ptr;
    ssize_t               ret;

    if (!pf->io.write)
        return -1;

    ret = pf->io.write(pf->cookie, buf, count);
    if (ret == 0)
        return -1;

    return ret;
}

static __off_t
fopencookie_seek(void *ptr, __off_t offset, int whence)
{
    struct __file_cookie *pf = ptr;

    if (!pf->io.seek)
        return (__off_t)-1;

    if (pf->io.seek(pf->cookie, &offset, whence) == -1)
        return (__off_t)-1;

    return offset;
}

static int
fopencookie_close(void *ptr)
{
    struct __file_cookie *pf = ptr;

    if (!pf->io.close)
        return 0;

    return pf->io.close(pf->cookie);
}

FILE *
fopencookie(void *cookie, const char *mode, cookie_io_functions_t io_funcs)
{
    struct __file_cookie *pf;
    int                   open_flags;
    int                   o;

    open_flags = __stdio_flags(mode, &o);
    if (!open_flags)
        return NULL;

    pf = calloc(1, sizeof(struct __file_cookie));
    if (pf == NULL)
        return NULL;

    pf->cookie = cookie;
    pf->io = io_funcs;

    pf->bfile = (struct __file_bufio)FDEV_SETUP_BUFIO_PTR(
        pf, pf->buf, BUFSIZ, fopencookie_read, fopencookie_write,
        io_funcs.seek ? fopencookie_seek : NULL, fopencookie_close, open_flags, __BFALL);

    return &pf->bfile.xfile.cfile.file;
}

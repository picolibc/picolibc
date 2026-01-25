/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 Keith Packard
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

int
__bufio_setvbuf(FILE *f, char *buf, int mode, size_t size)
{
    struct __file_bufio *bf = (struct __file_bufio *)f;
    int                  ret = -1;

    __bufio_lock(f);
    if (__bufio_flush_locked(f) < 0) {
        ret = _FDEV_ERR;
        goto bail;
    }
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
    if (bf->bflags & __BALL)
        bf->xfile.cfile.close = __bufio_close;
    bf->buf = buf;
    bf->size = size;
    ret = 0;
bail:
    __bufio_unlock(f);
    return ret;
}

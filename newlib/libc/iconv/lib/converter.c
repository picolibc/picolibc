/*-
 * Copyright (c) 1999,2000
 *    Konstantin Chuguev.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *    iconv (Charset Conversion Library) v2.0
 */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <reent.h>
#include <limits.h>
#include "local.h"

static iconv_converter_t *
_DEFUN(converter_init, (rptr, conv_func, close_func, extra),
                        struct _reent *rptr      _AND
                        iconv_conv_t conv_func   _AND
                        iconv_close_t close_func _AND
                        size_t extra)
{
    iconv_converter_t *res = _malloc_r(rptr, sizeof(iconv_converter_t) + extra);
        memset(res, 0, sizeof(iconv_converter_t) + extra);
    if (res) {
        res->convert = conv_func;
        res->close = close_func;
    }
    return res;
}

static int
_DEFUN(unicode_close, (rptr, data),
                      struct _reent *rptr _AND
                      _VOID_PTR data)
{
    int res;
    unicode_converter_t *uc = (unicode_converter_t *)data;

    res = ICONV_CES_CLOSE(rptr, &(uc->from));
    res = ICONV_CES_CLOSE(rptr, &(uc->to)) || res;
    return res;
}

static size_t
_DEFUN(unicode_conv, (rptr, data, inbuf, inbytesleft, outbuf, outbytesleft),
                     struct _reent *rptr          _AND
                     _VOID_PTR data               _AND
                     _CONST unsigned char **inbuf _AND
                     size_t *inbytesleft          _AND
                     unsigned char **outbuf       _AND
                     size_t *outbytesleft)
{
        size_t res = 0;
        unicode_converter_t *uc = (unicode_converter_t *)data;

        if (inbuf == NULL || *inbuf == NULL) {
            if (ICONV_CES_CONVERT_FROM_UCS(&(uc->to), UCS_CHAR_NONE,
                                           outbuf, outbytesleft) <= 0) {
            __errno_r(rptr) = E2BIG;
            return (size_t)(-1);
        }
        ICONV_CES_RESET(&(uc->from));
        ICONV_CES_RESET(&(uc->to));
        return res;
    }
    if (inbytesleft == NULL || *inbytesleft == 0)
        return 0;
    while (*inbytesleft > 0) {
        ssize_t size;
        _CONST unsigned char *ptr = *inbuf;
        ucs_t ch = ICONV_CES_CONVERT_TO_UCS(&(uc->from), inbuf,
                                            inbytesleft);

        if (ch == UCS_CHAR_NONE) {
            /* Incomplete character in input buffer */
            __errno_r(rptr) = EINVAL;
            return (size_t)(-1);
        }
        if (ch == UCS_CHAR_INVALID) {
            /* Invalid character in source buffer */
            *inbytesleft += *inbuf - ptr;
            *inbuf = ptr;
            __errno_r(rptr) = EILSEQ;
            return (size_t)(-1);
        }
        size = ICONV_CES_CONVERT_FROM_UCS(&(uc->to), ch,
                                          outbuf, outbytesleft);

        if (size < 0) {
            /* No equivalent in destination charset. */
                        
            size = ICONV_CES_CONVERT_FROM_UCS(&(uc->to),
                                               uc->missing,
                                               outbuf, outbytesleft);
            if (size)
                res ++;
        }
        if (!size) {
            /* Not enough space in output buffer */
            *inbytesleft += *inbuf - ptr;
            *inbuf = ptr; 
            __errno_r(rptr) = E2BIG;
            return (size_t)(-1);
        }
    }
    return res;
}

iconv_converter_t *
_DEFUN(_iconv_unicode_conv_init, (rptr, to, from),
                                 struct _reent *rptr _AND
                                 _CONST char *to     _AND
                                 _CONST char *from)
{
    unicode_converter_t *uc;
    iconv_converter_t *ic = converter_init(rptr, unicode_conv, unicode_close,
                                         sizeof(unicode_converter_t));

    if (ic == NULL)
        return NULL;
    uc = (unicode_converter_t *)(ic + 1);
    if (!_iconv_ces_init(rptr, &(uc->from), from)) {
        if(!_iconv_ces_init(rptr, &(uc->to), to))
        {
            uc->missing = '_';
            return ic;
        }
        ICONV_CES_CLOSE(rptr, &(uc->from));
    }
    _free_r(rptr, ic);
    return NULL;
}

static int
_DEFUN(null_close, (rptr, data),
                   struct _reent *rptr _AND
                   _VOID_PTR data)
{
    return 0;
}

static size_t
_DEFUN(null_conv, (rptr, data, inbuf, inbytesleft, outbuf, outbytesleft),
                  struct _reent *rptr          _AND
                  _VOID_PTR data               _AND
                  _CONST unsigned char **inbuf _AND
                  size_t *inbytesleft          _AND
                  unsigned char **outbuf       _AND
                  size_t *outbytesleft)
{
    if (inbuf && *inbuf && inbytesleft && *inbytesleft > 0 && outbuf
                                         && *outbuf && outbytesleft) {
        size_t result, len;
        if (*inbytesleft < *outbytesleft) {
            result = 0;
            len = *inbytesleft;
        } else {
            result = (size_t)(-1);
            __errno_r(rptr) = E2BIG;
            len = *outbytesleft;
        }
        bcopy(*inbuf, *outbuf, len);
        *inbuf += len;
        *inbytesleft -= len;
        *outbuf += len;
        *outbytesleft -= len;

        return result;
    }

    return 0;
}

iconv_converter_t *
_DEFUN(_iconv_null_conv_init, (rptr, to, from),
                              struct _reent *rptr _AND
                              _CONST char *to     _AND
                              _CONST char *from)
{
    return converter_init(rptr, null_conv, null_close, 0);
}


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
#include <sys/types.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <reent.h>
#include "local.h"

static int
_DEFUN(table_init, (rptr, data, name, desc_data),
                   struct _reent *rptr _AND
                   _VOID_PTR *data     _AND
                   _CONST char *name   _AND
                   _CONST _VOID_PTR desc_data)
{
    int res;
    struct iconv_ccs *ccs = _malloc_r(rptr, sizeof(struct iconv_ccs));
    if (ccs == NULL)
        return __errno_r(rptr);
    res = _iconv_ccs_init(rptr, ccs, name);
    if (res)
        _free_r(rptr, ccs);
    else
        (struct iconv_ccs *)(*data) = ccs;
    return res;
}

static int
_DEFUN(table_close, (rptr, data),
                    struct _reent *rptr _AND
                    _VOID_PTR data)
{
    int res = 0;

    if (data != NULL)
        res = ((struct iconv_ccs *)data)->close(rptr, (struct iconv_ccs *)data);
    _free_r(rptr, data);
    return res;
}

static ssize_t
_DEFUN(convert_from_ucs, (ces, in, outbuf, outbytesleft),
                         struct iconv_ces *ces  _AND
                         ucs_t in               _AND
                         unsigned char **outbuf _AND
                         size_t *outbytesleft)
{
    ucs_t res;
    size_t bytes;

    if (in == UCS_CHAR_NONE)
        return 1;    /* No state reinitialization for table charsets */
    if (iconv_char32bit(in))
        return -1;
    res = ICONV_CCS_CONVERT_FROM_UCS((struct iconv_ccs *)(ces->data), in);
    if (res == UCS_CHAR_INVALID)
        return -1;    /* No character in output charset */
    bytes = res & 0xFF00 ? 2 : 1;
    if (*outbytesleft < bytes)
        return 0;    /* No space in output buffer */
    if (bytes == 2)
        *(*outbuf)++ = (res >> 8) & 0xFF;
    *(*outbuf)++ = res & 0xFF;
    *outbytesleft -= bytes;
    return 1;
}

static ucs_t
_DEFUN(convert_to_ucs, (ces, inbuf, inbytesleft),
                       struct iconv_ces *ces        _AND
                       _CONST unsigned char **inbuf _AND
                       size_t *inbytesleft)
{
    struct iconv_ccs *ccsd = (struct iconv_ccs *)(ces->data);
    unsigned char byte = *(*inbuf);
    ucs_t res = ICONV_CCS_CONVERT_TO_UCS(ccsd, byte);
    size_t bytes = (res == UCS_CHAR_INVALID && ccsd->nbits > 8) ? 2 : 1;

    if (*inbytesleft < bytes)
        return UCS_CHAR_NONE;    /* Not enough bytes in the input buffer */
    if (bytes == 2)
        res = ICONV_CCS_CONVERT_TO_UCS(ccsd, (byte << 8) | (* ++(*inbuf)));
    (*inbuf) ++;
    *inbytesleft -= bytes;
    return res;
}

_CONST struct iconv_ces_desc _iconv_ces_table_driven = {
        table_init,
        table_close,
        _iconv_ces_reset_null,
        convert_from_ucs,
        convert_to_ucs,
        NULL
};


/*
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
#include "deps.h"
#include <sys/types.h>
#include <stddef.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <wchar.h>
#include <reent.h>
#include "local.h"

typedef struct {
    int nccs;
    struct iconv_ccs ccs[1];
} iconv_ces_euc_state_t;

int
_DEFUN(_iconv_euc_init, (rptr, data, desc_data, num),
                        struct _reent *rptr        _AND
                        _VOID_PTR *data            _AND
                        _CONST _VOID_PTR desc_data _AND
                        size_t num)
{
    size_t stsz = sizeof(iconv_ces_euc_state_t) +
                  sizeof(struct iconv_ccs) * (num - 1);
    int i;
    iconv_ces_euc_state_t *state = (iconv_ces_euc_state_t *)_malloc_r(rptr, stsz);

    if (state == NULL)
        return __errno_r(rptr);
    for (i = 0; i < num; i++) {
        int res = _iconv_ccs_init(rptr, &(state->ccs[i]),
                           ((_CONST iconv_ces_euc_ccs_t *) desc_data)[i].name);
        if (res) {
            while (--i >= 0)
                state->ccs[i].close(rptr, &(state->ccs[i]));
            _free_r(rptr, state);
            return res;
        }
    }
    state->nccs = num;
    (iconv_ces_euc_state_t *)*data = state;
    return 0;
}

int
_DEFUN(_iconv_euc_close, (rptr, data),
                        struct _reent *rptr _AND
                        _VOID_PTR data)
{
#define state ((iconv_ces_euc_state_t *)data)
    int i, res = 0;

    for (i = 0; i < state->nccs; i++)
        res = state->ccs[i].close(rptr, &(state->ccs[i])) || res;
    _free_r(rptr, data);
    return res;
#undef state
}

#define is_7_14bit(data) ((data)->nbits & 7)
#define is_7bit(data) ((data)->nbits & 1)

ssize_t
_DEFUN(_iconv_euc_convert_from_ucs, (ces, in, outbuf, outbytesleft),
                                    struct iconv_ces *ces  _AND
                                    ucs_t in               _AND
                                    unsigned char **outbuf _AND
                                    size_t *outbytesleft)
{
    iconv_ces_euc_state_t *euc_state;
    size_t bytes;
    int i;

    if (in == UCS_CHAR_NONE)
        return 1;    /* No state reinitialization for table charsets */
    if (iconv_char32bit(in))
        return -1;
    euc_state = (iconv_ces_euc_state_t *)(ces->data);
    for (i = 0; i < euc_state->nccs; i++) {
        _CONST iconv_ces_euc_ccs_t *ccsattr;
        _CONST struct iconv_ccs *ccs = &(euc_state->ccs[i]);
        ucs_t res = ICONV_CCS_CONVERT_FROM_UCS(ccs, in);

        if (res == UCS_CHAR_INVALID)
            continue;
        ccsattr = &(((_CONST iconv_ces_euc_ccs_t *)(ces->desc->data))[i]);
        if (i) {
            if (is_7_14bit(ccs))
                res |= is_7bit(ccs) ? 0x80 : 0x8080;
            else if (!(res & 0x8080))
                continue;
        } else if (res & 0x8080)
            continue;
        bytes = (res & 0xFF00 ? 2 : 1) + ccsattr->prefixlen;
        if (*outbytesleft < bytes)
            return 0;    /* No space in the output buffer */
        if (ccsattr->prefixlen) {
            memcpy(*outbuf, ccsattr->prefix, ccsattr->prefixlen);
            (*outbuf) += ccsattr->prefixlen;
        }
        if (res & 0xFF00)
            *(*outbuf)++ = (unsigned char)(res >> 8);
        *(*outbuf)++ = (unsigned char)res;
        *outbytesleft -= bytes;
        return 1;
    }
    return -1;    /* No character in output charset */
}

static ucs_t
_DEFUN(cvt2ucs, (ccs, inbuf, inbytesleft, hi_plane, bufptr),
                struct iconv_ccs *ccs       _AND
                _CONST unsigned char *inbuf _AND
                size_t inbytesleft          _AND
                int hi_plane                _AND
                _CONST unsigned char **bufptr)
{
    size_t bytes = ccs->nbits > 8 ? 2 : 1;
    ucs_t ch = *(_CONST unsigned char *)inbuf++;

    if (inbytesleft < bytes)
        return UCS_CHAR_NONE;    /* Not enough bytes in the input buffer */
    if (bytes == 2)
        ch = (ch << 8) | *(_CONST unsigned char *)inbuf++;
    *bufptr = inbuf;
    if (hi_plane) {
        if (!(ch & 0x8080))
            return UCS_CHAR_INVALID;
        if (is_7_14bit(ccs))
            ch &= 0x7F7F;
    } else if (ch & 0x8080)
        return UCS_CHAR_INVALID;
    return ICONV_CCS_CONVERT_TO_UCS(ccs, ch);
}

ucs_t
_DEFUN(_iconv_euc_convert_to_ucs, (ces, inbuf, inbytesleft),
                                  struct iconv_ces *ces        _AND
                                  _CONST unsigned char **inbuf _AND
                                  size_t *inbytesleft)
{
    iconv_ces_euc_state_t *euc_state =
        (iconv_ces_euc_state_t *)(ces->data);
    ucs_t res = UCS_CHAR_INVALID;
    _CONST unsigned char *ptr;
    int i;

    if (**inbuf & 0x80) {
        for (i = 1; i < euc_state->nccs; i++) {
            _CONST iconv_ces_euc_ccs_t *ccsattr =
                 &(((_CONST iconv_ces_euc_ccs_t *)
                       (ces->desc->data))[i]);
            if (ccsattr->prefixlen + 1 > *inbytesleft)
                return UCS_CHAR_NONE;
            if (ccsattr->prefixlen &&
                memcmp(*inbuf, ccsattr->prefix, ccsattr->prefixlen))
                continue;
            res = cvt2ucs(&(euc_state->ccs[i]),
                          *inbuf + ccsattr->prefixlen,
                          *inbytesleft - ccsattr->prefixlen,
                          1, &ptr);
            if (res != UCS_CHAR_INVALID)
                break;
        }
        if (res == UCS_CHAR_INVALID)
            ptr = *inbuf + 1;
    } else
        res = cvt2ucs(euc_state->ccs, *inbuf, *inbytesleft, 0, &ptr);
    if (res == UCS_CHAR_NONE)
        return res;    /* Not enough bytes in the input buffer */
    *inbytesleft -= ptr - *inbuf;
    *inbuf = ptr;
    return res;
}


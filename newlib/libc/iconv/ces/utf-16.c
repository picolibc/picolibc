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
#include "../lib/deps.h"

#ifdef _ICONV_CONVERTER_UTF_16
#include <string.h>
#include "../lib/local.h"

static ssize_t
_DEFUN(convert_from_ucs, (ces, in, outbuf, outbytesleft),
                         struct iconv_ces *ces  _AND
                         ucs_t in               _AND
                         unsigned char **outbuf _AND
                         size_t *outbytesleft)
{
    unsigned char *cp;
    int *state;
    int bytes;

    if (in == UCS_CHAR_NONE)
        return 1;    /* No state reinitialization for table charsets */
    if (in > 0x10FFFF)
        return -1;
    bytes = *(state = (int *)(ces->data)) ? 2 : 4;
    if (in > 0xFFFF)
        bytes += 2;
    if (*outbytesleft < bytes)
        return 0;    /* No space in the output buffer */
    cp = *outbuf;
    if (*state == 0) {
        *cp++ = 0xFE;
        *cp++ = 0xFF;
        *state = 1;
    }
    if (in > 0xFFFF) {
        *cp++ = ((in -= 0x10000) >> 18) | 0xD8;
        *cp++ = (in >> 10) & 0xFF;
        *cp++ = ((in >> 8) & 3) | 0xDC;
    } else
        *cp++ = (in >> 8) & 0xFF;
    *cp++ = in & 0xFF;
    (*outbuf) += bytes;
    *outbytesleft -= bytes;
    return 1;
}

static __inline ucs_t
_DEFUN(msb, (buf), _CONST unsigned char *buf)
{
    return (buf[0] << 8) | buf[1];
}

static ucs_t
_DEFUN(convert_to_ucs, (ces, inbuf, inbytesleft),
                       struct iconv_ces *ces        _AND
                       _CONST unsigned char **inbuf _AND
                       size_t *inbytesleft)
{
    ucs_t res, res2;
    int *state, mark;

    if (*inbytesleft < 2)
        return UCS_CHAR_NONE;    /* Not enough bytes in the input buffer */
    state = (int *)(ces->data);
    res = msb(*inbuf);
    switch (res) {
        case UCS_CHAR_ZERO_WIDTH_NBSP:
        if (*state == 0)
            *state = 1;
        mark = 1;
        break;
        case UCS_CHAR_INVALID:
        if (*state == 0)
            *state = 2;
        mark = 1;
        break;
        default:
        if (*state == 0)
            *state = 1;
        mark = 0;
    }
    if (mark) {
        if (*inbytesleft < 4)
            return UCS_CHAR_NONE;    /* Not enough bytes in the input buffer */
        *inbytesleft -= 2;
        res = msb((*inbuf) += 2);
    }
    if (*state == 2) {        /* LSB order */
        res = (*(*inbuf) ++);
        res |= (*(*inbuf) ++) << 8;
    } else
        *inbuf += 2;
    *inbytesleft -= 2;
    if ((res & 0xFC00) != 0xD800)    /* Non-surrogate character */
        return res;
    if (*inbytesleft < 2)
        return UCS_CHAR_NONE;    /* Not enough bytes in the input buffer */
    if (*state == 2) {
        res2 = (*inbuf)[0];
        res2 |= (*inbuf)[1] << 8;
    } else
        res2 = msb(*inbuf);
    if ((res2 & 0xFC00) != 0xDC00)    /* Broken surrogate pair */
        return -1;
    (*inbuf) += 2;
    (*inbytesleft) -= 2;
    return (((res & 0x3FF) << 10) | (res2 & 0x3FF)) + 0x10000;
}

ICONV_CES_STATEFUL_MODULE_DECL(utf_16);

#endif /* #ifdef _ICONV_CONVERTER_UTF_16 */


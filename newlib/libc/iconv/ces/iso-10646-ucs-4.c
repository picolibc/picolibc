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

#ifdef _ICONV_CONVERTER_ISO_10646_UCS_4
#include "../lib/local.h"

#define out_char(ptr, ch) \
    do { \
        *(*(ptr))++ = ((ch) >> 24) & 0xFF; \
        *(*(ptr))++ = ((ch) >> 16) & 0xFF; \
        *(*(ptr))++ = ((ch) >> 8) & 0xFF; \
        *(*(ptr))++ = (ch) & 0xFF; \
    } while (0)
    
static ssize_t
_DEFUN(convert_from_ucs, (ces, in, outbuf, outbytesleft),
                          struct iconv_ces *ces  _AND
                          ucs_t in               _AND
                          unsigned char **outbuf _AND
                          size_t *outbytesleft)
{
    int *state, bytes;

    if (in == UCS_CHAR_NONE)
        return 1;    /* No state reinitialization for table charsets */
    bytes = *(state = (int *)(ces->data)) ? 4 : 8;
    if (*outbytesleft < bytes)
        return 0;    /* No space in output buffer */
    if (*state == 0) {    /* First character in sequence is byte order */
        out_char(outbuf, UCS_CHAR_ZERO_WIDTH_NBSP);
        *state = 1;
    }
    out_char(outbuf, in);
    *outbytesleft -= bytes;
    return 1;
}

static __inline ucs_t
_DEFUN(msb, (buf), _CONST unsigned char *buf)
{
    return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

static ucs_t
_DEFUN(convert_to_ucs, (ces, inbuf, inbytesleft),
                       struct iconv_ces *ces        _AND
                       _CONST unsigned char **inbuf _AND
                       size_t *inbytesleft)
{
    ucs_t res;
    int *state, mark;

    if (*inbytesleft < 4)
        return UCS_CHAR_NONE;    /* Not enough bytes in the input buffer */
    state = (int *)(ces->data);
    res = msb(*inbuf);
    switch (res) {
        case UCS_CHAR_ZERO_WIDTH_NBSP:
        *state = 1;
        mark = 1;
        break;
        case UCS_CHAR_INVALID:
        *state = 2;
        mark = 1;
        break;
        default:
        mark = 0;
    }
    if (mark) {
        if (*inbytesleft < 8)
            return UCS_CHAR_NONE;    /* Not enough bytes in the input buffer */
        *inbytesleft -= 4;
        *inbuf += 4;
        if (*state != 2)
            res = msb(*inbuf);
    }
    if (*state == 2) {
        res = (unsigned char)(*(*inbuf) ++);
        res |= (unsigned char)(*(*inbuf) ++) << 8;
        res |= (unsigned char)(*(*inbuf) ++) << 16;
        res |= (unsigned char)(*(*inbuf) ++) << 24;
    } else
        *inbuf += 4;
    *inbytesleft -= 4;
    return res;
}

ICONV_CES_STATEFUL_MODULE_DECL(iso_10646_ucs_4);

#endif /* #ifdef _ICONV_CONVERTER_ISO_10646_UCS_4 */


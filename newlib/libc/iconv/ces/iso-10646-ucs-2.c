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

#ifdef _ICONV_CONVERTER_ISO_10646_UCS_2
#include "../lib/local.h"

#define out_char(ptr, ch) \
    do { \
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
    size_t bytes;
    int *state;

    if (in == UCS_CHAR_NONE)
            return 1; /* No state reinitialization for table charsets */
    if (iconv_char32bit(in))
        return -1;    /* No such character in UCS-2 */
    bytes = *(state = (int *)(ces->data)) ? 2 : 4;
    if (*outbytesleft < bytes)
        return 0;     /* No space in the output buffer */
    if (*state) {
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
    return (buf[0] << 8) | buf[1];
}

static ucs_t 
_DEFUN(convert_to_ucs, (ces, inbuf, inbytesleft),
                       struct iconv_ces *ces        _AND
                       _CONST unsigned char **inbuf _AND
                       size_t *inbytesleft)
{
    ucs_t res;
    int *state, mark;

    if (*inbytesleft < 2)
        return UCS_CHAR_NONE; /* Not enough bytes in the input buffer */
    state = (int *)(ces->data);
    res = msb((_CONST unsigned char*)*inbuf);
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
        if (*inbytesleft < 4)
            return UCS_CHAR_NONE; /* Not enough bytes in the input buffer */
        *inbytesleft -= 2;
        res = msb(*inbuf += 2);
    }
    if (*state == 2) {
        res = (unsigned char)(*(*inbuf) ++);
        res |= (unsigned char)(*(*inbuf) ++) << 8;
    } else
        *inbuf += 2;
    *inbytesleft -= 2;
    return res;
}

ICONV_CES_STATEFUL_MODULE_DECL(iso_10646_ucs_2);

#endif /* #ifdef _ICONV_CONVERTER_ISO_10646_UCS_2 */


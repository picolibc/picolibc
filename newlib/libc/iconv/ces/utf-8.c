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

#ifdef _ICONV_CONVERTER_UTF_8
#include "../lib/local.h"

#define cont_byte(b) (((b) & 0x3F) | 0x80)

static ssize_t
_DEFUN(convert_from_ucs,  (ces, in, outbuf, outbytesleft),
                          struct iconv_ces *ces  _AND
                          ucs_t in               _AND
                          unsigned char **outbuf _AND
                          size_t *outbytesleft)
{
    unsigned char *cp;
    int n;
    if (in == UCS_CHAR_NONE)
        return 1;    /* No state reinitialization for table charsets */
    if (in < 0x80) {
        n = 1;
    } else if (in < 0x800) {
        n = 2;
    } else if (in < 0x10000) {
        n = 3;
    } else if (in < 0x200000) {
        n = 4;
    } else if (in < 0x4000000) {
        n = 5;
    } else if (in <= 0x7FFFFFFF) {
        n = 6;
    } else
        return -1;
    if (*outbytesleft < n)
        return 0;
    cp = *outbuf;
    switch (n) {
        case 1:
        *cp = (unsigned char)in;
        break;
        case 2:
        *cp++ = (unsigned char)((in >> 6) | 0xC0);
        *cp++ = (unsigned char)cont_byte(in);
        break;
        case 3:
        *cp++ = (unsigned char)((in >> 12) | 0xE0);
        *cp++ = (unsigned char)cont_byte(in >> 6);
        *cp++ = (unsigned char)cont_byte(in);
        break;
        case 4:
        *cp++ = (unsigned char)((in >> 18) | 0xF0);
        *cp++ = (unsigned char)cont_byte(in >> 12);
        *cp++ = (unsigned char)cont_byte(in >> 6);
        *cp++ = (unsigned char)cont_byte(in);
        break;
        case 5:
        *cp++ = (unsigned char)((in >> 24) | 0xF8);
        *cp++ = (unsigned char)cont_byte(in >> 18);
        *cp++ = (unsigned char)cont_byte(in >> 12);
        *cp++ = (unsigned char)cont_byte(in >> 6);
        *cp++ = (unsigned char)cont_byte(in);
        break;
        case 6:
        *cp++ = (unsigned char)((in >> 30) | 0xFC);
        *cp++ = (unsigned char)cont_byte(in >> 24);
        *cp++ = (unsigned char)cont_byte(in >> 18);
        *cp++ = (unsigned char)cont_byte(in >> 12);
        *cp++ = (unsigned char)cont_byte(in >> 6);
        *cp++ = (unsigned char)cont_byte(in);
        break;
    }
    (*outbytesleft) -= n;
    (*outbuf) += n;
    return 1;
}

static ucs_t
_DEFUN(convert_to_ucs, (ces, inbuf, inbytesleft),
                       struct iconv_ces *ces        _AND
                       _CONST unsigned char **inbuf _AND
                       size_t *inbytesleft)
{
    _CONST unsigned char *in = *inbuf;
    unsigned char byte = *in++;
    ucs_t res = byte;

    if (byte >= 0xC0) {
        if (byte < 0xE0) {
            if (*inbytesleft < 2)
                return UCS_CHAR_NONE;
            if (((byte & ~0x1F) == 0xC0)
                && ((in[0] & 0xC0) == 0x80)) {
                res = ((byte & 0x1F) << 6) | (*in++ & 0x3F);
            } else
                res = UCS_CHAR_INVALID;
        } else if (byte < 0xF0) {
            if (*inbytesleft < 3)
                return UCS_CHAR_NONE;
            if (((byte & ~0x0F) == 0xE0)
                && ((in[0] & 0xC0) == 0x80)
                && ((in[1] & 0xC0) == 0x80)) {
                res = ((byte & 0x0F) << 12) | ((in[0] & 0x3F) << 6)
                                            | (in[1] & 0x3F);
                in += 2;
                } else
                res = UCS_CHAR_INVALID;
        } else if (byte < 0xF8) {
            if (*inbytesleft < 4)
                return UCS_CHAR_NONE;
            if (((byte & ~0x7) == 0xF0)
                && ((in[0] & 0xC0) == 0x80)
                && ((in[1] & 0xC0) == 0x80)
                && ((in[2] & 0xC0) == 0x80)) {
                res = ((byte & 0x7) << 18) | ((in[0] & 0x3F) << 12)
                    | ((in[1] & 0x3F) << 6) | (in[2] & 0x3F);
                in += 3;
            } else
                res = UCS_CHAR_INVALID;
        } else if (byte < 0xFC) {
            if (*inbytesleft < 5)
                return UCS_CHAR_NONE;
            if (((byte & ~0x3) == 0xF8)
                && ((in[0] & 0xC0) == 0x80)
                && ((in[1] & 0xC0) == 0x80)
                && ((in[2] & 0xC0) == 0x80)
                && ((in[3] & 0xC0) == 0x80)) {
                res = ((byte & 0x3) << 24) | ((in[0] & 0x3F) << 18)
                    | ((in[1] & 0x3F) << 12) | ((in[2] & 0x3F) << 8)
                    | (in[3] & 0x3F);
                in += 4;
            } else
                res = UCS_CHAR_INVALID;
        } else if (byte <= 0xFD) {
            if (*inbytesleft < 6)
                return UCS_CHAR_NONE;
            if (((byte & ~0x1) == 0xFC)
                && ((in[0] & 0xC0) == 0x80)
                && ((in[1] & 0xC0) == 0x80)
                && ((in[2] & 0xC0) == 0x80)
                && ((in[3] & 0xC0) == 0x80)
                && ((in[4] & 0xC0) == 0x80)) {
                res = ((byte & 0x1) << 30) | ((in[0] & 0x3F) << 24)
                    | ((in[1] & 0x3F) << 18) | ((in[2] & 0x3F) << 12)
                    | ((in[3] & 0x3F) << 8) | (in[4] & 0x3F);
                in += 5;
            } else
                res = UCS_CHAR_INVALID;
        } else
            res = UCS_CHAR_INVALID;
    } else if (byte & 0x80)
        res = UCS_CHAR_INVALID;

    (*inbytesleft) -= (in - *inbuf);
    *inbuf = in;
    return res;
}

ICONV_CES_STATELESS_MODULE_DECL(utf_8);

#endif /* #ifdef _ICONV_CONVERTER_UTF_8 */


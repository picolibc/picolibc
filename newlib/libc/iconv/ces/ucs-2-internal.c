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

#ifdef _ICONV_CONVERTER_UCS_2_INTERNAL
#include "../lib/local.h"

static ssize_t
_DEFUN(convert_from_ucs, (ces, in, outbuf, outbytesleft),
                         struct iconv_ces *ces  _AND
                         ucs_t in               _AND
                         unsigned char **outbuf _AND
                         size_t *outbytesleft)
{
    if (in == UCS_CHAR_NONE)
        return 1;    /* No state reinitialization for table charsets */
    if (iconv_char32bit(in))
        return -1;    /* No corresponding character in UCS-2 */
    if (*outbytesleft < sizeof(ucs2_t))
        return 0;    /* No space in the output buffer */
    *((ucs2_t *)(*outbuf))++ = in;
    (*outbytesleft) -= sizeof(ucs2_t);
    return 1;
}

static ucs_t
_DEFUN(convert_to_ucs, (ces, inbuf, inbytesleft),
                       struct iconv_ces *ces        _AND
                       _CONST unsigned char **inbuf _AND
                       size_t *inbytesleft)
{
    if (*inbytesleft < sizeof(ucs2_t))
        return UCS_CHAR_NONE;    /* Not enough bytes in the input buffer */
    (*inbytesleft) -= sizeof(ucs2_t);
    return *((_CONST ucs2_t *)(*inbuf))++;
}

ICONV_CES_STATELESS_MODULE_DECL(ucs_2_internal);

#endif /* #ifdef _ICONV_CONVERTER_UCS_2_INTERNAL */


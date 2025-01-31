/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2025 Keith Packard
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

#include "iconv_private.h"

size_t
iconv (iconv_t ic,
       char **__restrict inbuf, size_t *__restrict inbytesleft,
       char **__restrict outbuf, size_t *__restrict outbytesleft)
{
    if (!inbuf || !inbytesleft)
        return 0;

    char        *in = *inbuf;
    char        *out = *outbuf;
    size_t      inbytes = *inbytesleft;
    size_t      outbytes = *outbytesleft;
    size_t      tocopy;
#ifdef _MB_CAPABLE
    int         ret;
    wchar_t     wc;
    size_t      char_count = 0;

    for (;;) {
        if (outbytes && ic->buf_len) {
            tocopy = ic->buf_len;
            if (tocopy > outbytes)
                tocopy = outbytes;
            memcpy(out, ic->buf + ic->buf_off, tocopy);
            out += tocopy;
            ic->buf_off += tocopy;
            if (ic->buf_off == ic->buf_len) {
                char_count++;
                ic->buf_off = ic->buf_len = 0;
            }
        } else if (inbytes && ic->buf_len == 0) {
            ret = ic->in_mbtowc(&wc, in, inbytes, &ic->in_state);
            switch (ret) {
            case -1:
                goto fail;
            case -2:
                goto done;
            default:
                in += ret;
                inbytes -= ret;
                break;
            }
            if (outbytes >= MB_LEN_MAX) {
                ret = ic->out_wctomb(out, wc, &ic->out_state);
                if (ret == -1)
                    goto fail;
                out += ret;
                outbytes -= ret;
            } else {
                ret = ic->out_wctomb(ic->buf, wc, &ic->out_state);
                if (ret == -1)
                    goto fail;
                ic->buf_len = ret;
            }
        } else {
            break;
        }
    }
done:
    *inbuf = in;
    *inbytesleft = inbytes;
    *outbuf = out;
    *outbytesleft = outbytes;
    return char_count;
fail:
    *inbuf = in;
    *inbytesleft = inbytes;
    errno = EILSEQ;
    return (size_t) -1;
#else
    (void) ic;

    tocopy = inbytes;
    if (tocopy < outbytes)
        tocopy = outbytes;
    memcpy(out, in, tocopy);
    in += tocopy;
    inbytes -= tocopy;
    out += tocopy;
    outbytes -= tocopy;
    *inbuf = in;
    *inbytesleft = inbytes;
    *outbuf = out;
    *outbytesleft = outbytes;
    return tocopy;
#endif
}

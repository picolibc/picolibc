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

#define _DEFAULT_SOURCE
#include "iconv_private.h"

iconv_t
iconv_open (const char *tocode, const char *fromcode)
{
    if (!tocode || !fromcode)
        goto fail;

    enum locale_id      toid = __find_charset(tocode);
    enum locale_id      fromid = __find_charset(fromcode);

    if (toid == locale_INVALID || fromid == locale_INVALID)
        goto fail;

#ifdef __MB_CAPABLE
    iconv_t     ic;
    const char  *mode_name;
    static const struct {
        const char              *name;
        enum __iconv_mode       mode;
    } modes[] = {
        { .name = "IGNORE", .mode = iconv_ignore },
        { .name = "NON_IDENTICAL_DISCARD", .mode = iconv_discard },
        { .name = "TRANSLIT", .mode = iconv_translit },
    };
#define NMODE (sizeof(modes)/sizeof(modes[0]))
    size_t      m;
    enum __iconv_mode mode = iconv_default;

    mode_name = strchr(tocode, '/');
    if (mode_name) {
        if (mode_name[1] != '/')
            goto fail;
        mode_name = mode_name + 2;
        for (m = 0; m < NMODE; m++) {
            if (strcmp(modes[m].name, mode_name) == 0) {
                mode = modes[m].mode;
                break;
            }
        }
        if (m == NMODE)
            goto fail;
    }

    ic = calloc(1, sizeof(*ic));
    if (!ic)
        goto fail;

    ic->in_mbtowc = __get_mbtowc(fromid);
    ic->out_wctomb = __get_wctomb(toid);
    ic->mode = mode;
    return ic;
#else
    return NULL;
#endif

fail:
    errno = EINVAL;
    return (iconv_t)-1;
}

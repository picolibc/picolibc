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
#include "locale_private.h"
#include "../ctype/ctype_.h"
#include "../stdlib/local.h"

#ifdef _HAVE_POSIX_LOCALE_API

locale_t
newlocale (int category_mask, const char *locale, locale_t base)
{
    enum locale_id id;
    int category;

    id = __find_locale(locale);
    /*
     * Make sure the mask doesn't contain invalid bits and that locale
     * specifies a known locale value
     */
    if ((category_mask & ~LC_ALL_MASK) != 0 || id == locale_INVALID) {
        errno = EINVAL;
        return NULL;
    }
    if (!base) {
        base = calloc(1, sizeof (struct __locale_t));
        if (!base)
            return NULL;
        base->id[LC_ALL] = id;
    } else {
        if (base == LC_GLOBAL_LOCALE)
            return NULL;
    }

    for (category = LC_ALL + 1; category < _LC_LAST; category++)
        if (category_mask & (1 << category))
            base->id[category] = id;

#ifdef _MB_CAPABLE
    base->wctomb = __get_wctomb(base->id[LC_CTYPE]);
    base->mbtowc = __get_mbtowc(base->id[LC_CTYPE]);
#ifdef _MB_EXTENDED_CHARSETS_ANY
    base->ctype = __get_ctype(base->id[LC_CTYPE]);
#endif
#endif

    return base;
}

#endif

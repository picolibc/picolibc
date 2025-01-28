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

#include "locale_private.h"
#include "../ctype/ctype_.h"

char *
setlocale(int category, const char *locale)
{
    enum locale_id      id;

    if (category < LC_ALL || category >= _LC_LAST)
    {
        errno = EINVAL;
        return NULL;
    }

    /* Return current locale value */
    if (locale == NULL)
        return (char *) __locale_name(__global_locale.id[category]);

    /* Set the global locale */
    id = __find_locale(locale);
    if (id == locale_INVALID)
        return NULL;

    __global_locale.id[category] = id;
    if (category == LC_ALL) {
        for (category = LC_ALL + 1; category < _LC_LAST; category++)
            __global_locale.id[category] = id;
    }
#ifdef _MB_CAPABLE
    __set_wc_funcs(__global_locale.id[LC_CTYPE], &__global_locale.wctomb, &__global_locale.mbtowc);
#ifdef _MB_EXTENDED_CHARSETS_ANY
    __set_ctype(__global_locale.id[LC_CTYPE], &__global_locale.ctype);
#endif
#endif
    return (char *) __locale_name(id);
}

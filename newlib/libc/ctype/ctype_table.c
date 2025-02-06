/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2024 Keith Packard
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

#define _GNU_SOURCE
#include <ctype.h>
#include <wctype.h>
#include <langinfo.h>
#include "local.h"

#ifdef _MB_CAPABLE

static struct {
    wchar_t             code;
    uint16_t            category;
} ctype_table[] = {
#include "ctype_table.h"
};

#define N_CAT_TABLE     (sizeof(ctype_table) / sizeof(ctype_table[0]))

uint16_t
__ctype_table_lookup(wint_t ic, locale_t locale)
{
    size_t      low = 0;
    size_t      high = N_CAT_TABLE - 1;
    size_t      mid;
    wchar_t     c;

    if (ic == WEOF)
        return CLASS_none;

    c = (wchar_t) ic;

    /* Be compatible with glibc where the C locale has no classes outside of ASCII */
    if (c >= 0x80 && __locale_is_C(locale))
        return CLASS_none;

    while (low < high) {
        mid = (low + high) >> 1;
        if (c < ctype_table[mid].code) {
            high = mid - 1;
        } else if (mid >= N_CAT_TABLE || c >= ctype_table[mid+1].code) {
            low = mid + 1;
        } else {
            high = mid;
            break;
        }
    }
    return ctype_table[high].category;
}
#endif /* _MB_CAPABLE */

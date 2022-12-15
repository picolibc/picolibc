/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2021 Keith Packard
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

#include <stdlib.h>
#include <string.h>
#include "stdio_private.h"

int strfroml(char *restrict str, size_t n,
	     const char *restrict format, long double fp)
{
    char nformat[32];
    const char *f;
    bool found_percent;
    bool inserted_L;
    char *nf;

    /* if there's not space for a replacement format, just bail */
    if (strlen(format) > sizeof(nformat) - 2)
        return __d_snprintf(str, n, format, (double) fp);
    nf = nformat;
    f = format;
    found_percent = false;
    inserted_L = false;
    while (*f) {
        char c = *f++;
        switch (c) {
        case '%':
            found_percent = true;
            break;
        case 'e': case 'E':
        case 'f': case 'F':
        case 'g': case 'G':
        case 'a': case 'A':
            if (found_percent && !inserted_L) {
                *nf++ = 'L';
                inserted_L = true;
            }
            found_percent = false;
            break;
        default:
            break;
        }
        *nf++ = c;
    }
    *nf = '\0';
    return __d_snprintf(str, n, nformat, fp);
}

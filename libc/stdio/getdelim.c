/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2023 Keith Packard
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

#include "local-stdio.h"

#define INCR 16

ssize_t
getdelim(char ** restrict lineptr, size_t * restrict nptr, int delim, FILE * restrict stream)
{
    char   *line = *lineptr;
    size_t  n = line ? *nptr : 0;
    ssize_t count = 0;

    __flockfile(stream);
    for (;;) {
        int c = getc_unlocked(stream);
        int is_eof = (c == EOF);

        /* EOF at the start: return -1 */
        if (is_eof && count == 0) {
            count = -1;
            goto bail;
        }

        /* Make space for char (if not eof) and '\0' terminator */
        if ((size_t)count + !is_eof >= n) {

            /* Check for overflow of ssize_t */
            if (n > SSIZE_MAX - INCR) {
                errno = EOVERFLOW;
                count = -1;
                goto bail;
            }
            n += INCR;

            line = realloc(line, n);
            if (line == NULL) {
                count = -1;
                goto bail;
            }
        }

        if (is_eof)
            break;

        line[count++] = c;
        if (c == delim)
            break;
    }
    line[count] = '\0';

    *lineptr = line;
    *nptr = n;
bail:
    __funlock_return(stream, count);
}

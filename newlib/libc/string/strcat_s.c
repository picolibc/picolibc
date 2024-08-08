/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2024, Synopsys Inc.
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
#define __STDC_WANT_LIB_EXT1__ 1
#include <string.h>
#include <stdbool.h>
#include "string_private.h"

__errno_t
strcat_s(char *restrict s1, rsize_t s1max, const char *restrict s2)
{
    const char *msg = "";
    size_t s1_len = 0;
    bool write_null = true;

    if (s1 == NULL) {
        msg = "strcat_s: dest is NULL";
        write_null = false;
        goto handle_error;
    }

    if ((s1max == 0) || (CHECK_RSIZE(s1max))) {
        msg = "strcat_s: dest buffer size is 0 or exceeds RSIZE_MAX";
        write_null = false;
        goto handle_error;
    }

    if (s2 == NULL) {
        msg = "strcat_s: source is NULL";
        goto handle_error;
    }

    /* It is a constraint violation if s1max is not large enough to contain
     * the concatenation s2: no truncation permitted.
     * It is also a constraint violation if the string pointed to by s2
     * overlaps s1 in any way.
     * The C11 Rationale says we are permitted to proceed with the copy and
     * detect dest buffer overrun and overlapping memory blocks as a byproduct
     * of performing the copy operation.  This is to avoid calling strlen on
     * s2 to detect these violations prior to attempting the copy.
     */
    // compute chars available in s1
    s1_len = strnlen_s(s1, s1max);
    if (s1_len == s1max) {
        msg = "strcat_s: string 1 length exceeds buffer size";
        goto handle_error;
    }

    const char *overlap_point;
    bool check_s1_for_overlap;
    unsigned m = s1max - s1_len;
    char *s1cp = s1 + s1_len;
    const char *s2cp = s2;

    if (s1 <= s2) {
        // if we ever reach s2 when storing to s1 we have overlap
        overlap_point = s2;
        check_s1_for_overlap = true;
        // make sure source does not lie within initial dest string.
        if (s2 <= s1cp) {
            msg = "strcat_s: overlapping copy";
            goto handle_error;
        }
    } else {
        // if we ever reach s1 when reading from s2 we have overlap
        overlap_point = s1;
        check_s1_for_overlap = false;
        // issue with checking initial dest string does not apply in this
        // case, overlap will be detected only by hitting overlap_point.
    }

    unsigned written = 0;
    char c = '.';
    while (written < m) {
        if (check_s1_for_overlap) {
            if (s1cp == overlap_point) {
                msg = "strcat_s: overlapping copy";
                goto handle_error;
            }
        } else if (s2cp == overlap_point) {
            msg = "strcat_s: overlapping copy";
            goto handle_error;
        }

        c = *s2cp++;
        *s1cp++ = c;
        written++;
        if (c == '\0') {
            break;
        }
    }

    if (c != '\0') {
        msg = "strcat_s: dest buffer size insufficent to append string";
        goto handle_error;
    }

    // Normal return path
    return 0;

handle_error:
    if (write_null && s1 != NULL) {
        *s1 = '\0';
    }

    if (__cur_handler != NULL) {
        __cur_handler(msg, NULL, -1);
    }

    return -1;
}

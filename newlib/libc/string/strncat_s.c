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
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

errno_t strncat_s(char *restrict s1, rsize_t s1max, const char *restrict s2,
        rsize_t n)
{
    bool constraint_failure = false;
    const char *msg = "";
    size_t s1_len = 0;
    bool write_null = true;
    errno_t rtn = 0;

    s1_len = strnlen_s(s1, s1max);

    if (s1 == NULL)
    {
        constraint_failure = true;
        msg = "strncat_s: dest is NULL";
        write_null = false;
    }
    else if ( (s1max == 0u) || (s1max >  RSIZE_MAX))
    {
        constraint_failure = true;
        msg = "strncat_s: dest buffer size is 0 or exceeds RSIZE_MAX";
        write_null = false;
    }
    else if (s2 == NULL)
    {
        constraint_failure = true;
        msg = "strncat_s: source is NULL";
    }
    else if (n > RSIZE_MAX)
    {
        constraint_failure = true;
        msg = "strncat_s: copy count exceeds RSIZE_MAX";
    }

    /* It is a constraint violation if s1max is not large enough to contain
     * the concatenation of s2.
     * It is also a constraint violation if the string pointed to by s2
     * overlaps s1 in any way.
     * The C11 Rationale says we are permitted to proceed with the copy and
     * detect dest buffer overrun and overlapping memory blocks as a byproduct
     * of performing the copy operation.  This is to avoid calling strlen on
     * s2 to detect these violations prior to attempting the copy.
     */
    // compute chars available in s1

    else if (s1_len == s1max)
    {
        constraint_failure = true;
        msg = "strcat_s: string 1 length exceeds buffer size";
    }
    else
    {
        // compute chars available in s1
        uint32_t m = (s1max - s1_len);
        uint32_t i = 0;
        char *s1cp = s1;

        for (i = 0u; i < s1_len; i++)
        {
            s1cp++;
        }

        // Question; at this point should we just return
        // strncpy_s(s1cp, m, s2, n)  ?
        // perhaps not since overlap check needs to be over entire s1 vs. s2?

        const char *overlap_point;
        bool check_s1_for_overlap;
        const char *s2cp = s2;

        if (s1 <= s2)
        {
            // if we ever reach s2 when storing to s1 we have overlap
            overlap_point = s2;
            check_s1_for_overlap = true;
            // make sure source does not lie within initial dest string.
            if (s2 <= s1cp)
            {
                constraint_failure = true;
                msg = "strcat_s: overlapping copy";
            }
        }
        else
        {
            // if we ever reach s1 when reading from s2 we have overlap
            overlap_point = s1;
            check_s1_for_overlap = false;
        }

        uint32_t written = 0;
        char c = '.';

        while ((written < m) && (written < n))
        {
            if (check_s1_for_overlap == true)
            {
                if (s1cp == overlap_point)
                {
                    constraint_failure = true;
                }
            }
            else if (s2cp == overlap_point)
            {
                constraint_failure = true;
            }
            else
            {
                /* Normal case*/
            }

            if (constraint_failure == false)
            {
                c = *s2cp;
                s2cp++;
                *s1cp = c;
                s1cp++;
                written++;
            }

            if ((constraint_failure == true) || (c == '\0'))
            {
                break;
            }
        }

        if ( (c != '\0') && (written == n) && (written < m))
        {
            // we copied n chars from s2 and there is room for null char in s1
            if ((check_s1_for_overlap== true) && (s1cp == overlap_point))
            {
                constraint_failure = true;
            }
            else
            {
                c = '\0';
                *s1cp = '\0';
            }
        }

        if (constraint_failure == true)
        {
            msg = "strncat_s: overlapping copy";
        }
        else if (c != '\0')
        {
            constraint_failure = true;
            msg = "strncat_s: dest buffer size insufficent to copy string";
        }
        else
        {
            /*No Error*/
        }
    }

    if (constraint_failure == true)
    {
        constraint_handler_t handler = set_constraint_handler_s(NULL);
        (void)set_constraint_handler_s(handler);

        if (write_null == true)
        {
            *s1 = '\0';
        }

        ( *handler)(msg, NULL, -1);

        rtn = -1;
    }
    else
    {
        rtn = 0;
    }

    return rtn;
}

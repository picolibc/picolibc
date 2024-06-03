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
#include <stdint.h>
#include <stdlib.h>

errno_t memcpy_s(void* restrict s1, rsize_t s1max, const void* restrict s2, rsize_t n)
{
    bool constraint_failure = false;
    bool zero_dest = true;
    const char* msg = "";
    errno_t rtn = 0;

    if (s1 == NULL)
    {
        constraint_failure = true;
        msg = "memcpy_s: dest is NULL";
        zero_dest = false;
    }

    if ((constraint_failure == false) && (s1max > RSIZE_MAX))
    {
        constraint_failure = true;
        msg = "memcpy_s: buffer size exceeds RSIZE_MAX";
        zero_dest = false;
    }

    if ((constraint_failure == false) && (s2 == NULL))
    {
        constraint_failure = true;
        msg = "memcpy_s: source is NULL";
    }

    if ((constraint_failure == false) && (n > RSIZE_MAX))
    {
        constraint_failure = true;
        msg = "memcpy_s: copy count exceeds RSIZE_MAX";
    }

    if ((constraint_failure == false) && (n > s1max))
    {
        constraint_failure = true;
        msg = "memcpy_s: copy count exceeds buffer size";
    }

    if (constraint_failure == false)
    {
        const char* s1cp = (const char*) s1;
        const char* s2cp = (const char*) s2;
        const char* s1cp_limit = &s1cp[n];
        const char* s2cp_limit = &s2cp[n];

        if (((s1cp_limit <= s2cp) || (s2cp_limit <= s1cp)) == false)
        {
            constraint_failure = true;
            msg = "memcpy_s: overlapping copy";
        }
    }

    if (constraint_failure == true)
    {
        constraint_handler_t handler = set_constraint_handler_s(NULL);
        (void) set_constraint_handler_s(handler);

        if (zero_dest == true)
        {
            (void) memset(s1, (int32_t)'\0', s1max);
        }
        else
        {
            /* No action required */
        }

        (*handler)(msg, NULL, -1);
        rtn = -1;
    }
    else
    {
        (void) memcpy(s1, s2, n);
        rtn = 0;
    }

    return rtn;
}

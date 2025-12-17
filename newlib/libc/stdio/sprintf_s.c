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
#include "stdio_private.h"
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include "../stdlib/local_s.h"

int
sprintf_s(char * __restrict s, rsize_t bufsize, const char * __restrict fmt, ...)
{
    bool        write_null = true;
    const char *msg = "";
    va_list     args;
    int         rc;

    if (s == NULL) {
        write_null = false;
        msg = "dest buffer is null";
        goto handle_error;
    } else if ((bufsize == 0) || (CHECK_RSIZE(bufsize))) {
        write_null = false;
        msg = "invalid buffer size";
        goto handle_error;
    } else {
        va_start(args, fmt);
        rc = vsnprintf_s(s, bufsize, fmt, args);
        va_end(args);
    }

    if (rc < 0) {
        rc = 0;
    }

    // Normal return path
    return rc;

handle_error:
    if (__cur_handler != NULL) {
        __cur_handler(msg, NULL, -1);
    }

    rc = 0; /* standard stipulates this */

    if (write_null && s != NULL) {
        s[0] = '\0'; /* again, standard requires this */
    }

    return rc;
}

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
vsnprintf_s(char *__restrict s, rsize_t n, const char *__restrict fmt, va_list arg)
{
    bool write_null = true;
    const char *msg = "";
    int rc;

    if (s == NULL) {
        write_null = false;
        msg = "dest buffer is null";
        goto handle_error;
    } else if ((n == 0) || (CHECK_RSIZE(n))) {
        write_null = false;
        msg = "invalid buffer size";
        goto handle_error;
    } else if (fmt == NULL) {
        msg = "null format string";
        goto handle_error;
    } else {
        if ((int)n < 0) {
            n = (unsigned)INT_MAX + 1;
        }
        struct __file_str f = FDEV_SETUP_STRING_WRITE(s, n ? n - 1 : 0);
        rc = vfprintf_s(&f.file, fmt, arg);
        if (n) {
            *f.pos = '\0';
        }
    }

    if (rc < 0) {
        msg = "output error";
        goto handle_error;
    } else if (rc > INT_MAX) {
        msg = "output size exceeds max limit";
        goto handle_error;
    } else if ((unsigned int)rc >= n) {
        msg = "dest buffer overflow";
        goto handle_error;
    } else {
        s[rc] = '\0';
    }

    // Normal return path
    return rc;

handle_error:
    if (__cur_handler != NULL) {
        __cur_handler(msg, NULL, -1);
    }

    rc = -1;

    if (write_null && s != NULL && n > 0) {
        s[0] = '\0'; /* again, standard requires this */
    }

    return rc;
}
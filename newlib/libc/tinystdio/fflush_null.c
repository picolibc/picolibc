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

#include "stdio_private.h"

extern FILE *const stdout _ATTRIBUTE((__weak__));

static size_t   num_files;
static FILE     **files;

int _fflush_register_real(FILE *stream)
{
    int ret = -1;

    if (!stream->flush)
        return 0;

    __LIBC_LOCK();
    FILE ** new_files = reallocarray(files, num_files+1, sizeof(FILE *));
    if (new_files) {
        new_files[num_files++] = stream;
        files = new_files;
        ret = 0;
    }
    __LIBC_UNLOCK();
    return ret;
}

void _fflush_unregister_real(FILE *stream)
{
    size_t i;

    if (!stream->flush)
        return;

    __LIBC_LOCK();
    for (i = 0; i < num_files; i++)
        if (files[i] == stream) {
            memmove(&files[i], &files[i+1], (num_files - i - 1) * sizeof(FILE *));
            num_files--;
            break;
        }
    __LIBC_UNLOCK();
}

int _fflush_null(void)
{
    int ret = 0;
    size_t i;
    __LIBC_LOCK();
    if (&stdout)
        ret = _fflush_nonnull(stdout);
    for (i = 0; i < num_files; i++)
        ret = _fflush_nonnull(files[i]) || ret;
    __LIBC_UNLOCK();
    return ret;
}

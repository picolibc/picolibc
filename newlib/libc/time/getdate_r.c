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

#define _GNU_SOURCE
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

/* The  DATEMSK environment variable is not defined, or its value is an empty string. */
#define GETDATE_DATEMSK 1

/* The template file specified by DATEMSK cannot be opened for reading. */
#define GETDATE_NOACCESS 2

/* Failed to get file status information. */
#define GETDATE_NOSTATUS 3

/* The template file is not a regular file. */
#define GETDATE_NOTREGULAR 4

/* An error was encountered while reading the template file. */
#define GETDATE_READERROR 5

/* Memory allocation failed (not enough memory available). */
#define GETDATE_NOMEMORY 6

/* There is no line in the file that matches the input. */
#define GETDATE_NOMATCH 7

/* Invalid input specification. */
#define GETDATE_INVALID_INPUT 8

int
getdate_r(const char * restrict string, struct tm * restrict tm)
{
    char        line[64];
    const char *datemsk_name;
    FILE       *datemsk;
    char       *ret = NULL;

    datemsk_name = getenv("DATEMSK");
    if (!datemsk_name || datemsk_name[0] == '\0')
        return GETDATE_DATEMSK;

    datemsk = fopen(datemsk_name, "r");
    if (!datemsk) {
        if (errno == ENOMEM)
            return GETDATE_NOMEMORY;
        return GETDATE_NOACCESS;
    }
    while (fgets(line, sizeof(line), datemsk)) {
        ret = strptime(string, line, tm);
        if (ret != NULL)
            break;
    }
    fclose(datemsk);
    if (ret)
        return 0;
    return GETDATE_NOMATCH;
}

/*
Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
SPDX-License-Identifier: BSD-3-Clause-Clear

Redistribution and use in source and binary forms, with or without
modification, are permitted (subject to the limitations in the
disclaimer below) provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials provided
    with the distribution.

  * Neither the name of Qualcomm Technologies, Inc. nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * ATEXIT_MAX registrations must be available to the application. Internal
 * library handlers must not come out of that budget.
 *
 * A buffered stream is opened first because the exit flush handler is
 * registered from a constructor in the buffered stdio code, which is only
 * linked in once something uses it.
 */

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

/*
 * ATEXIT_MAX is optional. Implementations with no fixed limit, such as
 * glibc, do not define it, so fall back to the value POSIX requires an
 * implementation to support at minimum.
 */
#ifndef ATEXIT_MAX
#define ATEXIT_MAX 32
#endif

#ifndef TEST_FILE_NAME
#define TEST_FILE_NAME "atexit-max-test-file"
#endif

static const char file_name[] = TEST_FILE_NAME;

static void
noop(void)
{
}

int
main(void)
{
    FILE *f;
    int   i;

    f = fopen(file_name, "w");
    if (!f) {
        printf("failed to open \"%s\" for writing\n", file_name);
        return 1;
    }
    fputs("hello, world\n", f);

    for (i = 0; i < ATEXIT_MAX; i++) {
        if (atexit(noop) != 0) {
            printf("atexit failed at registration %d of %d\n", i, ATEXIT_MAX);
            fclose(f);
            remove(file_name);
            return 1;
        }
    }

    fclose(f);
    remove(file_name);
    printf("success\n");
    return 0;
}

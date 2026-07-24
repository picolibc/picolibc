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
 * test-libdl-load.c - happy-path libdl test.
 *
 * Builds a shared object (testmod.so, path passed at compile time via
 * TESTMOD_SO_PATH), then:
 *   1. dlopen()s it,
 *   2. dlsym()s an exported function and calls it,
 *   3. dlsym()s an exported data symbol and reads it,
 *   4. dlsym()s an exported function returning a string,
 *   5. dlclose()s it.
 */

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

#ifndef TESTMOD_SO_PATH
#error "TESTMOD_SO_PATH must be defined by the build"
#endif

#define FAIL(msg)                                                   \
    do {                                                            \
        const char *e = dlerror();                                  \
        printf("FAIL: %s%s%s\n", (msg), e ? ": " : "", e ? e : ""); \
        return 1;                                                   \
    } while (0)

int
main(void)
{
    void *h = dlopen(TESTMOD_SO_PATH, RTLD_NOW);
    if (!h)
        FAIL("dlopen(" TESTMOD_SO_PATH ")");

    /* Exported function */
    int (*add)(int, int) = (int (*)(int, int))dlsym(h, "testmod_add");
    if (!add)
        FAIL("dlsym(testmod_add)");
    if (add(2, 3) != 5) {
        printf("FAIL: testmod_add(2, 3) != 5 (got %d)\n", add(2, 3));
        return 1;
    }

    /* Exported initialised data symbol */
    int *value = (int *)dlsym(h, "testmod_value");
    if (!value)
        FAIL("dlsym(testmod_value)");
    if (*value != 42) {
        printf("FAIL: testmod_value != 42 (got %d)\n", *value);
        return 1;
    }

    /* Exported function returning a static string (relocated pointer) */
    const char *(*name)(void) = (const char *(*)(void))dlsym(h, "testmod_name");
    if (!name)
        FAIL("dlsym(testmod_name)");
    if (strcmp(name(), "testmod") != 0) {
        printf("FAIL: testmod_name() != \"testmod\" (got \"%s\")\n", name());
        return 1;
    }

    if (dlclose(h) != 0)
        FAIL("dlclose");

    printf("PASS\n");
    return 0;
}

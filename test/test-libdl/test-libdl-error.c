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
 * test-libdl-error.c - libdl error-path test.
 *
 * Exercises the failure and defensive paths that do not require a valid
 * shared object to be loaded:
 *   1. dlopen() of a nonexistent file returns NULL and sets dlerror().
 *   2. dlerror() is cleared (returns NULL) after being read once.
 *   3. dlsym(NULL, ...) and dlsym(handle, NULL) return NULL safely.
 *   4. dlclose(NULL) is a safe no-op.
 */

#include <dlfcn.h>
#include <stdio.h>

#define FAIL(msg)                    \
    do {                             \
        printf("FAIL: %s\n", (msg)); \
        return 1;                    \
    } while (0)

int
main(void)
{
    /* 1. dlopen of a nonexistent file must fail. */
    void *h = dlopen("/no/such/file.so", RTLD_NOW);
    if (h != NULL)
        FAIL("dlopen of nonexistent file should return NULL");

    /* 2. dlerror() must be set after the failure, then cleared. */
    if (dlerror() == NULL)
        FAIL("dlerror should be set after failed dlopen");
    if (dlerror() != NULL)
        FAIL("dlerror should be cleared after being read");

    /* 3. NULL-argument safety for dlsym. */
    if (dlsym(NULL, "testmod_add") != NULL)
        FAIL("dlsym(NULL, ...) should return NULL");
    if (dlsym((void *)0x1, NULL) != NULL)
        FAIL("dlsym(handle, NULL) should return NULL");

    /* 4. dlclose(NULL) must be a safe no-op returning 0. */
    if (dlclose(NULL) != 0)
        FAIL("dlclose(NULL) should return 0");

    printf("PASS\n");
    return 0;
}

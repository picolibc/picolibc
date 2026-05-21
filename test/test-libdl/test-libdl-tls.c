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
 * test-libdl-tls.c - verify dynamic TLS in a dlopen()ed shared object.
 *
 * testmod.so is built with -ftls-model=global-dynamic and contains several
 * thread-local variables covering the distinct loader cases:
 *
 *   testmod_tdata1/2/3  int       = 111/222/333    (.tdata, copied)
 *   testmod_tbss1/2/3   int       = 0              (.tbss, zeroed)
 *   testmod_wide        long long = 0x1122334455667788  (.tdata, aligned)
 *
 * Through exported accessor functions this test checks that:
 *   1. The initialised .tdata values are visible after dlopen() (template
 *      copied) and the .tbss values are zero (block zeroed), i.e. the
 *      loader allocated the TLS block, copied the template, applied the
 *      DTPMOD/DTPREL relocations, resolved __tls_get_addr and set the
 *      thread pointer.
 *   2. Writes through the module round-trip via the getters.
 *   3. All variables are independent (distinct offsets, no aliasing),
 *      which exercises the non-zero DTPREL offset path across .tdata/.tbss.
 *   4. The address the module computes for a variable aliases its storage.
 */

#include <dlfcn.h>
#include <stdio.h>

#ifndef TESTMOD_SO_PATH
#error "TESTMOD_SO_PATH must be defined by the build"
#endif

#define FAIL(msg)                                                   \
    do {                                                            \
        const char *e = dlerror();                                  \
        printf("FAIL: %s%s%s\n", (msg), e ? ": " : "", e ? e : ""); \
        return 1;                                                   \
    } while (0)

/* Resolve an int getter/setter pair; fail the test if either is missing. */
#define SYM(var, type, name)         \
    type var = (type)dlsym(h, name); \
    if (!var)                        \
    FAIL("dlsym(" name ")")

int
main(void)
{
    void *h = dlopen(TESTMOD_SO_PATH, RTLD_NOW);
    if (!h)
        FAIL("dlopen(" TESTMOD_SO_PATH ")");

    typedef int  (*getter_t)(void);
    typedef void (*setter_t)(int);

    SYM(td1_get, getter_t, "testmod_tdata1_get");
    SYM(td1_set, setter_t, "testmod_tdata1_set");
    SYM(td2_get, getter_t, "testmod_tdata2_get");
    SYM(td2_set, setter_t, "testmod_tdata2_set");
    SYM(td3_get, getter_t, "testmod_tdata3_get");
    SYM(td3_set, setter_t, "testmod_tdata3_set");
    SYM(tb1_get, getter_t, "testmod_tbss1_get");
    SYM(tb1_set, setter_t, "testmod_tbss1_set");
    SYM(tb2_get, getter_t, "testmod_tbss2_get");
    SYM(tb2_set, setter_t, "testmod_tbss2_set");
    SYM(tb3_get, getter_t, "testmod_tbss3_get");
    SYM(tb3_set, setter_t, "testmod_tbss3_set");

    int      *(*td1_addr)(void) = (int *(*)(void))dlsym(h, "testmod_tdata1_addr");
    long long (*wide_get)(void) = (long long (*)(void))dlsym(h, "testmod_wide_get");
    void      (*wide_set)(long long) = (void (*)(long long))dlsym(h, "testmod_wide_set");
    if (!td1_addr || !wide_get || !wide_set)
        FAIL("dlsym(testmod_tdata1_addr / testmod_wide_*)");

    /* 1a. Initialised .tdata values visible. */
    if (td1_get() != 111 || td2_get() != 222 || td3_get() != 333) {
        printf("FAIL: initial .tdata values wrong (%d,%d,%d)\n", td1_get(), td2_get(), td3_get());
        return 1;
    }
    /* 1b. .tbss values zeroed. */
    if (tb1_get() != 0 || tb2_get() != 0 || tb3_get() != 0) {
        printf("FAIL: initial .tbss values not zero (%d,%d,%d)\n", tb1_get(), tb2_get(), tb3_get());
        return 1;
    }
    /* 1c. Wide (aligned) .tdata value visible. */
    if (wide_get() != 0x1122334455667788LL) {
        printf("FAIL: initial wide != 0x1122334455667788 (got %llx)\n",
               (unsigned long long)wide_get());
        return 1;
    }

    /* 2 & 3. Write distinct values to every variable, then confirm each
     * reads back independently (catches offset collisions / wrong DTPREL). */
    td1_set(0x0a0a0001);
    td2_set(0x0a0a0002);
    td3_set(0x0a0a0003);
    tb1_set(0x0b0b0001);
    tb2_set(0x0b0b0002);
    tb3_set(0x0b0b0003);
    wide_set(0x0c0c0c0c0d0d0d0dLL);

    if (td1_get() != 0x0a0a0001 || td2_get() != 0x0a0a0002 || td3_get() != 0x0a0a0003) {
        printf("FAIL: .tdata aliased (%x,%x,%x)\n", td1_get(), td2_get(), td3_get());
        return 1;
    }
    if (tb1_get() != 0x0b0b0001 || tb2_get() != 0x0b0b0002 || tb3_get() != 0x0b0b0003) {
        printf("FAIL: .tbss aliased (%x,%x,%x)\n", tb1_get(), tb2_get(), tb3_get());
        return 1;
    }
    if (wide_get() != 0x0c0c0c0c0d0d0d0dLL) {
        printf("FAIL: wide aliased (got %llx)\n", (unsigned long long)wide_get());
        return 1;
    }

    /* 4. The module's computed address must alias the stored value. */
    int *p = td1_addr();
    if (p == NULL) {
        printf("FAIL: testmod_tdata1_addr() returned NULL\n");
        return 1;
    }
    if (*p != 0x0a0a0001) {
        printf("FAIL: *testmod_tdata1_addr() != 0x0a0a0001 (got %x)\n", *p);
        return 1;
    }
    *p = 0x0e0e0e0e;
    if (td1_get() != 0x0e0e0e0e) {
        printf("FAIL: tdata1 after *addr write != 0x0e0e0e0e (got %x)\n", td1_get());
        return 1;
    }

    if (dlclose(h) != 0)
        FAIL("dlclose");

    printf("PASS\n");
    return 0;
}

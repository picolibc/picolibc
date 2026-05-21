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
 * TLS test: verify that TLS variables in libc.so (errno) are accessible
 * after dlopen() by calling strtol() which writes errno via __tls_get_addr.
 *
 * strtol() uses stack protection (__stack_chk_guard).  libc.so's guard is
 * initialised by its DT_INIT_ARRAY constructor to a fixed value via
 * stub_getentropy.  We copy the test executable's guard into libc.so's BSS
 * before calling any stack-protected function from libc.so.
 *
 * strtol() writes errno=0 via __tls_get_addr(UGP + tls_offset).  If UGP is
 * not correctly set to libc.so's TLS block, the write corrupts memory and
 * the function never returns cleanly.  Correct return values confirm TLS
 * is working.
 */

#include "test-libdl.h"

/* The test executable's own stack canary (from libc.a) */
extern long __stack_chk_guard;

int main(void)
{
    void *h = open_libc();
    if (!h) return 1;

    /* Synchronise libc.so's __stack_chk_guard with the test exe's value
     * so that stack-protected functions in libc.so pass their canary check.
     */
    long *so_guard = dlsym(h, "__stack_chk_guard");
    if (!so_guard) FAIL("dlsym(__stack_chk_guard)");
    *so_guard = __stack_chk_guard;

    long (*my_strtol)(const char *, char **, int) = dlsym(h, "strtol");
    if (!my_strtol) FAIL("dlsym(strtol)");

    /* Each call writes errno=0 via __tls_get_addr(UGP + 0x18).
     * If UGP is wrong, the write corrupts memory and the function
     * never returns cleanly.  Correct return values confirm TLS works.
     */
    long v = my_strtol("42", NULL, 10);
    if (v != 42)     FAIL("strtol(\"42\") != 42");

    long neg = my_strtol("-100", NULL, 10);
    if (neg != -100) FAIL("strtol(\"-100\") != -100");

    long hex = my_strtol("ff", NULL, 16);
    if (hex != 255)  FAIL("strtol(\"ff\", 16) != 255");

    dlclose(h);
    printf("PASS\n");
    return 0;
}

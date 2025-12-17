/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2024, Synopsys Inc.
 * Copyright © 2024, Solid Sands B.V.
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

#include <stdio.h>
#include <string.h>

#define EXPECTED "123456789\n"

int
main(void)
{

#if defined(__PICOLIBC__) && !defined(__IO_PERCENT_N)
    printf("printf: %%n unsupported\n");
    return 77;
#else
    int  res;
    char result[128];
    int  a, b, c, d;

    /* res should be equal to 10:
     * "" reaching the 1st '%n' ---------------- counting 0 -> a
     * "12345" between the 1st and 2nd '%n' ---- counting 5 -> b
     * "6789" between the 2nd and 3rd '%n'  ---- counting 9 -> c
     * "\n" between the 3rd and 4th '%n' ------- counting 10 -> d */

    res = snprintf(result, sizeof(result), "%n12345%n6789%n\n%n", &a, &b, &c, &d);

    if (res != 10 || a != 0 || b != 5 || c != 9 || d != 10) {
        printf("Test Failed: Failed to count characters correctly\n");
        return 1;
    }

    if (strcmp(result, EXPECTED) != 0) {
        printf("Test failed: Incorrect result got \"%s\" != expected \"%s\"\n", result, EXPECTED);
        return 1;
    }

    printf("Test Passed: Characters' count is correct\n");

    return 0;
#endif
}

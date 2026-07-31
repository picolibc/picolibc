/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2022 Keith Packard
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
#include <stdlib.h>

#if defined(__riscv) && (defined(__riscv_zcmt) || defined(__riscv_xqccmt))

const char    *get_message(void);

_Noreturn void print_message(const char *);

/*
 * The Jump Table has 256 entries, the first 32 are for `cm.jt` and the rest are
 * for `cm.jalt`. The former is for `tail` and the latter is for `call`.
 */
__attribute__((aligned(64))) __attribute__((section(".riscv.jvt"))) void *func_table[256] = {
    [0] = &print_message,
    [32] = &get_message,
};

__asm__(".weak __jvt_base$\n"
        ".set __jvt_base$, func_table\n");

const char *
get_message(void)
{
    return "retrieved message correctly";
}

void
print_message(const char *message)
{
    printf("SUCCESS: %s\n", message);
    exit(0);
}

#endif

int
main(void)
{
    printf("executing jvt tests\n");

    /*
     * This is a RISC-V specific feature.
     */
#if defined(__riscv) && defined(__riscv_zcmt)
    __asm__("cm.jalt 32\n" // This is equivalent to `call func_table[32]`
            "cm.jt 0\n"    // This is equivalent to `tail func_table[0]`
    );

    printf("ERROR: cm.jt returned\n");
    return 1;
#elif defined(__riscv) && defined(__riscv_xqccmt)
    __asm__("qc.cm.jalt 32\n" // This is equivalent to `call func_table[32]`
            "qc.cm.jt 0\n"    // This is equivalent to `tail func_table[0]`
    );

    printf("ERROR: qc.cm.jt returned\n");
    return 1;
#else
    printf("jvt not supported for this target\n");
    return 77;
#endif
}

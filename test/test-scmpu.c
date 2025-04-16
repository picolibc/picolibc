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

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#ifdef __RX_ALLOW_STRING_INSNS__

struct scmpu {
    unsigned char       *R1, *R2;
    unsigned long       R3;
    bool                C;
    bool                Z;
};


static void
soft_scmpu(struct scmpu *scmpu)
{
    unsigned char       *R1 = scmpu->R1, *R2 = scmpu->R2;
    unsigned long       R3 = scmpu->R3;
    bool                C = scmpu->C;
    bool                Z = scmpu->Z;
    unsigned char       tmp0, tmp1;
    int                 diff;

    while ( R3 != 0 ) {
        tmp0 = *R1++;
        tmp1 = *R2++;
        R3--;
        diff = (int) ((unsigned int) tmp0 - (unsigned int) tmp1);
        C = diff >= 0;
        if ( diff ) {
            Z = 0;
            break;
        }
        Z = 1;
        if ( tmp0 == '\0' )
            break;
    }
    scmpu->R1 = R1;
    scmpu->R2 = R2;
    scmpu->R3 = R3;
    scmpu->C = C;
    scmpu->Z = Z;
}

static void
hard_scmpu(struct scmpu *scmpu)
{
    register unsigned char *R1 __asm__("r1") = scmpu->R1;
    register unsigned char *R2 __asm__("r2") = scmpu->R2;
    register unsigned long R3 __asm__("r3") = scmpu->R3;

    unsigned long psw;
    __asm__("mvfc PSW, %0" : "=r" (psw));

    psw = (psw & ~3) | (!!scmpu->C) | ((!!scmpu->Z) << 1);
    __asm__("mvtc %0, PSW" : : "r" (psw));
    __asm__("scmpu\n"
            "mvfc PSW, %3"
            : "=r" (R1), "=r" (R2), "=r" (R3), "=r" (psw) :
              "r" (R1), "r" (R2), "r" (R3) :
              "cc");
    scmpu->R1 = R1;
    scmpu->R2 = R2;
    scmpu->R3 = R3;
    scmpu->C = !!(psw & 1);
    scmpu->Z = !!(psw & 2);
}

static int
check(unsigned i, unsigned j, const char *src, const char *dst, unsigned long len)
{
    unsigned char *u_src = (unsigned char *) src;
    unsigned char *u_dst = (unsigned char *) dst;
    struct scmpu scmpu_soft = {
        .R1 = u_src,
        .R2 = u_dst,
        .R3 = len,
        .C = 0,
        .Z = 0
    };
    struct scmpu scmpu_hard = {
        .R1 = u_src,
        .R2 = u_dst,
        .R3 = len,
        .C = 0,
        .Z = 0
    };
    int ret = 1;

    soft_scmpu(&scmpu_soft);
    hard_scmpu(&scmpu_hard);
#if 1
    printf("%d %d src '%s' dst '%s' len %ld strncmp %d R1 %zd '%s' R2 %zd '%s' R3 %ld C %d Z %d\n",
           i, j, src, dst, len, strncmp(src, dst, len),
           scmpu_soft.R1 - u_src, scmpu_soft.R1,
           scmpu_soft.R2 - u_dst, scmpu_soft.R2,
           scmpu_soft.R3, scmpu_soft.C, scmpu_soft.Z);
#endif
    if (scmpu_soft.R1 != scmpu_hard.R1) {
        printf("test %d,%d R1 soft %p %s hard %p %s\n",
               i, j,
               scmpu_soft.R1, scmpu_soft.R1,
               scmpu_hard.R1, scmpu_hard.R1);
        ret = 0;
    }
    if (scmpu_soft.R2 != scmpu_hard.R2) {
        printf("R2 soft %p %s hard %p %s\n",
               scmpu_soft.R2, scmpu_soft.R2,
               scmpu_hard.R2, scmpu_hard.R2);
        ret = 0;
    }
    if (scmpu_soft.R3 != scmpu_hard.R3) {
        printf("R3 soft %ld hard %ld\n",
               scmpu_soft.R3, scmpu_hard.R3);
        ret = 0;
    }
    if (scmpu_soft.C != scmpu_hard.C) {
        printf("C soft %d hard %d\n",
               scmpu_soft.C, scmpu_hard.C);
        ret = 0;
    }
    if (scmpu_soft.Z != scmpu_hard.Z) {
        printf("Z soft %d hard %d\n",
               scmpu_soft.Z, scmpu_hard.Z);
        ret = 0;
    }
    return ret;
}

#endif

int main(void)
{
#ifdef __RX_ALLOW_STRING_INSNS__
    int ret = 0;
    const char hi[] = { 0xc1, 'A', 'B', 'C', 0, 'M', 'N', 'O', 'P', 'Q', 'R', 0 };
    const char lo[] = { 0x41, 'A', 'B', 'C', 0, 'M', 'N', 'O', 'P', 'Q', 'R', 0 };
    const char *const tests[] = {
#if 0
        "hello\0abcdef",
        "world\0ABCDEF",
        "\0uvwxyz",
        "foobar and bletch\0UVWXYZ",
        "hello world\0ghijkl",
        "hello word\0GHIJKL",
#endif
        hi, lo,
    };
    unsigned i, j;
    unsigned long len;
#define NTESTS (sizeof(tests)/sizeof(tests[0]))

    for (j = 0; j < NTESTS; j++)
        for (i = 0; i < NTESTS; i++)
            for (len = 0; len < 20; len++)
                if (!check (i, j, tests[i], tests[j], len))
                    ret = 1;
    return ret;
#else
    printf("No RX string instructions, test skipped\n");
    return 77;
#endif
}

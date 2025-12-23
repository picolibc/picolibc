/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2019 Keith Packard
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

#include "../../crt0.h"

static void __used __section(".init") _cstart(void)
{
    __start();
}

extern char __stack[];
extern void(* const __vector_table[]);

#ifdef __ARC64_ARCH64__
#define SRR  "srl"
#define LRR  "lrl"
#define PUSH "pushl"
typedef uint64_t reg_t;
#else
#define SRR  "sr"
#define LRR  "lr"
#define PUSH "push"
typedef uint32_t reg_t;
#endif

void __naked __section(".init") __used _start(void)
{
    /* Set up the stack pointer */
    __asm__("mov_s sp, %0" : : "r"(__stack));

    /* Set up the vector table */
    __asm__(SRR " %0, [0x25]" : : "r"(__vector_table));

    /* Branch to C code */
    __asm__("j _cstart");

    /* Fill in any delay slot */
    __asm__("nop");
}

#ifdef CRT0_SEMIHOST

/*
 * Trap faults, print message and exit when running under semihost
 */

#include <unistd.h>
#include <stdio.h>

struct fault {
    reg_t eret;
    reg_t erbta;
    reg_t erstatus;
    reg_t ecr;
    reg_t efa;
    reg_t r[32];
};

static void
arc_fault_write_reg(const char *prefix, reg_t reg)
{
    fputs(prefix, stdout);

    for (unsigned i = 0; i < sizeof(reg) * 2; i++) {
        unsigned digitval = 0xF & (reg >> ((sizeof(reg) * 8 - 4) - 4 * i));
        char     digitchr = '0' + digitval + (digitval >= 10 ? 'a' - '0' - 10 : 0);
        putchar(digitchr);
    }

    putchar('\n');
}

#define REASON_MEMORY_ERROR      0
#define REASON_INSTRUCTION_ERROR 1
#define REASON_MACHINE_CHECK     2
#define REASON_TLB_MISS_I        3
#define REASON_TLB_MISS_D        4
#define REASON_PROT_V            5
#define REASON_PRIVILEGE_V       6
#define REASON_SWI               7
#define REASON_TRAP              8
#define REASON_EXTENSION         9
#define REASON_DIV_ZERO          10
#define REASON_DC_ERROR          11
#define REASON_MALIGNED          12

#define _REASON(r)               #r
#define REASON(r)                _REASON(r)

static const char * const reasons[] = {
    "memory_error\n", "instruction_error\n", "machine_check\n", "tlb_miss_i\n", "tlb_miss_d\n",
    "prot_v\n",       "privilege_v\n",       "swi\n",           "trap\n",       "extension\n",
    "div_zero\n",     "dc_error\n",          "maligned\n",
};

static void __used
arc_fault(struct fault *f, int reason)
{
    int r;
    fputs("ARC fault: ", stdout);
    fputs(reasons[reason], stdout);
    char prefix[] = "\tR##:      0x";
    for (r = 0; r <= 31; r++) {
        prefix[2] = '0' + r / 10; /* overwrite # with register number */
        prefix[3] = '0' + r % 10; /* overwrite # with register number */
        arc_fault_write_reg(prefix, f->r[r]);
    }
    arc_fault_write_reg("\tERET:     0x", f->eret);
    arc_fault_write_reg("\tERBTA:    0x", f->erbta);
    arc_fault_write_reg("\tERSTATUS: 0x", f->erstatus);
    arc_fault_write_reg("\tECR:      0x", f->ecr);
    arc_fault_write_reg("\tEFA:      0x", f->efa);
    _exit(1);
}

#define VECTOR_COMMON                                                   \
    __asm__(PUSH "  r31\n " PUSH " r30\n " PUSH " r29\n " PUSH " r28"); \
    __asm__(PUSH "  r27\n " PUSH " r26\n " PUSH " r25\n " PUSH " r24"); \
    __asm__(PUSH "  r23\n " PUSH " r22\n " PUSH " r21\n " PUSH " r20"); \
    __asm__(PUSH "  r19\n " PUSH " r18\n " PUSH " r17\n " PUSH " r16"); \
    __asm__(PUSH "  r15\n " PUSH " r14\n " PUSH " r13\n " PUSH " r12"); \
    __asm__(PUSH "  r11\n " PUSH " r10\n " PUSH "  r9\n " PUSH "  r8"); \
    __asm__(PUSH "   r7\n " PUSH "  r6\n " PUSH "  r5\n " PUSH "  r4"); \
    __asm__(PUSH "   r3\n " PUSH "  r2\n " PUSH "  r1\n " PUSH "  r0"); \
    __asm__(LRR " r0, [0x404]\n " PUSH " r0");                          \
    __asm__(LRR " r0, [0x403]\n " PUSH " r0");                          \
    __asm__(LRR " r0, [0x402]\n " PUSH " r0");                          \
    __asm__(LRR " r0, [0x401]\n " PUSH " r0");                          \
    __asm__(LRR " r0, [0x400]\n " PUSH " r0");                          \
    __asm__("mov_s r0, sp")

void __naked __section(".init") arc_memory_error_vector(void)
{
    VECTOR_COMMON;
    __asm__("mov_s r1, " REASON(REASON_MEMORY_ERROR));
    __asm__("j arc_fault");
}

void __naked __section(".init") arc_instruction_error_vector(void)
{
    VECTOR_COMMON;
    __asm__("mov_s r1, " REASON(REASON_INSTRUCTION_ERROR));
    __asm__("j arc_fault");
}

void __naked __section(".init") arc_machine_check_vector(void)
{
    VECTOR_COMMON;
    __asm__("mov_s r1, " REASON(REASON_MACHINE_CHECK));
    __asm__("j arc_fault");
}

void __naked __section(".init") arc_tlb_miss_i_vector(void)
{
    VECTOR_COMMON;
    __asm__("mov_s r1, " REASON(REASON_TLB_MISS_I));
    __asm__("j arc_fault");
}

void __naked __section(".init") arc_tlb_miss_d_vector(void)
{
    VECTOR_COMMON;
    __asm__("mov_s r1, " REASON(REASON_TLB_MISS_D));
    __asm__("j arc_fault");
}

void __naked __section(".init") arc_prot_v_vector(void)
{
    VECTOR_COMMON;
    __asm__("mov_s r1, " REASON(REASON_PROT_V));
    __asm__("j arc_fault");
}

void __naked __section(".init") arc_privilege_v_vector(void)
{
    VECTOR_COMMON;
    __asm__("mov_s r1, " REASON(REASON_PRIVILEGE_V));
    __asm__("j arc_fault");
}

void __naked __section(".init") arc_swi_vector(void)
{
    VECTOR_COMMON;
    __asm__("mov_s r1, " REASON(REASON_SWI));
    __asm__("j arc_fault");
}

void __naked __section(".init") arc_trap_vector(void)
{
    VECTOR_COMMON;
    __asm__("mov_s r1, " REASON(REASON_TRAP));
    __asm__("j arc_fault");
}

void __naked __section(".init") arc_extension_vector(void)
{
    VECTOR_COMMON;
    __asm__("mov_s r1, " REASON(REASON_EXTENSION));
    __asm__("j arc_fault");
}

void __naked __section(".init") arc_div_zero_vector(void)
{
    VECTOR_COMMON;
    __asm__("mov_s r1, " REASON(REASON_DIV_ZERO));
    __asm__("j arc_fault");
}

void __naked __section(".init") arc_dc_error_vector(void)
{
    VECTOR_COMMON;
    __asm__("mov_s r1, " REASON(REASON_DC_ERROR));
    __asm__("j arc_fault");
}

void __naked __section(".init") arc_maligned_vector(void)
{
    VECTOR_COMMON;
    __asm__("mov_s r1, " REASON(REASON_MALIGNED));
    __asm__("j arc_fault");
}
#endif

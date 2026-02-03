/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2026 MIPS Holding, Inc.
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

static void __used __section(".init")
_cstart(void)
{
    __start();
}

extern char __stack[];
extern void(* const __vector_table[]);
/**
 * Below 2 different macros are defined for MOV:
 *
 *     1. MOV. For 64-bit targets it uses movl and for 32-bit targets
 *        it uses mov_s. For this crt0 it's enough to pack all values
 *        to fit in encodings. We dont's used movl_s for 64-bit by default
 *        because it does not support the same range of registers as
 *        mov_s for 32-bit targets.
 *     2. MOV_S. This macro must be used if we are sure that everything
 *        will fit in mov_s/movl_s.
 */
#ifdef __ARC64_ARCH64__
#define SRR  "srl"
#define LRR  "lrl"
#define PUSH "pushl"
#define MOV "movl"
#define MOV_S "movl_s"
typedef uint64_t reg_t;
#else
#define SRR  "sr"
#define LRR  "lr"
#define PUSH "push"
#define MOV "mov_s"
#define MOV_S "mov_s"
typedef uint32_t reg_t;
#endif

void __naked __section(".init") __used
_start(void)
{
    /**
     * Set STATUS32.AD bit to enable referencing unaligned
     * memory addresses
     */
    __asm__(LRR " r2, [0xA]\n"
            "bset r2, r2, 19\n"
            "flag r2");

    /* Set up the stack pointer and the vector table */
#ifdef __ARC64_ARCH64__
    /**
     * 64-bit absolute addresses are loaded with a pair of instructions
     * which have an associated pair of relocations: R_ARC_LO32_ME and
     * R_ARC_HI32_ME.
     */
    __asm__("movhl_s sp, %0" : : "i"(__stack));
    __asm__("orl_s sp, sp, %0" : : "i"(__stack));

    __asm__("movhl_s r2, %0" : : "i"(__vector_table));
    __asm__("orl_s r2, r2, %0" : : "i"(__vector_table));
    __asm__("srl r2, [0x25]");
#else
    __asm__("mov_s sp, %0" : : "i"(__stack));
    __asm__("sr %0, [0x25]" : : "i"(__vector_table));
#endif

    /* Clear the registers except r26 (GP) and r28 (SP) */
    __asm__(MOV " r0, 0\n"
            MOV " r1, 0\n"
            MOV " r2, 0\n"
            MOV " r3, 0\n"
            MOV " r4, 0\n"
            MOV " r5, 0\n"
            MOV " r6, 0\n"
            MOV " r7, 0\n"
            MOV " r8, 0\n"
            MOV " r9, 0\n"
            MOV " r10, 0\n"
            MOV " r11, 0\n"
            MOV " r12, 0\n"
            MOV " r13, 0\n"
            MOV " r14, 0\n"
            MOV " r15, 0\n"
            MOV " r16, 0\n"
            MOV " r17, 0\n"
            MOV " r18, 0\n"
            MOV " r19, 0\n"
            MOV " r20, 0\n"
            MOV " r21, 0\n"
            MOV " r22, 0\n"
            MOV " r23, 0\n"
            MOV " r24, 0\n"
            MOV " r25, 0\n"
            MOV " r27, 0\n"
            MOV " ilink, 0\n"
            MOV " r30, 0\n");

    /* Branch to C code */
    __asm__("j _cstart");

    /* Fill in any delay slot */
    __asm__("nop");
}

#ifdef CRT0_SEMIHOST_TRAP

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
    __asm__(MOV " r0, sp")

void __naked __section(".init")
arc_memory_error_vector(void)
{
    VECTOR_COMMON;
    __asm__(MOV_S " r1, " REASON(REASON_MEMORY_ERROR));
    __asm__("j arc_fault");
}

void __naked __section(".init")
arc_instruction_error_vector(void)
{
    VECTOR_COMMON;
    __asm__(MOV_S " r1, " REASON(REASON_INSTRUCTION_ERROR));
    __asm__("j arc_fault");
}

void __naked __section(".init")
arc_machine_check_vector(void)
{
    VECTOR_COMMON;
    __asm__(MOV_S " r1, " REASON(REASON_MACHINE_CHECK));
    __asm__("j arc_fault");
}

void __naked __section(".init")
arc_tlb_miss_i_vector(void)
{
    VECTOR_COMMON;
    __asm__(MOV_S " r1, " REASON(REASON_TLB_MISS_I));
    __asm__("j arc_fault");
}

void __naked __section(".init")
arc_tlb_miss_d_vector(void)
{
    VECTOR_COMMON;
    __asm__(MOV_S " r1, " REASON(REASON_TLB_MISS_D));
    __asm__("j arc_fault");
}

void __naked __section(".init")
arc_prot_v_vector(void)
{
    VECTOR_COMMON;
    __asm__(MOV_S " r1, " REASON(REASON_PROT_V));
    __asm__("j arc_fault");
}

void __naked __section(".init")
arc_privilege_v_vector(void)
{
    VECTOR_COMMON;
    __asm__(MOV_S " r1, " REASON(REASON_PRIVILEGE_V));
    __asm__("j arc_fault");
}

void __naked __section(".init")
arc_swi_vector(void)
{
    VECTOR_COMMON;
    __asm__(MOV_S " r1, " REASON(REASON_SWI));
    __asm__("j arc_fault");
}

void __naked __section(".init")
arc_trap_vector(void)
{
    VECTOR_COMMON;
    __asm__(MOV_S " r1, " REASON(REASON_TRAP));
    __asm__("j arc_fault");
}

void __naked __section(".init")
arc_extension_vector(void)
{
    VECTOR_COMMON;
    __asm__(MOV_S " r1, " REASON(REASON_EXTENSION));
    __asm__("j arc_fault");
}

void __naked __section(".init")
arc_div_zero_vector(void)
{
    VECTOR_COMMON;
    __asm__(MOV_S " r1, " REASON(REASON_DIV_ZERO));
    __asm__("j arc_fault");
}

void __naked __section(".init")
arc_dc_error_vector(void)
{
    VECTOR_COMMON;
    __asm__(MOV_S " r1, " REASON(REASON_DC_ERROR));
    __asm__("j arc_fault");
}

void __naked __section(".init")
arc_maligned_vector(void)
{
    VECTOR_COMMON;
    __asm__(MOV_S " r1, " REASON(REASON_MALIGNED));
    __asm__("j arc_fault");
}
#endif

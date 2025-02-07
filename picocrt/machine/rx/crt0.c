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

#define _DEFAULT_SOURCE
#include "../../crt0.h"

extern char __stack[];

extern const void *__exception_vector[];
extern const void *__interrupt_vector[];
extern char _pid_base[];
extern char _gp[];

#ifdef _RX_PID
#define PID_REG "r13"
#ifdef _RX_SMALL_DATA
#define SMALL_DATA_REG "r12"
#endif
#else
#ifdef _RX_SMALL_DATA
#define SMALL_DATA_REG "r13"
#endif
#endif

void __section(".text.startup") __attribute__((used))
_start(void)
{
    __asm__("mov %0, r0" : : "i" (__stack));
    /* Not present in RXv1 */
    /* __asm__("mvtc %0, extb" : : "i" (__exception_vector)); */
    __asm__(".equ __my_exception_vector, ___exception_vector");
    __asm__("mvtc %0, intb" : : "i" (__interrupt_vector));
    __asm__("mvtc #0, psw");
#ifdef _RX_PID
    __asm__("mov %0, " PID_REG : : "i" (_pid_base));
#endif
#ifdef _RX_SMALL_DATA
    __asm__("mov %0, " SMALL_DATA_REG : : "i" (_gp));
#endif
    /* Enable the DN bit - this should have been done for us by
       the CPU reset, but it is best to make sure for ourselves.  */
    __asm__("mvtc    #0x100, fpsw");
    __start();
}

#ifdef CRT0_SEMIHOST

#include <unistd.h>
#include <stdio.h>

/* Trap exceptions, print message and exit when running under semihost */

static const char *const exceptions[] = {
    "privilege\n",
    "access\n",
    "illegal\n",
    "address\n",
    "fpu\n",
    "nmi\n",
};

struct fault {
    uint32_t    r[15];  /* R1-R15 */
    uint32_t    pc;
    uint32_t    psw;
};

static void rx_fault_write_reg(const char *prefix, uint32_t reg)
{
    fputs(prefix, stdout);

    for (uint32_t i = 0; i < 8; i++) {
        uint32_t digitval = 0xF & (reg >> (28 - 4*i));
        char digitchr = '0' + digitval + (digitval >= 10 ? 'a'-'0'-10 : 0);
        putchar(digitchr);
    }

    putchar('\n');
}

static void __attribute__((used))
rx_fault(struct fault *f, int exception)
{
    int r;
    fputs("RX fault: ", stdout);
    fputs(exceptions[exception], stdout);
    char prefix[] = "\tR##:   0x";
    for (r = 1; r <= 15; r++) {
        prefix[2] = '0' + r / 10;    /* overwrite # with register number */
        prefix[3] = '0' + r % 10;    /* overwrite # with register number */
        rx_fault_write_reg(prefix, f->r[r-1]);
    }
    rx_fault_write_reg("\tPC:    0x", f->pc);
    rx_fault_write_reg("\tPSW:   0x", f->psw);
    _exit(1);
}

void rx_privilege_exception(void)
{
    __asm__("pushm r1-r15");
    __asm__("mov r0, r1");
    __asm__("mov #0, r2");
    __asm__("bra _rx_fault");
}

void rx_access_exception(void)
{
    __asm__("pushm r1-r15");
    __asm__("mov r0, r1");
    __asm__("mov #1, r2");
    __asm__("bra _rx_fault");
}

void rx_illegal_exception(void)
{
    __asm__("pushm r1-r15");
    __asm__("mov r0, r1");
    __asm__("mov #2, r2");
    __asm__("bra _rx_fault");
}

void rx_address_exception(void)
{
    __asm__("pushm r1-r15");
    __asm__("mov r0, r1");
    __asm__("mov #3, r2");
    __asm__("bra _rx_fault");
}

void rx_fpu_exception(void)
{
    __asm__("pushm r1-r15");
    __asm__("mov r0, r1");
    __asm__("mov #4, r2");
    __asm__("bra _rx_fault");
}

void rx_nmi_exception(void)
{
    __asm__("pushm r1-r15");
    __asm__("mov r0, r1");
    __asm__("mov #5, r2");
    __asm__("bra _rx_fault");
}

#endif

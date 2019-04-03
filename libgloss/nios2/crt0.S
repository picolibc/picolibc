/* crt0.S -- startup code for Nios II QEMU generic-nommu board emulation.

   Copyright (c) 2018-2019 Mentor Graphics

   The authors hereby grant permission to use, copy, modify, distribute,
   and license this software and its documentation for any purpose, provided
   that existing copyright notices are retained in all copies and that this
   notice is included verbatim in any distributions. No written agreement,
   license, or royalty fee is required for any of the authorized uses.
   Modifications to this software may be copyrighted by their authors
   and need not follow the licensing terms described here, provided that
   the new terms are clearly indicated on the first page of each file where
   they apply.
 
   Parts of this file were derived from code written by Altera Corporation
   and distributed under the following license:

   Copyright (c) 2006 Altera Corporation, San Jose, California, USA.
   All rights reserved.

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.

   This agreement shall be governed in all respects by the laws of the State
   of California and by the laws of the United States of America.
*/

/* Cache sizes.  */

#define NIOS2_ICACHE_LINE_SIZE 32
#define NIOS2_ICACHE_SIZE 32768
#define NIOS2_DCACHE_LINE_SIZE 32
#define NIOS2_DCACHE_SIZE 32768

/* External symbols.  */
        .globl __stack
        .globl __gp
	.globl __bss_start
	.globl __bss_end
	.weak exit

/* Processor reset code lives at the base of RAM.  At reset, only the cache
   line that contains the reset vector is initialized; the reset code has
   to fit in this cache line and initialize the remainder of the instruction
   cache.  */

        .section        .text.entry, "ax"
	.align 5
	.set noat

        .globl  __reset
        .type   __reset, @function
__reset:

	/* Initialize the icache.  */
        movui r2, NIOS2_ICACHE_SIZE
0:
        initi r2
        addi r2, r2, -NIOS2_ICACHE_LINE_SIZE
        bgt r2, zero, 0b
1:
        /* Jump to the __start entry point. */
        movhi r1, %hi(__start)
        ori r1, r1, %lo(__start)
        jmp r1

	.size __reset, . - __reset

/* Provide a stub interrupt handler that waits in a busy loop.
   The alignment puts it at offset 0x20 from the base of RAM.  */

	.align 5

        .globl  __interrupt_handler
        .type   __interrupt_handler, @function
__interrupt_handler:
0:
	br 0b

	.size __interrupt_handler, . - __interrupt_handler

/* __start is the ELF entry point.  */

        .section        .text.start, "ax"
	.align 4
	.globl __start
        .type   __start, @function

__start:	

        /* Initialize the data cache.  */
        movui r2, NIOS2_DCACHE_SIZE
0:
        initd 0(r2)
        addi r2, r2, -NIOS2_DCACHE_LINE_SIZE
        bgt r2, zero, 0b
1:

        /* Initialize the stack pointer.  */
        movhi sp, %hi(__stack)
        ori sp, sp, %lo(__stack)

        /* Initialize the GP.  */
        movhi gp, %hi(_gp)
        ori gp, gp, %lo(_gp)

	/* Clear BSS.  */
        movhi r2, %hi(__bss_start)
        ori r2, r2, %lo(__bss_start)
        movhi r3, %hi(__bss_end)
        ori r3, r3, %lo(__bss_end)
        beq r2, r3, 1f
0:
        stw zero, (r2)
        addi r2, r2, 4
        bltu r2, r3, 0b
1:

	/* Run initializers.  */
	call __libc_init_array

	/* Call the C entry point.  */
	mov r4, zero	/* argc */
	mov r5, zero	/* argv */
	call main

	/* If main returns, call exit if it is defined.  */
        movhi r3, %hi(exit)
        ori r3, r3, %lo(exit)
	beq r3, zero, 2f
	mov r4, r2	/* exit status from main */
	callr r3

	/* Otherwise sit in busy loop.  */
2:	
	br 2b
	.size __start, . - __start

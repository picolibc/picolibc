/*
 * crt0.S -- startup codes for CK-CPU.
 *
 * Define a exception vector table
 *
 * Initial PSR register.
 * Initial VBR register, relocate exception vector table.
 * Initial sp(stack pointer) register.
 * Initial MGU (for ckcpu having MGU)
 * Initial Cache.
 * Initial SPM (for ckcpu having SPM).
 * Initial BSS section to "0"
 * Then goto "main".
 *
 * Copyright (c) 2020  C-SKY Microsystems All rights reserved.
 */

.file "crt0.S"
/*
 *         MGU & CACHE macro from "ck5a6.h"
 *
 * CONFIG_CKCPU_MGU_BLOCKS	---- MGU block priority setting value
 * CONFIG_CKCPU_MGU_REGION(1-4)
 *                          ---- MGU (1-4) block base address and size.
 * CONFIG_CKCPU_ICACHE		---- Instruction cache enable
 * CONFIG_CKCPU_DCACHE		---- Data cache enable
 *
 *         SPM macro from "ck5a6.h"
 *
 * CONFIG_CKCPU_SPM		---- CKCPU having SPM ?
 * CONFIG_CKCPU_SPM_ENABLE	---- SPM Enable
 * CONFIG_CKCPU_ISPM_MODE	---- Instruction SPM Mode(cache/local memory)
 * CONFIG_CKCPU_DSPM_MODE	---- Data SPM Mode (cache/local memory)
 * CONFIG_CKCPU_ISPM_ADDRESS
 *                          ---- Instruction SPM external memory base address,
 *                               Just for Local Memory Mode
 * CONFIG_CKCPU_DSPM_ADDRESS
 *                          ---- Data SPM external memory base address, Just
 *				                 for Local Memory Mode
 * CONFIG_CKCPU_ISPM_TRANSFER
 *                          ---- The content in Instruction SPM must be
 *                               initialed, Just for Local Memory Mode
 * CONFIG_CKCPU_DSPM_TRANSFER
 *                          ---- The content in Data SPM must be initialed,
 *                               Just for Local Memory Mode 
 */

/*
 * For importing variable or functions from script file "ckcpu.ld".
 */
.import __stack		/* The stack pointer from linking script file */
.import __fstack	/* The fast interrupt stack from linking script file */
.import	__sbss		/* The end of bss from linking script file */
.import __ebss		/* The end of bss from linking script file */
.import __csky_exit	/* For QEMU exit.  */

/*
 * For importing variable or functions from other c or assemble files.
 */
.import main				/* The c codes entry */
.import spm_init			/* Initial ckcpu SPM */
.import	exit
.import __libc_init_array

/*
 * For importing variable or functions from entry.S.
 */ 
/* default exception service routine */
/*
.import default_exception_handler
*/

/* For exporting varialbe or functions */
.export	ckcpu_vsr_table	/* Vector table base address. */
.export __start				/* The system entry. */

.text
/* Vector table space. */
.align 10
ckcpu_vsr_table:
.rept 128
.long __default_exception_handler
.endr

/* The ckcpu startup codes. */
.text
	/*
	 *	
	 * This is the codes first entry point. This is where it all
	 * begins...
	 */
.long __start

__start:
	/*
	 * Init psr value, enable exception, disable interrupt and fast interrupt.
	 * psr = 0x80000100
	 */
	lrw	r7, 0
	bseti	r7, 31
	bseti	r7, 8
	mtcr	r7, psr
	
	/*
	 * Setup initial vector base table for interrupts and exceptions
	 */
	lrw		r6, ckcpu_vsr_table
	mtcr	r6, vbr
	
	/* Initialize the normal stack pointer from the linker definition. */
	lrw		r7, __stack
	mov		sp, r7

#ifdef FAST_INTERRUPT_ENABLE
	/* Initialize the fast interrupt stack pointer . */
	psrset	af
	lrw		r6, __fstack
	mov		sp, r6
	psrclr	af
#endif

	/*
	 * Initial CKCPU MGU & Cache
	 * Step 1: Disable MGU & Cache.
	 */
	movi	r7, 0
	mtcr	r7, cr18

#if 0
	/*
	 * Zero out the bss region.
	 * NOTE: __sbss must align 4
	 */
	lrw		r7, __sbss	/* Get start of bss from linking script file */
	lrw		r6, __ebss	/* Get end of bss from linking script file */
	subu	r6, r7		/* Calculate size of bss */
	lsri	r6, r6, 2		/* Size of whole words */
	cmpnei	r6, 0
	bf		__goto_c
	movi	r5, 0		/* Set zero value to write */
2:
	stw		r5, (r7)	/* Zero next word */
	addi	r7, 4		/* Increase bss pointer */
	subi	r6, 1			/* Decrease counter */
	cmpnei	r6, 0
	bt		2b			/* Repeat for all bss */
#endif

	/*
	 * Assember start up done, C codes start here.
	 */
__goto_c:
	lrw     r0, __libc_fini_array   # Register global termination functions
	jbsr    atexit                  #  to be called upon exit
	/*jsri	main*/
	lrw	r5, __libc_init_array
	jsr	r5
	/*Set the argc and argv to NULL.  */
	movi	r0, 0
	movi	r1, 0
	lrw	r5, main
	jsr	r5

	/* Should never get here. */
	jbsr	exit

__csky_exit:
	br __csky_exit

__default_exception_handler:
    lrw     r4, __csky_exit
    movi    r5, 0x2
    stw     r5, (r4, 0)
    br __default_exception_handler
    rte

.data
.align 4
__stack_bottom:
.rept  128
.long  0x0
.endr

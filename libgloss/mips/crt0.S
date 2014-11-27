/*
 * crt0.S -- startup file for MIPS.
 *
 * Copyright (c) 1995, 1996, 1997, 2001 Cygnus Support
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

#ifdef __mips16
/* This file contains 32 bit assembly code.  */
	.set nomips16
#endif

#include "regs.S"

/*
 * Set up some room for a stack. We just grab a chunk of memory.
 */
#define STACK_SIZE  0x4000
#define GLOBAL_SIZE 0x2000

#define STARTUP_STACK_SIZE	0x0100

/* This is for referencing addresses that are not in the .sdata or
   .sbss section under embedded-pic, or before we've set up gp.  */
#ifdef __mips_embedded_pic
# ifdef __mips64
#  define LA(t,x) la t,x-PICBASE ; daddu t,s0,t
# else
#  define LA(t,x) la t,x-PICBASE ; addu t,s0,t
# endif
#else /* __mips_embedded_pic */
# define LA(t,x) la t,x
#endif /* __mips_embedded_pic */

	.comm	__memsize, 12
	.comm	__lstack, STARTUP_STACK_SIZE

	.text
	.align	2

/* Without the following nop, GDB thinks _start is a data variable.
 * This is probably a bug in GDB in handling a symbol that is at the
 * start of the .text section.
 */
	nop

	.globl	hardware_hazard_hook .text
	.globl	_start
	.ent	_start
_start:
#ifdef __mips_embedded_pic
#define PICBASE start_PICBASE
	.set	noreorder
	PICBASE = .+8
        bal	PICBASE
	nop
	move	s0,$31
	.set	reorder
#endif
#if __mips<3
#  define STATUS_MASK (SR_CU1|SR_PE)
#else
/* Post-mips2 has no SR_PE bit.  */
#  ifdef __mips64
/* Turn on 64-bit addressing and additional float regs.  */
#    define STATUS_MASK (SR_CU1|SR_FR|SR_KX|SR_SX|SR_UX)
#  else
#    if __mips_fpr==32
#      define STATUS_MASK (SR_CU1)
#    else
/* Turn on additional float regs.  */
#      define STATUS_MASK (SR_CU1|SR_FR)
#    endif
#  endif
#endif
	li	v0, STATUS_MASK
	mtc0	v0, C0_SR
	mtc0	zero, C0_CAUSE
	nop

	/* Avoid hazard from FPU enable and other SR changes.  */
	LA (t0, hardware_hazard_hook)
	beq	t0,zero,1f
	jalr	t0
1:

/* Check for FPU presence.  Don't check if we know that soft_float is
   being used.  (This also avoids illegal instruction exceptions.)  */

#ifndef __mips_soft_float
	li	t2,0xAAAA5555
	mtc1	t2,fp0		/* write to FPR 0 */
	mtc1	zero,fp1	/* write to FPR 1 */
	mfc1	t0,fp0
	mfc1	t1,fp1
	nop
	bne	t0,t2,1f	/* check for match */
	bne	t1,zero,1f	/* double check */
	j	2f		/* FPU is present. */
#endif
1:
	/* FPU is not present.  Set status register to say that. */
	li	v0, (STATUS_MASK-(STATUS_MASK & SR_CU1))
	mtc0	v0, C0_SR
	nop
	/* Avoid hazard from FPU disable.  */
	LA (t0, hardware_hazard_hook)
	beq	t0,zero,2f
	jalr	t0
2:


/* Fix high bits, if any, of the PC so that exception handling
   doesn't get confused.  */
	LA (v0, 3f)
	jr	v0
3:
	LA (gp, _gp)				# set the global data pointer
	.end _start

/*
 * zero out the bss section.
 */
	.globl	__memsize
	.globl	get_mem_info .text
	.globl	__stack
	.globl	__global
	.ent	zerobss
zerobss:
	LA (v0, _fbss)
	LA (v1, _end)
	beq	v0,v1,2f
1:
	addiu	v0,v0,4
	sw	zero,-4(v0)
	bne	v0,v1,1b
2:
	la	t0, __lstack			# make a small stack so we
	addiu	sp, t0, STARTUP_STACK_SIZE	# can run some C code
	la	a0, __memsize			# get the usable memory size
	jal	get_mem_info

	/* setup the stack pointer */
	LA (t0, __stack)			# is __stack set ?
	bne	t0,zero,4f

	/* NOTE: a0[0] contains the amount of memory available, and
	         not the last memory address. */
	la	a0, __memsize
	lw	t0,0(a0)			# last address of memory available
	la	t1,K0BASE			# cached kernel memory
	addu	t0,t0,t1			# get the end of memory address
	/* Allocate 32 bytes for the register parameters.  Allocate 16
	   bytes for a null argv and envp.  Round the result up to 64
	   bytes to preserve alignment.  */
	subu	t0,t0,64
4:
	move	sp,t0				# set stack pointer
	.end	zerobss

/*
 * initialize target specific stuff. Only execute these
 * functions it they exist.
 */
	.globl	hardware_init_hook .text
	.globl	software_init_hook .text
	.type	_fini,@function
	.type	_init,@function
	.globl	atexit .text
	.globl	exit .text
	.ent	init
init:
	LA (t9, hardware_init_hook)		# init the hardware if needed
	beq	t9,zero,6f
	jalr	t9
6:
	LA (t9, software_init_hook)		# init the hardware if needed
	beq	t9,zero,7f
	jalr	t9
7:
	LA (a0, _fini)
	jal	atexit

#ifdef GCRT0
	.globl	_ftext
	.globl	_extext
	LA (a0, _ftext)
	LA (a1, _etext)
	jal	monstartup
#endif


	jal	_init				# run global constructors

	addiu	a1,sp,32			# argv = sp + 32
	addiu	a2,sp,40			# envp = sp + 40
#if __mips64
	sd	zero,(a1)			# argv[argc] = 0
	sd	zero,(a2)			# envp[0] = 0
#else
	sw	zero,(a1)
	sw	zero,(a2)
#endif
	move	a0,zero				# set argc to 0
	jal	main				# call the program start function

	# fall through to the "exit" routine
	move	a0,v0				# pass through the exit code
	jal	exit				# call libc exit to run the G++
						# destructors
	.end	init

 
/* Assume the PICBASE set up above is no longer valid below here.  */
#ifdef __mips_embedded_pic
#undef PICBASE
#endif
	
/*
 * _exit -- Exit from the application. Normally we cause a user trap
 *          to return to the ROM monitor for another run. NOTE: This is
 *	    the only other routine we provide in the crt0.o object, since
 *          it may be tied to the "_start" routine. It also allows
 *          executables that contain a complete world to be linked with
 *          just the crt0.o object.
 */
	.globl	hardware_exit_hook .text
	.globl	_exit
	.ent _exit
_exit:
7:
#ifdef __mips_embedded_pic
	/* Need to reinit PICBASE, since we might be called via exit()
	   rather than via a return path which would restore old s0.  */
#define PICBASE exit_PICBASE
	.set	noreorder
	PICBASE = .+8
	bal	PICBASE
	nop
	move	s0,$31
	.set	reorder
#endif
#ifdef GCRT0
	LA (t0, _mcleanup)
	jalr	t0
#endif
	LA (t0, hardware_exit_hook)
	beq	t0,zero,1f
	jalr	t0
1:

	# break instruction can cope with 0xfffff, but GAS limits the range:
	break	1023
	b	7b				# but loop back just in-case
	.end _exit
 
/* Assume the PICBASE set up above is no longer valid below here.  */
#ifdef __mips_embedded_pic
#undef PICBASE
#endif

/* EOF crt0.S */

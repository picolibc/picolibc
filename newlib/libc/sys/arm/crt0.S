#include "newlib.h"
#include "arm.h"
#include "swi.h"

/* ANSI concatenation macros.  */
#define CONCAT(a, b) CONCAT2(a, b)
#define CONCAT2(a, b) a ## b

#ifdef __USER_LABEL_PREFIX__
#define FUNCTION( name ) CONCAT (__USER_LABEL_PREFIX__, name)
#else
#error __USER_LABEL_PREFIX is not defined
#endif

#ifdef HAVE_INITFINI_ARRAY
#define _init	__libc_init_array
#define _fini	__libc_fini_array
#endif

#if defined(__ARM_EABI__) && defined(__thumb__) && !defined(__thumb2__)
/* For Thumb1 we need to force the architecture to be sure that we get the
   correct attributes on the object file; otherwise the assembler will get
   confused and mark the object as being v6T2.  */
#if defined(__ARM_ARCH_4T__)
	.arch armv4t
#elif defined(__ARM_ARCH_5T__) || defined(__ARM_ARCH_5TE__)
	/* Nothing in this object requires higher than v5.  */
	.arch armv5t
#elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) \
	|| defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) \
	|| defined(__ARM_ARCH_6ZK__)
	/* Nothing in this object requires higher than v6.  */
	.arch armv6
#elif defined(__ARM_ARCH_6M__)
#ifdef ARM_RDP_MONITOR
	/* Object file uses SVC, so mark as v6s-m.  */
	.arch armv6s-m
#else
	.arch armv6-m
#endif
#endif
#endif

/* .text is used instead of .section .text so it works with arm-aout too.  */
	.text
	.syntax unified
#ifdef PREFER_THUMB
	.thumb
.macro FUNC_START name
	.global \name
	.thumb_func
\name:
.endm	
#else
	.code 32
.macro FUNC_START name
	.global	\name
\name:
.endm	
#endif

.macro indirect_call reg
#ifdef HAVE_CALL_INDIRECT
	blx \reg
#else
	mov	lr, pc
	mov	pc, \reg
#endif
.endm

	.align 	0

	FUNC_START	_mainCRTStartup
	FUNC_START	_start
#if defined(__ELF__) && !defined(__USING_SJLJ_EXCEPTIONS__)
	/* Annotation for EABI unwinding tables.  */
	.fnstart
#endif

/* Start by setting up a stack.  */
#ifdef ARM_RDP_MONITOR
	/*  Issue Demon SWI to read stack info.  */
	swi	SWI_GetEnv	/*  Returns command line in r0.  */
	mov	sp,r1		/*  and the highest memory address in r1.  */

	/*  Stack limit is at end of data.  */
	/*  Allow slop for stack overflow handling and small frames.  */
#ifdef __ARM_ARCH_6M__
	ldr	r0, .LC2
	adds	r0, #128
	adds	r0, #128
	mov	sl, r0
#else
	ldr	sl, .LC2
	add	sl, sl, #256
#endif
#else
#ifdef ARM_RDI_MONITOR
	/*  Issue Angel SWI to read stack info.  */
	movs	r0, #AngelSWI_Reason_HeapInfo
	adr	r1, .LC0	/*  Point at ptr to 4 words to receive data.  */
#ifdef THUMB_VXM
	bkpt	AngelSWI
#elif defined(__thumb2__)
	/*  We are in thumb mode for startup on armv7 architectures.  */
	AngelSWIAsm	AngelSWI
#else
	/*  We are always in ARM mode for startup on pre armv7 archs.  */
	AngelSWIAsm	AngelSWI_ARM
#endif
	ldr	r0, .LC0	/*  Point at values read.  */

	ldr     r1, [r0, #0]
	cmp     r1, #0
	bne     .LC32
	/* If the heap base value [r0, #0] is 0 then the heap base is actually 
	   at the end of program data (i.e. __end__). See:
           http://infocenter.arm.com/help/topic/com.arm.doc.dui0471-/Bacbefaa.html
	   for more information.  */
	ldr     r1, .LC31
	str     r1, [r0, #0]
.LC32:	
	ldr	r1, [r0, #8]
	ldr	r2, [r0, #12]
	/*  We skip setting sp/sl if 0 returned from semihosting. 
	    - According to semihosting docs, if 0 returned from semihosting,
	      the system was unable to calculate the real value, so it's ok
	      to skip setting sp/sl to 0 here.
	    - Considering M-profile processors, We might want to initialize
	      sp by the first entry of vector table and return 0 to SYS_HEAPINFO
	      semihosting call, which will be skipped here.  */
	cmp	r1, #0
	beq	.LC26
	mov	sp, r1
.LC26:
	cmp	r2, #0
	beq	.LC27

	/*  Allow slop for stack overflow handling and small frames.  */
#ifdef __ARM_ARCH_6M__
	adds	r2, #128
	adds	r2, #128
	mov	sl, r2
#else
	add	sl, r2, #256
#endif

.LC27:
#else
	/*  Set up the stack pointer to a fixed value.  */
	/*  Changes by toralf:
	    - Allow linker script to provide stack via __stack symbol - see
	      defintion of .Lstack
	    - Provide "hooks" that may be used by the application to add
	      custom init code - see .Lhwinit and .Lswinit	
	    - Go through all execution modes and set up stack for each of them.
	      Loosely based on init.s from ARM/Motorola example code.
              Note: Mode switch via CPSR is not allowed once in non-privileged
		    mode, so we take care not to enter "User" to set up its sp,
		    and also skip most operations if already in that mode.  */

	ldr	r3, .Lstack
	cmp	r3, #0
#ifdef __thumb2__
	it	eq
#endif	
#ifdef __ARM_ARCH_6M__
	bne	.LC28
	ldr	r3, .LC0
.LC28:
#else
	ldreq	r3, .LC0
#endif
	/* Note: This 'mov' is essential when starting in User, and ensures we
		 always get *some* sp value for the initial mode, even if we 
		 have somehow missed it below (in which case it gets the same
		 value as FIQ - not ideal, but better than nothing).  */
	mov	sp, r3
#ifdef PREFER_THUMB
	/* XXX Fill in stack assignments for interrupt modes.  */
#else
	mrs	r2, CPSR
	tst	r2, #0x0F	/* Test mode bits - in User of all are 0.  */
	beq	.LC23		/* "eq" means r2 AND #0x0F is 0.  */
	msr     CPSR_c, #0xD1	/* FIRQ mode, interrupts disabled.  */
	mov 	sp, r3
	sub	sl, sp, #0x1000	/* This mode also has its own sl (see below).  */
	
	mov	r3, sl	
	msr     CPSR_c, #0xD7	/* Abort mode, interrupts disabled.  */
	mov	sp, r3
	sub	r3, r3, #0x1000

	msr     CPSR_c, #0xDB	/* Undefined mode, interrupts disabled.  */
	mov	sp, r3
	sub	r3, r3, #0x1000

	msr     CPSR_c, #0xD2	/* IRQ mode, interrupts disabled.  */
	mov	sp, r3
	sub	r3, r3, #0x2000
		
	msr     CPSR_c, #0xD3	/* Supervisory mode, interrupts disabled.  */

	mov	sp, r3
	sub	r3, r3, #0x8000	/* Min size 32k.  */
	bic	r3, r3, #0x00FF	/* Align with current 64k block.  */
	bic	r3, r3, #0xFF00

	str	r3, [r3, #-4]	/* Move value into user mode sp without */ 
	ldmdb	r3, {sp}^       /* changing modes, via '^' form of ldm.  */ 
	orr	r2, r2, #0xC0	/* Back to original mode, presumably SVC, */
	msr	CPSR_c, r2	/* with FIQ/IRQ disable bits forced to 1.  */
#endif	
.LC23:
	/* Setup a default stack-limit in-case the code has been
	   compiled with "-mapcs-stack-check".  Hard-wiring this value
	   is not ideal, since there is currently no support for
	   checking that the heap and stack have not collided, or that
	   this default 64k is enough for the program being executed.
	   However, it ensures that this simple crt0 world will not
	   immediately cause an overflow event:  */
#ifdef __ARM_ARCH_6M__
	movs	r2, #64
	lsls	r2, r2, #10
	subs	r2, r3, r2
	mov	sl, r2
#else
	sub	sl, r3, #64 << 10	/* Still assumes 256bytes below sl.  */
#endif
#endif
#endif
	/* Zero the memory in the .bss section.  */
	movs 	a2, #0			/* Second arg: fill value.  */
	mov	fp, a2			/* Null frame pointer.  */
	mov	r7, a2			/* Null frame pointer for Thumb.  */
	
	ldr	a1, .LC1		/* First arg: start of memory block.  */
	ldr	a3, .LC2	
	subs	a3, a3, a1		/* Third arg: length of block.  */
	

#if __thumb__ && !defined(PREFER_THUMB)
	/* Enter Thumb mode...  */
	add	a4, pc, #1	/* Get the address of the Thumb block.  */
	bx	a4		/* Go there and start Thumb decoding.  */

	.code 16
	.global __change_mode
	.thumb_func
__change_mode:	
#endif
	
	bl	FUNCTION (memset)
#if !defined (ARM_RDP_MONITOR) && !defined (ARM_RDI_MONITOR)
/* Changes by toralf: Taken from libgloss/m68k/crt0.S
   initialize target specific stuff. Only execute these
   functions it they exist.  */
	ldr	r3, .Lhwinit
	cmp	r3, #0
	beq	.LC24
	indirect_call r3
.LC24:	
	ldr	r3, .Lswinit
	cmp	r3, #0
	beq	.LC25
	indirect_call r3

.LC25:	
	movs	r0, #0		/* No arguments.  */
	movs	r1, #0		/* No argv either.  */
#else
	/* Need to set up standard file handles.  */
	bl	FUNCTION (initialise_monitor_handles)
	
#ifdef ARM_RDP_MONITOR
	swi	SWI_GetEnv	/* Sets r0 to point to the command line.  */
	movs	r1, r0
#else
	movs	r0, #AngelSWI_Reason_GetCmdLine
	adr	r1, .LC30	/* Space for command line.  */
#ifdef THUMB_VXM
	bkpt	AngelSWI
#else
 	AngelSWIAsm	AngelSWI
#endif
	ldr	r1, .LC30
#endif
	/*  Parse string at r1.  */
	movs	r0, #0		/* Count of arguments so far.  */
	/* Push a NULL argument onto the end of the list.  */
#ifdef __thumb__
	push	{r0}
#else
	stmfd	sp!, {r0}
#endif
.LC10:
/*  Skip leading blanks.  */
#ifdef __thumb__
	ldrb	r3, [r1]
	adds	r1, #1
#else
	ldrb	r3, [r1], #1
#endif
	cmp	r3, #0
	beq	.LC12
	cmp	r3, #' '
	beq	.LC10

/* See whether we are scanning a string.  */
	cmp	r3, #'\"'
#ifdef __thumb__
	beq	.LC20
	cmp	r3, #'\''
	bne	.LC21
.LC20:
	movs	r2, r3
	b	.LC22

.LC21:
	movs	r2, #' '	/* Terminator type.  */
	subs	r1, r1, #1	/* Adjust back to point at start char.  */
.LC22:
#else
	cmpne	r3, #'\''
	moveq	r2, r3
	movne	r2, #' '	/* Terminator type.  */
	subne	r1, r1, #1	/* Adjust back to point at start char.  */
#endif

/*  Stack a pointer to the current argument.  */
#ifdef __thumb__
	push	{r1}
#else
	stmfd	sp!, {r1}
#endif
	adds	r0, r0, #1
.LC11:
#ifdef __thumb__
	ldrb	r3, [r1]
	adds	r1, #1
#else
	ldrb	r3, [r1], #1
#endif
	cmp	r3, #0
	beq	.LC12
	cmp	r2, r3		/* Reached terminator ?  */
	bne	.LC11
	movs	r2, #0
	subs	r3, r1, #1
	strb	r2, [r3]	/* Terminate the arg string.  */
	b	.LC10

.LC12:
	mov	r1, sp		/* Point at stacked arg pointers.  */
	/* We've now got the stacked args in order, reverse them.  */
#ifdef __thumb__
	movs	r2, r0
	lsls	r2, #2
	add	r2, sp
	mov	r3, sp
.LC15:	cmp	r2, r3
	bls	.LC14
	subs	r2, #4
	ldr	r4, [r2]
	ldr	r5, [r3]
	str	r5, [r2]
	str	r4, [r3]
	adds	r3, #4
	b	.LC15
.LC14:	
	/* Ensure doubleword stack alignment.  */
	mov	r4, sp
	movs	r5, #7
	bics	r4, r5
	mov	sp, r4
#else
	add	r2, sp, r0, LSL #2	/* End of args.  */
	mov	r3, sp			/* Start of args.  */
.LC13:	cmp	r2, r3
	ldrhi	r4,[r2, #-4]		/* Reverse ends of list.  */
	ldrhi	r5, [r3]
	strhi	r5, [r2, #-4]!
	strhi	r4, [r3], #4
	bhi	.LC13
	/* Ensure doubleword stack alignment.  */
	bic	sp, sp, #7
#endif
#endif

#ifdef __USES_INITFINI__
	/* Some arm/elf targets use the .init and .fini sections
	   to create constructors and destructors, and for these
	   targets we need to call the _init function and arrange
	   for _fini to be called at program exit.  */
	movs	r4, r0
	movs	r5, r1
	ldr	r0, .Lfini
	bl	FUNCTION (atexit)
	bl	FUNCTION (_init)
	movs	r0, r4
	movs	r1, r5
#endif	
	bl	FUNCTION (main)

	bl	FUNCTION (exit)		/* Should not return.  */

#if __thumb__ && !defined(PREFER_THUMB)
	/* Come out of Thumb mode.  This code should be redundant.  */
	mov	a4, pc
	bx	a4

	.code 32
	.global change_back
change_back:
	/* Halt the execution.  This code should never be executed.  */
	/* With no debug monitor, this probably aborts (eventually).
	   With a Demon debug monitor, this halts cleanly.
	   With an Angel debug monitor, this will report 'Unknown SWI'.	 */
	swi	SWI_Exit
#endif
	
	/* For Thumb, constants must be after the code since only 
	   positive offsets are supported for PC relative addresses.  */	
	.align 0
.LC0:
#ifdef ARM_RDI_MONITOR
	.word	HeapBase
#else
#ifndef ARM_RDP_MONITOR
	/* Changes by toralf: Provide alternative "stack" variable whose value
	   may be defined externally; .Lstack will be used instead of .LC0 if
	   it points to a non-0 value. Also set up references to "hooks" that
           may be used by the application to provide additional init code.  */	
#ifdef __pe__
	.word	0x800000
#else
	.word	0x80000			/* Top of RAM on the PIE board.  */
#endif

.Lstack:	
	.word	__stack
.Lhwinit:	
	.word	FUNCTION (hardware_init_hook)
.Lswinit:
	.word	FUNCTION (software_init_hook)

	/* Set up defaults for the above variables in the form of weak symbols
	   - so that application will link correctly, and get value 0 in
	   runtime (meaning "ignore setting") for the variables, when the user
	   does not provide the symbols. (The linker uses a weak symbol if,
	   and only if, a normal version of the same symbol isn't provided
	   e.g. by a linker script or another object file).  */	

	.weak __stack
	.weak FUNCTION (hardware_init_hook) 
	.weak FUNCTION (software_init_hook)
#endif
	
#endif
#if defined(__ELF__) && !defined(__USING_SJLJ_EXCEPTIONS__)
	/* Protect against unhandled exceptions.  */
	.cantunwind
	.fnend
#endif
.LC1:
	.word	__bss_start__
.LC2:
	.word	__bss_end__
#ifdef __USES_INITFINI__
.Lfini:
	.word	FUNCTION(_fini)
#endif
#ifdef ARM_RDI_MONITOR
.LC30:
	.word	CommandLine
	.word	255
.LC31:
	.word	__end__

/*  Workspace for Angel calls.  */
	.data
/*  Data returned by monitor SWI.  */
.global	__stack_base__
HeapBase:	.word	0
HeapLimit:	.word	0
__stack_base__:	.word	0
StackLimit:	.word	0
CommandLine:	.space	256,0	/*  Maximum length of 255 chars handled.  */
#endif
	
#ifdef __pe__
	.section .idata$3
	.long	0,0,0,0,0,0,0,0
#endif

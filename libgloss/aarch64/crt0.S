/* Copyright (c) 2009, 2010, 2011, 2012 ARM Ltd.  All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:
 1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
 3. The name of the company may not be used to endorse or promote
    products derived from this software without specific prior written
    permission.

 THIS SOFTWARE IS PROVIDED BY ARM LTD ``AS IS'' AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL ARM LTD BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#include "newlib.h"
#include "svc.h"

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

/* In ELF64, the large addressing model is used and R_AARCH64_ABS64
   reloc is generated to relocate a 64-bit address.  Since 64-bit
   relocation is not available in ELF32, in order to have
   a single code path for both ELF64 and ELF32 classes, we synthesize
   a 64-bit relocation by using R_AARCH64_P32_ABS32 on one of the two
   .word directives, depending on the endianness.  */

.macro GEN_DWORD name
#if defined(__ILP32__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	.word \name
	.word 0
#elif defined(__ILP32__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	.word 0
	.word \name
#else
	.dword \name
#endif
.endm

/* Help tackle the pointer size difference between ELF64 and ELF32.  */
#ifdef __ILP32__
#define PTR_REG(n)	w##n
#define PTR_SIZE	4
#define PTR_LOG_SIZE	2
#else
#define PTR_REG(n)	x##n
#define PTR_SIZE	8
#define PTR_LOG_SIZE	3
#endif

	.text
.macro FUNC_START name
	.global \name
\name:
.endm

	.align	2

	FUNC_START	_mainCRTStartup
	FUNC_START	_start

/* Start by setting up a stack */
#ifdef ARM_RDI_MONITOR
	/*  Issue Angel SVC to read memory info.

	    ptr to ptr to 4 words to receive data.  */
	adr	x1, .LC0
	mov	w0, #AngelSVC_Reason_HeapInfo
	AngelSVCAsm AngelSVC

	/* Initialise the stack pointer */

	/* We currently choose to use the heap_limit field rather than
	   stack_base because the AEM validation model
	   returns sane values in the heap fields, but 0 in the stack
	   fields.  Note on the VE AEM model it is necessary to pass
	   command line options to the AEM in order to define the values
	   exposed here in the HeapInfo Angel call.  */
	ldr	x0, .LC0		/* point at returned values */
	ldr	x1, [x0, #8]		/* get heap_limit */

	/* Set __heap_limit.  */
#ifdef __ILP32__
	/* Sanity check on the __heap_limit.  */
	tst	x1, #0xffffffff00000000
	bne	.Linsanepar
#endif
	cmp     x1, xzr
	beq     .LC4
	adrp	x2, __heap_limit
	add	x2, x2, #:lo12:__heap_limit
	str     x1, [x2]
.LC4:

	ldr	x1, [x0]		/* get heap_base */
#ifdef __ILP32__
	/* Sanity check on the heap base.  */
	tst	x1, #0xffffffff00000000
	bne	.Linsanepar
#endif
	cmp     x1, xzr
	bne     .LC5
	/* If the heap base value [x0, #0] is 0 then the heap base is actually
	   at the end of program data (i.e. __end__) */
	ldr     x1, .LC3
	str     x1, [x0, #0]
.LC5:
	ldr	x1, [x0, #16] /* get stack_base */

#ifdef __ILP32__
	/* Sanity check on the stack_base.  */
	tst	x1, #0xffffffff00000000
	bne	.Linsanepar
#endif
	cmp	x1, xzr
	bne	.LC6
#endif
	ldr     x1, .Lstack /*  Set up the stack pointer to a fixed value */
.LC6:

	/* Ensure quad-word stack alignment.  */
	and	x0, x1, #~15
	mov	sp, x0

	/* Setup an initial dummy frame with saved fp=0 and saved lr=0 */
	mov	x29, 0
	stp	x29, x29, [sp, #-16]!
	mov	x29, sp

       /* Initialize exception vector table, flatmap, etc.  */
        bl      FUNCTION (_cpu_init_hook)

	/* Zero the memory in the .bss section.  */
	ldr	x0, .LC1		/* First arg: start of memory block */
	mov	w1, #0			/* Second arg: fill value */
	ldr	x2, .LC2
	sub	x2, x2, x0		/* Third arg: length of block */
	bl	FUNCTION (memset)

#ifdef ARM_RDI_MONITOR
	/* Need to set up standard file handles */
	bl	FUNCTION (initialise_monitor_handles)
#endif

	/* .init and .fini sections are used to create constructors
	   and destructors.  Here we call the _init function and arrange
	   for _fini to be called at program exit.  */
	ldr	x0, .Lfini
	bl	FUNCTION (atexit)

	bl	FUNCTION (_init)

#ifdef ARM_RDI_MONITOR
	/* Fetch and parse the command line.  */
	ldr	x1, .Lcmdline		/* Command line descriptor.  */
	mov	w0, #AngelSVC_Reason_GetCmdLine
	AngelSVCAsm AngelSVC
	ldr	x8, .Lcmdline
	ldr	x8, [x8]

	mov	x0, #0		/* argc */
	mov	x1, sp		/* argv */
	ldr	x2, .Lenvp	/* envp */

	/* Put NULL at end of argv array.  */
	str	PTR_REG (0), [x1, #-PTR_SIZE]!

	/* Skip leading blanks.  */
.Lnext: ldrb	w3, [x8], #1
	cbz	w3, .Lendstr
	cmp	w3, #' '
	b.eq	.Lnext

	mov	w4, #' '	/* Terminator is space.  */

	/* See whether we are scanning a quoted string by checking for
	   opening quote (" or ').  */
	subs	w9, w3, #'\"'
	sub	x8, x8, #1	/* Backup if no match.  */
	ccmp	w9, #('\'' - '\"'), 0x4 /* FLG_Z */, ne
	csel	w4, w3, w4, eq	/* Terminator = quote if match.  */
	cinc	x8, x8, eq

	/* Push arg pointer to argv, and bump argc.  */
	str	PTR_REG (8), [x1, #-PTR_SIZE]!
	add	x0, x0, #1

	/* Find end of arg string.  */
1:	ldrb	w3, [x8], #1
	cbz	w3, .Lendstr
	cmp	w4, w3		/* Reached terminator?  */
	b.ne	1b

	/* Terminate the arg string with NUL char.  */
	mov	w4, #0
	strb	w4, [x8, #-1]
	b	.Lnext

	/* Reverse argv array.  */
.Lendstr:
	add	x3, x1, #0			/* sp = &argv[0] */
	add	x4, x1, w0, uxtw #PTR_LOG_SIZE	/* ep = &argv[argc] */
	cmp	x4, x3
	b.lo	2f
1:	ldr	PTR_REG (5), [x4, #-PTR_SIZE]	/* PTR_REG (5) = ep[-1] */
	ldr	PTR_REG (6), [x3]		/* PTR_REG (6) = *sp */
	str	PTR_REG (6), [x4, #-PTR_SIZE]!	/* *--ep = PTR_REG (6) */
	str	PTR_REG (5), [x3], #PTR_SIZE	/* *sp++ = PTR_REG (5) */
	cmp	x4, x3
	b.hi	1b
2:
	/* Move sp to the 16B boundary below argv.  */
	and	x4, x1, ~15
	mov	sp, x4

#else
	mov	x0, #0	/* argc = 0 */
	mov	x1, #0	/* argv = NULL */
#endif

	bl	FUNCTION (main)

	b	FUNCTION (exit)		/* Cannot return.  */

#if defined (ARM_RDI_MONITOR) && defined (__ILP32__)
.Linsanepar:
	/* Exit with 1 if the parameter is not within the 32-bit address
	   space.  */
	mov	x1, ADP_Stopped_ApplicationExit & 0xff
	movk	x1, ADP_Stopped_ApplicationExit >> 16, lsl #16
	adrp	x0, HeapBase	/* Reuse to construct the parameter block.  */
	add	x0, x0, #:lo12:HeapBase
	str	x1, [x0]
	mov	x1, 1
	str	x1, [x0, #8]
	mov	w1, #AngelSVC_Reason_ReportException
	AngelSVCAsm AngelSVC
	b       .
#endif

/* Function initializing exception vector table, flatmap, etc.
   Declared as weak symbol so that user can override this definition
   by linking in their own version of the function.  */
	.weak FUNCTION (_cpu_init_hook)
FUNCTION (_cpu_init_hook):
	ret

	.align 3
#ifdef ARM_RDI_MONITOR
.LC0:
	GEN_DWORD HeapBase
.LC3:
	GEN_DWORD __end__
#endif
.Lstack:
	GEN_DWORD __stack
	.weak __stack

.LC1:
	GEN_DWORD __bss_start__
.LC2:
	GEN_DWORD __bss_end__
.Lfini:
	GEN_DWORD FUNCTION(_fini)
#ifdef ARM_RDI_MONITOR
.Lenvp:
	GEN_DWORD env
.Lcmdline:
	GEN_DWORD AngelSVCArgs
/*  Workspace for Angel calls.  */
	.data
	.align 3
/*  Data returned by monitor SVC.  */
#if defined(__ILP32__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	.set __stack_base__, StackBase + 4
#else
	.set __stack_base__, StackBase
#endif
	.global	__stack_base__
HeapBase:	.dword	0
HeapLimit:	.dword	0
StackBase:	.dword	0
StackLimit:	.dword	0
env:		.dword	0	/* Dummy environment array */
CommandLine:	.space	256,0	/*  Maximum length of 255 chars handled.  */
AngelSVCArgs:
	GEN_DWORD CommandLine
	.dword	255
#endif
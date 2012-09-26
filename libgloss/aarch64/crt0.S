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

	.text
.macro FUNC_START name
	.global \name
\name:
.endm

	.align	2

	FUNC_START	_mainCRTStartup
	FUNC_START	_start
	FUNC_START	start

/* Start by setting up a stack */

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
	ldr	x0, [x0, #8]		/* get heap_limit */

	/* Ensure quad-word stack alignment.  */
	and	x0, x0, #~15
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

	/* Need to set up standard file handles */
	bl	FUNCTION (initialise_monitor_handles)

	/* .init and .fini sections are used to create constructors
	   and destructors.  Here we call the _init function and arrange
	   for _fini to be called at program exit.  */
	ldr	x0, .Lfini
	bl	FUNCTION (atexit)

	bl	FUNCTION (_init)

	/* Fetch and parse the command line.  */
	adr	x1, .Lcmdline		/* Command line descriptor.  */
	mov	w0, #AngelSVC_Reason_GetCmdLine
	AngelSVCAsm AngelSVC
	ldr	x8, .Lcmdline

	mov	x0, #0		/* argc */
	mov	x1, sp		/* argv */
	ldr	x2, .Lenvp	/* envp */

	/* Put NULL at end of argv array.  */
	str	x0, [x1, #-8]!

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
	str	x8, [x1, #-8]!
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
	add	x3, x1, #0		/* sp = &argv[0] */
	add	x4, x1, w0, uxtw #3	/* ep = &argv[argc] */
	cmp	x4, x3
	b.lo	2f
1:	ldr	x5, [x4, #-8]		/* x5 = ep[-1] */
	ldr	x6, [x3]		/* x6 = *sp */
	str	x6, [x4, #-8]!		/* *--ep = x6 */
	str	x5, [x3], #8		/* *sp++ = x5 */
	cmp	x4, x3
	b.hi	1b
2:
	/* Move sp to the 16B boundary below argv.  */
	and	x4, x1, ~15
	mov	sp, x4

	bl	FUNCTION (main)

	b	FUNCTION (exit)		/* Cannot return.  */

/* Function initializing exception vector table, flatmap, etc.
   Declared as weak symbol so that user can override this definition
   by linking in their own version of the function.  */
	.weak FUNCTION (_cpu_init_hook)
FUNCTION (_cpu_init_hook):
	ret

	.align 3
.LC0:
	.dword	HeapBase
.LC1:
	.dword	__bss_start__
.LC2:
	.dword	__bss_end__
.Lfini:
	.dword	FUNCTION(_fini)
.Lenvp:
	.dword	env
.Lcmdline:
	.dword	CommandLine
	.dword	255

/*  Workspace for Angel calls.  */
	.data
	.align 3
/*  Data returned by monitor SVC.  */
	.global	__stack_base__
HeapBase:	.dword	0
HeapLimit:	.dword	0
__stack_base__:	.dword	0
StackLimit:	.dword	0
env:		.dword	0	/* Dummy environment array */
CommandLine:	.space	256,0	/*  Maximum length of 255 chars handled.  */

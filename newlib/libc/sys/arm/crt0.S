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

/* .text is used instead of .section .text so it works with arm-aout too.  */
	.text
	.syntax unified
#ifdef THUMB_V7_V6M
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
	FUNC_START	start
#if defined(__ELF__) && !defined(__USING_SJLJ_EXCEPTIONS__)
	/* Annotation for EABI unwinding tables.  */
	.fnstart
#endif

/* Start by setting up a stack */
#ifdef ARM_RDP_MONITOR
	/*  Issue Demon SWI to read stack info */
	swi	SWI_GetEnv	/*  Returns command line in r0 */
	mov	sp,r1		/*  and the highest memory address in r1 */

	/*  stack limit is at end of data */
	/*  allow slop for stack overflow handling and small frames */
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
	/*  Issue Angel SWI to read stack info */
	movs	r0, #AngelSWI_Reason_HeapInfo
	adr	r1, .LC0	/*  point at ptr to 4 words to receive data */
#ifdef THUMB_V7M_V6M
	bkpt	AngelSWI
#elif defined(__thumb2__)
	/*  We are in thumb mode for startup on armv7 architectures. */
	AngelSWIAsm	AngelSWI
#else
	/*  We are always in ARM mode for startup on pre armv7 archs. */
	AngelSWIAsm	AngelSWI_ARM
#endif
	ldr	r0, .LC0	/*  point at values read */

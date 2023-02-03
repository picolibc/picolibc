/* This is a simple version of setjmp and longjmp.

   Nick Clifton, Cygnus Solutions, 13 June 1997.  */

#include "arm-acle-compat.h"

/* ANSI concatenation macros.  */
#define CONCAT(a, b)  CONCAT2(a, b)
#define CONCAT2(a, b) a##b

#ifndef __USER_LABEL_PREFIX__
#error  __USER_LABEL_PREFIX__ not defined
#endif

#define SYM(x) CONCAT (__USER_LABEL_PREFIX__, x)

#ifdef __ELF__
#define TYPE(x) .type SYM(x),function
#define SIZE(x) .size SYM(x), . - SYM(x)
#else
#define TYPE(x)
#define SIZE(x)
#endif

/* Jump buffer allocation sizes.  */
#define JUMPBUF_CORE_REGS_SIZE (10 * 4)
#define JUMPBUF_FP_REGS_SIZE (8 * 8)
#define JUMPBUF_PAC (JUMPBUF_CORE_REGS_SIZE + JUMPBUF_FP_REGS_SIZE + 0)

/* Arm/Thumb interworking support:

   The interworking scheme expects functions to use a BX instruction
   to return control to their parent.  Since we need this code to work
   in both interworked and non-interworked environments as well as with
   older processors which do not have the BX instruction we do the
   following:
	Test the return address.
	If the bottom bit is clear perform an "old style" function exit.
	(We know that we are in ARM mode and returning to an ARM mode caller).
	Otherwise use the BX instruction to perform the function exit.

   We know that we will never attempt to perform the BX instruction on
   an older processor, because that kind of processor will never be
   interworked, and a return address with the bottom bit set will never
   be generated.

   In addition, we do not actually assemble the BX instruction as this would
   require us to tell the assembler that the processor is an ARM7TDMI and
   it would store this information in the binary.  We want this binary to be
   able to be linked with binaries compiled for older processors however, so
   we do not want such information stored there.

   If we are running using the APCS-26 convention however, then we never
   test the bottom bit, because this is part of the processor status.
   Instead we just do a normal return, since we know that we cannot be
   returning to a Thumb caller - the Thumb does not support APCS-26.

   Function entry is much simpler.  If we are compiling for the Thumb we
   just switch into ARM mode and then drop through into the rest of the
   function.  The function exit code will take care of the restore to
   Thumb mode.

   For Thumb-2 do everything in Thumb mode.  */

	.syntax unified

/*  GCC 12.1 and later will tell the assembler exactly which floating
    point (or MVE) unit is required and we don't want to override
    that.  Conversely, older versions of the compiler don't pass this
    information so we need to enable the VFP version that is most
    appropriate.  The choice here should support all suitable VFP
    versions that the older toolchains can handle.  */
#if __GNUC__ && __GNUC__ < 12
/*  Ensure that FPU instructions are correctly compiled and, likewise,
    the appropriate build attributes are added to the resulting object
    file.  Check whether the MVE extension is present and whether
    we have support for hardware floating point-operations.  VFPxd
    covers all the cases we need in this file for hardware
    floating-point and should be compatible with all required FPUs
    that we need to support.  */
# if __ARM_FP
	.fpu vfpxd
# endif
# if __ARM_FEATURE_MVE
	.arch_extension mve
# endif
#endif

#if __ARM_ARCH_ISA_THUMB == 1 && !__ARM_ARCH_ISA_ARM
/* ARMv6-M-like has to be implemented in Thumb mode.  */

.thumb
.thumb_func
	.globl SYM (setjmp)
	TYPE (setjmp)
SYM (setjmp):
	/* Save registers in jump buffer.  */
	stmia	r0!, {r4, r5, r6, r7}
	mov	r1, r8
	mov	r2, r9
	mov	r3, r10
	mov	r4, fp
	mov	r5, sp
	mov	r6, lr
	stmia	r0!, {r1, r2, r3, r4, r5, r6}
	subs	r0, r0, #40
	/* Restore callee-saved low regs.  */
	ldmia	r0!, {r4, r5, r6, r7}
	/* Return zero.  */
	movs	r0, #0
	bx lr

.thumb_func
	.globl SYM (longjmp)
	TYPE (longjmp)
SYM (longjmp):
	/* Restore High regs.  */
	adds	r0, r0, #16
	ldmia	r0!, {r2, r3, r4, r5, r6}
	mov	r8, r2
	mov	r9, r3
	mov	r10, r4
	mov	fp, r5
	mov	sp, r6
	ldmia	r0!, {r3} /* lr */
	/* Restore low regs.  */
	subs	r0, r0, #40
	ldmia	r0!, {r4, r5, r6, r7}
	/* Return the result argument, or 1 if it is zero.  */
	movs	r0, r1
	bne	1f
	movs	r0, #1
1:
	bx	r3

#else

#ifdef __APCS_26__
#define RET	movs		pc, lr
#elif defined(__thumb2__)
#define RET	bx lr
#else
#define RET	tst		lr, #1; \
	        moveq		pc, lr ; \
.inst           0xe12fff1e	/* bx lr */
#endif

#ifdef __thumb2__
.macro COND where when
	i\where	\when
.endm
#else
.macro COND where when
.endm
#endif

#if defined(__thumb2__)
.macro MODE
	.thumb
	.thumb_func
.endm
.macro PROLOGUE name
.endm

#elif defined(__thumb__)
#define	MODE		.thumb_func
.macro PROLOGUE name
	.code 16
	bx	pc
	nop
	.code 32
SYM (.arm_start_of.\name):
.endm
#else /* Arm */
#define	MODE		.code 32
.macro PROLOGUE name
.endm
#endif

.macro FUNC_START name
	.text
	.align 2
	MODE
	.globl SYM (\name)
	.fnstart
	.cfi_startproc
	TYPE (\name)
SYM (\name):
	PROLOGUE \name
.endm

.macro FUNC_END name
	RET
	.cfi_endproc
	.fnend
	SIZE (\name)
.endm

/* --------------------------------------------------------------------
                 int setjmp (jmp_buf);
   -------------------------------------------------------------------- */

	FUNC_START setjmp

#if __ARM_FEATURE_PAC_DEFAULT
# if __ARM_FEATURE_BTI_DEFAULT
	pacbti	ip, lr, sp
# else
	pac	ip, lr, sp
# endif /* __ARM_FEATURE_BTI_DEFAULT */
	mov r3, ip
	str r3, [r0, #JUMPBUF_PAC]
	.cfi_register 143, 12
#else
# if __ARM_FEATURE_BTI_DEFAULT
	bti
# endif /* __ARM_FEATURE_BTI_DEFAULT */
#endif /* __ARM_FEATURE_PAC_DEFAULT */

	/* Save all the callee-preserved registers into the jump buffer.  */
#ifdef __thumb2__
	mov		ip, sp
	stmia		r0!, { r4-r10, fp, ip, lr }
#else
	stmia		r0!, { r4-r10, fp, sp, lr }
#endif
#if defined __ARM_FP || defined __ARM_FEATURE_MVE
	vstm		r0, { d8-d15 }
#endif

	/* When setting up the jump buffer return 0.  */
	mov		r0, #0
#if __ARM_FEATURE_PAC_DEFAULT
	mov ip, r3
	aut ip, lr, sp
#endif /* __ARM_FEATURE_PAC_DEFAULT */

	FUNC_END setjmp

/* --------------------------------------------------------------------
		volatile void longjmp (jmp_buf, int);
   -------------------------------------------------------------------- */

	FUNC_START longjmp

#if __ARM_FEATURE_BTI_DEFAULT
	bti
#endif /* __ARM_FEATURE_BTI_DEFAULT */

#if __ARM_FEATURE_PAC_DEFAULT
	/* Keep original jmpbuf address for retrieving pac-code
	   for authentication.  */
	mov	r2, r0
#endif /* __ARM_FEATURE_PAC_DEFAULT */

	/* If we have stack extension code it ought to be handled here.  */

	/* Restore the registers, retrieving the state when setjmp() was called.  */
#ifdef __thumb2__
	ldmia		r0!, { r4-r10, fp, ip, lr }
	mov		sp, ip
#else
	ldmia		r0!, { r4-r10, fp, sp, lr }
#endif
#if defined __ARM_FP || defined __ARM_FEATURE_MVE
	vldm		r0, { d8-d15 }
#endif

	/* Put the return value into the integer result register.
	   But if it is zero then return 1 instead.  */
	movs		r0, r1
	it		eq
	moveq		r0, #1

#if __ARM_FEATURE_PAC_DEFAULT
	ldr ip, [r2, #JUMPBUF_PAC]
	aut ip, lr, sp
#endif /* __ARM_FEATURE_PAC_DEFAULT */

	FUNC_END longjmp
#endif

/* This is a simple version of setjmp and longjmp.

   Nick Clifton, Cygnus Solutions, 13 June 1997.  */

/* ANSI concatenation macros.  */
#define CONCAT(a, b) CONCAT2(a, b)
#define CONCAT2(a, b) a ## b

#ifdef __USER_LABEL_PREFIX__
#define FUNCTION( name ) CONCAT (__USER_LABEL_PREFIX__, name)
#else
#error __USER_LABEL_PREFIX__ is not defined
#endif
	

	.text
	.code 32
	.align 2
	
/* int setjmp (jmp_buf);  */
	.globl	FUNCTION(setjmp)
FUNCTION(setjmp):

	/* Save all the callee-preserved registers into the jump buffer. */
	stmea		a1!, { v1-v7, fp, ip, sp, lr }
	
#if 0	/* Simulator does not cope with FP instructions yet.... */
#ifndef __SOFTFP__
	/* Save the floating point registers */
	sfmea		f4, 4, [a1]
#endif
#endif		
	/* When setting up the jump buffer return 0. */
	mov		a1, #0

	/* Return to caller, see comment in longjmp below */
#ifdef __APCS_26__
	movs		pc, lr
#else
	tst		lr, #1
	moveq		pc, lr
.word   0xe12fff1e	/*  bx lr */
#endif
	

/* volatile void longjmp (jmp_buf, int);  */
	.globl	FUNCTION(longjmp)
FUNCTION(longjmp):

	/* If we have stack extension code it ought to be handled here. */
	
	/* Restore the registers, retrieving the state when setjmp() was called. */
	ldmfd		a1!, { v1-v7, fp, ip, sp, lr }
	
#if 0	/* Simulator does not cope with FP instructions yet.... */
#ifndef __SOFTFP__
	/* Restore floating point registers as well */
	lfmfd		f4, 4, [a1]
#endif
#endif	
	/* Put the return value into the integer result register.
	   But if it is zero then return 1 instead.  */	
	movs		a1, a2
	moveq		a1, #1

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
	   returning to a Thumb caller - the Thumb doe snot support APCS-26
	*/

#ifdef __APCS_26__
	movs		pc, lr
#else
	tst		lr, #1
	moveq		pc, lr
.word   0xe12fff1e	/* bx lr */
#endif


/* This is a simple version of setjmp and longjmp for MIPS 32 and 64.

   Ian Lance Taylor, Cygnus Support, 13 May 1993.  */

#ifdef __mips16
/* This file contains 32 bit assembly code.  */
	.set nomips16
#endif

#define GPR_LAYOUT		\
	GPR_OFFSET ($16, 0);	\
	GPR_OFFSET ($17, 1);	\
	GPR_OFFSET ($18, 2);	\
	GPR_OFFSET ($19, 3);	\
	GPR_OFFSET ($20, 4);	\
	GPR_OFFSET ($21, 5);	\
	GPR_OFFSET ($22, 6);	\
	GPR_OFFSET ($23, 7);	\
	GPR_OFFSET ($29, 8);	\
	GPR_OFFSET ($30, 9);	\
	GPR_OFFSET ($31, 10)

#define NUM_GPRS_SAVED 11

#ifdef __mips_hard_float
#if _MIPS_SIM == _ABIN32
#define FPR_LAYOUT		\
	FPR_OFFSET ($f20, 0);	\
	FPR_OFFSET ($f22, 1);	\
	FPR_OFFSET ($f24, 2);	\
	FPR_OFFSET ($f26, 3);	\
	FPR_OFFSET ($f28, 4);	\
	FPR_OFFSET ($f30, 5);
#elif _MIPS_SIM == _ABI64
#define FPR_LAYOUT		\
	FPR_OFFSET ($f24, 0);	\
	FPR_OFFSET ($f25, 1);	\
	FPR_OFFSET ($f26, 2);	\
	FPR_OFFSET ($f27, 3);	\
	FPR_OFFSET ($f28, 4);	\
	FPR_OFFSET ($f29, 5);	\
	FPR_OFFSET ($f30, 6);	\
	FPR_OFFSET ($f31, 7);
#elif __mips_fpr == 0 || __mips_fpr == 64

/* This deals with the o32 FPXX and FP64 cases.  Here we must use
   SDC1 and LDC1 to access the FPRs.  These instructions require
   8-byte aligned addresses.
   Unfortunately, the MIPS jmp_buf only guarantees 4-byte alignment
   and this cannot be increased without breaking compatibility with
   pre-existing objects built against newlib.  There are 11 GPRS
   saved in the jmp_buf so a buffer that happens to be 8-byte aligned
   ends up leaving the FPR slots 4-byte aligned and an (only) 4-byte
   aligned buffer leads to the FPR slots being 8-byte aligned!
   
   To resolve this, we move the location of $31 to the last slot
   in the jmp_buf when the overall buffer is 8-byte aligned.  $31
   is simply loaded/stored twice to avoid adding complexity to the
   GPR_LAYOUT macro above as well as FPR_LAYOUT.

   The location of the last slot is index 22 which is calculated
   from there being 11 GPRs saved and then 12 FPRs saved so the
   index of the last FPR is 11+11.
   
   The base of the jmp_buf is modified in $4 to allow the
   FPR_OFFSET macros to just use the usual constant slot numbers
   regardless of whether the realignment happened or not.  */

#define FPR_LAYOUT		\
	and $8, $4, 4;	 	\
	beq $8, $0, 1f;		\
	GPR_OFFSET ($31, 22);	\
	addiu $4, $4, -4;	\
1:				\
	FPR_OFFSET ($f20, 0);  	\
	FPR_OFFSET ($f22, 2);	\
	FPR_OFFSET ($f24, 4);	\
	FPR_OFFSET ($f26, 6);	\
	FPR_OFFSET ($f28, 8);	\
	FPR_OFFSET ($f30, 10);
#else /* Assuming _MIPS_SIM == _ABIO32 */
#define FPR_LAYOUT		\
	FPR_OFFSET ($f20, 0);	\
	FPR_OFFSET ($f21, 1);	\
	FPR_OFFSET ($f22, 2);	\
	FPR_OFFSET ($f23, 3);	\
	FPR_OFFSET ($f24, 4);	\
	FPR_OFFSET ($f25, 5);	\
	FPR_OFFSET ($f26, 6);	\
	FPR_OFFSET ($f27, 7);	\
	FPR_OFFSET ($f28, 8);	\
	FPR_OFFSET ($f29, 9);	\
	FPR_OFFSET ($f30, 10);	\
	FPR_OFFSET ($f31, 11);
#endif
#else
#define FPR_LAYOUT
#endif
	
#ifdef __mips64
#define BYTES_PER_WORD 8
#define LOAD_GPR ld
#define LOAD_FPR ldc1
#define STORE_GPR sd
#define STORE_FPR sdc1
#else
#define LOAD_GPR lw
#define STORE_GPR sw
#define BYTES_PER_WORD 4
#if __mips_fpr == 0 || __mips_fpr == 64
#define LOAD_FPR ldc1
#define STORE_FPR sdc1
#else
#define LOAD_FPR lwc1
#define STORE_FPR swc1
#endif
#endif

#define GPOFF(INDEX) (INDEX * BYTES_PER_WORD)
#define FPOFF(INDEX) ((INDEX + NUM_GPRS_SAVED) * BYTES_PER_WORD)

/* int setjmp (jmp_buf);  */
	.globl	setjmp
	.ent	setjmp
setjmp:
	.frame	$sp,0,$31

#define GPR_OFFSET(REG, INDEX) STORE_GPR REG,GPOFF(INDEX)($4)
#define FPR_OFFSET(REG, INDEX) STORE_FPR REG,FPOFF(INDEX)($4)
	GPR_LAYOUT
	FPR_LAYOUT
#undef GPR_OFFSET
#undef FPR_OFFSET

	move	$2,$0
	j	$31

	.end	setjmp

/* volatile void longjmp (jmp_buf, int);  */
	.globl	longjmp
	.ent	longjmp
longjmp:
	.frame	$sp,0,$31

#define GPR_OFFSET(REG, INDEX) LOAD_GPR REG,GPOFF(INDEX)($4)
#define FPR_OFFSET(REG, INDEX) LOAD_FPR REG,FPOFF(INDEX)($4)
	GPR_LAYOUT
	FPR_LAYOUT
#undef GPR_OFFSET
#undef FPR_OFFSET

	bne	$5,$0,1f
	li	$5,1
1:
	move	$2,$5
	j	$31

	.end longjmp

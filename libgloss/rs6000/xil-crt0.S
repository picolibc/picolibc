/*-----------------------------------------------------------------------------
//
// Copyright (c) 2004, 2009 Xilinx, Inc.  All rights reserved. 
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
// 1.  Redistributions source code must retain the above copyright notice,
// this list of conditions and the following disclaimer. 
// 
// 2.  Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution. 
// 
// 3.  Neither the name of Xilinx nor the names of its contributors may be
// used to endorse or promote products derived from this software without
// specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//---------------------------------------------------------------------------*/

	.file	"xil-crt0.S"
	.section ".got2","aw"
	.align	2

.LCTOC1 = . + 32768

.Lsbss_start = .-.LCTOC1
	.long	__sbss_start

.Lsbss_end = .-.LCTOC1
	.long	__sbss_end

.Lbss_start = .-.LCTOC1
	.long	__bss_start

.Lbss_end = .-.LCTOC1
	.long	__bss_end

.Lstack = .-.LCTOC1
	.long	__stack

.Lsda = .-.LCTOC1
    .long   _SDA_BASE_                      /* address of the first small data area */

.Lsda2 = .-.LCTOC1
    .long   _SDA2_BASE_                     /* address of the second small data area */

    
	.text
	.globl	_start
_start:
        bl      __cpu_init              /* Initialize the CPU first (BSP provides this) */

    	lis	5,.LCTOC1@h
	ori	5,5,.LCTOC1@l

        lwz     13,.Lsda(5)             /* load r13 with _SDA_BASE_ address */
        lwz     2,.Lsda2(5)             /* load r2 with _SDA2_BASE_ address */

#ifndef SIMULATOR
        /* clear sbss */
	lwz	6,.Lsbss_start(5)	/* calculate beginning of the SBSS */
	lwz	7,.Lsbss_end(5)		/* calculate end of the SBSS */

	cmplw	1,6,7
	bc	4,4,.Lenclsbss          /* If no SBSS, no clearing required */

      	li	0,0			/* zero to clear memory */
    	subf	8,6,7			/* number of bytes to zero */
        srwi.   9,8,2                   /* number of words to zero */
        beq     .Lstbyteloopsbss        /* Check if the number of bytes was less than 4 */
        mtctr   9        
	addi	6,6,-4			/* adjust so we can use stwu */
.Lloopsbss:
	stwu	0,4(6)			/* zero sbss */
	bdnz	.Lloopsbss

.Lstbyteloopsbss:
        andi.   9,8,3                   /* Calculate how many trailing bytes we have */
        beq     0,.Lenclsbss
        mtctr   9
        addi    6,6,-1                  /* adjust, so we can use stbu */

.Lbyteloopsbss:  
        stbu    0,1(6)
        bdnz    .Lbyteloopsbss
    
.Lenclsbss:  
.Lstclbss:
    
	/* clear bss */
	lwz	6,.Lbss_start(5)	/* calculate beginning of the BSS */
	lwz	7,.Lbss_end(5)		/* calculate end of the BSS */

	cmplw	1,6,7
	bc	4,4,.Lenclbss           /* If no BSS, no clearing required */

    	li	0,0			/* zero to clear memory */
	subf	8,6,7			/* number of bytes to zero */
        srwi.   9,8,2                   /* number of words to zero */
        beq     .Lstbyteloopbss         /* Check if the number of bytes was less than 4 */
        mtctr   9
	addi	6,6,-4			/* adjust so we can use stwu */
.Lloopbss:
	stwu	0,4(6)			/* zero bss */
	bdnz	.Lloopbss

.Lstbyteloopbss:    
        andi.   9,8,3                   /* Calculate how many trailing bytes we have */
        beq     0,.Lenclbss             /* If zero, we are done */
        mtctr   9
        addi    6,6,-1                  /* adjust, so we can use stbu */

.Lbyteloopbss:  
        stbu    0,1(6)
        bdnz    .Lbyteloopbss
    
.Lenclbss:
#endif /* SIMULATOR */

	/* set stack pointer */
	lwz	1,.Lstack(5)		/* stack address */

	/* set up initial stack frame */
	addi	1,1,-8			/* location of back chain */
	lis	0,0
	stw	0,0(1)			/* set end of back chain */
	
	/* initialize base timer to zero */
	mtspr	0x11c,0
	mtspr	0x11d,0

#ifdef HAVE_XFPU    
	/* On the Xilinx PPC405 and PPC440, the MSR
           must be explicitly set to mark the prescence
           of an FPU */
	mfpvr	0
	rlwinm	0,0,0,12,15
	cmpwi	7,0,8192
        mfmsr   0
        ori     0,0,8192
	beq-	7,fpu_init_done
do_405:
        oris    0,0,512
fpu_init_done:
        mtmsr   0
#endif    
    
#ifdef PROFILING
	/* Setup profiling stuff */
	bl	_profile_init
#endif /* PROFILING */

	/* Call __init */
	bl	__init

	/* Let her rip */
	bl	main

        /* Invoke the language cleanup functions */        
        bl      __fini

#ifdef PROFILING
	/* Cleanup profiling stuff */
	bl	_profile_clean
#endif /* PROFILING */

	/* Call __init */
        /* All done */
	bl	exit
    
/* Trap has been removed for both simulation and hardware */
	.globl _exit
_exit:
	b _exit

.Lstart:
	.size	_start,.Lstart-_start


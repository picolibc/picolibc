/*
 * Copyright (c) 2011 Aeroflex Gaisler
 *
 * BSD license:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include <asm-leon/leonstack.h>
#include <asm-leon/winmacros.h>
                         
/* Registers to not touch at all. */
#define t_psr     l0
#define t_pc      l1
#define t_npc     l2
#define t_wim     l3
#define twin_tmp1 l4
#define glob_tmp  g4
#define curptr    g6

	/* Number of register windows */
	.global _nwindows_min1, _nwindows

        .text
	.align 4
	.globl	leonbare_trapreturn_fast, schedule_callback


/* rtap return special for irqtrap.S */
leonbare_trapreturn_fast:

        /* a optional scheduler can be called here */
        set schedule_callback, %g2
        ld [%g2], %g2
        cmp %g2,%g0
        beq 3f
         nop
                 
        jmpl %g2,%o7
#ifndef _SOFT_FLOAT
	 add	%sp, FW_REGS_SZ + 8 + SF_REGS_SZ , %o1	! pt_regs ptr
#else
	 add	%sp, SF_REGS_SZ , %o1		   ! pt_regs ptr
#endif
	
3:
	
#ifndef _SOFT_FLOAT
	ld [%sp + (SF_REGS_SZ + PT_REGS_SZ + FW_REGS_SZ - 4)],%g2
	sethi %hi(fpustate_current), %g3 
	st %g2, [%g3+%lo(fpustate_current)]
	sethi	%hi(fpustate_owner), %g3
	ld	[%g3+%lo(fpustate_owner)], %g3
	cmp	%g2, %g3
	bne	didusefpu
	 nop
	
	/* avoid fpu exception */
	ld 	[%sp + (SF_REGS_SZ + PT_REGS_SZ + FW_REGS_SZ - 8)], %g2
	set	SPARC_PSR_EF_MASK, %g3
	and 	%g2, %g3, %g2
	andn	%t_psr, %g3, %t_psr
	or	%t_psr, %g2, %t_psr
	ba,a	1f
	
didusefpu:
	add 	%sp,SF_REGS_SZ + PT_REGS_SZ,%g2
	cmp	%g2, %g3
	bne	1f

	sethi	%hi(fpustate_owner), %g3
	st	%g0, [%g3+%lo(fpustate_owner)]
	
1:	
#endif
		
	wr	%t_psr, 0x0, %psr       ! enable nesting again, clear ET
	
#ifndef _FLAT
	/* Will the rett land us in the invalid window? */
	mov	2, %g1
	sll	%g1, %t_psr, %g1

	sethi %hi(_nwindows), %g2	!NWINDOWS
	ld [%g2+%lo(_nwindows)], %g2
	
       	srl	%g1, %g2, %g2
	or	%g1, %g2, %g1
	rd	%wim, %g2
	andcc	%g2, %g1, %g0
	be	1f		! Nope, just return from the trap
	 sll	%g2, 0x1, %g1

        	/* We have to grab a window before returning. */
		sethi %hi(_nwindows_min1), %g3	!NWINDOWS-1
		ld [%g3+%lo(_nwindows_min1)], %g3

                srl	%g2, %g3,  %g2
                or	%g1, %g2, %g1
                and	%g1, 0xff, %g1

                wr	%g1, 0x0, %wim

        	/* Grrr, make sure we load from the right %sp... */
                PT_LOAD_ALL_FAST(sp, t_psr, t_pc, t_npc, g1)
	
        	restore	%g0, %g0, %g0
                RW_LOAD(sp)
                b	2f
                 save	%g0, %g0, %g0

	/* Reload the entire frame in case this is from a
	 * kernel system call or whatever...
	 */
1:
#endif
 	PT_LOAD_ALL_FAST(sp, t_psr, t_pc, t_npc, g1)
	
2:      /*PT_LOAD_GLOBALS(sp)*/

#ifdef _FLAT
	restore
	RW_LOAD(sp)
	save
#endif
	
	wr	%t_psr, 0x0, %psr
	nop; nop; nop  

	jmp	%t_pc
	rett	%t_npc




        
#ifdef _FLAT
#warning _FLAT not implemented
#endif
        


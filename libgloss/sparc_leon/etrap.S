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
#define t_psr        l0 /* Set by caller */
#define t_pc         l1 /* Set by caller */
#define t_npc        l2 /* Set by caller */
#define t_wim        l3 /* Set by caller */
#define t_twinmask   l4 /* Set at beginning of this entry routine. */
#define t_kstack     l5 /* Set right before pt_regs frame is built */
#define t_retpc      l6 /* If you change this, change winmacro.h header file */
#define t_systable   l7 /* Never touch this, could be the syscall table ptr. */
#define curptr       g6 /* Set after pt_regs frame is built */

	/* Number of register windows */
	.global _nwindows_min1, _nwindows

        .text
	.align 4
	.globl	leonbare_trapsetup

leonbare_trapsetup:

#ifdef 	_FLAT
	restore
	RW_STORE(sp)
	save
#endif

#ifndef _SOFT_FLOAT
	/* build a pt_regs trap frame. */
        sub	%fp, (SF_REGS_SZ + PT_REGS_SZ + FW_REGS_SZ), %t_kstack
	PT_STORE_ALL(t_kstack, t_psr, t_pc, t_npc, g2)
	
	/* build a fp_regs trap frame. */
	sethi %hi(fpustate_current), %g2
	ld [%g2+%lo(fpustate_current)], %g3
	st %g3,[%t_kstack + (SF_REGS_SZ + PT_REGS_SZ + FW_REGS_SZ - 4)] 
	add %t_kstack,SF_REGS_SZ + PT_REGS_SZ,%g3
	st %g3, [%g2+%lo(fpustate_current)]
	
#else	
	/* build a pt_regs trap frame.
	 */
        sub	%fp, (SF_REGS_SZ + PT_REGS_SZ), %t_kstack
	PT_STORE_ALL(t_kstack, t_psr, t_pc, t_npc, g2)
#endif
	

#ifndef _FLAT
        /* See if we are in the trap window. */
	mov	1, %t_twinmask
	sll	%t_twinmask, %t_psr, %t_twinmask ! t_twinmask = (1 << psr)
	andcc	%t_twinmask, %t_wim, %g0
	beq	1f		! in trap window, clean up
	 nop

                /*-------------------------------------------------*/
                /* Spill , adjust %wim and go. */
                srl	%t_wim, 0x1, %g2	! begin computation of new %wim
		
		sethi %hi(_nwindows_min1), %g3
		ld [%g3+%lo(_nwindows_min1)], %g3

                sll	%t_wim, %g3, %t_wim	! NWINDOWS-1
                or	%t_wim, %g2, %g2
                and	%g2, 0xff, %g2		
        
                save	%g0, %g0, %g0           ! get in window to be saved
        
        	/* Set new %wim value */
                wr	%g2, 0x0, %wim

        	/* Save the kernel window onto the corresponding stack. */
                RW_STORE(sp)
        
        	restore	%g0, %g0, %g0
                /*-------------------------------------------------*/
        
1:                
#endif
	/* Trap from kernel with a window available.
	 * Just do it...
	 */
	jmpl	%t_retpc + 0x8, %g0	! return to caller
	 mov	%t_kstack, %sp		! jump onto new stack
        
        

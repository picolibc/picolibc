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


#include <asm-leon/leon.h>
#include <asm-leon/leonstack.h>
#include <asm-leon/asmmacro.h>
        
        .seg    "data"
        .global _lb_spillglobals, _lb_issideflush
	.align  4
_lb_spillglobals:
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0
_lb_issideflush:
	.word 0 /* off: 28 */
	
	.seg    "text"

/* =============================================== */
	
#define _SV	save	%sp, -SF_REGS_SZ, %sp
#define _RS     restore 

	/* ------- */
  	.weak	_flush_windows
	.set	_flush_windows,__flush_windows
	.weak	_flush_windows_svt
	.set	_flush_windows_svt,__flush_windows_svt
	/* ------- */
	!.global	_flush_windows,_flush_windows_svt
__flush_windows_svt:
	rd %wim, %l3

__flush_windows:
        SAVE_ALL
	
#ifndef _FLAT

	set	_lb_issideflush, %l3
	st	%l0, [%l3] 	/* mark as inside flush */
	
	wr	%l0, SPARC_PSR_ET_MASK, %psr
	nop; nop; nop 

	_SV; _SV; _SV; _SV; _SV; _SV; _SV; 
	_RS; _RS; _RS; _RS; _RS; _RS; _RS;

	set	_lb_issideflush, %l3
	st	%g0, [%l3] 	/* mark as outside flush */
	
	/* Advance over the trap instruction. */
	ld	[%sp + SF_REGS_SZ + PT_NPC], %l1
	add	%l1, 0x4, %l2
	st	%l1, [%sp + SF_REGS_SZ + PT_PC]
	st	%l2, [%sp + SF_REGS_SZ + PT_NPC]
#endif
	
	RESTORE_ALL

/* =============================================== */

_irqcall_flush_windows:
#ifndef _FLAT
	set    _lb_spillglobals,%l4
	st     %g1,[%l4+0]
	st     %g4,[%l4+4]
	st     %l0,[%l4+16]
	st     %l1,[%l4+20]
	st     %l2,[%l4+24]
	st     %l4,[%l4+28] /* mark as inside flush */
	
	restore 

	mov     %psr, %g1
	or	%g1, SPARC_PSR_PIL_MASK, %g1
	wr	%g1, SPARC_PSR_ET_MASK, %psr         /* disable irq, enable traps */
	nop
	nop
	nop
	
	sethi %hi(_nwindows_min1), %g4               /* flush registers */
	ld [%g4+%lo(_nwindows_min1)], %g4
1:	save				             /* NWINDOWS-1 times */
	sub    %g4,1,%g4

	/*****************/
	andncc   %g4,0xff,%g0
	be    .lab1
	 nop
	nop
	
.lab1:	/*****************/
	
	cmp    %g4,%g0
	bne    1b
	 nop
	
	sethi %hi(_nwindows_min1), %g4
	ld [%g4+%lo(_nwindows_min1)], %g4
2:	restore		                             /* NWINDOWS-1 times */
	 
	/*****************/
	andncc   %g4,0xff,%g0
	be    .lab2
	 nop
	nop
	
.lab2:	/*****************/
	
	sub    %g4,1,%g4
	cmp    %g4,%g0
	bne    2b
	 nop

	save

	set    _lb_spillglobals,%l4
	ld     [%l4+4], %g4
	ld     [%l4+0], %g1
	ld     [%l4+16],%l0
	ld     [%l4+20],%l1
	ld     [%l4+24],%l2
	st     %g0,[%l4+28] /* clean inside flush mark */
	
#endif
	
	wr	%l0, 0, %psr                         /* restore psr */
	nop
	nop
	nop 
	
        jmpl    %l2, %g0
        rett    %l2 + 4

/* =============================================== */

	/* ------- */
  	.weak	_irqcall_disableirq
	.set	_irqcall_disableirq,__irqcall_disableirq
	.weak	_irqcall_disableirq_svt
	.set	_irqcall_disableirq_svt,__irqcall_disableirq_svt
	/* ------- */
	
__irqcall_disableirq:	
__irqcall_disableirq_svt:	
	or    	%l0, SPARC_PSR_PIL_MASK, %l0
	mov   	%l0, %psr
	nop; nop; nop
        jmpl    %l2, %g0
        rett    %l2 + 4

/* =============================================== */
	
        /*
         *  system call (ta 0x2):       
         *  2: irq_disable:
         *      o1 = 2
         *  3: irq_enable:
         *      o0 = old_flags
         *      o1 = 3
         *  4: enter supervisor mode (from user mode):
         *      o1 = 4
         *  5: enter user mode:
         *      o1 = 5
	 *  6: flush windows
         *
         *  On entry:
         *
         *    l0 = psr (from trap table)
         *    l1 = pc
         *    l2 = npc
         *    i0 = system call id
         */

	/* ------- */
  	.weak	_irqcall
	.set	_irqcall,__irqcall
	.weak	_irqcall_svt
	.set	_irqcall_svt,__irqcall_svt
	/* ------- */
        !.global _irqcall,_irqcall_svt
__irqcall_svt:	
__irqcall:

	subcc   %i1, 2, %g0		! syscall 2, disable interrupts
	bne 	3f
	or    	%l0, 0x0f00, %l4	! set PIL=15
	mov   	%l4, %psr
        or	%l0, SPARC_PSR_ET_MASK, %i0	! return old psr with ET=1
	ba,a	9f
3:
	subcc   %i1, 3, %g0		! syscall 3, enable interrupts
	bne 	4f
	and	%i0, SPARC_PSR_PIL_MASK, %l4
	andn	%l0, SPARC_PSR_PIL_MASK, %l5
	or	%l5, %l4, %l4
	mov   	%l4, %psr
        ba,a	9f		
4:	
	subcc   %i1, 4, %g0		! syscall 4, enter supervisor
	bne 	5f
	
	mov	%psr, %l4
        or      %l4,SPARC_PSR_PS_MASK,%l4
	mov     %l4, %psr		! set previous supervisor %psr
	nop; nop; nop
        ba,a	9f
        
5:	
	subcc   %i1, 5, %g0		! syscall 5, enter user
	bne 	6f
	
	mov	%psr, %l4
        andn    %l4,SPARC_PSR_PS_MASK,%l4
	mov     %l4, %psr		! clear previous supervisor %psr, return to user mode
	nop; nop; nop
	ba,a	9f
	
6:	
	subcc   %i1, 6, %g0		! syscall 6,  flush windows
	bne 	1f
	 nop
	
	ba,a	_irqcall_flush_windows
	
1:					
	ta	0			! halt 
9:					! leave
        jmpl    %l2, %g0
        rett    %l2 + 4

/* =============================================== */
	
	/* call _irqcall through trap */
	.global leonbare_enable_traps !void leonbare_enable_traps(unsigned long old_flags); 
leonbare_enable_traps:
        set 3,%o1
        retl
        ta 0x2
        
/* =============================================== */
	
	/* call _irqcall through trap */
        .global leonbare_disable_traps !unsigned long leonbare_disable_traps(); 
leonbare_disable_traps:
        set 2,%o1
        retl
        ta 0x2        

/* =============================================== */
	
	/* flush all windows */
        .global leonbare_flush_windows !void leonbare_flush_windows(); 
leonbare_flush_windows:
        set 6,%o1
        retl
         ta 0x2        


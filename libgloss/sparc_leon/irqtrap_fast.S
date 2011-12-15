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
#include <asm-leon/asmmacro.h>
#include <asm-leon/winmacros.h>
#include <asm-leon/leon.h>

	/* l0: psr
	   l1: pc
	   l2: npc
	   l3: wim
	   l7: irqnr */	 
         
  	.seg    "text"

	/* ------- */
  	.weak	_leonbare_irq_entry_svt
	.set	_leonbare_irq_entry_svt,__leonbare_irq_entry_svt
  	.weak	leonbare_irq_entry
	.set	leonbare_irq_entry,_leonbare_irq_entry
	/* ------- */
        !.global leonbare_irq_entry,_leonbare_irq_entry_svt
        .global _irqtbl, _irqtrap, handler_irq, fpustate_current

#define FASTIRQ_ENABLE	
/*#define FASTIRQ_DYNAMIC*/	/* depend on FASTIRQ_ENABLE */
	
__leonbare_irq_entry_svt:	 /* irq from svt trap dispatcher */
	sub %l6,0x10, %l7 
	rd %wim, %l3
	
_leonbare_irq_entry:	

	SAVE_ALL_FAST(.L3)                        ! fast irq processing, volatile %g6, use frame 
	
.L3:
#ifdef __threadx__	
	set	_tx_thread_system_state, %o0
	ld      [%o0],%o1
	add	%o1,1,%o1
	st      %o1,[%o0]
#endif
	
	set     nestcount,%o0
	ld      [%o0],%o1
	add	%o1,1,%o1
	st      %o1,[%o0]
	
#ifdef CONFIG_LEONBARE_NONESTEDIRQ	
        or	%l0, SPARC_PSR_PIL_MASK, %o0       ! no nested irqs
	wr	%o0, SPARC_PSR_ET_MASK, %psr
	WRITE_PAUSE
#else
	sll     %l7,SPARC_PSR_PIL_SHIFT,%o1
	andn	%l0,SPARC_PSR_PIL_MASK,%o0
	or	%o0, %o1, %o1
	set     nestedirq,%o0
	ld      [%o0],%o0
        cmp     %g0,%o0                            ! no nested irqs?
	beq,a   .L1
        or	%o1, SPARC_PSR_PIL_MASK, %o1       
.L1:	
	wr	%o1, SPARC_PSR_ET_MASK, %psr
	WRITE_PAUSE
#endif
	
	mov	%l7, %o0                           ! irq level
        call	handler_irq                        ! void handler_irq (int irq, struct leonbare_pt_regs *pt_regs) 
#ifndef _SOFT_FLOAT
	 add	%sp, FW_REGS_SZ + 8 + SF_REGS_SZ , %o1	! pt_regs ptr
#else
	 add	%sp, SF_REGS_SZ , %o1		   ! pt_regs ptr
#endif
	
	or      %l0, SPARC_PSR_PIL_MASK, %o1
	wr	%o1, SPARC_PSR_ET_MASK, %psr	   ! enable nesting again, keep ET up
	WRITE_PAUSE

	set     nestcount,%o0
	ld      [%o0],%o1
	sub	%o1,1,%o1
	st      %o1,[%o0]
	
#ifdef __threadx__	
	set	_tx_thread_system_state, %o0
	ld      [%o0],%o1
	sub	%o1,1,%o1
	st      %o1,[%o0]
#endif
	
	RESTORE_ALL_FAST

        .seg    "data"
        .global nestedirq
	.align  4
nestedirq:
        .long   0
        .global fastirq
	.align  4
fastirq:
        .long   0
        .global nestcount
	.align  4
nestcount:
        .long   0

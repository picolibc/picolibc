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


#include <asm-leon/elfmacro.h>
#include <asm-leon/leon.h>
#include <asm-leon/leonstack.h>
#include <asm-leon/contextswitch.h>
#include <asm-leon/winmacros.h>
#include <asm-leon/leonbare_kernel.h>

	FUNC_EXPORT(_leonbare_kernel_switchto)
	FUNC_EXPORT(_leonbare_Stop)

	FUNC_IMPORT(leonbare_disable_traps)

	.text
	
/* unsigned int _leonbare_kernel_switchto(struct leonbare_thread_ctx *thread,struct leonbare_thread_ctx *thread) */
FUNC_BEGIN(_leonbare_kernel_switchto)
	
	/* =================================*/
	/*        save context              */
	/* =================================*/
	
	mov	%o0, %g1
	mov	%o1, %g2
	mov	%o7, %g3
	rd	%psr, %g4                       /* psr.cwp should stay same because irq path rely on it. */
	
	call	leonbare_disable_traps		/* psr in %o0, modify %o0, %o1, %o7 */
	 nop

	set	TACODE_IRQCALL_FLUSH,%o1
        ta	TACODE_IRQCALL
	
	st	%g4, [%g1 + LEONBARE_THREAD_CTX_STACK_PSR]	/* psr */
	set	LEONBARE_THREAD_CTX_MAGIC,%g4
	st	%g4, [%g1 + LEONBARE_THREAD_CTX_STACK_MAGIC]
	
	mov	%g3, %o7			! restore %o7
	
	LEONBARE_THREAD_CTX_STORE_INS(g1)
	LEONBARE_THREAD_CTX_STORE_LOCALS(g1)
	LEONBARE_THREAD_CTX_STORE_OUTS(g1)

	/* =================================*/
	/*        restore context           */
	/* =================================*/
	
	/* check valid context stack area */
	ld	[%g2 + LEONBARE_THREAD_CTX_STACK_MAGIC], %o1
	set	LEONBARE_THREAD_CTX_MAGIC,%o2
	cmp	%o1, %o2
	beq	1f
	 nop
	
	/* stop all */
	ta	0x0
	
1:	
	/* get psr */
	ld	[%g2 + LEONBARE_THREAD_CTX_STACK_PSR],%g1            /* psr.cwp should stay same because irq path rely on it. */
	set	SPARC_PSR_EF_MASK,%g3		! clear ef bit
	andn	%g1, %g3, %g1		
	
	wr	%g0,%wim
	nop; nop; nop;

	andn	%g1, SPARC_PSR_ET_MASK, %g3	! disable traps, up to PSR_EF imm andn ok
	wr	%g3, %psr
	nop; nop; nop;

	LEONBARE_THREAD_CTX_LOAD_INS(g2)
	LEONBARE_THREAD_CTX_LOAD_LOCALS(g2)
	LEONBARE_THREAD_CTX_LOAD_OUTS(g2)
	
	SET_WIM_CWPMIN1(g1,o1,o2,o3,o4)		! calc wim from psr_cwp so that next restore traps
	
	wr	%g1,%psr
	nop; nop; nop;
	
	retl
	 nop
	
FUNC_END(_leonbare_kernel_switchto)


FUNC_BEGIN(_leonbare_Stop)
	ta 0x0
FUNC_END(_leonbare_Stop)

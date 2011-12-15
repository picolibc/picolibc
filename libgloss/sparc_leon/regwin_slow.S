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
        
  .seg    "text"

	
/* Number of register windows */
	.global _nwindows_min1, _nwindows
	
        ! Window overflow trap handler on save.
        ! Touches %g1
	/* ------- */
	.weak	_window_overflow
	.set	_window_overflow,__window_overflow
	.weak	_window_overflow_svt
	.set	_window_overflow_svt,__window_overflow_svt
	/* ------- */
  	!.global _window_overflow,_window_overflow_svt
  	.global __window_overflow_rettseq,__window_overflow_rettseq_ret,__window_overflow_slow1

__window_overflow_svt:		
__window_overflow:
#ifndef _FLAT
__window_overflow_rettseq:		
  	mov  %wim, %l3       		! Calculate next WIM
  	mov  %g1, %l7           
  	srl  %l3, 1, %g1        
	
__window_overflow_rettseq_ret:		
	sethi %hi(_nwindows_min1), %l4	! NWINDOWS-1
	ld [%l4+%lo(_nwindows_min1)], %l4
	
	sll  %l3, %l4 , %l4
  	or   %l4, %g1, %g1

  	save                              ! Get into window to be saved.
  	mov  %g1, %wim
  	nop; nop; nop
        std   %l0, [%sp + 0];
        std   %l2, [%sp + 8];
        std   %l4, [%sp + 16]; 
        std   %l6, [%sp + 24]; 
        std   %i0, [%sp + 32]; 
        std   %i2, [%sp + 40]; 
        std   %i4, [%sp + 48]; 
        std   %i6, [%sp + 56]; 
  	restore				! Go back to trap window.
  	mov  %l7, %g1
	
  	jmp  %l1			! Re-execute save.
  	rett %l2
	nop
	
__window_overflow_slow1:		! space for possible stackcheck patch
	nop
	nop
#else	
	ta	0			! halt 
__window_overflow_rettseq:		
__window_overflow_rettseq_ret:		
__window_overflow_slow1:		
	nop
	nop
	nop
#endif
	
  /* Window underflow trap handler on restore.  */

        ! Touches %g1
	/* ------- */
  	.weak	_window_underflow
	.set	_window_underflow,__window_underflow
	.weak	_window_underflow_svt
	.set	_window_underflow_svt,__window_underflow_svt
	/* ------- */
  	!.global  _window_underflow,_window_underflow_svt

__window_underflow_svt:
__window_underflow:
#ifndef _FLAT
	mov  %wim, %l3			! Calculate next WIM
	sll  %l3, 1, %l4

	sethi %hi(_nwindows_min1), %l5	! NWINDOWS-1
	ld [%l5+%lo(_nwindows_min1)], %l5
	
	srl  %l3, %l5, %l5
  	or   %l5, %l4, %l5
  	mov  %l5, %wim
  	nop; nop; nop
  	restore				! Two restores to get into the
  	restore				! window to restore
        ldd   [%sp + 0], %l0; 		! Restore window from the stack
        ldd   [%sp + 8], %l2; 
        ldd   [%sp + 16], %l4; 
        ldd   [%sp + 24], %l6; 
        ldd   [%sp + 32], %i0; 
        ldd   [%sp + 40], %i2; 
        ldd   [%sp + 48], %i4; 
        ldd   [%sp + 56], %i6; 
  	save				! Get back to the trap window.
  	save
  	jmp  %l1			! Re-execute restore.
  	rett  %l2
#else	
	ta	0			! halt 
#endif

	

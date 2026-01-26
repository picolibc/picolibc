;/*
; * C library -- _setjmp, _longjmp
; *
; *      _longjmp(a,v)
; * will generate a "return(v?v:1)" from
; * the last call to
; *      _setjmp(a)
; * by unwinding the call stack.
; * The previous signal state is NOT restored.
; *
; *
; * Copyright (c) 2003 Altera Corporation
; * All rights reserved.
; * 
; * Redistribution and use in source and binary forms, with or without
; * modification, are permitted provided that the following conditions
; * are met:
; * 
; *    o Redistributions of source code must retain the above copyright
; *      notice, this list of conditions and the following disclaimer. 
; *    o Redistributions in binary form must reproduce the above copyright
; *      notice, this list of conditions and the following disclaimer in the 
; *      documentation and/or other materials provided with the distribution. 
; *    o Neither the name of Altera Corporation nor the names of its 
; *      contributors may be used to endorse or promote products derived from
; *      this software without specific prior written permission. 
; *  
; * THIS SOFTWARE IS PROVIDED BY ALTERA CORPORATION, THE COPYRIGHT HOLDER,
; * AND ITS CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
; * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
; * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
; * THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
; * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
; * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
; * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
; * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
; * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
; * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
; */

	.section	.text
	.align	3
	.globl	setjmp
	.type	setjmp,@function
	.globl	longjmp
	.type	longjmp,@function


setjmp:
	stw	r16, 0(r4)
	stw	r17, 4(r4)
	stw	r18, 8(r4)
	stw	r19, 12(r4)
	stw	r20, 16(r4)
	stw	r21, 20(r4)
	stw	r22, 24(r4)
	stw	r23, 28(r4)
	stw	gp, 32(r4)
	stw	sp, 36(r4)
	stw	fp, 40(r4)
	stw	ra, 44(r4)
	mov	r2, zero
	ret

longjmp:
	ldw	r16, 0(r4)
	ldw	r17, 4(r4)
	ldw	r18, 8(r4)
	ldw	r19, 12(r4)
	ldw	r20, 16(r4)
	ldw	r21, 20(r4)
	ldw	r22, 24(r4)
	ldw	r23, 28(r4)
	ldw	gp, 32(r4)
	ldw	sp, 36(r4)
	ldw	fp, 40(r4)
	ldw	ra, 44(r4)
	mov	r2, r5
	bne	r2, zero, 1f
	movi	r2, 1	
1:
	ret

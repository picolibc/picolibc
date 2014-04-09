/*

Copyright (c) 2011 Red Hat Incorporated.
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met: 

    Redistributions of source code must retain the above copyright 
    notice, this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    The name of Red Hat Incorporated may not be used to endorse 
    or promote products derived from this software without specific 
    prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL RED HAT INCORPORATED BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 

*/

#ifdef __RL78_G10__
; clobberable
r8	=	0xffec8
r9	=	0xffec9
r10	=	0xffeca
r11	=	0xffecb
r12	=	0xffecc
r13	=	0xffecd
r14	=	0xffece
r15	=	0xffecf
; preserved
r16	=	0xffed0
r17	=	0xffed1
r18	=	0xffed2
r19	=	0xffed3
r20	=	0xffed4
r21	=	0xffed5
r22	=	0xffed6
r23	=	0xffed7
#else
; clobberable
r8	=	0xffef0
r9	=	0xffef1
r10	=	0xffef2
r11	=	0xffef3
r12	=	0xffef4
r13	=	0xffef5
r14	=	0xffef6
r15	=	0xffef7
; preserved
r16	=	0xffee8
r17	=	0xffee9
r18	=	0xffeea
r19	=	0xffeeb
r20	=	0xffeec
r21	=	0xffeed
r22	=	0xffeee
r23	=	0xffeef
#endif

/* The jump buffer has the following structure:
	R0 .. R23  3*8 bytes
	SP	   2 bytes
	ES	   1 byte
	CS	   1 byte
	PC         4 bytes
*/

	.macro	_saveb	ofs,reg
	mov	a,\reg
	mov	[hl+\ofs],a
	.endm
	.macro	_save	ofs,reg
	movw	ax,\reg
	movw	[hl+\ofs],ax
	.endm

	.global	_setjmp
	.type	_setjmp, @function
_setjmp:
	;; R8 = setjmp (jmp_buf *[sp+4].w)
	;; must return zero !!
	push	ax
	push	hl
	push	ax
	movw	ax, [sp+10]
	movw	hl, ax
	pop	ax
	movw	[hl], ax
	_save	2, bc
	_save	4, de
	pop	ax
	movw	[hl+6], ax
	_save	8, r8
	_save	10, r10
	_save	12, r12
	_save	14, r14
	_save	16, r16
	_save	18, r18
	_save	20, r20
	_save	22, r22

	;; The sp we have now includes one more pushed reg, plus $PC
	movw	ax, sp
	addw	ax, #6
	movw	[hl+24], ax

	_saveb	26, es
	_saveb	27, cs
	_save	28, [sp+2]
	_save	30, [sp+4]

	clrw	ax
	movw	r8, ax
	pop	ax
	ret

	.size	_setjmp, . - _setjmp

	.macro	_loadb	ofs,reg
	mov	a,[hl+\ofs]
	mov	\reg,a
	.endm
	.macro	_load	ofs,reg
	movw	ax,[hl+\ofs]
	movw	\reg,ax
	.endm
	.macro	_push	ofs
	movw	ax,[hl+\ofs]
	push	ax
	.endm

	.global	_longjmp
	.type _longjmp, @function
_longjmp:
	;; noreturn longjmp (jmp_buf *[sp+4].w, int [sp+6].w)
	movw	ax, [sp+6]
	cmpw	ax,#0
	sknz
	onew	ax
	movw	r8, ax

	movw	ax, [sp+4]
	movw	hl, ax
	movw	ax, [hl+24]
	movw	sp, ax		; this is the *new* stack

	_push	30		; high half of PC
	_push	28		; low half of PC
	_push	6		; HL
	_push	0		; AX

	_load	2, bc
	_load	4, de

	_load	10, r10
	_load	12, r12
	_load	14, r14
	_load	16, r16
	_load	18, r18
	_load	20, r20
	_load	22, r22

	_loadb	26, es
	_loadb	27, cs

	pop	ax
	pop	hl


	ret	; pops PC (4 bytes)

	.size	_longjmp, . - _longjmp


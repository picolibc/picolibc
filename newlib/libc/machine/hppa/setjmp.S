/* Copyright (c) 1995 Cygnus Support.
   All rights reserved.

   Redistribution and use in source and binary forms are permitted
   provided that the above copyright notice and this paragraph are
   duplicated in all such forms and that any documentation,
   advertising materials, and other materials related to such
   distribution and use acknowledge that the software was developed
   at Cygnus Support, Inc.  Cygnus Support, Inc. may not be used to
   endorse or promote products derived from this software without
   specific prior written permission.
   THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  */

/* Note I don't know an easy way to get the FP version into the
   traditional C library and the non-FP version into the soft-float
   library.  Maybe we'll have to have -msoft-float trigger something
   like -DSOFT_FLOAT if this issue ever arises.  */

	.SPACE $PRIVATE$
	.SUBSPA $DATA$,QUAD=1,ALIGN=8,ACCESS=31
	.SUBSPA $BSS$,QUAD=1,ALIGN=8,ACCESS=31,ZERO,SORT=82
	.SPACE $TEXT$
	.SUBSPA $LIT$,QUAD=0,ALIGN=8,ACCESS=44
	.SUBSPA $CODE$,QUAD=0,ALIGN=8,ACCESS=44,CODE_ONLY
	.IMPORT $global$,DATA
	.IMPORT $$dyncall,MILLICODE
; gcc_compiled.:
	.SPACE $TEXT$
	.SUBSPA $CODE$

	.align 4
	.EXPORT setjmp,ENTRY,PRIV_LEV=3,ARGW0=GR,RTNVAL=GR
setjmp
	.PROC
	.CALLINFO FRAME=64,NO_CALLS,SAVE_SP,ENTRY_GR=3
	.ENTRY
	stwm  %r30,4(%r26)
	stwm  %r2,4(%r26)
	stwm  %r3,4(%r26)
	stwm  %r4,4(%r26)
	stwm  %r5,4(%r26)
	stwm  %r6,4(%r26)
	stwm  %r7,4(%r26)
	stwm  %r8,4(%r26)
	stwm  %r9,4(%r26)
	stwm  %r10,4(%r26)
	stwm  %r11,4(%r26)
	stwm  %r12,4(%r26)
	stwm  %r13,4(%r26)
	stwm  %r14,4(%r26)
	stwm  %r15,4(%r26)
	stwm  %r16,4(%r26)
	stwm  %r17,4(%r26)
	stwm  %r18,4(%r26)
	stwm  %r27,4(%r26)
#ifdef FP
	; jmp_buf may only have a 4 byte alignment, so handle FP stores
	; very carefully.
	fstds %fr12,-16(%r30)
	ldw -16(%r30),%r28
	stwm  %r28,4(%r26)
	ldw -12(%r30),%r28
	stwm  %r28,4(%r26) 
	fstds %fr13,-16(%r30)
	ldw -16(%r30),%r28
	stwm  %r28,4(%r26)
	ldw -12(%r30),%r28
	stwm  %r28,4(%r26) 
	fstds %fr14,-16(%r30)
	ldw -16(%r30),%r28
	stwm  %r28,4(%r26)
	ldw -12(%r30),%r28
	stwm  %r28,4(%r26) 
	fstds %fr15,-16(%r30)
	ldw -16(%r30),%r28
	stwm  %r28,4(%r26)
	ldw -12(%r30),%r28
	stwm  %r28,4(%r26) 
#endif

	bv 0(%r2)
	copy %r0,%r28
	.EXIT
	.PROCEND
	.align 4
	.EXPORT longjmp,ENTRY,PRIV_LEV=3,ARGW0=GR,ARGW1=GR,RTNVAL=GR
longjmp
	.PROC
	.CALLINFO FRAME=64,NO_CALLS,SAVE_SP,ENTRY_GR=3
	.ENTRY
	ldwm 4(%r26),%r30
	ldwm 4(%r26),%r2
	ldwm 4(%r26),%r3
	ldwm 4(%r26),%r4
	ldwm 4(%r26),%r5
	ldwm 4(%r26),%r6
	ldwm 4(%r26),%r7
	ldwm 4(%r26),%r8
	ldwm 4(%r26),%r9
	ldwm 4(%r26),%r10
	ldwm 4(%r26),%r11
	ldwm 4(%r26),%r12
	ldwm 4(%r26),%r13
	ldwm 4(%r26),%r14
	ldwm 4(%r26),%r15
	ldwm 4(%r26),%r16
	ldwm 4(%r26),%r17
	ldwm 4(%r26),%r18
	ldwm 4(%r26),%r27
#ifdef FP
	ldwm 4(%r26),%r28
	stw %r28,-16(%r30)
	ldwm 4(%r26),%r28
	stw %r28,-12(%r30)
	fldds -16(%r30),%fr12
	ldwm 4(%r26),%r28
	stw %r28,-16(%r30)
	ldwm 4(%r26),%r28
	stw %r28,-12(%r30)
	fldds -16(%r30),%fr13
	ldwm 4(%r26),%r28
	stw %r28,-16(%r30)
	ldwm 4(%r26),%r28
	stw %r28,-12(%r30)
	fldds -16(%r30),%fr14
	ldwm 4(%r26),%r28
	stw %r28,-16(%r30)
	ldwm 4(%r26),%r28
	stw %r28,-12(%r30)
	fldds -16(%r30),%fr15
#endif
	comclr,<> %r0,%r25,%r0
	ldi 1,%r25
	bv 0(%r2)
	copy %r25,%r28
	.EXIT
	.PROCEND

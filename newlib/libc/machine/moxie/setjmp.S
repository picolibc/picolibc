/* A setjmp.c for Moxie
   Copyright (C) 2009  Anthony Green

   The authors hereby grant permission to use, copy, modify, distribute,
   and license this software and its documentation for any purpose, provided
   that existing copyright notices are retained in all copies and that this
   notice is included verbatim in any distributions. No written agreement,
   license, or royalty fee is required for any of the authorized uses.
   Modifications to this software may be copyrighted by their authors
   and need not follow the licensing terms described here, provided that
   the new terms are clearly indicated on the first page of each file where
   they apply.  */

# setjmp/longjmp for moxie.  The jmpbuf looks like this:
#	
# Register	jmpbuf offset
#   $r0		  0x00
#   $r1		  0x04
#   $r2		  0x08
#   $r3		  0x0c
#   $r4		  0x10
#   $r5		  0x14
#   $r6		  0x18
#   $r7		  0x1c
#   $r8		  0x20
#   $r9		  0x24
#   $r10	  0x28
#   $r11	  0x2c
#   $r12	  0x30
#   $r13	  0x34
#   $fp		  0x38
#   $sp		  0x3c

	.text
	.global	setjmp
	.type	setjmp,@function
setjmp:
	st.l	($r0),     $r0
	sto.l	0x04($r0), $r1
	sto.l	0x08($r0), $r2
	sto.l	0x0c($r0), $r3
	sto.l	0x10($r0), $r4
	sto.l	0x14($r0), $r5
	sto.l	0x18($r0), $r6
	sto.l	0x1c($r0), $r7
	sto.l	0x20($r0), $r8
	sto.l	0x24($r0), $r9
	sto.l	0x28($r0), $r10
	sto.l	0x2c($r0), $r11
	sto.l	0x30($r0), $r12
	sto.l	0x34($r0), $r13
	sto.l	0x38($r0), $sp
	sto.l	0x3c($r0), $fp
	xor	$r0, $r0
	ret
.Lend1:
	.size	setjmp,.Lend1-setjmp

	.global	longjmp
	.type	longjmp,@function
longjmp:
	ldi.l	$r2, 0x00
	cmp	$r1, $r2
	beq	.Lreturn1
	ldo.l	$r1, 0x04($r0)
	ldo.l	$r2, 0x08($r0)
	ldo.l	$r3, 0x0c($r0)
	ldo.l	$r4, 0x10($r0)
	ldo.l	$r5, 0x14($r0)
	ldo.l	$r6, 0x18($r0)
	ldo.l	$r7, 0x1c($r0)
	ldo.l	$r8, 0x20($r0)
	ldo.l	$r9, 0x24($r0)
	ldo.l	$r10, 0x28($r0)
	ldo.l	$r11, 0x2c($r0)
	ldo.l	$r12, 0x30($r0)
	ldo.l	$r13, 0x34($r0)
	ldo.l	$sp, 0x38($r0)
	ldo.l	$fp, 0x3c($r0)
	mov	$r0, $r1
	ret
.Lreturn1:
	ldo.l	$r1, 0x04($r0)
	ldo.l	$r2, 0x08($r0)
	ldo.l	$r3, 0x0c($r0)
	ldo.l	$r4, 0x10($r0)
	ldo.l	$r5, 0x14($r0)
	ldo.l	$r6, 0x18($r0)
	ldo.l	$r7, 0x1c($r0)
	ldo.l	$r8, 0x20($r0)
	ldo.l	$r9, 0x24($r0)
	ldo.l	$r10, 0x28($r0)
	ldo.l	$r11, 0x2c($r0)
	ldo.l	$r12, 0x30($r0)
	ldo.l	$r13, 0x34($r0)
	ldo.l	$sp, 0x38($r0)
	ldo.l	$fp, 0x3c($r0)
	ldi.l	$r0, 0x01
	ret
.Lend2:
	.size	longjmp,.Lend2-longjmp

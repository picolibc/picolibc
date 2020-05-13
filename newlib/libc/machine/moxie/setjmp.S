/* A setjmp.c for Moxie
   Copyright (C) 2009, 2019  Anthony Green

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
# Register        jmpbuf offset
#   $r6             0x00
#   $r7             0x04
#   $r8             0x08
#   $r9             0x0c
#   $r10            0x10
#   $fp             0x14
#   $sp             0x18
#   stack frame fp  0x1c
#   stack frame ra  0x20
#   stack frame sc  0x25
       
        .text
        .global setjmp
        .type   setjmp,@function
setjmp:
	st.l	($r0),     $r6
	sto.l	0x04($r0), $r7
	sto.l	0x08($r0), $r8
	sto.l	0x0c($r0), $r9
	sto.l	0x10($r0), $r10
	sto.l	0x14($r0), $sp
	sto.l	0x18($r0), $fp
	ldo.l   $r1,       0x00($fp)
	sto.l   0x1c($r0), $r1
	ldo.l   $r1,       0x04($fp)
	sto.l   0x20($r0), $r1
	ldo.l   $r1,       0x08($fp)
	sto.l   0x24($r0), $r1
	xor	$r0, $r0
	ret
.Lend1:
	.size	setjmp,.Lend1-setjmp

	.global	longjmp
	.type	longjmp,@function
longjmp:
	ldo.l	$r6, 0x00($r0)
	ldo.l	$r7, 0x04($r0)
	ldo.l	$r8, 0x08($r0)
	ldo.l	$r9, 0x0c($r0)
	ldo.l	$r10, 0x10($r0)
	ldo.l	$sp, 0x14($r0)
	ldo.l	$fp, 0x18($r0)
	ldo.l   $r2, 0x1c($r0)
	sto.l   0x0($fp), $r2
	ldo.l   $r2, 0x20($r0)
	sto.l   0x4($fp), $r2
	ldo.l   $r2, 0x24($r0)
	sto.l   0x8($fp), $r2
	ldo.l	$r2, 0x08($r0)
	mov	$r0, $r1
	xor	$r2, $r2
	cmp	$r0, $r2
	beq	.Lreturn1
	ret
.Lreturn1:
	inc     $r0, 1
	ret
.Lend2:
	.size	longjmp,.Lend2-longjmp

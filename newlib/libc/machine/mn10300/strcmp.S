	.file	"strcmp.S"

	.section .text
	.global _strcmp
	.type	 _strcmp,@function
_strcmp:
	movm [d2,d3,a2,a3],(sp)
	mov d0,a0
	mov d1,a1
#ifndef __OPTIMIZE_SIZE__
	or d1,d0
	btst 3,d0
	bne .L11
	mov (a0),d1
	mov (a1),d0
	cmp d0,d1
	bne .L11
	mov -16843009,d3
.L5:
	setlb
	mov (a0),d0
	mov d3,d1
	add d0,d1
	not d0
	and d0,d1
	and -2139062144,d1
	beq .L6
	clr d0
	jmp .L13
.L6:
	inc4 a0
	inc4 a1
	mov (a0),d1
	mov (a1),d0
	cmp d0,d1
	leq
#endif
.L11:
	setlb
	movbu (a1),d2
	movbu (a0),d0
	cmp 0,d0
	beq .L9
	cmp d2,d0
	bne .L9
	inc a0
	inc a1
	lra
.L9:
	sub d2,d0
.L13:
	ret [d2,d3,a2,a3],16
.Lfe1:
	.size	 _strcmp,.Lfe1-_strcmp

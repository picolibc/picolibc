	.file "strlen.S"

	.section .text
	.global _strlen
	.type	 _strlen,@function
_strlen:
	movm [d2,d3,a2,a3],(sp)
	mov d0,a0
	mov a0,a2
#ifndef __OPTIMIZE_SIZE__
	btst 3,d0
	bne .L21
	mov (a0),d0
	mov -16843009,a1
	mov a1,d1
	add d0,d1
	not d0
	and d0,d1
	mov -2139062144,d2
	btst -2139062144,d1
	bne .L21
.L15:
	setlb
	inc4 a0
	mov (a0),d0
	mov a1,d1
	add d0,d1
	not d0
	and d0,d1
	and d2,d1
	leq
	jmp .L21
#endif
.L19:
	inc a0
.L21:
	movbu (a0),d3
	cmp 0,d3
	bne .L19
	sub a2,a0
	mov a0,d0
	ret [d2,d3,a2,a3],16
.Lfe1:
	.size	 _strlen,.Lfe1-_strlen

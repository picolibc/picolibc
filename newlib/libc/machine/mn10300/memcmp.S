	.file "memcmp.S"

	.section .text
	.global _memcmp
	.type	 _memcmp,@function
_memcmp:
	movm [d2,d3,a2,a3],(sp)
	mov d0,a0
	mov d1,a1
	mov (28,sp),a2
#ifndef __OPTIMIZE_SIZE__
	cmp 3,a2
	bls .L22
	mov a1,d2
	or d2,d0
	btst 3,d0
	bne .L22
.L17:
	setlb
	mov (a0),d1
	mov (a1),d0
	cmp d0,d1
	bne .L22
	inc4 a0
	inc4 a1
	add -4,a2
	cmp 3,a2
	lhi
#endif
.L22:
	cmp 0,a2
	beq .L24
.L18:
	setlb
	movbu (a0),d3
	movbu (a1),d2
	cmp d2,d3
	beq .L23
	mov d3,d0
	sub d2,d0
	jmp .L25
.L23:
	inc a0
	inc a1
.L26:
	sub 1,a2
	lne
.L24:
	clr d0
.L25:
	ret [d2,d3,a2,a3],16
.Lfe1:
	.size	 _memcmp,.Lfe1-_memcmp

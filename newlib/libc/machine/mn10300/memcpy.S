	.file "memcpy.S"

	.section .text
	.global _memcpy
	.type	 _memcpy,@function
_memcpy:
	movm [d2,d3,a2,a3],(sp)
	mov d0,d2
	mov d1,a0
	mov d2,a1
	mov (28,sp),d1
#ifndef __OPTIMIZE_SIZE__
	mov a0,d0
	or d2,d0
	btst 3,d0
	bne .L37
	cmp 15,d1
	bls .L34
.L29:
	setlb
	mov (a0),d0
	mov d0,(a1)
	inc4 a0
	inc4 a1
	mov (a0),d0
	mov d0,(a1)
	inc4 a0
	inc4 a1
	mov (a0),d0
	mov d0,(a1)
	inc4 a0
	inc4 a1
	mov (a0),d0
	mov d0,(a1)
	inc4 a0
	inc4 a1
	add -16,d1
	cmp 15,d1
	lhi
.L34:
	cmp 3,d1
	bls .L37
.L33:
	setlb
	mov (a0),d0
	mov d0,(a1)
	inc4 a0
	inc4 a1
	add -4,d1
	cmp 3,d1
	lhi
#endif
.L37:
	cmp 0,d1
	beq .L36
.L38:
	setlb
	movbu (a0),d0
	movbu d0,(a1)
	inc a0
	inc a1
	sub 1,d1
	lne
.L36:
	mov d2,a0
	ret [d2,d3,a2,a3],16
.Lfe1:
	.size	 _memcpy,.Lfe1-_memcpy

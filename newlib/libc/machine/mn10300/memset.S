	.file "memset.S"

	.section .text
	.global _memset
	.type	 _memset,@function
_memset:
	movm [d2,d3,a2,a3],(sp)
	mov d0,d3
	mov d1,d2
	mov (28,sp),a1
	mov d3,a0
#ifndef __OPTIMIZE_SIZE__
	cmp 3,a1
	bls .L41
	btst 3,d3
	bne .L41
	extbu d2
	mov d2,d1
	asl 8,d1
	or d2,d1
	mov d1,d0
	asl 16,d0
	or d0,d1
	cmp 15,a1
	bls .L36
.L33:
	setlb
	mov d1,(a0)
	inc4 a0
	mov d1,(a0)
	inc4 a0
	mov d1,(a0)
	inc4 a0
	mov d1,(a0)
	inc4 a0
	add -16,a1
	cmp 15,a1
	lhi
.L36:
	cmp 3,a1
	bls .L41
.L37:
	setlb
	mov d1,(a0)
	inc4 a0
	add -4,a1
	cmp 3,a1
	lhi
#endif
.L41:
	cmp 0,a1
	beq .L47
.L46:
	setlb
	movbu d2,(a0)
	inc a0
	sub 1,a1
	lne
.L47:
	mov d3,a0
	ret [d2,d3,a2,a3],16
.Lfe1:
	.size	 _memset,.Lfe1-_memset

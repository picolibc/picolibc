	.file "strcpy.S"

	.section .text
	.global _strcpy
	.type	 _strcpy,@function
_strcpy:
	movm [d2,d3,a2,a3],(sp)
	mov d0,d3
	mov d1,a0
	mov d3,a1
	mov a0,d0
#ifndef __OPTIMIZE_SIZE__
	or d3,d0
	btst 3,d0
	bne .L2
	mov (a0),d0
	mov -16843009,a2
	mov a2,d1
	add d0,d1
	not d0
	and d0,d1
	mov -2139062144,d2
	btst -2139062144,d1
	bne .L2
.L5:
	setlb
	mov (a0),d0
	mov d0,(a1)
	inc4 a0
	inc4 a1
	mov (a0),d0
	mov a2,d1
	add d0,d1
	not d0
	and d0,d1
	and d2,d1
	leq
#endif
.L2:
.L7:
	setlb
	movbu (a0),d0
	movbu d0,(a1)
	inc a0
	inc a1
	cmp 0,d0
	lne
	mov d3,a0
	ret [d2,d3,a2,a3],16
.Lfe1:
	.size	 _strcpy,.Lfe1-_strcpy

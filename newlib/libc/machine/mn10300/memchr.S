	.file "memchr.S"

	.section .text
	.global _memchr
	.type	 _memchr,@function
_memchr:
	movm [d2,d3,a2,a3],(sp)
	mov d0,a0
	mov d1,d2
	mov (28,sp),a1
#ifndef __OPTIMIZE_SIZE__
	cmp 3,a1
	bls .L44
	mov a0,d3
	btst 3,d3
	bne .L44
	mov a0,a2
	mov 0,a3
	clr d1
.L29:
	setlb
	mov a3,d0
	asl 8,d0
	mov d2,a3
	add d0,a3
	inc d1
	cmp 3,d1
	lls
	cmp 3,a1
	bls .L48
.L33:
	mov (a2),d0
	mov a3,d3
	xor d3,d0
	mov d0,d1
	not d1
	add -16843009,d0
	and d1,d0
	btst -2139062144,d0
	beq .L34
	mov a2,a0
	clr d1
.L38:
	setlb
	movbu (a0),d0
	cmp d2,d0
	beq .L47
	inc a0
	inc d1
	cmp 3,d1
	lls
.L34:
	add -4,a1
	inc4 a2
	cmp 3,a1
	bhi .L33
.L48:
	mov a2,a0
#endif
.L44:
	cmp 0,a1
	beq .L50
.L49:
	setlb
	movbu (a0),d0
	cmp d2,d0
	beq .L47
	inc a0
	sub 1,a1  
	lne
.L50:
	mov 0,a0
.L47:
	ret [d2,d3,a2,a3],16
.Lfe1:
	.size	 _memchr,.Lfe1-_memchr

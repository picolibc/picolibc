	.file "strchr.S"

	.section .text
	.global _strchr
	.type	 _strchr,@function
_strchr:
	movm [d2,d3,a2,a3],(sp)
	add -12,sp
	mov d0,a1
	movbu d1,(7,sp)
#ifndef __OPTIMIZE_SIZE__
	btst 3,d0
	bne .L20
	clr d0
.L24:
	setlb
	mov sp,a2
	mov d0,d3
	add d3,a2
	mov a2,a0
	add 12,a0
	movbu (7,sp),d3
	movbu d3,(-4,a0)
	inc d0
	cmp 3,d0
	lls
	mov a1,a0
	mov -16843009,a1
	mov (a0),d2
	mov a1,d1
	add d2,d1
	mov d2,d0
	not d0
	and d0,d1
	mov -2139062144,d3
	mov d3,(0,sp)
	btst -2139062144,d1
	bne .L27
	jmp .L38
.L28:
	inc4 a0
	mov (a0),d2
	mov a1,d1
	add d2,d1
	mov d2,d0
	not d0
	and d0,d1
	mov (0,sp),d3
	and d3,d1
	bne .L27
.L38:
	mov (8,sp),d0
	xor d2,d0
	mov a1,d1
	add d0,d1
	not d0
	and d0,d1
	and d3,d1
	beq .L28
.L27:
	mov a0,a1
#endif
.L20:
	movbu (a1),d0
	cmp 0,d0
	beq .L32
	movbu (7,sp),d1
.L35:
	setlb
	cmp d1,d0
	beq .L36
	inc a1
	movbu (a1),d0
	cmp 0,d0
	lne
.L32:
	movbu (7,sp),d0
	movbu (a1),d3
	cmp d0,d3
	beq .L36
	mov 0,a0
	jmp .L37
.L36:
	mov a1,a0
.L37:
	ret [d2,d3,a2,a3],28
.Lfe1:
	.size	 _strchr,.Lfe1-_strchr

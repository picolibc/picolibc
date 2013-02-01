# NEC V850 startup code

	.section .text
	.global	_start

_start:

#if defined(__v850e__) || defined(__v850e2__) || defined(__v850e2v3__) || defined(__v850e3v5__)
	
	movea   255,		r0,	r20
	mov     65535,		r21
	mov     hilo(_stack),	sp
	mov     hilo(__ep),	ep
	mov     hilo(__gp),	gp
	mov     hilo(__ctbp),	r6
	ldsr    r6,             ctbp
#if defined(__v850e2v3__) || defined(__v850e3v5__)
	// FPU enable
	stsr    psw,            r6
	movhi   1,              r0,      r7
	or      r7,             r6
	ldsr    r6,             psw
	// Initialize the FPSR
	movhi	2,		r0,	r6
	ldsr    r6, 	    	fpsr
#endif
	mov     hilo(_edata),	r6
	mov     hilo(_end),	r7
.L0:
	st.w    r0,		0[r6]
	addi    4,		r6,	r6
	cmp     r7,		r6
	bl      .L0
.L1:
	jarl    ___main,	r31
	addi    -16,		sp,	sp
	mov     0,		r6
	mov     0,		r7
	mov     0,		r8
	jarl    _main,		r31
	mov     r10,		r6
	jarl    _exit,		r31

# else
	movea   255,		r0,	r20
	mov     r0,		r21
	ori     65535,		r0,	r21
	movhi   hi(_stack),	r0,	sp
	movea   lo(_stack),	sp,	sp
	movhi   hi(__ep),	r0,	ep
	movea   lo(__ep),	ep,	ep
	movhi   hi(__gp),	r0,	gp
	movea   lo(__gp),	gp,	gp

	movhi   hi(_edata),	r0,	r6
	movea   lo(_edata),	r6,	r6
	movhi   hi(_end),	r0,	r7
	movea   lo(_end),	r7,	r7
.L0:
	st.b    r0,		0[r6]
	addi    1,		r6,	r6
	cmp     r7,		r6
	bl      .L0
.L1:
	jarl    ___main,	r31
	addi    -16,		sp,	sp
	mov     0,		r6
	mov     0,		r7
	mov     0,		r8
	jarl    _main,		r31
	mov     r10,		r6
	jarl    _exit,		r31
# endif

	.section .stack
_stack:	.long 	1

	.section .data
	.global ___dso_handle
	.weak   ___dso_handle
___dso_handle:
	.long	0

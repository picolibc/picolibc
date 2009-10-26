# setjmp/longjmp for Renesas RX.
# 
# The jmpbuf looks like this:
#	
# Register	jmpbuf offset
# R0		0x0
# R1		0x4
# R2		0x8
# R3		0xc
# R4		0x10
# R5		0x14
# R6		0x18
# R7		0x1c
# R8		0x20
# R9		0x24
# R10		0x28
# R11		0x2c
# R12		0x30
# R13		0x34
# R14		0x38
# R15		0x3c
# PC		0x40
#
# R1 contains the pointer to jmpbuf:
#
#	int R1 = setjmp (jmp_buf R1)
#	void longjmp (jmp_buf R1, int R2)
#
# The ABI allows for R1-R5 to be clobbered by functions.  We must be
# careful to always leave the stack in a usable state in case an
# interrupt happens.

	.text
	.global	_setjmp
	.type	_setjmp, @function
_setjmp:
	mov.l	r0, [r1]	; save all the general registers
	mov.l	r1, 0x4[r1]	; longjmp won't use this, but someone else might.
	mov.l	r2, 0x8[r1]
	mov.l	r3, 0xc[r1]
	mov.l	r4, 0x10[r1]
	mov.l	r5, 0x14[r1]
	mov.l	r6, 0x18[r1]
	mov.l	r7, 0x1c[r1]
	mov.l	r8, 0x20[r1]
	mov.l	r9, 0x24[r1]
	mov.l	r10, 0x28[r1]
	mov.l	r11, 0x2c[r1]
	mov.l	r12, 0x30[r1]
	mov.l	r13, 0x34[r1]
	mov.l	r14, 0x38[r1]
	mov.l	r15, 0x3c[r1]
	mov.l	[r0], r2	; get return address off the stack
	mov.l	r2, 0x40[r1]	; PC
	mov	#0, r1		; Return 0.
	rts
.Lend1:
	.size	_setjmp, .Lend1 - _setjmp


	.global	_longjmp
	.type	_longjmp, @function
_longjmp:
	tst	r2, r2		; Set the Z flag if r2 is 0.
	stz	#1, r2		; If the Z flag was set put 1 into the return register.
	mov	r2, 4[r1]	; Put r2 (our return value) into the setjmp buffer as r1.

	mov.l	[r1], r0	; Restore the stack - there's a slot for PC
	mov.l	0x40[r1], r2	; Get the saved PC
	mov.l	r2, [r0]	; Overwrite the old return address

	mov.l	0x3c[r1], r15
	mov.l	0x38[r1], r14
	mov.l	0x34[r1], r13
	mov.l	0x30[r1], r12
	mov.l	0x2c[r1], r11
	mov.l	0x28[r1], r10
	mov.l	0x24[r1], r9
	mov.l	0x20[r1], r8
	mov.l	0x1c[r1], r7
	mov.l	0x18[r1], r6
	mov.l	0x14[r1], r5
	mov.l	0x10[r1], r4
	mov.l	0xc[r1], r3
	mov.l	0x8[r1], r2
	mov.l	0x4[r1], r1	; This sets up the new return value
	rts
.Lend2:
	.size	_longjmp, .Lend2 - _longjmp

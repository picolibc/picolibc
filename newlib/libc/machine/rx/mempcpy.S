	.file	"mempcpy.S"

	.section .text
	.global  _mempcpy
	.type	 _mempcpy,@function
_mempcpy:
#ifdef __RX_DISALLOW_STRING_INSNS__
	/* Do not use the string instructions - they might prefetch
	   bytes from outside of valid memory.  This is particularly
	   dangerous in I/O space.  */

	cmp	 #0, r3	      	; If the count is zero, do nothing
	beq	 2f
	
1:	mov.b	 [r2+], r5
	mov.b	 r5, [r1+]
	sub	 #1, r3
	bne	 1b

2:	rts
#else
	smovf
	rts
#endif
	.size _mempcpy, . - _mempcpy

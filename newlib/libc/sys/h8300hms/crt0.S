; h8/300 and h8/300h start up file.

#ifdef __H8300__

	.section .text
	.global	_start
_start:
	mov.w	#_stack,sp
	mov.w	#_edata,r0
	mov.w	#_end,r1
	sub.w   r2,r2
.loop:	mov.w	r2,@r0
	adds	#2,r0
	cmp	r1,r0
	blo	.loop
	jsr	@___main
	jsr	@_main
	jsr	@_exit

	.section .stack
_stack:	.word 	1

#endif

#ifdef __H8300H__

	.h8300h
	.section .text
	.global	_start
_start:
	mov.l	#_stack,sp
	mov.l	#_edata,er0
	mov.l	#_end,er1
	sub.w   r2,r2           ; not sure about alignment requirements
.loop:	mov.w	r2,@er0		; playing it safe for now
	adds	#2,er0
	cmp.l	er1,er0
	blo	.loop
	jsr	@___main
	jsr	@_main
	jsr	@_exit

	.section .stack
_stack:	.long 	1

#endif

#ifdef __H8300S__

	.h8300s
	.section .text
	.global	_start
_start:
	mov.l	#_stack,sp
	mov.l	#_edata,er0
	mov.l	#_end,er1
	sub.w   r2,r2           ; not sure about alignment requirements
.loop:	mov.w	r2,@er0		; playing it safe for now
	adds	#2,er0
	cmp.l	er1,er0
	blo	.loop
	jsr	@___main
	jsr	@_main
	jsr	@_exit

	.section .stack
_stack:	.long 	1

#endif

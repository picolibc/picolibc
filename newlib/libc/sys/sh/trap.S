	.text
	.global	___trap34
___trap34:
	trapa	#34
	tst	r1,r1	! r1 is errno
	bt	ret
	mov.l	perrno,r2
	mov.l	r1,@r2
ret:
	rts
	nop
	
	.align	2
perrno:
	.long	_errno

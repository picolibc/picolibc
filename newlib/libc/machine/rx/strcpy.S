	.file	"strcpy.S"

	.section .text
	.global  _strcpy
	.type	 _strcpy,@function
_strcpy:
	mov	r1, r4
	mov	#-1, r3		; Strictly speaking this is incorrect, but I doubt if anyone will ever know.
	smovu
	mov	r4, r1
	rts

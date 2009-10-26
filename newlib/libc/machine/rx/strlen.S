	.file	"strlen.S"

	.section .text
	
	.global  _strlen
	.type	 _strlen,@function
_strlen:
	add	#0, r1, r4	; Save a copy of the string start address and set the condition flags.
	beq     null_string	; Test for a NULL pointer.
	mov	#-1, r3		; Set a limit on the number of bytes examined.
	mov	#0,  r2		; Stop searching when we find a NUL byte.
	suntil.b     		; Search until *r1 == r2
	sub	#1, r1		; suntil.b leaves r1 pointing to the byte beyond the match.
null_string:
	sub	r4, r1		; Compute the length.
	rts

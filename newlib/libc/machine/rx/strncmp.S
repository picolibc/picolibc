	.file	"strncmp.S"

	.section .text
	.global  _strncmp
	.type	 _strncmp,@function
_strncmp:
	scmpu			; Perform the string comparison
	bnc	1f		; If Carry is not set skip over
	scne.L	r1		; Set result based on Z flag
	rts			;
1:				;
	mov	#-1,r1		; Carry not set, result should be negative
	rts			;

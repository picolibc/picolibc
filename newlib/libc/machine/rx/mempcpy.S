	.file	"mempcpy.S"

	.section .text
	.global  _mempcpy
	.type	 _mempcpy,@function
_mempcpy:
	smovf
	rts

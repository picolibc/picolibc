/*
Copyright (c) 2009 Nick Clifton <nickc@redhat.com>
 */
	.file	"memset.S"

	.section .text
	.global  _memset
	.type	 _memset,@function
_memset:
	mov	r1, r4
	sstr.b
	mov	r4, r1
	rts

	.size _memset, . - _memset
	

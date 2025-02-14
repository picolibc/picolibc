/*
Copyright (c) 2009 Nick Clifton <nickc@redhat.com>
 */
#include <picolibc.h>

#ifdef __RX_ALLOW_STRING_INSNS__
	.file	"mempcpy.S"

	.section .text
	.global  _mempcpy
	.type	 _mempcpy,@function
_mempcpy:
	smovf
	rts
	.size _mempcpy, . - _mempcpy
#endif

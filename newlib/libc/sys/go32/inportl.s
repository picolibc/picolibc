#/* This is file INPORTL.S */
#/*
#** Copyright (C) 1991 DJ Delorie
#**
#** This file is distributed under the terms listed in the document
#** "copying.dj".
#** A copy of "copying.dj" should accompany this file; if not, a copy
#** should be available from where this file was obtained.  This file
#** may not be distributed without a verbatim copy of "copying.dj".
#**
#** This file is distributed WITHOUT ANY WARRANTY; without even the implied
#** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#*/

	.globl	_inportl
_inportl:
	movl	4(%esp),%edx
	inl	%dx,%eax
	ret

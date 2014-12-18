/* Copyright (c) 2012, 2013 Red Hat, Inc. All rights reserved.

   This copyrighted material is made available to anyone wishing to use, modify,
   copy, or redistribute it subject to the terms and conditions of the BSD
   License.   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY expressed or implied, including the implied warranties
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  A copy of this license
   is available at http://www.opensource.org/licenses. Any Red Hat trademarks that
   are incorporated in the source code or documentation are not subject to the BSD
   License and may only be used or replicated with the express permission of
   Red Hat, Inc.
*/

/* Empty syscall definitions for when we run on real hardware.  */

#include "../syscall.h"
#include "memmodel.h"
	
#define ENOSYS 88

.macro	sc,a
	sc2 \a,\a
.endm
	
.macro	sc2,name,num
	.weak	\name
	.global	\name
\name:
        call_   #__errno                 
        movx_   #ENOSYS, @R12                
	MOV.W	#-1,R12
	ret_
.endm

	.weak	exit
	.weak	_exit
	.global	exit
	.global	_exit
exit:
_exit:
	/* For some reason, the board fails to stop at a breakpoint
	   placed on top of a software breakpoint instruction.  */
/*	MOV.B	#0,R3		; this is a software breakpoint instruction */
1:	br_	#1b

#define SC(n) sc2 n,SYS_##n

	SC (open)
	SC (close)
	SC (read)
/*	SC (write)*/
	SC (fstat)
	SC (lseek)
	SC (kill)

	.weak	isatty
	.global	isatty
isatty:
	.weak	_isatty
	.global	_isatty
_isatty:
	MOV	#1,R12
	ret_
	
	.weak	getpid
	.global	getpid
getpid:
	MOV	#42,R12
	ret_

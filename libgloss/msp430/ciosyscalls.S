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

.macro START_FUNC name1, name2=foo
	.pushsection .text.\name1,"ax",@progbits
	.p2align 1
	.weak	\name1
	.global	\name1
\name1:
  .ifnc \name2,foo
	.weak	\name2
	.global	\name2
\name2:
  .endif
.endm

.macro END_FUNC name1, name2=foo
	.type \name1 , @function
	.size \name1 , . - \name1
  .ifnc \name2,foo
	.type \name2 , @function
	.size \name2 , . - \name2
  .endif
	.popsection
.endm


START_FUNC exit, _exit
	/* For some reason, the board fails to stop at a breakpoint
	   placed on top of a software breakpoint instruction.  */
/*	MOV.B	#0,R3		; this is a software breakpoint instruction */
1:	br_	#1b
END_FUNC exit, _exit
	

START_FUNC isatty,_isatty
	MOV	#1,R12
	ret_
END_FUNC isatty,_isatty


START_FUNC getpid
	MOV	#42,R12
	ret_
END_FUNC getpid


.macro	sc2,name,num
	START_FUNC \name
        call_   #__errno                 
        movx_   #ENOSYS, @R12                
	MOV.W	#-1,R12
	ret_
	END_FUNC \name
.endm

#define SC(n) sc2 n,SYS_##n

	SC (open)
	SC (close)
	SC (read)
/*	SC (write)*/
	SC (fstat)
	SC (lseek)
	SC (kill)

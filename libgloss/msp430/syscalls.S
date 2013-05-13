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

/* Be wary: the lower N bits of the *address* of the function
   determines the syscall used by the simulator.  Thus, the addresses
   listed here depend on the syscall numbers in ../syscalls.h.  */

/* As per the MSP430x200 Family Users Guide, section 1.5, "An
   instruction fetch from the address range 0x0000 - 0x01FF will reset
   the device."  We take advantage of that to do syscalls in the
   simulator, by trying to execute specific addresses in that range
   and letting the simulator catch them while simulating the CALL
   instruction.  In theory, this is an operation that the physical
   hardware will never attempt to do, so it won't interfere with the
   simulation's accuracy (i.e. we aren't abusing holes in the opcode
   map, for example).  */

#include "../syscall.h"
#include "memmodel.h"
	
.macro	sc,a
	sc2 \a,\a
.endm
	
.macro	sc2,name,num
	.weak	\name
	.global	\name
	\name = 0x180 + \num
.endm

#define SC(n) sc2 n,SYS_##n

	sc2	_exit,SYS_exit
	SC (exit)
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

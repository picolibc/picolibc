/*

Copyright (c) 2011 Red Hat Incorporated.
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met: 

    Redistributions of source code must retain the above copyright 
    notice, this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    The name of Red Hat Incorporated may not be used to endorse 
    or promote products derived from this software without specific 
    prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL RED HAT INCORPORATED BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 

*/

#include "syscall.h"
#include "vregs.h"

#define SYS__exit  SYS_exit

	.macro syscall_body  number
	 /* The RL78 doesn't really have an "interrupt" upcode, just
	    BRK, which we emulate exactly.  We use the STOP opcode,
	    which is a breakpoint in the simulator.  */
	mov	A, #\number
	stop
	ret
	.endm
      
	.macro do_syscall  name number
__\name:
	.global __\name
_\name:
        .weak _\name
	syscall_body \number
	.endm

   .macro	syscall_returns	name number
__\name:
	.global __\name
_\name:
        .weak _\name
	mov	r8, #\number
	ret
	.endm

#define S(name)         do_syscall	name, SYS_##name
#define SYSCALL(number)	syscall_body	number
#define ERR(name)	syscall_returns	name, -1
#define OK(name)	syscall_returns	name, 0
#define RET(name,val)	syscall_returns	name, val

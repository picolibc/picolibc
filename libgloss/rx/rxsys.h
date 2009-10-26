/*

Copyright (c) 2005, 2009 Red Hat Incorporated.
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

    /* This mixture of C pre-processor macros and assembler
       macros is to get around the fact that the RX assembler
       does not support a line separator character.  */

#define SYS__exit  SYS_exit

	.macro syscall_body  number
	 /* We use interrupt 255 as a system call,
	    with the call number in r5.  */
        mov	# \number, r5
	int	# 255
	rts
	.endm
      
    
	.macro do_syscall  name number
__\name:
	.global __\name
_\name:
        .weak _\name
	syscall_body \number
	.endm
      

#define S(name)         do_syscall    name SYS_##name
#define SYSCALL(number) syscall_body  number

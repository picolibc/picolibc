/*
Copyright (c) 2013 Andes Technology Corporation.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

    The name of the company may not be used to endorse or promote
    products derived from this software without specific prior written
    permission.

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
#ifndef _SYSCALL_EXTRA_H
#define _SYSCALL_EXTRA_H


/* These are additional syscalls for nds32 target. */
#define SYS_rename       3001
#define SYS_isatty       3002
#define SYS_system       3003

#define SYS_geterr       6001


/* Define macros that generate assembly output.  */
.macro SYS_WRAPPER name num
	.text
	.global	\name
	.type	\name, @function
	.align	2
\name:
	/* Make syscall with arg=`\num'.
	   Reture value `-1' stored in $r0 means there is something wrong.
	   If there is something wrong, make syscall to get `SYS_geterr' to get
	   error code to see what exactly happens and store it in errno  .  */
	syscall	\num		/* Make syscall with arg=`\num'.  */
	addi	$r1, $r0, 1
	bnez	$r1, 1f		/* Branch if success.  */
	syscall	SYS_geterr	/* There is something wrong.  */
	s.w	$r0, errno	/* Store error code into errno.  */
	move	$r0, -1
1:
	ret
	.size	\name, .-\name
.endm

#endif /* _SYSCALL_EXTRA_H */

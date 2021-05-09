/* SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * syscalls.S -- PRU system calls code
 *
 * Copyright (c) 2018-2019 Dimitar Dimitrov <dimitar@dinux.eu>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "newlib.h"

#include "syscall.h"

	.extern _impure_ptr

	/* Handle return from syscall.  */
	.section .text.__SC_ret, "ax"
	.global	__SC_ret
	.type	__SC_ret,@function
	.func
__SC_ret:
	/* Check for negative return code */
	qbbc	__SC_ret_skip_errno_set, r14, 31

	/* Invert return code and store to errno (first int in _impure_ptr).  */
	rsb	r14, r14, 0
	ldi32	r1, _impure_ptr
	lbbo	r1, r1, 0, 4
	sbbo	r14, r1, 0, 4
	/* Return -1 (for both int32_t or int64_t).  */
	fill	r14, 8

__SC_ret_skip_errno_set:
	ret
	.endfunc

.macro	SC	fname, id
	.section .text.\fname, "ax"
	.global	\fname
	.type	\fname,@function
	.func
\fname:
	ldi	r1, \id
	halt
	jmp	__SC_ret
	.endfunc
.endm

	/* Syscalls are used only by simulator.  Real HW
	   users use other methods for communicating with
	   the host - remoteproc, rpmsg, shared memory.  */
	SC	_exit, SYS_exit
	SC	_open, SYS_open
	SC	_close, SYS_close
	SC	_read, SYS_read
	SC	_write, SYS_write
	SC	_lseek, SYS_lseek
	SC	_unlink, SYS_unlink
	SC	_getpid, SYS_getpid
	SC	_kill, SYS_kill
	SC	_fstat, SYS_fstat

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
#include "../syscall.h"
#include "syscall_extra.h"

	.extern	_end
	.global	_sbrk

	.text
	.align	2
_sbrk:
	/* Get the value stored in heap_end (Top of Heap). If the value is zero,
	   initialize it with the ending of bss section and leave a 1024-byte
	   room to do low memory action.  */
	l.w	$r1, heap_end
	bnez	$r1, .L0
        /* Note: leave 1024 byte to do low memory action.  */
	la	$r1, _end + 1024
	s.w	$r1, heap_end

.L0:
	/* Try to increments heap_end by $r0 bytes. Check if collision happens ?
	   If collision happens, -1 is returned and errno is set to ENOMEM.
	   Otherwise, save new value to heap_end and return the previous
	   heap_end. */

	/* Make sure new heap_end is aligned on 8-byte boundary.  */
	addi	$r0, $r0, 7
	srli	$r0, $r0, 3
	slli	$r0, $r0, 3

	add	$r0, $r1, $r0	/* Set $r0 as new heap_end.  */
	slt	$r2, $sp, $r0	/* Set $r2 if $sp is lower than heap_end.  */
	bnez	$r2, .Lerror	/* Branch if collision happens.  */

	s.w	$r0, heap_end	/* Save new value to heap_end.  */
	move	$r0, $r1	/* Return the previous heap_end.  */
	ret

.Lerror:
	movi	$r0, 12
	s.w	$r0, errno	/* Set error code: 12 (ENOMEM) to errno.  */
	movi	$r0, -1		/* Reture value is -1.  */
	ret

	.section	.bss
	.align	2
heap_end:
	.word	0

/* These functions are almost verbatim FreeBSD code (even if the header of
   one file mentiones NetBSD), just wrapped in the minimum required code to
   make them work under the MS AMD64 ABI.
   See FreeBSD src/lib/libc/amd64/string/bcopy.S */

/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from locore.s.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the University nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

	.seh_proc _memcpy
_memcpy:
	movq	%rsi,8(%rsp)
	movq	%rdi,16(%rsp)
	.seh_endprologue
	movq	%rcx,%rdi
	movq	%rdx,%rsi
	movq	%r8,%rdx

	movq    %rdx,%rcx
	movq    %rdi,%r8
	subq    %rsi,%r8
	cmpq    %rcx,%r8	/* overlapping? */
	jb      1f
	cld                     /* nope, copy forwards. */
	shrq    $3,%rcx		/* copy by words */
	rep movsq
	movq    %rdx,%rcx
	andq    $7,%rcx		/* any bytes left? */
	rep movsb
	jmp	2f
1:
	addq    %rcx,%rdi	/* copy backwards. */
	addq    %rcx,%rsi
	std
	andq    $7,%rcx		/* any fractional bytes? */
	decq    %rdi
	decq    %rsi
	rep movsb
	movq    %rdx,%rcx	/* copy remainder by words */
	shrq    $3,%rcx
	subq    $7,%rsi
	subq    $7,%rdi
	rep movsq
	cld
2:
	movq	8(%rsp),%rsi
	movq	16(%rsp),%rdi
	ret
	.seh_endproc

	.globl  memmove
	.seh_proc memmove
memmove:
	.seh_endprologue
	movq	%rcx,%rax	/* return dst */
	jmp	_memcpy
	.seh_endproc

	.globl  memcpy
	.seh_proc memcpy
memcpy:
	.seh_endprologue
	movq	%rcx,%rax	/* return dst */
	jmp	_memcpy
	.seh_endproc

	.globl  mempcpy
	.seh_proc mempcpy
mempcpy:
	.seh_endprologue
	movq	%rcx,%rax	/* return dst  */
	addq	%r8,%rax	/*         + n */
	jmp	_memcpy
	.seh_endproc

	.globl  wmemmove
	.seh_proc wmemmove
wmemmove:
	.seh_endprologue
	shlq	$1,%r8		/* cnt * sizeof (wchar_t) */
	movq	%rcx,%rax	/* return dst */
	jmp	_memcpy
	.seh_endproc

	.globl  wmemcpy
	.seh_proc wmemcpy
wmemcpy:
	.seh_endprologue
	shlq	$1,%r8		/* cnt * sizeof (wchar_t) */
	movq	%rcx,%rax	/* return dst */
	jmp	_memcpy
	.seh_endproc

	.globl  wmempcpy
	.seh_proc wmempcpy
wmempcpy:
	.seh_endprologue
	shlq	$1,%r8		/* cnt * sizeof (wchar_t) */
	movq	%rcx,%rax	/* return dst */
	addq	%r8,%rax	/*         + n */
	jmp	_memcpy
	.seh_endproc

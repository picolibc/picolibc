/* These functions are almost verbatim FreeBSD code (even if the header of
   one file mentiones NetBSD), just wrapped in the minimum required code to
   make them work under the MS AMD64 ABI.
   See FreeBSD src/lib/libc/amd64/string/memset.S */

/*
 * Written by J.T. Conklin <jtc@NetBSD.org>.
 * Public domain.
 * Adapted for NetBSD/x86_64 by
 * Frank van der Linden <fvdl@wasabisystems.com>
 */

	.globl	memset
	.seh_proc memset
memset:
	movq	%rsi,8(%rsp)
	movq	%rdi,16(%rsp)
	.seh_endprologue
	movq	%rcx,%rdi
	movq	%rdx,%rsi
	movq	%r8,%rdx

	movq    %rsi,%rax
	andq    $0xff,%rax
	movq    %rdx,%rcx
	movq    %rdi,%r11

	cld			/* set fill direction forward */

	/* if the string is too short, it's really not worth the
	 * overhead of aligning to word boundries, etc.  So we jump to
	 * a plain unaligned set. */
	cmpq    $0x0f,%rcx
	jle     L1

	movb    %al,%ah		/* copy char to all bytes in word */
	movl    %eax,%edx
	sall    $16,%eax
	orl     %edx,%eax

	movl    %eax,%edx
	salq    $32,%rax
	orq     %rdx,%rax

	movq    %rdi,%rdx	/* compute misalignment */
	negq    %rdx
	andq    $7,%rdx
	movq    %rcx,%r8
	subq    %rdx,%r8

	movq    %rdx,%rcx	/* set until word aligned */
	rep
	stosb

	movq    %r8,%rcx
	shrq    $3,%rcx		/* set by words */
	rep
	stosq

	movq    %r8,%rcx	/* set remainder by bytes */
	andq    $7,%rcx
L1:     rep
	stosb
	movq    %r11,%rax

	movq	8(%rsp),%rsi
	movq	16(%rsp),%rdi
	ret
	.seh_endproc

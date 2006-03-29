/*  This routine potentially increases the stack size at runtime based on
    the _stklen variable.  Only used by DPMI code.
    Copyright (c) 1993  C. Sandmann
    Environment:  called by crt0.s (and gcrt0.s)
                  EAX, EBX, EBP, EDI, ESI disposable (cleared on return) */

	.text
	.globl	__setstack
__setstack:
	movl	%esp,%eax
	andl	$0xc0000000,%eax	/* clear all except upper bits */
	jne	ok_stack		/* obviously not DPMI! */
	movw	%ss,%ax
	lsll	%eax,%ebx		/* stack segment limit */
	movl	%esp,%eax		/* current location */
	subl	%ebx,%eax		/* Free stack */
	cmpl	%eax,__stklen
	jb	ok_stack

/* Not enough stack.  Call sbrk() to get a new area.  Copy current ESP + 20
   to end of new area (3 args + our stack).  Change ESP to new area.  Set new
   limit to start of new area using DPMI services.  */

	pushl	__stklen
	call	_sbrk			/* eax = got memory base */
	popl	%ebx			/* remove _stklen */
	cmpl	$0xffffffff,%eax	/* if eax = -1 failure */
	je	badstack
	addl	%eax,%ebx		/* ebx now is end of new stack area */
	andl	$0xfffffff0,%ebx	/* 16 byte alignment */
	addl	$0xfff,%eax		/* make stack base page aligned */
	andl	$0xfffff000,%eax	/* 4096 byte alignment */

/* Now copy old stack to new stack.  We only need our part + 4 words, 3 for
   the parameters to pass to main, one for our return EIP (4 extra safety) */
	movl	%esp, %esi		/* Source is current stack */
	subl	$0x20, %ebx		/* 8 longwords */
	movl	%ebx, %edi		/* Destination is new stack */
	movl	$8,%ecx
	rep
	movsl

/* New stack in place.  Change ESP to point to it.  Assumes new stack is
   higher in memory so we don't get caught by limit.  Change limit using 
   DPMI services. */

	movl	%ebx,%esp		/* Switch to new stack */
	subl	$1,%eax			/* Low 12 bits all 1s */
	pushl	%eax			/* Easiest way to move long to words */
	popw	%dx
	popw	%cx
	movl	$8,%eax			/* DPMI function Set Segment Limit */
	movw	%ss,%bx			/* Selector */
	int	$0x31			/* Do service */

	xor	%ecx,%ecx		/* Clean up */
	xor	%edx,%edx		/* Clean up */

ok_stack:
	ret				/* What we have is already bigger */

badstack:
	movl	$0x4c01,%eax
	int	$0x21
	jmp	badstack

	.data
	.globl	__stklen
	.comm	__stklen,4


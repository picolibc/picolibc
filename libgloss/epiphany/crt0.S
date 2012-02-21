# C Startup for EPIPHANY

# Copyright (c) 2011, Adapteva, Inc.
# All rights reserved.

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#  * Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of Adapteva nor the names of its contributors may be
#    used to endorse or promote products derived from this software without
#    specific prior written permission.

# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.


.section IVT,"a",@progbits     ;
        .global _start;
        .type   _start, %function;
_start:
		.balign 4 ;
         b .normal_start

        .balign 4 ; 0x4
         b .sw_exception_v

		.balign 4 ; 0x8
         b .page_miss_v;

		.balign 4 ; 0xc
         b .timer0_expired_v

		.balign 4 ; 0x10
         b .timer1_expired_v

		.balign 4 ; 0x14
		b .message_v

		.balign 4 ; 0x18
		 b .dma0_v

		.balign 4 ; 0x1c
		 b .dma1_v

		.balign 4 ; 0x20
		 b .wand_v

		.balign 4 ; 0x24
		 b .soft_v

.size  _start, .-_start

.section RESERVED_CRT0,"a",@progbits     ;

.global .normal_start;
.balign 4
.type   .normal_start, %function
.normal_start:
	mov  r3,%low(_external_start)
	movt r3,%high(_external_start)
	jalr r3
.size  .normal_start, .-.normal_start




.section .text;
	.org    0x0000                  ; Relative to start of text section
	.global _external_start
	.type   _external_start, %function
_external_start:

	.align  4

	;; Initialise the stack pointer and frame pointer. Hopefully __stack
	;; is somewhere meaningful.
	mov	sp,%low(___stack)
	movt	sp,%high(___stack)
	mov	fp,sp

	;; Zero the data space
	mov	r0,%low(___bss_start)
	movt	r0,%high(___bss_start)
	mov	r1,%low(_end)
	movt	r1,%high(_end)
	mov	r2,#0
	mov	r3,#0
.L0_init_:
	strd	r2,[r0],+#1
	sub	r5,r1,r0
	bne	.L0_init_

	;;  Setup destructors to be called from exit if main never returns
#if 0
	mov	r0,%low(fini)
	movt	r0,%high(fini)
	mov r2,%low(_atexit)
	movt r2,%high(_atexit)
	jalr r2
#else
	; calling atexit drags in malloc, so instead poke the function
	; address directly into the reent structure
	mov	r2,%low(__impure_ptr)
	movt	r2,%high(__impure_ptr)
	ldr	r2,[r2]
	mov	r1,%low(fini)
	movt	r1,%high(fini)
#ifdef __STRUCT_ALIGN_64__
#error
	add	r2,r2,need_to_find_out; &_GLOBAL_REENT->atexit0
	str	r2, [r2,-1];??or -2?; _GLOBAL_REENT->atexit
	mov	r0, 1
	str	r0, [r2,1]	; _GLOBAL_REENT->atexit0._ind
	str	r1, [r2,2]	; _GLOBAL_REENT->atexit0._fns[0]
#else  /* !__STRUCT_ALIGN_64__ */
	add	r0,r2,0x14c	; &_GLOBAL_REENT->atexit0
	str	r0, [r0,-1]	; _GLOBAL_REENT->atexit
	mov	r0, 1
	strd	r0, [r2,0x2a]	; _GLOBAL_REENT->atexit0._ind
#endif /* !__STRUCT_ALIGN_64__ */
#endif /* !0 */
	;; Call global and static constructors
	mov r2,%low(init)
	movt r2,%high(init)
	jalr r2


	;;return from reset ISR
	mov R0,%low(RDS)
	movt R0,%high(RDS)
	movts iret,r0
	rti
RDS:

	;;  Initialise argc, argv and envp to empty and call main
	mov	r0,#0
	mov	r1,#0
	mov	r2,#0
	mov r3,%low(_main)
	movt r3,%high(_main)
	jalr r3
	;;bl	_main

	;; Call exit
	mov r3,%low(_exit)
	movt r3,%high(_exit)
	jalr r3
	;;bl	_exit

	;; Should never reach here
	idle

.size   _external_start, .-_external_start

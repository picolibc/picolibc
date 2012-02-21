# EPIPHANY implementation of wrappers over user C ISR ()

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

.section RESERVED_CRT0,"a",@progbits     ;
.set sw_exception_v_n, 1
.set page_miss_v_n, 2
.set timer0_expired_v_n, 3;
.set timer1_expired_v_n, 4;
.set message_v_n, 5;
.set dma0_v_n, 6;
.set dma1_v_n, 7;
.set wand_v_n, 8;
.set soft_v_n, 9

// preserve isr-mods to ACTIVE, GID, KERNEL, WAND, & EXCAUSE bits in the status reg
.set status_mask, 0x0007000f;
// for the wand isr, preserve isr-mods to ACTIVE, GID, KERNEL, & EXCAUSE, but
// clear the WAND bit.
.set wand_status_mask, 0x00070007;

//IVT wrappers

.global _DEFAULT_ISR_CALLBACK;
.balign 4
.type   _DEFAULT_ISR_CALLBACK, %function
_DEFAULT_ISR_CALLBACK:
	rts
.size  _DEFAULT_ISR_CALLBACK, .-_DEFAULT_ISR_CALLBACK


.global _DEFAULT_ISR_CALLBACK_FOR_sw_exception_v;
.balign 4
.type   _DEFAULT_ISR_CALLBACK_FOR_sw_exception_v, %function
_DEFAULT_ISR_CALLBACK_FOR_sw_exception_v:
	trap 5;; FAIL
.size  _DEFAULT_ISR_CALLBACK_FOR_sw_exception_v, .-_DEFAULT_ISR_CALLBACK_FOR_sw_exception_v


.set NUMBER_ENTRIES_IN_IVT, 8
.global _ISR_VECTOR;
.balign 4
.type   _ISR_VECTOR, %object
_ISR_VECTOR:
	.word _DEFAULT_ISR_CALLBACK_FOR_sw_exception_v; reset
	.word _DEFAULT_ISR_CALLBACK_FOR_sw_exception_v; sw_exception
	.word _DEFAULT_ISR_CALLBACK_FOR_sw_exception_v; page_miss
	.rept   NUMBER_ENTRIES_IN_IVT
		.word _DEFAULT_ISR_CALLBACK
	.endr
.size  _ISR_VECTOR, .-_ISR_VECTOR

.macro IVT_ENTRY_CALL entry_v_
	;; assuming we have valid frame poiter

	str fp, [sp],-0x10
	str r0, [sp,+0xf]
	str r1, [sp,+0xe]
	mov r0, \entry_v_
.if \entry_v_ == wand_v_n
	mov   r1, %low(wand_status_mask)  ; this mask will exclude WAND, so it will
	movt  r1, %high(wand_status_mask) ; be forcibly cleared
.else
	mov   r1, %low(status_mask)
	movt  r1, %high(status_mask)
.endif
	str r1, [sp,0x9] ; save the status register isr-preserve mask on the stack
	b   __dispatcher
.endm

.global __dispatcher;
.balign 4
.type   __dispatcher, %function
__dispatcher:
	str lr, [sp,+0xd]
	movfs.l r1,status
	str r1, [sp,+0xc]
	movfs.l r1,iret
	str r1, [sp,+0xb]
	str r2, [sp,+0xa]
	str r3, [sp,+0x8]

	mov fp,sp

	//TODO allow nesting
	//gie

	lsl r2, r0, 2
	mov	r1, %low(_ISR_VECTOR)
	add r2, r2, r1
	ldr r1, [r2,0] ;; r1 = _ISR_VECTOR[entry_v]
	mov r2, 3
	add r0, r0, r2 ;; r0 = signum
	jalr r1

	mov sp,fp

	//gid

	// preserve isr-modifications to some of the bits in the status register,
	// but restore the rest to pre-interrupt values.
	// status = (status[post_isr] & status_mask) | (status[pre_isr] & ~status_mask)

	ldr.l r2, [sp, 9] ; fetch the status register isr preserve mask from the stack
	movfs r1, status
	and   r1, r1, r2; (status[post_isr] & status_mask)

	ldr  r0, [sp,+0xc]
	mov  r2, %low(~status_mask)
	movt r2, %high(~status_mask)
	and  r0, r0, r2; (status[pre_isr] & ~status_mask)

	orr r0, r0, r1 ; combine pre_isr & post_isr status bits
	movts.l status, r0

	ldr r0, [sp,+0xb]
	movts.l iret,r0

	ldr  lr,[sp,+0xd];
	ldr  r1,[sp,+0xe]
	ldr  r0,[sp,+0xf]
	ldr  r2,[sp,+0xa]
	ldr  r3,[sp,+0x8]
	ldr  fp,[sp],+0x10
	ldr  fp,[sp,0]
	rti
.size  __dispatcher, .-__dispatcher


.global .sw_exception_v;
.type   .sw_exception_v, %function
.sw_exception_v:
	IVT_ENTRY_CALL sw_exception_v_n;
.size  .sw_exception_v, .-.sw_exception_v

.global .page_miss_v;
.type   .page_miss_v, %function
.page_miss_v:
	IVT_ENTRY_CALL page_miss_v_n;
.size  .page_miss_v, .-.page_miss_v

.global .timer0_expired_v;
.timer0_expired_v:
	IVT_ENTRY_CALL timer0_expired_v_n;

.global .timer1_expired_v;
.timer1_expired_v:
	IVT_ENTRY_CALL timer1_expired_v_n;

.global .message_v;
.message_v:
	IVT_ENTRY_CALL message_v_n;

.global .dma0_v;
.dma0_v:
	IVT_ENTRY_CALL dma0_v_n;

.global .dma1_v;
.dma1_v:
	IVT_ENTRY_CALL dma1_v_n;

.global .wand_v;
.wand_v:
	IVT_ENTRY_CALL wand_v_n;

.global .soft_v;
.soft_v:
	IVT_ENTRY_CALL soft_v_n;

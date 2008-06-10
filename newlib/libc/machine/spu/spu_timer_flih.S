/*
(C) Copyright IBM Corp. 2008

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of IBM nor the names of its contributors may be
used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

/*  First-level interrupt handler.  */

/* The following two convenience macros assist in the coding of the
   saving and restoring the volatile register starting from register
   2 up to register 79.

   saveregs     first, last    Saves registers from first to the last.
   restoreregs  first, last    Restores registers from last down to first.

   Note:       first must be less than or equal to last.  */

.macro  saveregs        first, last
	stqd            $\first, -(STACK_SKIP+\first)*16($SP)
.if     \last-\first
	saveregs        "(\first+1)",\last
.endif
.endm


.macro  restoreregs     first, last
	lqd             $\last, (82-\last)*16($SP)
.if     \last-\first
	restoreregs     \first,"(\last-1)"
.endif
.endm

	.section        .interrupt,"ax"
	.align          3
	.type           spu_flih, @function
spu_flih:
	/* Adjust the stack pointer to skip the maximum register save area
	   (STACK_SKIP quadword registers) in case an interrupt occurred while
	   executing a leaf function that used the stack area without actually
	   allocating its own stack frame.  */
	.set            STACK_SKIP, 125

	/* Save the current link register on a new stack frame for the
	   normal spu_flih() version of this file.  */
	stqd            $0,  -(STACK_SKIP+80)*16($SP)
	stqd            $SP, -(STACK_SKIP+82)*16($SP)   /* Save back chain pointer.  */

	saveregs        2, 39

	il              $2,  -(STACK_SKIP+82)*16        /* Stack frame size.  */
	rdch            $3, $SPU_RdEventStat            /* Read event status.  */

	rdch            $6, $SPU_RdEventMask            /* Read event mask.  */
	hbrp                                            /* Open a slot for instruction prefetch.  */

	saveregs        40,59

	clz             $4, $3                          /* Get first slih index.  */
	stqd            $6,  -(STACK_SKIP+1)*16($SP)    /* Save event mask on stack.  */

	saveregs        60, 67

	/* Do not disable/ack the decrementer event here.
	   The timer library manages this and expects it
	   to be enabled upon entry to the SLIH. */
	il              $7, 0x20
	andc            $5, $3, $7
	andc            $7, $6, $5                      /* Clear event bits.  */
	saveregs        68, 69

	wrch            $SPU_WrEventAck, $3             /* Ack events(s) - include decrementer event.  */
	wrch            $SPU_WrEventMask, $7            /* Disable event(s) - exclude decrementer event.  */

	saveregs        70, 79

	a               $SP, $SP, $2                    /* Instantiate flih stack frame.  */
next_event:
	/* Fetch and dispatch the event handler for the first non-zero event. The
	   dispatch handler is indexed into the __spu_slih_handlers array using the
	   count of zero off the event status as an index.  */
	ila             $5, __spu_slih_handlers         /* Slih array offset.  */

	shli            $4, $4, 2                       /* Slih entry offset.  */
	lqx             $5, $4, $5                      /* Load slih address.  */
	rotqby          $5, $5, $4                      /* Rotate to word 0.  */
	bisl            $0, $5                          /* Branch to slih.  */

	clz             $4, $3                          /* Get next slih index.  */
	brnz            $3, next_event


	lqd             $2, 81*16($SP)                  /* Read event mask from stack.  */

	restoreregs     40, 79

	wrch            $SPU_WrEventMask, $2            /* Restore event mask.  */
	hbrp                                            /* Open a slot for instruction pre-fetch.  */

	restoreregs     2, 39

	/* Restore the link register from the new stack frame for the
	   normal spu_flih() version of this file.  */
	lqd             $0,  2*16($SP)

	lqd             $SP, 0*16($SP)                 /* restore stack pointer from back chain ptr.  */

	irete                                          /* Return from interrupt and re-enable interrupts.  */
	.size           spu_flih, .-spu_flih
/* spu_slih_handlers[]
   Here we initialize 33 default event handlers.  The first entry in this array
   corresponds to the event handler for the event associated with bit 0 of
   Channel 0 (External Event Status).  The 32nd entry in this array corresponds
   to bit 31 of Channel 0 (DMA Tag Status Update Event).  The 33rd entry in
   this array is a special case entry to handle "phantom events" which occur
   when the channel count for Channel 0 is 1, causing an asynchronous SPU
   interrupt, but the value returned for a read of Channel 0 is 0.  The index
   calculated into this array by spu_flih() for this case is 32, hence the
   33rd entry.  */
.data
	.align  4
	.extern __spu_default_slih
	.global __spu_slih_handlers
	.type   __spu_slih_handlers, @object
__spu_slih_handlers:
	.rept 33
	.long   __spu_default_slih
	.endr
	.size   __spu_slih_handlers, .-__spu_slih_handlers

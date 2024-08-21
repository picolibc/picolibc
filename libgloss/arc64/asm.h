/*
   Copyright (c) 2024, Synopsys, Inc. All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   1) Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

   2) Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

   3) Neither the name of the Synopsys, Inc., nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _ARC64_ASM_H
#define _ARC64_ASM_H

#if defined (__ARC64_ARCH32__)

/* Define 32-bit word.  */
.macro WORD w
  .word \w
.endm

/* Move register immediate (short): r:reg, i:immediate  */
.macro MOVRI_S r, i
  mov_s	  \r, \i
.endm

/* Move register immediate (short): r:reg, ri:reg/immediate  */
.macro MOVR_S r, ri
  mov_s	  \r, \ri
.endm

/* Move register immediate: r:reg, ri:reg/immediate  */
.macro MOVR r, ri
  mov	  \r, \ri
.endm

/* Push register: r:reg  */
.macro PUSHR   r
  push  \r
.endm

/* Pop register: r:reg  */
.macro POPR r
  pop	  \r
.endm

/* Subtract register: r1(reg), r2(reg), r3(reg)  */
.macro SUBR r1, r2, r3
  sub	  \r1, \r2, \r3
.endm

/* Add PCL-rel: r:reg, symb: symbol */
.macro ADDPCL r,symb
  add	  \r, pcl, \symb
.endm

#elif defined (__ARC64_ARCH64__)

/* Define 64-bit word.  */
.macro WORD w
  .xword \w
.endm

/* Move immediate (short): r:reg, i:immediate  */
.macro MOVRI_S r, i
  movhl_s \r, \i
  orl_s	  \r, \r, \i
.endm

/* Move register immediate (short): r:reg, ri:reg/immediate  */
.macro MOVR_S r, ri
  movl_s  \r, \ri
.endm

/* Move register immediate: r:reg, ri:reg/immediate  */
.macro MOVR r, ri
  movl	  \r, \ri
.endm

/* Push register: r:reg  */
.macro PUSHR   r
  pushl_s \r
.endm

/* Pop register: r:reg  */
.macro POPR   r
  popl_s  \r
.endm

/* Subtract register: r1(reg), r2(reg), r3(reg)  */
.macro SUBR r1, r2, r3
  subl	  \r1, \r2, \r3
.endm

/* Add PCL-rel: r:reg, symb: symbol */
.macro ADDPCL r, symb
  addl	  \r, pcl, \symb@pcl
.endm

#else /* !__ARC64_ARC32__  && !__ARC64_ARC64__  */
# error Please use either 32-bit or 64-bit version of arc64 compiler
#endif

#endif /* _ARC64_ASM_H  */

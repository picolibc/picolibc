/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (C) 2025 Dominik Kurz
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

.syntax  unified
.arm
.text
.balign 4
.global __mantissa_sqrt_asm
.type   __mantissa_sqrt_asm, %function
__mantissa_sqrt_asm:
    lsl    r1, r0, #7
    mov    r3, r0
    mov    r12, lr
    bl     __sqrt_core_asm
    lsr    r0, r0, #8
    lsl    r1, r3, #23
    umull  lr, r2, r0, r0
    subs   r1, r1, lr
    rsc    r2, r2, r3, lsr #9
    cmp    r0, r1
    mov    r1, #0
    sbcs   r1, r1, r2
    addlt  r0, r0, #1
    bx     r12

.arm
.text
.balign 4
.global __sqrt_core_asm
.type   __sqrt_core_asm, %function
__sqrt_core_asm: 
    //calculates rsqrt of 9.23 fixed point number in the range [1,4)
    lsls r0, r0, #7      
    .irp x,1,2,3,4,5,6,7,8,9,10,11,12,13 //seems to be enough
    adds   r2,r0,r0,lsr #\x  
    addscc r2,r2,r2,lsr #\x
    movcc  r0,r2
    addcc  r1, r1,r1,lsr #\x
    .endr 
    umull  r2,r0, r1, r0
    adds r1, r1,r1, lsr #1
    subs r0, r1, r0, lsr #1
    bx lr

.arm
.text
.balign    4
.global __mantissa_rsqrt_asm
.type   __mantissa_rsqrt_asm, %function
__mantissa_rsqrt_asm:
    mov    r12, lr
    mov    r1, #1073741824
    mov    r3, r0 
    bl     __sqrt_core_asm
    lsr    r1, r0, #6
    orr    r1, r1, #1
    umull  r0, lr, r1, r1
    umull  r2, r0, r3, r0
    mov    r2, #0
    umlal  r0, r2, lr, r3
    cmp    r2, #512
    addcc  r1, r1, #1
    lsr    r0, r1, #1
    bx     r12

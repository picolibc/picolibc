//Copyright (C) 2025 Dominik Kurz

//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU Lesser General Public
//License as published by the Free Software Foundation; either
//version 3 of the License, or (at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//Lesser General Public License for more details.

//You should have received a copy of the GNU Lesser General Public License
//along with this program; if not, write to the Free Software Foundation,
//Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

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

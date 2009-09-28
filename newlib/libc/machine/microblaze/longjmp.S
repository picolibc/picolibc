/* Copyright (c) 2001, 2009 Xilinx, Inc.  All rights reserved. 
   
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
   
   1.  Redistributions source code must retain the above copyright notice,
   this list of conditions and the following disclaimer. 
   
   2.  Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution. 
   
   3.  Neither the name of Xilinx nor the names of its contributors may be
   used to endorse or promote products derived from this software without
   specific prior written permission. 
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS
   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  
 * 
 * longjmp - non-local jump to a saved stack context
 * args    - r5 - jmp_buf
 *           r6 - val
 *
 * jmpbuf frame structure
 * ---------------------
 *
 *      +-------------+         + 0
 *      |     r1      |
 *      +-------------+         + 4    
 *      |     r13     |
 *      |      .      |
 *      |      .      |
 *      |      .      |
 *      |     r31     |         
 *      +-------------+         + 80
 *      |      .      |
 *      |      .      |        
 */        

    
.globl longjmp
.section .text
.align 2  
.ent longjmp    
longjmp:
    lwi     r1, r5, 0
    lwi     r13, r5, 4
    lwi     r14, r5, 8
    lwi     r15, r5, 12       
    lwi     r16, r5, 16
    lwi     r17, r5, 20
    lwi     r18, r5, 24       
    lwi     r19, r5, 28
    lwi     r20, r5, 32
    lwi     r21, r5, 36
    lwi     r22, r5, 40
    lwi     r23, r5, 44
    lwi     r24, r5, 48
    lwi     r25, r5, 52
    lwi     r26, r5, 56
    lwi     r27, r5, 60                        
    lwi     r28, r5, 64                        
    lwi     r29, r5, 68                        
    lwi     r30, r5, 72
    lwi     r31, r5, 76                                            

    rtsd    r15, 8
    or      r3, r0, r6
.end longjmp    

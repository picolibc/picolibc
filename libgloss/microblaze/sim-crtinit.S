## Copyright (c) 2001, 2009 Xilinx, Inc.  All rights reserved. 
## 
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are
## met:
## 
## 1.  Redistributions source code must retain the above copyright notice,
## this list of conditions and the following disclaimer. 
## 
## 2.  Redistributions in binary form must reproduce the above copyright
## notice, this list of conditions and the following disclaimer in the
## documentation and/or other materials provided with the distribution. 
## 
## 3.  Neither the name of Xilinx nor the names of its contributors may be
## used to endorse or promote products derived from this software without
## specific prior written permission. 
## 
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS
## IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
## TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
## PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
## HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
## SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
## TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
## PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
## LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
## NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
## SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# 
# sim-crtinit.s 
#
# Default second stage of C run-time initialization that does not peform
# BSS initialization to zero. Typical use is on a simulator.      
#    

	.globl _crtinit
	.align 2
	.ent _crtinit

_crtinit:
	addi	r1, r1, -20               	/* Save Link register	 */
	swi	r15, r1, 0

    	brlid	r15, _program_init              /* Initialize the program  */
	nop

        brlid   r15, __init                     /* Invoke language initialization functions */
        nop
    
	addi	r6, r0, 0                       /* Initialize argc = 1 and argv = NULL and envp = NULL  */
	addi	r7, r0, 0			
      	brlid	r15, main                       /* Execute the program */
	addi	r5, r0, 0			

        addik   r19, r3, 0                      /* Save return value */
    
        brlid   r15, __fini                     /* Invoke language cleanup functions */
        nop

	brlid	r15, _program_clean             /* Cleanup the program  */
	nop

	lw	r15, r1, r0                     /* Return back to CRT   */
        addik   r3, r19, 0                      /* Restore return value */
	rtsd	r15, 8
    	addi	r1, r1, 20
	.end _crtinit


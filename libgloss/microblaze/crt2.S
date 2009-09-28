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
  
        
        MicroBlaze Vector Map for standalone executables that are boot-strapped

         Address                Vector type                 Label
         -------                -----------                 ------    

    	# 0x00 #		(-- IMM --)
	# 0x04 #                Reset                       (-- Don't Care --)
    
	# 0x08 #		(-- IMM --)
	# 0x0c #		Software Exception          _exception_handler
    
	# 0x10 #		(-- IMM --)
	# 0x14 #		Hardware Interrupt          _interrupt_handler
    
        # 0x18 #                (-- IMM --)
        # 0x1C #                Breakpoint Exception        (-- Don't Care --)
    
        # 0x20 #                (-- IMM --) 
        # 0x24 #                Hardware Exception          _hw_exception_handler

*/      
    
        .section .vectors.sw_exception, "ax"
        .align 2
_vector_sw_exception:       
        brai    _exception_handler

        .section .vectors.interrupt, "ax"
        .align 2
_vector_interrupt:      
        brai    _interrupt_handler

        .section .vectors.hw_exception, "ax"
        .align 2
_vector_hw_exception:       
        brai    _hw_exception_handler

        .section .text
        .globl _start
        .align 2
        .ent _start
        .type _start, @function    
_start:
	la	r13, r0, _SDA_BASE_         /* Set the Small Data Anchors and the stack pointer */
	la	r2, r0, _SDA2_BASE_
	la	r1, r0, _stack-16           /* 16 bytes (4 words are needed by crtinit for args and link reg */

	brlid	r15, _crtinit               /* Initialize BSS and run program */
	nop

        brlid   r15, exit                   /* Call exit with the return value of main */
        addik   r5, r3, 0                   
       
        /* Control does not reach here */
    
        .end _start
    
/* 
        _exit 
        Our simple _exit      
*/    
        .globl _exit
        .align 2
        .ent _exit
        .type _exit, @function
_exit:
        bri     0
	.end _exit        

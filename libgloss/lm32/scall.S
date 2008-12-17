/*
 * Lattice Mico32 system calls.
 * Contributed by Jon Beniston <jon@beniston.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <syscall.h>

/* 
 * System call convention (as implemented in simulator:
 *    - System call number in register r8 
 *    - Return value in r1 and r2 (only if 64-bit value)
 *    - errno in r3      
 */

        .extern errno

        .global _write
_write: 
        mvi     r8, SYS_write
        scall   
        mvhi    r4, hi(errno)
        ori     r4, r4, lo(errno)
        sw      (r4+0), r3
        ret

        .global _read
_read:
        mvi     r8, SYS_read
        scall
        mvhi    r4, hi(errno)
        ori     r4, r4, lo(errno)
        sw      (r4+0), r3
        ret                

        .global _open
_open:    
        mvi     r8, SYS_open
        scall
        mvhi    r4, hi(errno)
        ori     r4, r4, lo(errno)
        sw      (r4+0), r3
        ret
        
        .global _close
_close:    
        mvi     r8, SYS_close
        scall
        mvhi    r4, hi(errno)
        ori     r4, r4, lo(errno)
        sw      (r4+0), r3
        ret                
            
        .global _lseek
_lseek:
        mvi     r8, SYS_lseek
        scall
        mvhi    r4, hi(errno)
        ori     r4, r4, lo(errno)
        sw      (r4+0), r3
        ret    
                    
        .global _fstat
_fstat:
        mvi     r8, SYS_fstat
        scall
        mvhi    r4, hi(errno)
        ori     r4, r4, lo(errno)
        sw      (r4+0), r3
        ret                                            

        .global _stat
_stat:
        mvi     r8, SYS_stat
        scall
        mvhi    r4, hi(errno)
        ori     r4, r4, lo(errno)
        sw      (r4+0), r3
        ret                                            

        .global _link
_link:
        mvi     r8, SYS_link
        scall
        mvhi    r4, hi(errno)
        ori     r4, r4, lo(errno)
        sw      (r4+0), r3
        ret                                            
            
        .global _unlink
_unlink:
        mvi     r8, SYS_unlink
        scall
        mvhi    r4, hi(errno)
        ori     r4, r4, lo(errno)
        sw      (r4+0), r3
        ret                                            
                        
        .global _exit
_exit:
        /* This call doesn't return */
        mvi     r8, SYS_exit
        scall   

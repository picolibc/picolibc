/*
 * Lattice Mico32 C startup code.
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

        .section    .boot, "ax", @progbits

        /* Program starts here */
        
        .global     _start  
_start:

        /* Setup stack and global pointer */
        mvhi    sp, hi(_fstack)
        ori     sp, sp, lo(_fstack)
        mvhi    gp, hi(_gp)
        ori     gp, gp, lo(_gp)
                                
        /* Clear BSS */
        mvhi    r1, hi(_fbss)
        ori     r1, r1, lo(_fbss)
        mvi     r2, 0
        mvhi    r3, hi(_ebss)
        ori     r3, r3, lo(_ebss)
        sub     r3, r3, r1
        calli   memset
       
        /* Call C++ constructors */
        calli   _init
        
        /* Call C++ destructors on exit */
        mvhi    r1, hi(_fini)
        ori     r1, r1, lo(_fini)
        calli   atexit
                                           
        /* Call main program */
        mvi     r1, 0
        mvi     r2, 0
        mvi     r3, 0
        calli   main
        
        /* Call exit, which doesn't return, to perform any clean up */
        calli   exit        

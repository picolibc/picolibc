/*
 * Copyright (c) 2011 Aeroflex Gaisler
 *
 * BSD license:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


	.text
	.global _start, main, _end

_start:

#ifndef _FLAT
	save	%sp, -64, %sp
#else
	add	%sp, -72, %sp
	st	%o7, [%sp+64]
#endif
        /* clear the bss */
 
        sethi %hi(__bss_start),%g2
        or    %g2,%lo(__bss_start),%g2  ! g2 = start of bss
        sethi %hi(_end),%g3
        or    %g3,%lo(_end),%g3         ! g3 = end of bss
        mov   %g0,%g1                   ! so std has two zeros
	sub   %g3, %g2, %g3
zerobss:
	subcc  %g3, 8, %g3
	bge,a   zerobss
	std    %g0,[%g2+%g3]

	set    _end, %o0
	st     %g0,[%o0]

	call	bdinit2
	 nop

	call	prelibchook
	 nop

	call	_call_initcalls        /* atexit uses __atexit lock */
	 nop
	
	set	_fini, %o0
	call	atexit, 1
	 nop
	
	call	_init
	 nop
        
	call    main
         nop
	call _exit
	nop
#ifndef _FLAT
	ret
	restore
#else
	ld	[%sp+64], %o7
	retl
	add	%sp, 72, %sp
#endif

        .seg    "data"
        .global .bdata
.bdata:
        .align  8
        .global _environ
_environ:
        .word   1
 



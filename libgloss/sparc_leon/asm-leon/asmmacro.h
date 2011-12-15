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


#ifndef _INCLUDE_LEON_ASMMACRO_h
#define _INCLUDE_LEON_ASMMACRO_h

#include <asm-leon/leonstack.h>

/* All trap entry points _must_ begin with this macro or else you
 * lose.  It makes sure the kernel has a proper window so that
 * c-code can be called.
 */
#define SAVE_ALL_HEAD \
	sethi	%hi(leonbare_trapsetup), %l4; \
	jmpl	%l4 + %lo(leonbare_trapsetup), %l6;
#define SAVE_ALL \
	SAVE_ALL_HEAD \
	 nop;

#define SAVE_ALL_FAST(l) \
        set     l-8, %l6; \
	sethi	%hi(leonbare_trapsetup_fast), %l4; \
	jmpl	%l4 + %lo(leonbare_trapsetup_fast), %g0; \
	 nop;

/* All traps low-level code here must end with this macro. */
#define RESTORE_ALL b leonbare_trapreturn; clr %l6;
#define RESTORE_ALL_FAST b leonbare_trapreturn_fast; clr %l6;

#define WRITE_PAUSE nop; nop; nop;

#endif /* !_INCLUDE_LEON_STACK_h */

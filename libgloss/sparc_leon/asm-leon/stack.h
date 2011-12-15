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


#ifndef _SYS_STACK_H_
#define	_SYS_STACK_H_

#if !defined(_ASM)
#include <sys/types.h>
#endif

#ifdef	__cplusplus
extern "C"
{
#endif

/*
 * A stack frame looks like:
 *
 * %fp->|				|
 *	|-------------------------------|
 *	|  Locals, temps, saved floats	|
 *	|-------------------------------|
 *	|  outgoing parameters past 6	|
 *	|-------------------------------|-\
 *	|  6 words for callee to dump	| |
 *	|  register arguments		| |
 *	|-------------------------------|  > minimum stack frame
 *	|  One word struct-ret address	| |
 *	|-------------------------------| |
 *	|  16 words to save IN and	| |
 * %sp->|  LOCAL register on overflow	| |
 *	|-------------------------------|-/
 */

/*
 * Constants defining a 32-bit stack frame.
 */
#define	WINDOWSIZE	(16*4)	/* size of window save area */
#define	ARGPUSHSIZE	(6*4)	/* size of arg dump area */
#define	ARGPUSH	        (WINDOWSIZE + 4)	/* arg dump area offset */
#define	MINFRAME	(WINDOWSIZE + ARGPUSHSIZE + 4)	/* min frame */

#define	STACK_GROWTH_DOWN	/* stacks grow from high to low addresses */

/*
 * Stack alignment macros.
 */
#define	STACK_ALIGN	8
#define	SA(X)		(((X)+(STACK_ALIGN-1)) & ~(STACK_ALIGN-1))

#ifdef	__cplusplus
}
#endif

#endif				/* _SYS_STACK_H */

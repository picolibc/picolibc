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


#ifndef H_LEONBARE_CONTEXTSWITCH_H
#define H_LEONBARE_CONTEXTSWITCH_H


/*
 * for this version, the index of THREAD_JB_SP must be even !!!
 * This way, we can speed up the context switch (using std).
 */
#define THREAD_JB_SP     0	/* aligned */
#define THREAD_JB_PC     1
#define THREAD_JB_SVMASK 3
#define THREAD_JB_MASK   4
#define THREAD_JB_FP     5
#define THREAD_JB_I7     6

#define THREAD_JB_PSR    8	/* aligned */
#define THREAD_JB_WIM    9

#define THREAD_JB_FPUCTX 10

#ifndef __ASSEMBLER__

extern unsigned long fpustate_current;

typedef int threadctx_t[14 + 2] __attribute__ ((aligned (8)));

int thread_setjmp (threadctx_t env, int val);
void thread_longjmp (threadctx_t env, int val);
void _switch_to (threadctx_t env, int val);

#endif /* __ASSEMBLER__ */

#endif

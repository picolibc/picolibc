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


#include <asm-leon/leon.h>
#include <asm-leon/leonstack.h>
#include <asm-leon/irq.h>
#include <asm-leon/irq.h>
#include <asm-leon/time.h>
#include <asm-leon/contextswitch.h>

/* This asm code relies on the following offsets (setjmp.h): 
#define THREAD_JB_SP     0 
#define THREAD_JB_PC     1
#define THREAD_JB_PSR    8 
#define THREAD_JB_WIM    9 */

int
_do_thread_setjmp (threadctx_t env, unsigned int savesigs)
{

#ifndef _FLAT
  /* first restore should trap */
  env[THREAD_JB_WIM] = 1 << ((env[THREAD_JB_PSR] & SPARC_PSR_WIN_MASK) + 1);
  env[THREAD_JB_WIM] |= env[THREAD_JB_WIM] >> SPARC_NUM_REGWIN;
#else
  env[THREAD_JB_WIM] = 0;
#endif

#ifndef _SOFT_FLOAT
  env[THREAD_JB_FPUCTX] = fpustate_current;
#endif
  return 0;
}

void
thread_longjmp (threadctx_t env, int val)
{

  if (!val)
    val = 1;

#ifndef _SOFT_FLOAT
  fpustate_current = env[THREAD_JB_FPUCTX];
#endif

  _switch_to (env, val);

/*   __asm__ __volatile__(                                                                      \ */
/* "        mov     %8,%%i0              /\* propagate on restore *\/                        \n\t"\ */
/* "        mov     %0,%%i1              /\* propagate on restore *\/                        \n\t"\ */
/* "        restore                                                                        \n\t"\ */
/* "        mov     %%o0,%%g6                                                              \n\t"\ */
/* "        mov     %%o1,%%g3            /\* former %%i1 (val) *\/                           \n\t"\ */
/* "                                                                                       \n\t"\ */
/* "        !ta      0x03                /\* flush registers *\/                             \n\t"\ */
/* "        save   %%sp, %7, %%sp                                                          \n\t"\ */
/* "        save   %%sp, %7, %%sp                                                          \n\t"\ */
/* "        save   %%sp, %7, %%sp                                                          \n\t"\ */
/* "        save   %%sp, %7, %%sp                                                          \n\t"\ */
/* "        save   %%sp, %7, %%sp                                                          \n\t"\ */
/* "        save   %%sp, %7, %%sp                                                          \n\t"\ */
/* "        save   %%sp, %7, %%sp                                                          \n\t"\ */
/* "                                                                                       \n\t"\ */
/* "        ldd     [%%g6+%5], %%g4      /\* load psr,wim *\/                                \n\t"\ */
/* "        wr      %%g4, 0x20, %%psr                                                      \n\t"\ */
/* "        nop                                                                            \n\t"\ */
/* "        nop                                                                            \n\t"\ */
/* "        nop                                                                            \n\t"\ */
/* "        ldd     [%%g6 +%1], %%sp     /\* load sp, pc to jump to *\/                      \n\t"\ */
/* "        wr      %%g5, 0x0, %%wim                                                       \n\t"\ */
/* "                                                                                       \n\t"\ */
/* "        ldd     [%%sp], %%l0         /\* restore window *\/                              \n\t"\ */
/* "        ldd     [%%sp+8], %%l2                                                         \n\t"\ */
/* "        ldd     [%%sp+16], %%l4                                                        \n\t"\ */
/* "        ldd     [%%sp+24], %%l6                                                        \n\t"\ */
/* "                                                                                       \n\t"\ */
/* "        ldd     [%%sp+32], %%i0                                                        \n\t"\ */
/* "        ldd     [%%sp+40], %%i2                                                        \n\t"\ */
/* "        ldd     [%%sp+48], %%i4                                                        \n\t"\ */
/* "        ldd     [%%sp+56], %%i6                                                        \n\t"\ */
/* "        wr      %%g4, 0x00, %%psr                                                      \n\t"\ */
/* "        nop                                                                            \n\t"\ */
/* "        nop                                                                            \n\t"\ */
/* "        nop                                                                            \n\t"\ */
/* "                                                                                       \n\t"\ */
/* "        jmp     %%o7 + 8             /\* success      *\/                                \n\t"\ */
/* "        mov     %%g3, %%o0           /\* return %%g3  *\/                                \n\t"\ */
/* : : /\* %0 *\/ "r" (val),                                                                      \ */
/*     /\* %1 *\/ "i" (sizeof(int) * THREAD_JB_SP),                                                      \ */
/*     /\* %2 *\/ "i" (sizeof(int) * THREAD_JB_I7),                                                      \ */
/*     /\* %3 *\/ "i" (sizeof(int) * THREAD_JB_FP),                                                      \ */
/*     /\* %4 *\/ "i" (sizeof(int) * THREAD_JB_PC),                                                      \ */
/*     /\* %5 *\/ "i" (sizeof(int) * THREAD_JB_PSR),                                                     \ */
/*     /\* %6 *\/ "i" (sizeof(int) * THREAD_JB_WIM),                                                     \ */
/*     /\* %7 *\/ "i" (-SF_REGS_SZ),                                                                     \ */
/*     /\* %8 *\/ "r" (env) );                                                                       */

  /* never come here */
}

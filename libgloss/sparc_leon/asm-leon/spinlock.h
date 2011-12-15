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


#ifndef _INCLUDE_LEONSPINLOCK_h
#define _INCLUDE_LEONSPINLOCK_h

typedef struct
{
  unsigned char lock;
} raw_spinlock_t;

#define __RAW_SPIN_LOCK_UNLOCKED	{ 0 }

typedef struct
{
  volatile unsigned int lock;
} raw_rwlock_t;

#define __RAW_RW_LOCK_UNLOCKED		{ 0 }

static inline void
__raw_spin_lock (raw_spinlock_t * lock)
{
  __asm__ __volatile__ ("\n1:\n\t" "ldstuba	[%0]1, %%g2\n\t"	/* ASI_LEON23_DCACHE_MISS */
			"orcc	%%g2, 0x0, %%g0\n\t" "bne,a	2f\n\t" " ldub	[%0], %%g2\n\t" ".subsection	2\n" "2:\n\t" "orcc	%%g2, 0x0, %%g0\n\t" "bne,a	2b\n\t" " ldub	[%0], %%g2\n\t" "b,a	1b\n\t" ".previous\n":	/* no outputs */
			:"r" (lock):"g2", "memory", "cc");
}

static inline int
__raw_spin_trylock (raw_spinlock_t * lock)
{
  unsigned int result;
  __asm__ __volatile__ ("ldstuba [%1]1, %0"	/* ASI_LEON23_DCACHE_MISS */
			:"=r" (result):"r" (lock):"memory");
  return (result == 0);
}

static inline void
__raw_spin_unlock (raw_spinlock_t * lock)
{
  __asm__ __volatile__ ("stb %%g0, [%0]"::"r" (lock):"memory");
}



#endif /* _INCLUDE_LEONSPINLOCK_h */
/* end of include file */

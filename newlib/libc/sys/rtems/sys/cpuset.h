/*
 * Copyright (c) 2013 On-Line Applications Research Corporation.
 * All rights reserved.
 *
 *  On-Line Applications Research Corporation
 *  7047 Old Madison Pike Suite 320
 *  Huntsville Alabama 35806
 *  <info@oarcorp.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 *  This file implements an API compatible with static portion of
 *  the GNU/Linux cpu_set_t macros but is independently implemented.
 *  The GNU/Linux manual page and the FreeBSD cpuset_t implementation
 *  were used as reference material.
 *
 *  Not implemented:
 *    + Linux CPU_XXX_S
 *    + FreeBSD CPU_SUBSET
 *    + FreeBSD CPU_OVERLAP
 */


#ifndef _SYS_CPUSET_H_
#define _SYS_CPUSET_H_

#if 0
#include <sys/types.h>
#endif
#include <stdint.h>

/* RTEMS supports a maximum of 32 CPU cores */
#ifndef CPU_SETSIZE
#define CPU_SETSIZE 32
#endif

/* word in the cpu set */
typedef uint32_t cpu_set_word_t;

/* Number of bits per cpu_set_t element */
#define _NCPUBITS  (sizeof(cpu_set_word_t) * NBBY) /* bits per mask */

/* Number of words in the cpu_set_t array */
/* NOTE: Can't use howmany() because of circular dependency */
#define _NCPUWORDS   (((CPU_SETSIZE)+((_NCPUBITS)-1))/(_NCPUBITS))

/* Define the cpu set structure */
typedef struct _cpuset {
  cpu_set_word_t __bits[_NCPUWORDS];
} cpu_set_t;

/* determine the mask for a particular cpu within the element */
static inline cpu_set_word_t  __cpuset_mask( size_t cpu )
{
  return ((cpu_set_word_t)1 << ((cpu) % _NCPUBITS));
}

/* determine the index for this cpu within the cpu set array */
static inline size_t __cpuset_index( size_t cpu )
{
  return ((cpu)/_NCPUBITS);
}

/* zero out set */
static inline void CPU_ZERO( cpu_set_t *set )
{
  size_t i;
  for (i = 0; i < _NCPUWORDS; i++)
    set->__bits[i] = 0;
}

/* fill set */
static inline void CPU_FILL( cpu_set_t *set )
{
  size_t i;
  for (i = 0; i < _NCPUWORDS; i++)
    set->__bits[i] = -1;
}

/* set cpu within set */
static inline void CPU_SET( size_t cpu, cpu_set_t *set )
{
  set->__bits[__cpuset_index(cpu)] |= __cpuset_mask(cpu);
}

/* clear cpu within set */
static inline void CPU_CLR( size_t cpu, cpu_set_t *set )
{
  set->__bits[__cpuset_index(cpu)] &= ~__cpuset_mask(cpu);
}

/* Return 1 is cpu is set in set, 0 otherwise */
static inline int const CPU_ISSET( size_t cpu, const cpu_set_t *set )
{
  return ((set->__bits[__cpuset_index(cpu)] & __cpuset_mask(cpu)) != 0);
}

/* copy src set to dest set */
static inline void CPU_COPY( cpu_set_t *dest, const cpu_set_t *src )
{
  *dest = *src;
}

/* logical and: dest set = src1 set and src2 set */
static inline void CPU_AND(
  cpu_set_t *dest, const cpu_set_t *src1, const cpu_set_t *src2
)
{
  size_t i;
  for (i = 0; i < _NCPUWORDS; i++)
    dest->__bits[i] = src1->__bits[i] & src2->__bits[i];
}

/* logical or: dest set = src1 set or src2 set */
static inline void CPU_OR(
  cpu_set_t *dest, const cpu_set_t *src1, const cpu_set_t *src2
)
{
  size_t i;
  for (i = 0; i < _NCPUWORDS; i++)
     dest->__bits[i] = src1->__bits[i] | src2->__bits[i];
}

/* logical xor: dest set = src1 set xor src2 set */
static inline void CPU_XOR(
  cpu_set_t *dest, const cpu_set_t *src1, const cpu_set_t *src2
)
{
  size_t i;
  for (i = 0; i < _NCPUWORDS; i++)
   dest->__bits[i] = src1->__bits[i] ^ src2->__bits[i];
}

/* logical nand: dest set = src1 set nand src2 set */
static inline void CPU_NAND(
  cpu_set_t *dest, const cpu_set_t *src1, const cpu_set_t *src2
)
{
  size_t i; 
  for (i = 0; i < _NCPUWORDS; i++)
    dest->__bits[i] = ~(src1->__bits[i] & src2->__bits[i]);
}

/* return the number of set cpus in set */
static inline int const CPU_COUNT( const cpu_set_t *set )
{
  size_t i;
  int    count = 0;

  for (i=0; i < _NCPUWORDS; i++)
    if (CPU_ISSET(i, set) != 0)
      count++;
  return count;
}

/* return 1 if the sets set1 and set2 are equal, otherwise return 0 */
static inline int const CPU_EQUAL( 
  const cpu_set_t *set1, const cpu_set_t *set2 
)
{
  size_t i;

  for (i=0; i < _NCPUWORDS; i++)
    if (set1->__bits[i] != set2->__bits[i] )
      return 0;
  return 1;
}

/* return 1 if the sets set1 and set2 are equal, otherwise return 0 */
static inline int const CPU_CMP( const cpu_set_t *set1, const cpu_set_t *set2 )
{
  return CPU_EQUAL(set1, set2);
}

/* return 1 if the set is empty, otherwise return 0 */
static inline int const CPU_EMPTY( const cpu_set_t *set )
{
  size_t i;

  for (i=0; i < _NCPUWORDS; i++)
    if (set->__bits[i] != 0 )
      return 0;
  return 1;
}

#endif

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

#include <sys/cdefs.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* RTEMS supports a maximum of 32 CPU cores */
#ifndef CPU_SETSIZE
#define CPU_SETSIZE 32
#endif

/* word in the cpu set */
typedef __uint32_t cpu_set_word_t;

/* Number of bits per cpu_set_t element */
#define _NCPUBITS  (sizeof(cpu_set_word_t) * 8)

/* Number of words in the cpu_set_t array */
#define _NCPUWORDS   (((CPU_SETSIZE)+((_NCPUBITS)-1))/(_NCPUBITS))

/* Define the cpu set structure */
typedef struct _cpuset {
  cpu_set_word_t __bits[_NCPUWORDS];
} cpu_set_t;

/* determine the mask for a particular cpu within the element */
static __inline cpu_set_word_t  __cpuset_mask(int cpu)
{
  return (cpu_set_word_t)1 << ((size_t)cpu % _NCPUBITS);
}

/* determine the index for this cpu within the cpu set array */
static __inline size_t __cpuset_index(int cpu)
{
  return (size_t)cpu / _NCPUBITS;
}

#define CPU_ALLOC_SIZE(_num_cpus) \
  (sizeof(cpu_set_word_t) * (((_num_cpus) + _NCPUBITS - 1) / _NCPUBITS))

cpu_set_t *__cpuset_alloc(int num_cpus);

static __inline cpu_set_t *CPU_ALLOC(int num_cpus)
{
  return __cpuset_alloc(num_cpus);
}

void __cpuset_free(cpu_set_t *set);

static __inline void CPU_FREE(cpu_set_t *set)
{
  __cpuset_free(set);
}

static __inline void CPU_ZERO_S(size_t setsize, cpu_set_t *set)
{
  cpu_set_word_t *w = &set->__bits[0];
  size_t n = setsize / sizeof(*w);
  size_t i;

  for (i = 0; i < n; ++i)
    w[i] = 0;
}

static __inline void CPU_ZERO(cpu_set_t *set)
{
  CPU_ZERO_S(sizeof(*set), set);
}

static __inline void CPU_FILL_S(size_t setsize, cpu_set_t *set)
{
  cpu_set_word_t *w = &set->__bits[0];
  size_t n = setsize / sizeof(*w);
  size_t i;

  for (i = 0; i < n; ++i)
    w[i] = ~(cpu_set_word_t)0;
}

static __inline void CPU_FILL(cpu_set_t *set)
{
  CPU_FILL_S(sizeof(*set), set);
}

static __inline void CPU_SET_S(int cpu, size_t setsize, cpu_set_t *set)
{
  cpu_set_word_t *w = &set->__bits[0];

  w[__cpuset_index(cpu)] |= __cpuset_mask(cpu);
}

static __inline void CPU_SET(int cpu, cpu_set_t *set)
{
  CPU_SET_S(cpu, sizeof(*set), set);
}

static __inline void CPU_CLR_S(int cpu, size_t setsize, cpu_set_t *set)
{
  cpu_set_word_t *w = &set->__bits[0];

  w[__cpuset_index(cpu)] &= ~__cpuset_mask(cpu);
}

static __inline void CPU_CLR(int cpu, cpu_set_t *set)
{
  CPU_CLR_S(cpu, sizeof(*set), set);
}

static __inline int CPU_ISSET_S(int cpu, size_t setsize, const cpu_set_t *set)
{
  const cpu_set_word_t *w = &set->__bits[0];

  return ((w[__cpuset_index(cpu)] & __cpuset_mask(cpu)) != 0);
}

static __inline int CPU_ISSET(int cpu, const cpu_set_t *set)
{
  return CPU_ISSET_S(cpu, sizeof(*set), set);
}

/* copy src set to dest set */
static __inline void CPU_COPY( cpu_set_t *dest, const cpu_set_t *src )
{
  *dest = *src;
}

static __inline void CPU_AND_S(size_t setsize, cpu_set_t *destset,
  const cpu_set_t *srcset1, const cpu_set_t *srcset2)
{
  cpu_set_word_t *wdest = &destset->__bits[0];
  const cpu_set_word_t *wsrc1 = &srcset1->__bits[0];
  const cpu_set_word_t *wsrc2 = &srcset2->__bits[0];
  size_t n = setsize / sizeof(*wdest);
  size_t i;

  for (i = 0; i < n; ++i)
    wdest[i] = wsrc1[i] & wsrc2[i];
}

static __inline void CPU_AND(cpu_set_t *destset, const cpu_set_t *srcset1,
  const cpu_set_t *srcset2)
{
  CPU_AND_S(sizeof(*destset), destset, srcset1, srcset2);
}

static __inline void CPU_OR_S(size_t setsize, cpu_set_t *destset,
  const cpu_set_t *srcset1, const cpu_set_t *srcset2)
{
  cpu_set_word_t *wdest = &destset->__bits[0];
  const cpu_set_word_t *wsrc1 = &srcset1->__bits[0];
  const cpu_set_word_t *wsrc2 = &srcset2->__bits[0];
  size_t n = setsize / sizeof(*wdest);
  size_t i;

  for (i = 0; i < n; ++i)
    wdest[i] = wsrc1[i] | wsrc2[i];
}

static __inline void CPU_OR(cpu_set_t *destset, const cpu_set_t *srcset1,
  const cpu_set_t *srcset2)
{
  CPU_OR_S(sizeof(*destset), destset, srcset1, srcset2);
}

static __inline void CPU_XOR_S(size_t setsize, cpu_set_t *destset,
  const cpu_set_t *srcset1, const cpu_set_t *srcset2)
{
  cpu_set_word_t *wdest = &destset->__bits[0];
  const cpu_set_word_t *wsrc1 = &srcset1->__bits[0];
  const cpu_set_word_t *wsrc2 = &srcset2->__bits[0];
  size_t n = setsize / sizeof(*wdest);
  size_t i;

  for (i = 0; i < n; ++i)
    wdest[i] = wsrc1[i] ^ wsrc2[i];
}

static __inline void CPU_XOR(cpu_set_t *destset, const cpu_set_t *srcset1,
  const cpu_set_t *srcset2)
{
  CPU_XOR_S(sizeof(*destset), destset, srcset1, srcset2);
}

static __inline void CPU_NAND_S(size_t setsize, cpu_set_t *destset,
  const cpu_set_t *srcset1, const cpu_set_t *srcset2)
{
  cpu_set_word_t *wdest = &destset->__bits[0];
  const cpu_set_word_t *wsrc1 = &srcset1->__bits[0];
  const cpu_set_word_t *wsrc2 = &srcset2->__bits[0];
  size_t n = setsize / sizeof(*wdest);
  size_t i;

  for (i = 0; i < n; ++i)
    wdest[i] = ~(wsrc1[i] & wsrc2[i]);
}

static __inline void CPU_NAND(cpu_set_t *destset, const cpu_set_t *srcset1,
  const cpu_set_t *srcset2)
{
  CPU_NAND_S(sizeof(*destset), destset, srcset1, srcset2);
}

static __inline int CPU_COUNT_S(size_t setsize, const cpu_set_t *set)
{
  int count = 0;
  const cpu_set_word_t *w = &set->__bits[0];
  size_t n = setsize / sizeof(*w);
  size_t i;
  int cpu;

  for (i = 0; i < n; ++i)
    for (cpu = 0; cpu < (int)_NCPUBITS; ++cpu)
      count += (w[i] & __cpuset_mask(cpu)) != 0;

  return count;
}

static __inline int CPU_COUNT(const cpu_set_t *set)
{
  return CPU_COUNT_S(sizeof(*set), set);
}

static __inline int CPU_EQUAL_S(size_t setsize, const cpu_set_t *set1,
  const cpu_set_t *set2)
{
  const cpu_set_word_t *w1 = &set1->__bits[0];
  const cpu_set_word_t *w2 = &set2->__bits[0];
  size_t n = setsize / sizeof(*w1);
  size_t i;

  for (i = 0; i < n; ++i)
    if (w1[i] != w2[i])
      return 0;

  return 1;
}

static __inline int CPU_EQUAL(const cpu_set_t *set1, const cpu_set_t *set2)
{
  return CPU_EQUAL_S(sizeof(*set1), set1, set2);
}

/* return 1 if the sets set1 and set2 are equal, otherwise return 0 */
static __inline int CPU_CMP( const cpu_set_t *set1, const cpu_set_t *set2 )
{
  return CPU_EQUAL(set1, set2);
}

/* return 1 if the set is empty, otherwise return 0 */
static __inline int CPU_EMPTY( const cpu_set_t *set )
{
  size_t i;

  for (i=0; i < _NCPUWORDS; i++)
    if (set->__bits[i] != 0 )
      return 0;
  return 1;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

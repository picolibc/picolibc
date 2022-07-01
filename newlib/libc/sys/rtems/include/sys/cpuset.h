/*-
 * Copyright (c) 2008,	Jeffrey Roberson <jeff@freebsd.org>
 * All rights reserved.
 *
 * Copyright (c) 2008 Nokia Corporation
 * All rights reserved.
 *
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
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
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
 *
 * $FreeBSD$
 */

#ifndef _SYS_CPUSET_H_
#define	_SYS_CPUSET_H_

#include <sys/cdefs.h>
#include <sys/_cpuset.h>
#include <sys/bitset.h>

#define	_NCPUBITS	_BITSET_BITS
#define	_NCPUWORDS	__bitset_words(CPU_SETSIZE)

#define	CPUSETBUFSIZ	((2 + sizeof(long) * 2) * _NCPUWORDS)

#define	CPU_SETOF(n, p)			__BIT_SETOF(CPU_SETSIZE, n, p)
#define	CPU_ISFULLSET(p)		__BIT_ISFULLSET(CPU_SETSIZE, p)
#define	CPU_SUBSET(p, c)		__BIT_SUBSET(CPU_SETSIZE, p, c)
#define	CPU_OVERLAP(p, c)		__BIT_OVERLAP(CPU_SETSIZE, p, c)
#define	CPU_CLR_ATOMIC(n, p)		__BIT_CLR_ATOMIC(CPU_SETSIZE, n, p)
#define	CPU_SET_ATOMIC(n, p)		__BIT_SET_ATOMIC(CPU_SETSIZE, n, p)
#define	CPU_SET_ATOMIC_ACQ(n, p)	__BIT_SET_ATOMIC_ACQ(CPU_SETSIZE, n, p)
#define	CPU_AND_ATOMIC(n, p)		__BIT_AND_ATOMIC(CPU_SETSIZE, n, p)
#define	CPU_OR_ATOMIC(d, s)		__BIT_OR_ATOMIC(CPU_SETSIZE, d, s)
#define	CPU_COPY_STORE_REL(f, t)	__BIT_COPY_STORE_REL(CPU_SETSIZE, f, t)
#define	CPU_FFS(p)			__BIT_FFS(CPU_SETSIZE, p)
#define	CPU_FLS(p)			__BIT_FLS(CPU_SETSIZE, p)
#define	CPU_FOREACH_ISSET(i, p)		__BIT_FOREACH_ISSET(CPU_SETSIZE, i, p)
#define	CPU_FOREACH_ISCLR(i, p)		__BIT_FOREACH_ISCLR(CPU_SETSIZE, i, p)
#define	CPUSET_FSET			__BITSET_FSET(_NCPUWORDS)
#define	CPUSET_T_INITIALIZER		__BITSET_T_INITIALIZER

typedef cpuset_t cpu_set_t;

#define	_cpu_set_bits(_setsize) (8 * (_setsize))

#define CPU_ALLOC_SIZE(_s)		__BITSET_SIZE(_s)

__BEGIN_DECLS

#ifndef _KERNEL
static __inline cpu_set_t *CPU_ALLOC(int num_cpus)
{
  return __cpuset_alloc(num_cpus);
}

static __inline void CPU_FREE(cpu_set_t *set)
{
  __cpuset_free(set);
}
#endif

static __inline void CPU_ZERO_S(size_t setsize, cpu_set_t *set)
{
  __BIT_ZERO(_cpu_set_bits(setsize), set);
}

static __inline void CPU_ZERO(cpu_set_t *set)
{
  CPU_ZERO_S(sizeof(*set), set);
}

static __inline void CPU_FILL_S(size_t setsize, cpu_set_t *set)
{
  __BIT_FILL(_cpu_set_bits(setsize), set);
}

static __inline void CPU_FILL(cpu_set_t *set)
{
  CPU_FILL_S(sizeof(*set), set);
}

static __inline void CPU_SET_S(int cpu, size_t setsize, cpu_set_t *set)
{
  __BIT_SET(_cpu_set_bits(setsize), cpu, set);
}

static __inline void CPU_SET(int cpu, cpu_set_t *set)
{
  CPU_SET_S(cpu, sizeof(*set), set);
}

static __inline void CPU_CLR_S(int cpu, size_t setsize, cpu_set_t *set)
{
  __BIT_CLR(_cpu_set_bits(setsize), cpu, set);
}

static __inline void CPU_CLR(int cpu, cpu_set_t *set)
{
  CPU_CLR_S(cpu, sizeof(*set), set);
}

static __inline int CPU_ISSET_S(int cpu, size_t setsize, const cpu_set_t *set)
{
  return __BIT_ISSET(_cpu_set_bits(setsize), cpu, set);
}

static __inline int CPU_ISSET(int cpu, const cpu_set_t *set)
{
  return CPU_ISSET_S(cpu, sizeof(*set), set);
}

static __inline void CPU_COPY(const cpu_set_t *src, cpu_set_t *dest)
{
  __BIT_COPY(_cpu_set_bits(setsize), src, dest);
}

static __inline void CPU_AND_S(size_t setsize, cpu_set_t *destset,
  const cpu_set_t *srcset1, const cpu_set_t *srcset2)
{
  __BIT_AND2(_cpu_set_bits(setsize), destset, srcset1, srcset2);
}

static __inline void CPU_AND(cpu_set_t *destset, const cpu_set_t *srcset1,
  const cpu_set_t *srcset2)
{
  CPU_AND_S(sizeof(*destset), destset, srcset1, srcset2);
}

static __inline void CPU_OR_S(size_t setsize, cpu_set_t *destset,
  const cpu_set_t *srcset1, const cpu_set_t *srcset2)
{
  __BIT_OR2(_cpu_set_bits(setsize), destset, srcset1, srcset2);
}

static __inline void CPU_OR(cpu_set_t *destset, const cpu_set_t *srcset1,
  const cpu_set_t *srcset2)
{
  CPU_OR_S(sizeof(*destset), destset, srcset1, srcset2);
}

static __inline void CPU_XOR_S(size_t setsize, cpu_set_t *destset,
  const cpu_set_t *srcset1, const cpu_set_t *srcset2)
{
  __BIT_XOR2(_cpu_set_bits(setsize), destset, srcset1, srcset2);
}

static __inline void CPU_XOR(cpu_set_t *destset, const cpu_set_t *srcset1,
  const cpu_set_t *srcset2)
{
  CPU_XOR_S(sizeof(*destset), destset, srcset1, srcset2);
}

static __inline void CPU_ANDNOT_S(size_t setsize, cpu_set_t *destset,
  const cpu_set_t *srcset1, const cpu_set_t *srcset2)
{
  __BIT_ANDNOT2(_cpu_set_bits(setsize), destset, srcset1, srcset2);
}

static __inline void CPU_ANDNOT(cpu_set_t *destset, const cpu_set_t *srcset1,
  const cpu_set_t *srcset2)
{
  CPU_ANDNOT_S(sizeof(*destset), destset, srcset1, srcset2);
}

static __inline int CPU_COUNT_S(size_t setsize, const cpu_set_t *set)
{
  return (int)__BIT_COUNT(_cpu_set_bits(setsize), set);
}

static __inline int CPU_COUNT(const cpu_set_t *set)
{
  return CPU_COUNT_S(sizeof(*set), set);
}

static __inline int CPU_EQUAL_S(size_t setsize, const cpu_set_t *set1,
  const cpu_set_t *set2)
{
  return __BIT_CMP(_cpu_set_bits(setsize), set1, set2) == 0;
}

static __inline int CPU_EQUAL(const cpu_set_t *set1, const cpu_set_t *set2)
{
  return CPU_EQUAL_S(sizeof(*set1), set1, set2);
}

static __inline int CPU_CMP(const cpu_set_t *set1, const cpu_set_t *set2)
{
  return __BIT_CMP(CPU_SETSIZE, set1, set2);
}

static __inline int CPU_EMPTY(const cpu_set_t *set)
{
  return __BIT_EMPTY(_cpu_set_bits(sizeof(*set)), set);
}

__END_DECLS

#ifdef _KERNEL
/* Header file provided outside of Newlib */
#include <machine/_kernel_cpuset.h>
#endif
#endif /* !_SYS_CPUSET_H_ */

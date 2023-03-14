/* sys/cpuset.h

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _SYS_CPUSET_H_
#define _SYS_CPUSET_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef __SIZE_TYPE__ __cpu_mask;
#define __CPU_SETSIZE 1024  // maximum number of logical processors tracked
#define __NCPUBITS (8 * sizeof (__cpu_mask))  // max size of processor group
#define __CPU_GROUPMAX (__CPU_SETSIZE / __NCPUBITS)  // maximum group number

#define __CPUELT(cpu)  ((cpu) / __NCPUBITS)
#define __CPUMASK(cpu) ((__cpu_mask) 1 << ((cpu) % __NCPUBITS))

typedef struct
{
  __cpu_mask __bits[__CPU_GROUPMAX];
} cpu_set_t;

#if __GNU_VISIBLE
int __sched_getaffinity_sys (pid_t, size_t, cpu_set_t *);

/* These macros alloc or free dynamically-sized cpu sets of size 'num' cpus.
   Allocations are padded such that full-word operations can be done easily. */
#define CPU_ALLOC_SIZE(num) __cpuset_alloc_size (num)
static inline size_t
__cpuset_alloc_size (int num)
{
  return (size_t) (((num + __NCPUBITS - 1) / __NCPUBITS) * sizeof (__cpu_mask));
}

#define CPU_ALLOC(num) __cpuset_alloc (num)
static inline cpu_set_t *
__cpuset_alloc (int num)
{
  return (cpu_set_t *) __builtin_malloc (CPU_ALLOC_SIZE(num));
}

#define CPU_FREE(set) __cpuset_free (set)
static inline void
__cpuset_free (cpu_set_t *set)
{
  __builtin_free (set);
}

/* These _S macros operate on dynamically-sized cpu sets of size 'siz' bytes */
#define CPU_ZERO_S(siz, set) __cpuset_zero_s (siz, set)
static inline void
__cpuset_zero_s (size_t siz, cpu_set_t *set)
{
  (void) __builtin_memset (set, 0, siz);
}

#define CPU_SET_S(cpu, siz, set) __cpuset_set_s (cpu, siz, set)
static inline void
__cpuset_set_s (int cpu, size_t siz, cpu_set_t *set)
{
  if (cpu >= 0 && cpu < 8 * siz)
    (set)->__bits[__CPUELT(cpu)] |= __CPUMASK(cpu);
}

#define CPU_CLR_S(cpu, siz, set) __cpuset_clr_s (cpu, siz, set)
static inline void
__cpuset_clr_s (int cpu, size_t siz, cpu_set_t *set)
{
  if (cpu >= 0 && cpu < 8 * siz)
    (set)->__bits[__CPUELT(cpu)] &= ~(__CPUMASK(cpu));
}

#define CPU_ISSET_S(cpu, siz, set) __cpuset_isset_s (cpu, siz, set)
static inline int
__cpuset_isset_s (int cpu, size_t siz, cpu_set_t *set)
{
  int res = 0;
  if (cpu >= 0 && cpu < 8 * siz)
    res = !!((set)->__bits[__CPUELT(cpu)] & __CPUMASK(cpu));
  return res;
}

#define CPU_COUNT_S(siz, set) __cpuset_count_s (siz, set)
static inline int
__cpuset_count_s (size_t siz, cpu_set_t *set)
{
  int res = 0;
  for (int i = 0; i < siz / sizeof (__cpu_mask); i++)
    res += __builtin_popcountl ((set)->__bits[i]);
  return res;
}

#define CPU_AND_S(siz, dst, src1, src2) __cpuset_and_s (siz, dst, src1, src2)
static inline void
__cpuset_and_s (size_t siz, cpu_set_t *dst, cpu_set_t *src1, cpu_set_t *src2)
{
  for (int i = 0; i < siz / sizeof (__cpu_mask); i++)
    (dst)->__bits[i] = (src1)->__bits[i] & (src2)->__bits[i];
}

#define CPU_OR_S(siz, dst, src1, src2) __cpuset_or_s (siz, dst, src1, src2)
static inline void
__cpuset_or_s (size_t siz, cpu_set_t *dst, cpu_set_t *src1, cpu_set_t *src2)
{
  for (int i = 0; i < siz / sizeof (__cpu_mask); i++)
    (dst)->__bits[i] = (src1)->__bits[i] | (src2)->__bits[i];
}

#define CPU_XOR_S(siz, dst, src1, src2) __cpuset_xor_s (siz, dst, src1, src2)
static inline void
__cpuset_xor_s (size_t siz, cpu_set_t *dst, cpu_set_t *src1, cpu_set_t *src2)
{
  for (int i = 0; i < siz / sizeof (__cpu_mask); i++)
    (dst)->__bits[i] = (src1)->__bits[i] ^ (src2)->__bits[i];
}

#define CPU_EQUAL_S(siz, src1, src2) __cpuset_equal_s (siz, src1, src2)
static inline int
__cpuset_equal_s (size_t siz, cpu_set_t *src1, cpu_set_t *src2)
{
  int res = 1;
  for (int i = 0; res && i < siz / sizeof (__cpu_mask); i++)
    res &= (src1)->__bits[i] == (src2)->__bits[i];
  return res;
}

/* These macros operate on fixed-size cpu sets of size __CPU_SETSIZE cpus */
#define CPU_ZERO(set)             CPU_ZERO_S(sizeof (cpu_set_t), set)

#define CPU_SET(cpu, set)         CPU_SET_S(cpu, sizeof (cpu_set_t), set)
#define CPU_CLR(cpu, set)         CPU_CLR_S(cpu, sizeof (cpu_set_t), set)
#define CPU_ISSET(cpu, set)       CPU_ISSET_S(cpu, sizeof (cpu_set_t), set)

#define CPU_COUNT(set)            CPU_COUNT_S(sizeof (cpu_set_t), set)
#define CPU_AND(dst, src1, src2)  CPU_AND_S(sizeof (cpu_set_t), dst, src1, src2)
#define CPU_OR(dst, src1, src2)   CPU_OR_S(sizeof (cpu_set_t), dst, src1, src2)
#define CPU_XOR(dst, src1, src2)  CPU_XOR_S(sizeof (cpu_set_t), dst, src1, src2)
#define CPU_EQUAL(src1, src2)     CPU_EQUAL_S(sizeof (cpu_set_t), src1, src2)

#define CPU_SETSIZE               __CPU_SETSIZE
#endif /* __GNU_VISIBLE */

#ifdef __cplusplus
}
#endif

#endif /* _SYS_CPUSET_H_ */

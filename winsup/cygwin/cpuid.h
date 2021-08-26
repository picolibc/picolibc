/* cpuid.h: Define cpuid instruction

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef CPUID_H
#define CPUID_H

static inline void __attribute ((always_inline))
cpuid (uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d, uint32_t ain,
       uint32_t cin = 0)
{
  asm volatile ("cpuid"
		: "=a" (*a), "=b" (*b), "=c" (*c), "=d" (*d)
		: "a" (ain), "c" (cin));
}

#ifdef __x86_64__
static inline bool __attribute ((always_inline))
can_set_flag (uint32_t long flag)
{
  uint32_t long r1, r2;

  asm volatile ("pushfq\n"
		"popq %0\n"
		"movq %0, %1\n"
		"xorq %2, %0\n"
		"pushq %0\n"
		"popfq\n"
		"pushfq\n"
		"popq %0\n"
		"pushq %1\n"
		"popfq\n"
		: "=&r" (r1), "=&r" (r2)
		: "ir" (flag)
  );
  return ((r1 ^ r2) & flag) != 0;
}
#else
static inline bool __attribute ((always_inline))
can_set_flag (uint32_t flag)
{
  uint32_t r1, r2;

  asm volatile ("pushfl\n"
		"popl %0\n"
		"movl %0, %1\n"
		"xorl %2, %0\n"
		"pushl %0\n"
		"popfl\n"
		"pushfl\n"
		"popl %0\n"
		"pushl %1\n"
		"popfl\n"
		: "=&r" (r1), "=&r" (r2)
		: "ir" (flag)
  );
  return ((r1 ^ r2) & flag) != 0;
}
#endif

#endif // !CPUID_H

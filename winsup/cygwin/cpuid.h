#ifndef CPUID_H
#define CPUID_H

extern inline void
cpuid (unsigned *a, unsigned *b, unsigned *c, unsigned *d, unsigned in)
{
  asm ("cpuid"
       : "=a" (*a),
	 "=b" (*b),
	 "=c" (*c),
	 "=d" (*d)
       : "a" (in));
}

#ifdef __x86_64__
extern inline bool
can_set_flag (register unsigned long flag)
{
  register unsigned long r1, r2;
  asm("pushfq\n"
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
extern inline bool
can_set_flag (register unsigned flag)
{
  register unsigned r1, r2;
  asm("pushfl\n"
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

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

extern inline bool
can_set_flag (unsigned flag)
{
  unsigned r1, r2;
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

#endif // !CPUID_H

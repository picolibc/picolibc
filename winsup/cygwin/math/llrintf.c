/**
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is part of the mingw-w64 runtime package.
 * No warranty is given; refer to the file DISCLAIMER.PD within this package.
 */
#include <math.h>

long long llrintf (float x) 
{
  long long retval = 0ll;
#if defined(_AMD64_) || defined(__x86_64__) || defined(_X86_) || defined(__i386__)
  __asm__ __volatile__ ("fistpll %0"  : "=m" (retval) : "t" (x) : "st");
#else
  retval = (long long)x;
#endif
  return retval;
}

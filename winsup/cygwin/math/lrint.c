/**
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is part of the mingw-w64 runtime package.
 * No warranty is given; refer to the file DISCLAIMER.PD within this package.
 */
#include <math.h>

#if defined(__arm__) || defined(_ARM_)
/* This works around a compiler bug */
long __lrint_internal( double x );
asm(".def __lrint_internal; .scl 2; .type 32; .endef\n"
    "\t.text\n"
    "\t.align 4\n"
    "\t.globl __lrint_internal\n"
    "__lrint_internal:\n"
    "\tvcvtr.s32.f64    s0, d0\n"
    "\tfmrs             r0, s0\n"
    "\tbx lr");
#endif /* defined(__arm__) || defined(_ARM_) */

long lrint (double x) 
{
  long retval = 0L;
#if defined (__x86_64__) && defined (__CYGWIN__)
  __asm__ __volatile__ ("fistpll %0"  : "=m" (retval) : "t" (x) : "st");
#elif defined(_AMD64_) || defined(__x86_64__) || defined(_X86_) || defined(__i386__)
  __asm__ __volatile__ ("fistpl %0"  : "=m" (retval) : "t" (x) : "st");
#elif defined(__arm__) || defined(_ARM_)
    retval = __lrint_internal(x);
#endif
  return retval;
}

/**
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is part of the mingw-w64 runtime package.
 * No warranty is given; refer to the file DISCLAIMER.PD within this package.
 */
#include <math.h>

#if defined(__arm__) || defined(_ARM_)
/* This works around a compiler bug */
double __rint_internal( double x );
asm(".def __rint_internal; .scl 2; .type 32; .endef\n"
    "\t.text\n"
    "\t.align 4\n"
    "\t.globl __rint_internal\n"
    "__rint_internal:\n"
    "\tvcvtr.s32.f64    s0, d0\n"
    "\tvcvt.f64.s32     d0, s0\n"
    "\tbx lr");
#endif /* defined(__arm__) || defined(_ARM_) */

double rint (double x) {
  double retval = 0.0;
#if defined(_AMD64_) || defined(__x86_64__) || defined(_X86_) || defined(__i386__)
  __asm__ __volatile__ ("frndint;" : "=t" (retval) : "0" (x));
#elif defined(__arm__) || defined(_ARM_)
    retval = __rint_internal(x);
#endif
  return retval;
}

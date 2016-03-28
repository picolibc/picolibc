/**
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is part of the mingw-w64 runtime package.
 * No warranty is given; refer to the file DISCLAIMER.PD within this package.
 */
#include <math.h>

#if defined(__arm__) || defined(_ARM_)
/* This works around a compiler bug */
long __lrintf_internal( float x );
asm(".def __lrintf_internal; .scl 2; .type 32; .endef\n"
    "\t.text\n"
    "\t.align 4\n"
    "\t.globl __lrintf_internal\n"
    "__lrintf_internal:\n"
    "\tvcvtr.s32.f32    s0, s0\n"
    "\tfmrs             r0, s0\n"
    "\tbx lr");
#endif /* defined(__arm__) || defined(_ARM_) */

long lrintf (float x) 
{
  long retval = 0l;
#if defined(_AMD64_) || defined(__x86_64__) || defined(_X86_) || defined(__i386__)
  __asm__ __volatile__ ("fistpl %0"  : "=m" (retval) : "t" (x) : "st");
#elif defined(__arm__) || defined(_ARM_)
    retval = __lrintf_internal(x);
#endif
  return retval;
}

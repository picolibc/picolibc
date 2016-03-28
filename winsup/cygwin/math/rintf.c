/**
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is part of the mingw-w64 runtime package.
 * No warranty is given; refer to the file DISCLAIMER.PD within this package.
 */
#include <math.h>

#if defined(__arm__) || defined(_ARM_)
/* This works around a compiler bug */
float __rintf_internal( float x );
asm(".def __rintf_internal; .scl 2; .type 32; .endef\n"
    "\t.text\n"
    "\t.align 4\n"
    "\t.globl __rintf_internal\n"
    "__rintf_internal:\n"
    "\tvcvtr.s32.f32    s0, s0\n"
    "\tvcvt.f32.s32     s0, s0\n"
    "\tbx lr");
#endif /* defined(__arm__) || defined(_ARM_) */

float rintf (float x) {
  float retval = 0.0F;
#if defined(_AMD64_) || defined(__x86_64__) || defined(_X86_) || defined(__i386__)
  __asm__ __volatile__ ("frndint;": "=t" (retval) : "0" (x));
#elif defined(__arm__) || defined(_ARM_)
    retval = __rintf_internal(x);
#endif
  return retval;
}

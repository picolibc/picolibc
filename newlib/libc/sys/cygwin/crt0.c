/* crt0.c.

   Copyright 2001 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifdef __PPC__
/* For the PowerPC, we want to make this function have its structured
   exception table exception function point to something we control.  */

extern void __cygwin_exception_handler();
extern void mainCRTStartup(void) __attribute__((__exception__(__cygwin_exception_handler)));
#endif

/* In the following ifdef'd i386 code, the FPU precision is set to 80 bits
   and all FPU exceptions are masked.  The former is needed to make long
   doubles work correctly.  The latter causes the FPU to generate NaNs and
   Infinities instead of signals for certain operations.
*/

#ifdef __i386__
#define FPU_RESERVED 0xF0C0
#define FPU_DEFAULT  0x033f

/* For debugging on *#!$@ windbg.  bp for breakpoint.  */
int __cygwin_crt0_bp = 0;
#endif

extern int main (int argc, char **argv);

void
mainCRTStartup ()
{
#ifdef __i386__
  if (__cygwin_crt0_bp)
    asm volatile ("int3");

  {
    volatile unsigned short cw;

    /* Get Control Word */
    __asm__ volatile ("fnstcw %0" : "=m" (cw) : );

    /* mask in */
    cw &= FPU_RESERVED;
    cw |= FPU_DEFAULT;

    /* set cw */
    __asm__ volatile ("fldcw %0" :: "m" (cw));
  }
#endif

  cygwin_crt0 (main);
}

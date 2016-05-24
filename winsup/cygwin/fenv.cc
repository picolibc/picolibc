/* fenv.cc

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "fenv.h"
#include "errno.h"
#include "wincap.h"
#include <string.h>

/*  Mask and shift amount for rounding bits.  */
#define FE_CW_ROUND_MASK	(0x0c00)
#define FE_CW_ROUND_SHIFT	(10)
/*  Same, for SSE MXCSR.  */
#define FE_MXCSR_ROUND_MASK	(0x6000)
#define FE_MXCSR_ROUND_SHIFT	(13)

/*  Mask and shift amount for precision bits.  */
#define FE_CW_PREC_MASK		(0x0300)
#define FE_CW_PREC_SHIFT	(8)

/*  In x87, exception status bits and mask bits occupy
   corresponding bit positions in the status and control
   registers, respectively.  In SSE, they are both located
   in the control-and-status register, with the status bits
   corresponding to the x87 positions, and the mask bits
   shifted by this amount to the left.  */
#define FE_SSE_EXCEPT_MASK_SHIFT (7)

/* These are writable so we can initialise them at startup.  */
static fenv_t fe_dfl_env;
static fenv_t fe_nomask_env;

/* These pointers provide the outside world with read-only access to them.  */
const fenv_t *_fe_dfl_env = &fe_dfl_env;
const fenv_t *_fe_nomask_env = &fe_nomask_env;

/*  Although Cygwin assumes i686 or above (hence SSE available) these
   days, and the compiler feels free to use it (depending on compile-
   time flags of course), we should avoid needlessly breaking any
   purely integer mode apps (or apps compiled with -mno-sse), so we
   only manage SSE state in this fenv module if we detect that SSE
   instructions are available at runtime.  If we didn't do this, all
   applications run on older machines would bomb out with an invalid
   instruction exception right at startup; let's not be *that* WJM!  */
static bool use_sse = false;

/*  This function enables traps for each of the exceptions as indicated
   by the parameter except. The individual exceptions are described in
   [ ... glibc manual xref elided ...]. Only the specified exceptions are
   enabled, the status of the other exceptions is not changed.
    The function returns the previous enabled exceptions in case the
   operation was successful, -1 otherwise.  */
int
feenableexcept (int excepts)
{
  unsigned short cw, old_cw;
  unsigned int mxcsr = 0;

  if (excepts & ~FE_ALL_EXCEPT)
    return -1;

  /* Get control words.  */
  __asm__ volatile ("fnstcw %0" : "=m" (old_cw) : );
  if (use_sse)
    __asm__ volatile ("stmxcsr %0" : "=m" (mxcsr) : );

  /* Enable exceptions by clearing mask bits.  */
  cw = old_cw & ~excepts;
  mxcsr &= ~(excepts << FE_SSE_EXCEPT_MASK_SHIFT);

  /* Store updated control words.  */
  __asm__ volatile ("fldcw %0" :: "m" (cw));
  if (use_sse)
    __asm__ volatile ("ldmxcsr %0" :: "m" (mxcsr));

  /* Return old value.  We assume SSE and x87 stay in sync.  Note that
     we are returning a mask of enabled exceptions, which is the opposite
     of the flags in the register, which are set to disable (mask) their
     related exceptions.  */
  return (~old_cw) & FE_ALL_EXCEPT;
}

/*  This function disables traps for each of the exceptions as indicated
   by the parameter except. The individual exceptions are described in
   [ ... glibc manual xref elided ...]. Only the specified exceptions are
   disabled, the status of the other exceptions is not changed.
    The function returns the previous enabled exceptions in case the
   operation was successful, -1 otherwise.  */
int
fedisableexcept (int excepts)
{
  unsigned short cw, old_cw;
  unsigned int mxcsr = 0;

  if (excepts & ~FE_ALL_EXCEPT)
    return -1;

  /* Get control words.  */
  __asm__ volatile ("fnstcw %0" : "=m" (old_cw) : );
  if (use_sse)
    __asm__ volatile ("stmxcsr %0" : "=m" (mxcsr) : );

  /* Disable exceptions by setting mask bits.  */
  cw = old_cw | excepts;
  mxcsr |= (excepts << FE_SSE_EXCEPT_MASK_SHIFT);

  /* Store updated control words.  */
  __asm__ volatile ("fldcw %0" :: "m" (cw));
  if (use_sse)
    __asm__ volatile ("ldmxcsr %0" :: "m" (mxcsr));

  /* Return old value.  We assume SSE and x87 stay in sync.  Note that
     we are returning a mask of enabled exceptions, which is the opposite
     of the flags in the register, which are set to disable (mask) their
     related exceptions.  */
  return (~old_cw) & FE_ALL_EXCEPT;
}

/*  This function returns a bitmask of all currently enabled exceptions. It
   returns -1 in case of failure.  */
int
fegetexcept (void)
{
  unsigned short cw;

  /* Get control word.  We assume SSE and x87 stay in sync.  */
  __asm__ volatile ("fnstcw %0" : "=m" (cw) : );

  /* Exception is *dis*abled when mask bit is set.  */
  return (~cw) & FE_ALL_EXCEPT;
}

/*  Store the floating-point environment in the variable pointed to by envp.
   The function returns zero in case the operation was successful, a non-zero
   value otherwise.  */
int
fegetenv (fenv_t *envp)
{
  __asm__ volatile ("fnstenv %0" : "=m" (envp->_fpu) : );
  if (use_sse)
    __asm__ volatile ("stmxcsr %0" : "=m" (envp->_sse_mxcsr) : );
  return 0;
}

/*  Store the current floating-point environment in the object pointed to
   by envp. Then clear all exception flags, and set the FPU to trap no
   exceptions.  Not all FPUs support trapping no exceptions; if feholdexcept
   cannot set this mode, it returns nonzero value.  If it succeeds, it
   returns zero.  */
int
feholdexcept (fenv_t *envp)
{
  unsigned int mxcsr;
  fegetenv (envp);
  mxcsr = envp->_sse_mxcsr & ~FE_ALL_EXCEPT;
  if (use_sse)
    __asm__ volatile ("ldmxcsr %0" :: "m" (mxcsr));
  __asm__ volatile ("fnclex");
  fedisableexcept (FE_ALL_EXCEPT);
  return 0;
}

/*  Set the floating-point environment to that described by envp.  The
   function returns zero in case the operation was successful, a non-zero
   value otherwise.  */
int
fesetenv (const fenv_t *envp)
{
  __asm__ volatile ("fldenv %0" :: "m" (envp->_fpu) );
  if (use_sse)
    __asm__ volatile ("ldmxcsr %0" :: "m" (envp->_sse_mxcsr));
  return 0;
}

/*  Like fesetenv, this function sets the floating-point environment to
   that described by envp. However, if any exceptions were flagged in the
   status word before feupdateenv was called, they remain flagged after
   the call.  In other words, after feupdateenv is called, the status
   word is the bitwise OR of the previous status word and the one saved
   in envp.  The function returns zero in case the operation was successful,
   a non-zero value otherwise.  */
int
feupdateenv (const fenv_t *envp)
{
  fenv_t envcopy;
  unsigned int mxcsr = 0;
  unsigned short sw;

  /* Don't want to modify *envp, but want to update environment atomically,
     so take a copy and merge the existing exceptions into it.  */
  memcpy (&envcopy, envp, sizeof *envp);
  __asm__ volatile ("fnstsw %0" : "=m" (sw) : );
  if (use_sse)
    __asm__ volatile ("stmxcsr %0" : "=m" (mxcsr) : );
  envcopy._fpu._fpu_sw |= (sw & FE_ALL_EXCEPT);
  envcopy._sse_mxcsr |= (mxcsr & FE_ALL_EXCEPT);

  return fesetenv (&envcopy);
}

/*  This function clears all of the supported exception flags indicated by
   excepts.  The function returns zero in case the operation was successful,
   a non-zero value otherwise.  */
int
feclearexcept (int excepts)
{
  fenv_t fenv;

  if (excepts & ~FE_ALL_EXCEPT)
    return EINVAL;

  /* Need to save/restore whole environment to modify status word.  */
  fegetenv (&fenv);

  /* Mask undesired bits out.  */
  fenv._fpu._fpu_sw &= ~excepts;
  fenv._sse_mxcsr &= ~excepts;

  /* Set back into FPU state.  */
  return fesetenv (&fenv);
}

/*  This function raises the supported exceptions indicated by
   excepts.  If more than one exception bit in excepts is set the order
   in which the exceptions are raised is undefined except that overflow
   (FE_OVERFLOW) or underflow (FE_UNDERFLOW) are raised before inexact
   (FE_INEXACT). Whether for overflow or underflow the inexact exception
   is also raised is also implementation dependent.  The function returns
   zero in case the operation was successful, a non-zero value otherwise.  */
int
feraiseexcept (int excepts)
{
  fenv_t fenv;

  if (excepts & ~FE_ALL_EXCEPT)
    return EINVAL;

  /* Need to save/restore whole environment to modify status word.  */
  __asm__ volatile ("fnstenv %0" : "=m" (fenv) : );

  /* Set desired exception bits.  */
  fenv._fpu._fpu_sw |= excepts;

  /* Set back into FPU state.  */
  __asm__ volatile ("fldenv %0" :: "m" (fenv));

  /* And trigger them - whichever are unmasked.  */
  __asm__ volatile ("fwait");

  return 0;
}

/*  Test whether the exception flags indicated by the parameter except
   are currently set. If any of them are, a nonzero value is returned
   which specifies which exceptions are set. Otherwise the result is zero.  */
int
fetestexcept (int excepts)
{
  unsigned short sw;
  unsigned int mxcsr = 0;

  if (excepts & ~FE_ALL_EXCEPT)
    return EINVAL;

  /* Get status registers.  */
  __asm__ volatile ("fnstsw %0" : "=m" (sw) : );
  if (use_sse)
    __asm__ volatile ("stmxcsr %0" : "=m" (mxcsr) : );

  /* Mask undesired bits out and return result.  */
  return (sw | mxcsr) & excepts;
}
/*  This function stores in the variable pointed to by flagp an
   implementation-defined value representing the current setting of the
   exception flags indicated by excepts.  The function returns zero in
   case the operation was successful, a non-zero value otherwise.  */
int
fegetexceptflag (fexcept_t *flagp, int excepts)
{
  unsigned short sw;
  unsigned int mxcsr = 0;

  if (excepts & ~FE_ALL_EXCEPT)
    return EINVAL;

  /* Get status registers.  */
  __asm__ volatile ("fnstsw %0" : "=m" (sw) : );
  if (use_sse)
    __asm__ volatile ("stmxcsr %0" : "=m" (mxcsr) : );

  /* Mask undesired bits out and set result.  */
  *flagp = (sw | mxcsr) & excepts;

  return 0;
}

/*  This function restores the flags for the exceptions indicated by
   excepts to the values stored in the variable pointed to by flagp.  */
int
fesetexceptflag (const fexcept_t *flagp, int excepts)
{
  fenv_t fenv;

  if (excepts & ~FE_ALL_EXCEPT)
    return EINVAL;

  /* Need to save/restore whole environment to modify status word.  */
  fegetenv (&fenv);

  /* Set/Clear desired exception bits.  */
  fenv._fpu._fpu_sw &= ~excepts;
  fenv._fpu._fpu_sw |= excepts & *flagp;
  fenv._sse_mxcsr &= ~excepts;
  fenv._sse_mxcsr |= excepts & *flagp;

  /* Set back into FPU state.  */
  return fesetenv (&fenv);
}

/*  Returns the currently selected rounding mode, represented by one of the
   values of the defined rounding mode macros.  */
int
fegetround (void)
{
  unsigned short cw;

  /* Get control word.  We assume SSE and x87 stay in sync.  */
  __asm__ volatile ("fnstcw %0" : "=m" (cw) : );

  return (cw & FE_CW_ROUND_MASK) >> FE_CW_ROUND_SHIFT;
}

/*  Changes the currently selected rounding mode to round. If round does
   not correspond to one of the supported rounding modes nothing is changed.
   fesetround returns zero if it changed the rounding mode, a nonzero value
   if the mode is not supported.  */
int
fesetround (int round)
{
  unsigned short cw;
  unsigned int mxcsr = 0;

  /* Will succeed for any valid value of the input parameter.  */
  if (round < FE_TONEAREST || round > FE_TOWARDZERO)
    return EINVAL;

  /* Get control words.  */
  __asm__ volatile ("fnstcw %0" : "=m" (cw) : );
  if (use_sse)
    __asm__ volatile ("stmxcsr %0" : "=m" (mxcsr) : );

  /* Twiddle bits.  */
  cw &= ~FE_CW_ROUND_MASK;
  cw |= (round << FE_CW_ROUND_SHIFT);
  mxcsr &= ~FE_MXCSR_ROUND_MASK;
  mxcsr |= (round << FE_MXCSR_ROUND_SHIFT);

  /* Set back into FPU state.  */
  __asm__ volatile ("fldcw %0" :: "m" (cw));
  if (use_sse)
    __asm__ volatile ("ldmxcsr %0" :: "m" (mxcsr));

  /* Indicate success.  */
  return 0;
}

/*  Returns the currently selected precision, represented by one of the
   values of the defined precision macros.  */
int
fegetprec (void)
{
  unsigned short cw;

  /* Get control word.  */
  __asm__ volatile ("fnstcw %0" : "=m" (cw) : );

  return (cw & FE_CW_PREC_MASK) >> FE_CW_PREC_SHIFT;
}

/*  Changes the currently selected precision to prec. If prec does not
   correspond to one of the supported rounding modes nothing is changed.
   fesetprec returns zero if it changed the precision, or a nonzero value
   if the mode is not supported.  */
int
fesetprec (int prec)
{
  unsigned short cw;

  /* Will succeed for any valid value of the input parameter.  */
  if (prec < FE_SINGLEPREC || prec > FE_EXTENDEDPREC)
    return EINVAL;

  /* Get control word.  */
  __asm__ volatile ("fnstcw %0" : "=m" (cw) : );

  /* Twiddle bits.  */
  cw &= ~FE_CW_PREC_MASK;
  cw |= (prec << FE_CW_PREC_SHIFT);

  /* Set back into FPU state.  */
  __asm__ volatile ("fldcw %0" :: "m" (cw));

  /* Indicate success.  */
  return 0;
}

/*  Set up the FPU and SSE environment at the start of execution.  */
void
_feinitialise (void)
{
  unsigned int edx, eax, mxcsr;

  /* Check for presence of SSE: invoke CPUID #1, check EDX bit 25.  */
  eax = 1;
  __asm__ volatile ("cpuid" : "=d" (edx), "+a" (eax) :: "%ecx", "%ebx");
  /* If this flag isn't set we'll avoid trying to execute any SSE.  */
  if ((edx & (1 << 25)) != 0)
    use_sse = true;

  /* Reset FPU: extended prec, all exceptions cleared and masked off.  */
  __asm__ volatile ("fninit");
  /* The default cw value, 0x37f, is rounding mode zero.  The MXCSR has
     no precision control, so the only thing to do is set the exception
     mask bits.  */
  mxcsr = FE_ALL_EXCEPT << FE_SSE_EXCEPT_MASK_SHIFT;
  if (use_sse)
    __asm__ volatile ("ldmxcsr %0" :: "m" (mxcsr));

  /* Setup unmasked environment.  */
  feenableexcept (FE_ALL_EXCEPT);
  fegetenv (&fe_nomask_env);

  /* Restore default exception masking (all masked).  */
  fedisableexcept (FE_ALL_EXCEPT);

  /* Finally cache state as default environment. */
  fegetenv (&fe_dfl_env);
}


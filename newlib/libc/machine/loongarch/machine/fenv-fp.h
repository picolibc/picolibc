/*
  (c) Copyright 2017 Michael R. Neilly
  (c) Copyright 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  * Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  * Neither the names of the copyright holders nor the names of their
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

/* This implementation is intended to comply with the following
 * specification:
 *
 * http://pubs.opengroup.org/onlinepubs/009695399/functions/feclearexcept.html
 * Referred to as 'feclearexcept.html below.
 *
 * "The feclearexcept() function shall attempt to clear the supported
 * floating-point exceptions represented by excepts."
 */

#define _FPU_DEFAULT 0x0
#define _FPU_IEEE 0x1F

__declare_fenv_inline(int) feclearexcept(int excepts)
{
  int fcsr;

  excepts &= FE_ALL_EXCEPT;
  _movfcsr2gr(fcsr);
  fcsr &= ~(excepts | (excepts << _FCSR_CAUSE_LSHIFT));
  _movgr2fcsr(fcsr);

  /* Per 'feclearexcept.html
   * "If the argument is zero or if all the specified exceptions were
   * successfully cleared, feclearexcept() shall return zero. Otherwise,
   * it shall return a non-zero value."
   */

  return 0;
}

/* This implementation is intended to comply with the following
 * specification:
 *
 * http://pubs.opengroup.org/onlinepubs/009695399/functions/fegetenv.html
 * Referred to as 'fegetenv.html below.
 *
 * "The fegetenv() function shall attempt to store the current
 * floating-point environment in the object pointed to by envp."
 *
 */

__declare_fenv_inline(int) fegetenv(fenv_t *envp)
{

  /* Get the current environment (FCSR) */
  int fcsr;

  _movfcsr2gr(fcsr);
  *envp = fcsr;

  /* Per 'fegetenv.html:
   *
   * "If the representation was successfully stored, fegetenv() shall
   * return zero. Otherwise, it shall return a non-zero value.
   */

  return 0;
}

/* This implementation is intended to comply with the following
 * specification:
 *
 * http://pubs.opengroup.org/onlinepubs/009695399/functions/fegetexceptflag.html
 * Referred to as 'fegetexceptflag.html below.
 *
 * "The fegetexceptflag() function shall attempt to store an
 * implementation-defined representation of the states of the
 * floating-point status flags indicated by the argument excepts in
 * the object pointed to by the argument flagp."
 */

__declare_fenv_inline(int) fegetexceptflag(fexcept_t *flagp, int excepts)
{

  int fcsr;

  _movfcsr2gr(fcsr);

  *flagp = fcsr & excepts & FE_ALL_EXCEPT;

  /* Per 'fegetexceptflag.html:
   *
   * "If the representation was successfully stored, fegetexceptflag()
   * shall return zero. Otherwise, it shall return a non-zero
   * value."
   */

  return 0;
}

/* This implementation is intended to comply with the following
 * specification:
 *
 * http://pubs.opengroup.org/onlinepubs/009695399/functions/fegetround.html
 * Referred to as 'fegetround.html below.
 *
 * "The fegetround() function shall get the current rounding direction."
 */

__declare_fenv_inline(int) fegetround(void)
{

  /* Get current rounding mode */
  int fcsr;

  _movfcsr2gr(fcsr);

  /* Per 'fegetround.html:
   *
   * "The fegetround() function shall return the value of the rounding
   * direction macro representing the current rounding direction or a
   * negative value if there is no such rounding direction macro or
   * the current rounding direction is not determinable."
   */

  /* Return the rounding mode */

  return fcsr & FE_RMODE_MASK;

}

/* This implementation is intended to comply with the following
 * specification:
 *
 * http://pubs.opengroup.org/onlinepubs/009695399/functions/feholdexcept.html
 * Referred to as 'feholdexcept.html below.
 *
 * "The feholdexcept() function shall save the current floating-point
 * environment in the object pointed to by envp, clear the
 * floating-point status flags, and then install a non-stop (continue
 * on floating-point exceptions) mode, if available, for all
 * floating-point exceptions."
 */

__declare_fenv_inline(int) feholdexcept(fenv_t *envp)
{

  /* Store the current FP environment in envp*/
  int fcsr;

  _movfcsr2gr(fcsr);
  *envp = fcsr;

  /* Clear all exception enable bits and flags.  */
  fcsr &= ~(FE_ALL_EXCEPT >> _FCSR_ENABLE_RSHIFT | FE_ALL_EXCEPT);
  _movgr2fcsr(fcsr);


  /* Per 'feholdexcept.html:
   *
   * "The feholdexcept() function shall return zero if and only if
   * non-stop floating-point exception handling was successfully
   * installed."
   */

  return 0;
}

/* This implementation is intended to comply with the following
 * specification:
 *
 * http://pubs.opengroup.org/onlinepubs/009695399/functions/feraiseexcept.html
 * Referred to as 'feraiseexcept.html below.
 *
 * "The feraiseexcept() function shall attempt to raise the supported
 * floating-point exceptions represented by the excepts argument. The
 * order in which these floating-point exceptions are raised is
 * unspecified, except that if the excepts argument represents IEC
 * 60559 valid coincident floating-point exceptions for atomic
 * operations (namely overflow and inexact, or underflow and inexact),
 * then overflow or underflow shall be raised before inexact. Whether
 * the feraiseexcept() function additionally raises the inexact
 * floating-point exception whenever it raises the overflow or
 * underflow floating-point exception is implementation-defined."
 */

__declare_fenv_inline(int) feraiseexcept(int excepts)
{
  const float fp_zero = 0.0f;
  const float fp_one = 1.0f;
  const float fp_max = __FLT_MAX__;
  const float fp_min = __FLT_MIN__;
  const float fp_1e32 = 1.0e32f;
  const float fp_two = 2.0f;
  const float fp_three = 3.0f;

  if (FE_INVALID & excepts)
    __asm__ __volatile__("fdiv.s $f0,%0,%0\n\t"
			 :
			 : "f"(fp_zero)
			 : "$f0");

  if (FE_DIVBYZERO & excepts)
    __asm__ __volatile__("fdiv.s $f0,%0,%1\n\t"
			 :
			 : "f"(fp_one), "f"(fp_zero)
			 : "$f0");

  if (FE_OVERFLOW & excepts)
    __asm__ __volatile__("fadd.s $f0,%0,%1\n\t"
			 :
			 : "f"(fp_max), "f"(fp_1e32)
			 : "$f0");

  if (FE_UNDERFLOW & excepts)
    __asm__ __volatile__("fdiv.s $f0,%0,%1\n\t"
			 :
			 : "f"(fp_min), "f"(fp_three)
			 : "$f0");

  if (FE_INEXACT & excepts)
    __asm__ __volatile__("fdiv.s $f0, %0, %1\n\t"
			 :
			 : "f"(fp_two), "f"(fp_three)
			 : "$f0");

  /* Per 'feraiseexcept.html:
   * "If the argument is zero or if all the specified exceptions were
   * successfully raised, feraiseexcept() shall return
   * zero. Otherwise, it shall return a non-zero value."
   */

  return 0;
}

__declare_fenv_inline(int) fesetexcept(int excepts)
{
  int fcsr;

  _movfcsr2gr(fcsr);
  fcsr |= excepts & FE_ALL_EXCEPT;
  _movgr2fcsr(fcsr);

  return 0;
}

/* This implementation is intended to comply with the following
 * specification:
 *
 * http://pubs.opengroup.org/onlinepubs/009695399/functions/fegetenv.html
 * Referred to as 'fegetenv.html below.
 *
 * "The fegetenv() function shall attempt to store the current
 * floating-point environment in the object pointed to by envp.
 *
 * The fesetenv() function shall attempt to establish the
 * floating-point environment represented by the object pointed to by
 * envp. The argument envp shall point to an object set by a call to
 * fegetenv() or feholdexcept(), or equal a floating-point environment
 * macro. The fesetenv() function does not raise floating-point
 * exceptions, but only installs the state of the floating-point
 * status flags represented through its argument."
 */

__declare_fenv_inline(int) fesetenv(const fenv_t *envp)
{

  /* Set environment (FCSR) */

  fenv_t fcsr;
  
  /* Hazard handling */
  _movfcsr2gr(fcsr);
  fcsr = *envp;
  _movgr2fcsr(fcsr);


  /* Per 'fegetenv.html:
   *
   * "If the environment was successfully established, fesetenv()
   * shall return zero. Otherwise, it shall return a non-zero value.
   */

  return 0;
}

/* This implementation is intended to comply with the following
 * specification:
 *
 * http://pubs.opengroup.org/onlinepubs/009695399/functions/fesetexceptflag.html
 * Referred to as 'fesetexceptflag.html below.
 *
 * "The fesetexceptflag() function shall attempt to set the
 * floating-point status flags indicated by the argument excepts to
 * the states stored in the object pointed to by flagp. The value
 * pointed to by flagp shall have been set by a previous call to
 * fegetexceptflag() whose second argument represented at least those
 * floating-point exceptions represented by the argument excepts. This
 * function does not raise floating-point exceptions, but only sets
 * the state of the flags."
 *
 */

__declare_fenv_inline(int) fesetexceptflag(const fexcept_t *flagp, int excepts)
{
  int fcsr;

  _movfcsr2gr(fcsr);

  excepts &= FE_ALL_EXCEPT;
  fcsr = (fcsr & ~excepts) | (*flagp & excepts);

  _movgr2fcsr(fcsr);

  /* Per 'fesetexceptflag.html:
   *
   * "If the excepts argument is zero or if all the specified
   * exceptions were successfully set, fesetexceptflag() shall return
   * zero. Otherwise, it shall return a non-zero value."
   */

  return 0;
}

/* This implementation is intended to comply with the following
 * specification:
 *
 * http://pubs.opengroup.org/onlinepubs/009695399/functions/fesetround.html
 * Referred to as 'fegetenv.html below.
 *
 * "The fesetround() function shall establish the rounding direction
 * represented by its argument round. If the argument is not equal to
 * the value of a rounding direction macro, the rounding direction is
 * not changed."
 */

__declare_fenv_inline(int) fesetround(int round)
{

  int fcsr;

  if ((round & ~FE_RMODE_MASK) != 0)
    return -1;

  _movfcsr2gr(fcsr);
  fcsr &= ~FE_RMODE_MASK;
  fcsr |= round;
  _movgr2fcsr(fcsr);


  /* Per 'fesetround.html:
   *
   * "The fesetround() function shall return a zero value if and only
   * if the requested rounding direction was established."
   */

  return 0;
}

/* This implementation is intended to comply with the following
 * specification:
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/fetestexcept.html
 *
 * "The fetestexcept() function shall determine which of a specified
 * subset of the floating-point exception flags are currently set. The
 * excepts argument specifies the floating-point status flags to be
 * queried."
 */

__declare_fenv_inline(int) fetestexcept(int excepts)
{
  int fcsr;

  _movfcsr2gr(fcsr);

  /* "The fetestexcept() function shall return the value of the
   * bitwise-inclusive OR of the floating-point exception macros
   * corresponding to the currently set floating-point exceptions
   * included in excepts."
   */

  return (fcsr & excepts & FE_ALL_EXCEPT);
}

/* This implementation is intended to comply with the following
 * specification:
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/functions/feupdateenv.html
 *
 * "The feupdateenv() function shall attempt to save the currently
 * raised floating-point exceptions in its automatic storage, attempt
 * to install the floating-point environment represented by the object
 * pointed to by envp, and then attempt to raise the saved
 * floating-point exceptions. The argument envp shall point to an
 * object set by a call to feholdexcept() or fegetenv(), or equal a
 * floating-point environment macro."
 */

__declare_fenv_inline(int) feupdateenv(const fenv_t *envp)
{
  int fcsr;

  _movfcsr2gr(fcsr);

  fesetenv(envp);

  feraiseexcept(fcsr & FE_ALL_EXCEPT);

  /* "The feupdateenv() function shall return a zero value if and only
   * if all the required actions were successfully carried out."
   */

  return 0;

}

__declare_fenv_inline(int) feenableexcept(int excepts)
{
  int fcsr, old_excepts;

  _movfcsr2gr(fcsr);

  old_excepts = (fcsr << _FCSR_ENABLE_RSHIFT) & FE_ALL_EXCEPT;

  excepts &= FE_ALL_EXCEPT;

  fcsr |= excepts >> _FCSR_ENABLE_RSHIFT;
  _movgr2fcsr(fcsr);

  return old_excepts;
}

__declare_fenv_inline(int) fedisableexcept(int excepts)
{
  int fcsr, old_excepts;

  _movfcsr2gr(fcsr);

  old_excepts = (fcsr << _FCSR_ENABLE_RSHIFT) & FE_ALL_EXCEPT;

  excepts &= FE_ALL_EXCEPT;

  fcsr &= ~(excepts >> _FCSR_ENABLE_RSHIFT);
  _movgr2fcsr(fcsr);

  return old_excepts;
}

__declare_fenv_inline(int) fegetexcept(void)
{
  int fcsr;

  _movfcsr2gr(fcsr);
  return (fcsr << _FCSR_ENABLE_RSHIFT) & FE_ALL_EXCEPT;
}

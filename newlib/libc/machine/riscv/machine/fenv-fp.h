/*
  (c) Copyright 2017 Michael R. Neilly
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

__declare_fenv_inline(int) feclearexcept(int excepts)
{

  /* Mask excepts to be sure only supported flag bits are set */

  excepts &= FE_ALL_EXCEPT;

  /* Per "The RISC-V Instruction Set Manual: Volume I: User-Level ISA:
   * Version 2.1":
   *
   * "The CSRRC (Atomic Read and Clear Bits in CSR) instruction reads
   * the value of the CSR, zeroextends the value to XLEN bits, and
   * writes it to integer register rd. The initial value in integer
   * register rs1 is treated as a bit mask that specifies bit
   * positions to be cleared in the CSR. Any bit that is high in rs1
   * will cause the corresponding bit to be cleared in the CSR, if
   * that CSR bit is writable. Other bits in the CSR are unaffected."
   */

  /* Clear the requested flags */

  __asm__ volatile("csrrc zero, fflags, %0" : : "r"(excepts));

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

  fenv_t fcsr;
  __asm__ volatile("frcsr %0" : "=r"(fcsr));

  /* Store FCSR in envp */

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

  /* Mask excepts to be sure only supported flag bits are set */

  excepts &= FE_ALL_EXCEPT;

  /* Get current exception flags */

  fexcept_t flags;
  __asm__ volatile("frflags %0" : "=r"(flags));

  /* Return the requested flags in flagp */

  *flagp = flags & excepts;

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

  fenv_t frm;
  __asm__ volatile("frrm %0" : "=r"(frm));

  /* Per 'fegetround.html:
   *
   * "The fegetround() function shall return the value of the rounding
   * direction macro representing the current rounding direction or a
   * negative value if there is no such rounding direction macro or
   * the current rounding direction is not determinable."
   */

  /* Return the rounding mode */

  return frm;

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

  fenv_t fcsr;
  __asm__ volatile("frcsr %0" : "=r"(fcsr));
  *envp = fcsr;

  /* Clear all flags */

  __asm__ volatile("csrrc zero, fflags, %0" : : "r"(FE_ALL_EXCEPT));

  /* RISC-V does not raise FP traps so it is always in a "non-stop" mode */

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

  /* Mask excepts to be sure only supported flag bits are set */

  excepts &= FE_ALL_EXCEPT;

  /* Set the requested exception flags */

  __asm__ volatile("csrs fflags, %0" : : "r"(excepts));

  /* Per 'feraiseexcept.html:
   * "If the argument is zero or if all the specified exceptions were
   * successfully raised, feraiseexcept() shall return
   * zero. Otherwise, it shall return a non-zero value."
   */

  /* Per "The RISC-V Instruction Set Manual: Volume I: User-Level ISA:
   * Version 2.1":
   *
   * "As allowed by the standard, we do not support traps on
   * floating-point exceptions in the base ISA, but instead require
   * explicit checks of the flags in software."
   *
   */

  return 0;
}

__declare_fenv_inline(int) fesetexcept(int excepts)
{
    return feraiseexcept(excepts);
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

  fenv_t fcsr = *envp;
  __asm__ volatile("fscsr %0" : : "r"(fcsr));

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

  /* Mask excepts to be sure only supported flag bits are set */

  excepts &= FE_ALL_EXCEPT;

  /* Set the requested flags */

  fexcept_t flags = *flagp & FE_ALL_EXCEPT;

  /* Set new the flags */

  __asm__ volatile("csrc fflags, %0" : : "r"(excepts));
  __asm__ volatile("csrs fflags, %0" : : "r"(flags & excepts));

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

  /* Mask round to be sure only valid rounding bits are set */

  round &= FE_RMODE_MASK;

  /* Set the rounding mode */

  __asm__ volatile("fsrm %0" : : "r"(round));

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

  /* Mask excepts to be sure only supported flag bits are set */

  excepts &= FE_ALL_EXCEPT;

  /* Read the current flags */

  fexcept_t flags;
  __asm__ volatile("frflags %0" : "=r"(flags));

  /* "The fetestexcept() function shall return the value of the
   * bitwise-inclusive OR of the floating-point exception macros
   * corresponding to the currently set floating-point exceptions
   * included in excepts."
   */

  return (flags & excepts);
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

  /* Get current exception flags */

  fexcept_t flags;
  __asm__ volatile("frflags %0" : "=r"(flags));

  /* Set the environment as requested */

  fenv_t fcsr = *envp; /* Environment to install */
  __asm__ volatile("fscsr %0" : : "r"(fcsr)); /* Install environment */

  /* OR in the saved exception flags */

  __asm__ volatile("csrs fflags, %0" : : "r"(flags));

  /* "The feupdateenv() function shall return a zero value if and only
   * if all the required actions were successfully carried out."
   */

  return 0;

}

__declare_fenv_inline(int) feenableexcept(int excepts)
{
    return excepts == 0 ? 0 : -1;
}

__declare_fenv_inline(int) fedisableexcept(int excepts)
{
    (void) excepts;
    return excepts == 0 ? 0 : -1;
}

__declare_fenv_inline(int) fegetexcept(void)
{
    return 0;
}

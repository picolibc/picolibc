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

#include <fenv.h>

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

int feraiseexcept(int excepts)
{

  /* Mask excepts to be sure only supported flag bits are set */

  excepts &= FE_ALL_EXCEPT;

#if __riscv_flen

  /* Set the requested exception flags */

  asm volatile("csrs fflags, %0" : : "r"(excepts));

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

#endif

  return (excepts != 0);
}

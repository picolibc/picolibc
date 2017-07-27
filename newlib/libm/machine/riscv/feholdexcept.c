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
 * http://pubs.opengroup.org/onlinepubs/009695399/functions/feholdexcept.html
 * Referred to as 'feholdexcept.html below.
 *
 * "The feholdexcept() function shall save the current floating-point
 * environment in the object pointed to by envp, clear the
 * floating-point status flags, and then install a non-stop (continue
 * on floating-point exceptions) mode, if available, for all
 * floating-point exceptions."
 */

int feholdexcept(fenv_t *envp)
{

#if __riscv_flen

  /* Store the current FP environment in envp*/

  fenv_t fcsr;
  asm volatile("frcsr %0" : "=r"(fcsr));
  *envp = fcsr;

  /* Clear all flags */

  asm volatile("csrrc zero, fflags, %0" : : "r"(FE_ALL_EXCEPT));

  /* RISC-V does not raise FP traps so it is always in a "non-stop" mode */

  /* Per 'feholdexcept.html:
   *
   * "The feholdexcept() function shall return zero if and only if
   * non-stop floating-point exception handling was successfully
   * installed."
   */

#endif

  return 0;
}

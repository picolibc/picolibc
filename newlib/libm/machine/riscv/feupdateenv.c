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

int feupdateenv(const fenv_t *envp)
{

#if __riscv_flen

  /* Get current exception flags */

  fexcept_t flags;
  asm volatile("frflags %0" : "=r"(flags));

  /* Set the environment as requested */

  fenv_t fcsr = *envp; /* Environment to install */
  asm volatile("fscsr %0" : : "r"(fcsr)); /* Install environment */

  /* OR in the saved exception flags */

  asm volatile("csrs fflags, %0" : : "r"(flags));

  /* "The feupdateenv() function shall return a zero value if and only
   * if all the required actions were successfully carried out."
   */

  return 0;

#else

  /* For soft float */

#if defined FE_NOMASK_ENV && FE_ALL_EXCEPT != 0

  if (envp == FE_NOMASK_ENV)
      return 1;

#endif

  return 0;

#endif
}

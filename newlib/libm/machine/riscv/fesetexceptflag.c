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

int fesetexceptflag(const fexcept_t *flagp, int excepts)
{

#if __riscv_flen

  /* Mask excepts to be sure only supported flag bits are set */

  excepts &= FE_ALL_EXCEPT;

  /* Set the requested flags */

  fexcept_t fflags = (excepts & *flagp);

  /* Set new the flags */

  asm volatile("csrs fflags, %0" : : "r"(fflags));

  /* Per 'fesetexceptflag.html:
   *
   * "If the excepts argument is zero or if all the specified
   * exceptions were successfully set, fesetexceptflag() shall return
   * zero. Otherwise, it shall return a non-zero value."
   */

#endif

  return 0;
}

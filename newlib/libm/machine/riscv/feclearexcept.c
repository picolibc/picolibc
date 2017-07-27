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
#include <stddef.h>

/* This implementation is intended to comply with the following
 * specification:
 *
 * http://pubs.opengroup.org/onlinepubs/009695399/functions/feclearexcept.html
 * Referred to as 'feclearexcept.html below.
 *
 * "The feclearexcept() function shall attempt to clear the supported
 * floating-point exceptions represented by excepts."
 */

int feclearexcept(int excepts)
{

#if __riscv_flen

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

  asm volatile("csrrc zero, fflags, %0" : : "r"(excepts));

  /* Per 'feclearexcept.html
   * "If the argument is zero or if all the specified exceptions were
   * successfully cleared, feclearexcept() shall return zero. Otherwise,
   * it shall return a non-zero value."
   */

#endif

  return 0;
}

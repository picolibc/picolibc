/* EPIPHANY implementation of _close ()

   Copyright (c) 2011, 2012 Adapteva, Inc.
   All rights reserved.

   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com> for Adapteva Inc

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Adapteva nor the names of its contributors may be
      used to endorse or promote products derived from this software without
      specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.                                               */


#include <errno.h>
#include "epiphany-syscalls.h"

/* ------------------------------------------------------------------------- */
/*!Close an open file

   When run with the simulator, asm_close () uses the host close routine,
   otherwise it is up to the hardware to provide a suitable BSP
   implementation. In either case it returns 0 on success or -1 on failure,
   and updates errno on failure.

   @param[in] filedes  The file descriptor to close

   @return  0 on success, -1 on failure with an error code in errno.         */
/* ------------------------------------------------------------------------- */
int __attribute__ ((section ("libgloss_epiphany")))
_close (int  fildes)
{
  return  asm_close (fildes);

}	/* _close () */

/* EPIPHANY implementation of _unlink ()

   Copyright (c) 2011, Adapteva, Inc.
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


#include <unistd.h>
#include <syscall.h>


/* ------------------------------------------------------------------------- */
/*!Remove a file

   Call an assembly function which will generate a system call trap. Either
   the simulator host file system or the hardware BSP must provide the
   functionality.

   @param[in] name  Name of file to remove

   @return  0 on success, -1 on failure with an error code in errno.         */
/* ------------------------------------------------------------------------- */
int __attribute__ ((section ("libgloss_epiphany")))
_unlink (const char *name)
{
  return asm_syscall (name, NULL, NULL, SYS_unlink);


}	/* _unlink () */

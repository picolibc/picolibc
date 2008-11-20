/*
(C) Copyright IBM Corp. 2007, 2008

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of IBM nor the names of its contributors may be
used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*/

#include <stddef.h>
#include <sys/syscall.h>
#include "ea_internal.h"
#include <ea.h>

typedef struct
{
  unsigned int memptr;
  unsigned int pad0[3];
  unsigned long long align;
  unsigned int pad1[2];
  unsigned long long size;
  unsigned int pad2[2];
} memalign_ea_t;

COMPAT_EA_ALIAS (posix_memalign_ea);

int
posix_memalign_ea (__ea void **memptr, size_ea_t align, size_ea_t size)
{
  memalign_ea_t arg;

  /*
   * Note that memptr is an LS address that will store an EA address. So,
   * it fits in 32 bits.
   *
   * The assist call will store 32 or 64 bits, depending on whether it's
   * 32 or 64 bit ppu code.
   */
  arg.memptr = (unsigned int) memptr;
  arg.align = (unsigned long long) align;
  arg.size = (unsigned long long) size;
  return __send_to_ppe (JSRE_LIBEA_SIGNALCODE, SPE_LIBEA_POSIX_MEMALIGN,
			&arg);
}

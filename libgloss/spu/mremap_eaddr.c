/*
(C) Copyright IBM Corp. 2007

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

#include <sys/mman.h>
#include "jsre.h"

typedef struct
{
  unsigned long long old_addr;
  unsigned int pad0[2];
  unsigned int old_size;
  unsigned int pad1[3];
  unsigned int new_size;
  unsigned int pad2[3];
  unsigned int flags;
  unsigned int pad3[3];
} syscall_mremap_t;

unsigned long long mremap_eaddr(unsigned long long old_addr, size_t old_size,
                                size_t new_size, int flags)
{
  syscall_mremap_t sys;

  sys.old_addr = old_addr;
  sys.old_size = (unsigned int) old_size;
  sys.new_size = (unsigned int) new_size;
  sys.flags = (unsigned int) flags;
  __send_to_ppe (JSRE_POSIX1_SIGNALCODE, JSRE_MREMAP, &sys);
  /*
   * Extract 64 bit result from the result stored in sys.
   */
  return *(unsigned long long *) (&sys);
}

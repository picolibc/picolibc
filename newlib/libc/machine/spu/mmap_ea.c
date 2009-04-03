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
#include <errno.h>
#include <sys/mman.h>
#include "ea_internal.h"
#include <ea.h>

COMPAT_EA_ALIAS (mmap_ea);

__ea void *mmap_ea (__ea void *start, size_ea_t length, int prot, int
             flags, int fd, off_t offset)
{
#ifdef __EA64__
  if (length > 0xffffffffULL) {
    errno = ENOMEM;
    return MAP_FAILED_EADDR;
  } else {
    return (__ea void *) mmap_eaddr ((unsigned long long) start,
                                     (size_t) length, prot, flags, fd, offset);
  }
#else /* __EA32__ */
  unsigned long long res;
  /*
   * mmap_eaddr returns 64 bits. For ea32, only the lower 32 bits can be
   * returned.
   */
  res = mmap_eaddr ((unsigned long long) (unsigned int) start, length,
                    prot, flags, fd, offset);
  if (res != MAP_FAILED_EADDR && res > 0xffffffffULL) {
    munmap_eaddr (res, length);
    errno = ENOMEM;
    res = MAP_FAILED_EADDR;
  }
  return (__ea void *) (int) res;
#endif
}

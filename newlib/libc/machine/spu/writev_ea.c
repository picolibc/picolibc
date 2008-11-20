/*
(C) Copyright IBM Corp. 2008

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

Author: Ken Werner <ken.werner@de.ibm.com>
*/

#include "ea_internal.h"
#include <ea.h>
#include <spu_cache.h>
#include "sys/linux_syscalls.h"

extern void __cache_flush (void) __attribute__ ((weak));

COMPAT_EA_ALIAS (writev_ea);

ssize_ea_t
writev_ea (int fd, struct iovec_ea *vector, int count)
{
#ifdef __EA32__
  int i;
#endif
  struct spu_syscall_block s = {
    __NR_writev,
    {
     fd,
     (size_ea_t) (__ea void *) vector,
     count,
     0,
     0,
     0}
  };
#ifdef __EA32__
  for (i = 0; i < count; ++i)
    {
      vector[i].__pad1 = 0x0;	/* 32 bit padding */
      vector[i].__pad2 = 0x0;	/* 32 bit padding */
    }
#endif
  /* Flush cache only if the application really uses the software cache.  */
  if (__cache_flush)
    __cache_flush ();
  return __linux_syscall (&s);
}

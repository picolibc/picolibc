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

#include <string.h>
#include <stddef.h>
#include "ea_internal.h"
#include <ea.h>
#include <spu_cache.h>
#include "../../string/local.h"

COMPAT_EA_ALIAS (memcpy_ea);

__ea void *
__inhibit_loop_to_libcall
memcpy_ea (__ea void *dest, __ea const void *src, size_ea_t n)
{
  __ea void *curr_dest = dest;
  __ea const void *curr_src = src;
  void *l_dest;
  void *l_src;
  size_ea_t local_n;
  size_ea_t src_n;
  size_ea_t dst_n;

  while (n)
    {
      l_src = __cache_fetch ((__ea void *) curr_src);

      /*
       * use the smaller of the size left to copy (n), the space left in the
       * src cacheline (src_n), or the space left in the destination
       * cacheline (dst_n)
       */
      src_n = ROUND_UP_NEXT_128 ((size_ea_t) curr_src) - (size_ea_t) curr_src;
      dst_n =
	ROUND_UP_NEXT_128 ((size_ea_t) curr_dest) - (size_ea_t) curr_dest;
      local_n = three_way_min (src_n, dst_n, n);

      l_dest = __cache_fetch_dirty (curr_dest, local_n);

      memcpy (l_dest, l_src, local_n);

      /* update values to take into account what we copied */
      curr_src += local_n;
      curr_dest += local_n;
      n -= local_n;
    }

  return dest;
}

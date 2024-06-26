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

#include <picolibc.h>

#include <string.h>
#include <stddef.h>
#include <spu_intrinsics.h>
#include <spu_cache.h>
#include "ea_internal.h"
#include "strncmp.h"
#include <ea.h>

COMPAT_EA_ALIAS (strncmp_ea);

int
strncmp_ea (__ea void *s1, __ea const void *s2, size_ea_t n3)
{
  __ea void *curr_s1 = (__ea void *) s1;
  __ea void *curr_s2 = (__ea void *) s2;
  void *l_s1;
  void *l_s2;
  int min;
  size_ea_t s2_n;
  size_ea_t s1_n;
  int ret;
  vec_uint4 end_v;

  ret = 0;			/* in case n3 is 0 */
  while (n3)
    {
      l_s2 = __cache_fetch (curr_s2);
      l_s1 = __cache_fetch (curr_s1);

      /*
       * Use the smaller of the size left to compare (n3), the space left in
       * s2 cacheline (s2_n), or the space left in the s1 cacheline (s1_n)
       */
      s2_n = ROUND_UP_NEXT_128 ((size_ea_t) curr_s2) - (size_ea_t) curr_s2;
      s1_n = ROUND_UP_NEXT_128 ((size_ea_t) curr_s1) - (size_ea_t) curr_s1;
      min = three_way_min (s2_n, s1_n, n3);

      ret = _strncmp_internal (l_s1, l_s2, min, &end_v, 1);
      /*
       * Only the first slot of end_v is set.
       */
      /* if (ret || spu_extract(spu_cmpeq(end_v, 0), 0)) { */
      /* if (ret || spu_extract(spu_gather(spu_cmpeq(end_v, 0)), 0)) { */
      if (ret || spu_extract (end_v, 0))
	/*
	 * If any NUL values were seen (end_v values of zero) we still have
	 * to return ret, as it might not be zero.
	 */
	return ret;

      curr_s2 += min;
      curr_s1 += min;
      n3 -= min;
    }
  return ret;
}

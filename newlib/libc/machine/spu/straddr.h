/*
  (C) Copyright 2008
  International Business Machines Corporation,
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
    * Neither the names of the copyright holders nor the names of their
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
  PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <spu_intrinsics.h>
#include "vec_literal.h"

/*
 * Supply the inline _straddr for use by strncpy and strncat.
*
* _straddr: search the string s, and return the address of the first byte
* containing zero.
*/
static inline char *_straddr(const char *s)
{
  unsigned int cnt, cmp, skip, mask;
  vec_uchar16 *ptr, data;

  /*
   * Compensate for unaligned strings.
   */
  ptr = (vec_uchar16 *)s; /* implicit (s & ~0xf) */
  skip = (unsigned int)(ptr) & 0xf;
  /*
   * skip the first skip bytes starting at (s & ~0xf).
   */
  mask = 0xFFFF >> skip;

  data = *ptr;
  cmp = spu_extract(spu_gather(spu_cmpeq(data, 0)), 0);
  cmp &= mask;

  cnt = spu_extract(spu_cntlz(spu_promote(cmp, 0)), 0);

  while (cnt == 32) {
    data = *++ptr;
    cnt = spu_extract(spu_cntlz(spu_gather(spu_cmpeq(data, 0))), 0);
    /*
     * The first 16 bits for gather on a byte vector are zero, so if cnt
     * is 32, none of the 16 bytes in data was zero. And, there are (cnt -
     * 16) non-zero bytes in data.
     */
  }
  /*
   * The first non-zero byte is at ptr aligned down plus the number of
   * non-zero bytes seen.
   */
  return ((char*) (((int) ptr & ~0xf) + (cnt - 16)));
}

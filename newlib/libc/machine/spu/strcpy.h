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
#include <string.h>

/*
 * Supply an inline _strncpy for strcpy/cat and strncpy/cat. Relies on
 * checklen and lastzero code being optimized out when they are constant
 * zero values.
 */
static inline void * _strncpy(char * __restrict__ dest, const char *
                              __restrict__ src, size_t maxlen, int
                              checklen, int lastzero)
{
  int adjust, offset, soffset, doffset, shift;
  vec_uchar16 *vsrc, *vdest;
  vec_uchar16 sdata1, sdata2, sdata, shuffle;
  vec_uchar16 mask1, maskzero, cmp0;
  vec_uint4 nonzeroes, gathered_cmp, vtmp, vtmp2;
  vec_uint4 curlen; /* assumes size_t is 4 bytes */
  const vec_uint4 val31 = { 31, 31, 31, 31 };
  const vec_uint4 val_0123 = { 0x00010203, 0x04050607, 0x08090A0B, 0x0C0D0E0F };
  const vec_uchar16 all_ones = { 0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,
                                 0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff };

  vsrc = (vec_uchar16 *) src;
  vdest = (vec_uchar16 *) dest;
  soffset = (int) src & 15;
  doffset = (int) dest & 15;

  if (checklen) {
    /*
     * Set curlen so it is the number of bytes we would copy if starting
     * from vdest & ~0xf.
     *
     * curlen could probably be replaced by comparing vdest plus some
     * offset to dest + maxlen, that would help mainly in the while loop
     * but would lose only one instruction (the curlen -= 16).
     */
    curlen = spu_splats((unsigned int) (maxlen + doffset));
  }

  /*
   * Setup a shuffle pattern to align the source string with the
   * alignment of the destination string.
   */
  vtmp = spu_cmpgt(spu_promote(doffset, 0), spu_promote(soffset, 0));
  adjust = spu_extract(vtmp, 0);
  offset  = soffset - doffset;
  offset += adjust & 16;
  shuffle = spu_splats((unsigned char) offset);
  shuffle = (vec_uchar16) spu_add((vec_uint4) shuffle, val_0123);

  vsrc += adjust;
  sdata1 = *vsrc++;
  sdata2 = *vsrc++;
  sdata = spu_shuffle(sdata1, sdata2, shuffle);

  /*
   * mask out leading bytes
   */
  mask1 = spu_rlmaskqwbyte(all_ones, -doffset);

  cmp0 = spu_and(mask1, spu_cmpeq(sdata, 0));
  nonzeroes = spu_cntlz(spu_gather(cmp0));
  /*
   * First element of nonzeroes - 15 is the number of leading non-zero
   * bytes plus 1 for the zero byte.
   */
  if (checklen) {
    vtmp = spu_add(curlen, 15);
    vtmp2 = spu_cmpgt(nonzeroes, vtmp);
    nonzeroes = spu_sel(nonzeroes, vtmp, vtmp2);
  }

  vtmp = spu_cmpgt(nonzeroes, val31);
  /*
   * Note: using immediate (constant 31) vs a vector value (val31) does
   * not give different results, and we have to have a vector val31 for
   * the spu_sel below, so use val31 everywhere.
   */
  vtmp = spu_sel(nonzeroes, val31, vtmp);
  /*
   * So vtmp is now min(nonzeroes, 31), the number of bytes + 16 that we
   * want to copy from the first 16 bytes of the source.
   */
  if (checklen) {
    curlen = spu_sub(vtmp, curlen);
    curlen = spu_sub(15, curlen);
  }

  /*
   * We want a right shift 0xff with fill by ones of (vtmp - 15) bytes, but
   * that doesn't exist so use spu_slqwbyte and vtmp all ones left by
   * (31 - vtmp). Note: this can also use spu_rlqwbytebc with spu_rlqw.
   */
  shift = spu_extract(spu_sub(val31, vtmp), 0);
  maskzero = spu_slqwbyte(all_ones, shift);
  maskzero = spu_and(mask1, maskzero);
  *vdest = spu_sel(*vdest, sdata, maskzero);

  vtmp = spu_cmpgt(nonzeroes, val31);
  if (checklen) {
    vtmp2 = spu_cmpgt(curlen, 0);
    vtmp = spu_and(vtmp, vtmp2);
  }
  if (spu_extract(vtmp, 0)) {
    sdata1 = sdata2;
    sdata2 = *vsrc++;
    sdata = spu_shuffle(sdata1, sdata2, shuffle);
    cmp0 = spu_cmpeq(sdata, 0);
    gathered_cmp = spu_gather(cmp0);
    /*
     * Copy 16 bytes at a time.
     */
    while ((spu_extract(gathered_cmp, 0) == 0) &&
           (!checklen || (spu_extract(curlen, 0) > 15))) {
      if (checklen)
        curlen = spu_add(curlen, -16);
      *++vdest = sdata;
      sdata1 = sdata2;
      sdata2 = *vsrc++;
      sdata = spu_shuffle(sdata1, sdata2, shuffle);
      cmp0 = spu_cmpeq(sdata, 0);
      gathered_cmp = spu_gather(cmp0);
    }
    /*
     * Copy 0 to 15 trailing bytes, either up to the smaller of curlen or
     * the number of non-zero bytes.
     */
    nonzeroes = spu_cntlz(gathered_cmp);
    if (checklen) {
      vtmp = spu_add(curlen, 15);
      vtmp2 = spu_cmpgt(nonzeroes, vtmp);
      nonzeroes = spu_sel(nonzeroes, vtmp, vtmp2);
      curlen = spu_sub(nonzeroes, curlen);
      curlen = spu_sub(15, curlen);
    }
    shift = spu_extract(spu_sub(val31, nonzeroes), 0);
    maskzero = spu_slqwbyte(all_ones, shift);
    ++vdest;
    *vdest = spu_sel(*vdest, sdata, maskzero);
  }

  if (checklen && lastzero) {
    /*
     * For strncat.
     */
    dest[maxlen - spu_extract(curlen, 0)] = '\0';
  }

  /* Pad null bytes if the length of the "src" is less than "n" (strncpy).  */
  if (checklen && !lastzero && spu_extract(curlen, 0))
    memset(dest + maxlen - spu_extract(curlen, 0), 0, spu_extract(curlen, 0));
  return (dest);
}

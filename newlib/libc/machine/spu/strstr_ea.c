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
#include "ea_internal.h"
#include <ea.h>
#include <string.h>
#include <spu_cache.h>

COMPAT_EA_ALIAS (strstr_ea);

__ea char *strstr_ea (__ea const char *s1, __ea const char *s2)
{
  __ea char *ret;
  __ea char *curr;
  size_ea_t s2_length;
  size_ea_t s1_length;
  int i;
  int found;

  ret = (__ea char *) s1;
  s2_length = strlen_ea (s2);
  s1_length = strlen_ea (s1);
  while (ret < (s1 + s1_length)) {
    /* search for first letter */
    //temporary hack for broken 64 bit compiler
    ret = strchr_ea (ret, s2[0]);
    /*if we find it search for the rest */
    if (ret) {
      found = 1;
      for (i = 1; i < s2_length; i++) {
        //temporary hack for broken 64 bit compiler
        curr = strchr_ea (ret, s2[i]);
        /* if the letter doesn't exist or isn't in the right spot we unfind */
        if (!curr || (curr != (ret + i)))
          found = 0;
      }
    }
    if (found) {
      return ret;
    } else {
      ret++;
    }
    /*go back and try again with the rest of it */
  }
  return NULL;
}

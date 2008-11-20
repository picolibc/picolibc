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

static __ea char *
similie_ls_to_ea (char *l1, __ea char *ea1, char *l2)
{
  return (__ea char *) ((size_ea_t) ea1 +
			((size_ea_t) (int) l2 - (size_ea_t) (int) l1));
}

COMPAT_EA_ALIAS (strrchr_ea);

__ea char *
strrchr_ea (__ea const char *s, int c)
{
  size_ea_t string_length;
  char *curr_ptr;
  __ea char *string_start_local;
  __ea char *end_of_string_ea;
  __ea char *start_of_cacheline_ea;
  char *start_of_cachline_local;

  string_start_local = __cache_fetch ((__ea char *) s);
  string_length = strlen_ea (s);
  end_of_string_ea = (__ea char *) s + string_length;

  start_of_cacheline_ea = round_down_128_ea (end_of_string_ea - 1);
  start_of_cachline_local = __cache_fetch (start_of_cacheline_ea);
  /*this next line should be the same cacheline, just the end of the string */
  curr_ptr = __cache_fetch (end_of_string_ea - 1);

  while (1)
    {
      /*search backwards through this cachline */
      while (curr_ptr >= start_of_cachline_local)
	{
	  if (*curr_ptr == (char) c)
	    return similie_ls_to_ea (start_of_cachline_local,
				     start_of_cacheline_ea, curr_ptr);
	  curr_ptr--;
	  if (curr_ptr < string_start_local)
	    return NULL;
	}

      /* iterate cacheline backwards */
      start_of_cacheline_ea -= 128;
      start_of_cachline_local = __cache_fetch (start_of_cacheline_ea);
      curr_ptr = __cache_fetch (start_of_cacheline_ea + 128);
    }
}

/* Copyright (c) 2002,2004,2005 Joerg Wunsch
   Copyright (c) 2008  Dmitry Xmelkov
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.

   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

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

#include "dtoa_engine.h"

double
__atod_engine(uint64_t u64, int exp)
{
    double flt;
    const double *f;
    flt = u64;
    int exp_cur;

    if (exp < 0) {
	f = __dtoa_scale_down + DTOA_SCALE_DOWN_NUM - 1;
	exp = -exp;
	exp_cur = 1 << (DTOA_SCALE_DOWN_NUM - 1);
    } else {
	f = __dtoa_scale_up + DTOA_SCALE_UP_NUM - 1;
	exp_cur = 1 << (DTOA_SCALE_UP_NUM - 1);
    }

    while (exp) {
	if (exp >= exp_cur) {
	    flt *= *f;
	    exp -= exp_cur;
	}
	f--;
	exp_cur >>= 1;
    }
    return flt;
}

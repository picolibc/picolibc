/* Copyright © 2018, Keith Packard
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
  POSSIBILITY OF SUCH DAMAGE. */

#ifndef	_LDTOA_ENGINE_H_
#define	_LDTOA_ENGINE_H_

#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include <stdbool.h>

#if __LDBL_MANT_DIG__ == 113
#define LDTOA_MAX_DIG    34
#elif __LDBL_MANT_DIG__ == 106
#define LDTOA_MAX_DIG    32
#elif __LDBL_MANT_DIG__ == 64
#define LDTOA_MAX_DIG    20
#endif

struct ldtoa {
	int32_t	exp;
	uint8_t	flags;
	char	digits[LDTOA_MAX_DIG + 1];
};

int
__ldtoa_engine(long double x, struct ldtoa *dtoa, int max_digits, bool fmode, int max_decimals);

/* '__ldtoa_engine' flags return value */
#define	LDTOA_MINUS	1
#define	LDTOA_ZERO	2
#define	LDTOA_INF	4
#define	LDTOA_NAN	8
#define	LDTOA_CARRY	16	/* Carry was to most significant position. */

#endif	/* !_LDTOA_ENGINE_H_ */

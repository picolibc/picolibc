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

#ifndef	_DTOA_H_
#define	_DTOA_H_

#include "stdio_private.h"
#include "../../libm/common/math_config.h"

#define	DTOA_MINUS	1
#define	DTOA_ZERO	2
#define	DTOA_INF	4
#define	DTOA_NAN	8
#define	DTOA_CARRY	16	/* Carry was to most significant position. */

#if __LDBL_MANT_DIG__ == 113
#define LDTOA_MAX_DIG    34
#elif __LDBL_MANT_DIG__ == 106
#define LDTOA_MAX_DIG    32
#elif __LDBL_MANT_DIG__ == 64
#define LDTOA_MAX_DIG    20
#endif

#define DTOA_MAX_DIG            17
#define DTOA_MAX_10_EXP         308
#define DTOA_MIN_10_EXP         (-307)
#define DTOA_SCALE_UP_NUM       9
#define DTOA_ROUND_NUM          (DTOA_MAX_DIG + 1)
#define DTOA_MAX_EXP            1024

#define FTOA_MAX_10_EXP         38
#define FTOA_MIN_10_EXP         (-37)
#define FTOA_MAX_DIG	        9
#define FTOA_SCALE_UP_NUM       6
#define FTOA_ROUND_NUM          (FTOA_MAX_DIG + 1)

#ifdef _NEED_IO_LONG_DOUBLE
# if __SIZEOF_LONG_DOUBLE__ == 4
#  define _NEED_IO_FLOAT32
#  define LONG_FLOAT_MAX_DIG    FTOA_MAX_DIG
#  define __lfloat_d_engine     __ftoa_engine
#  define __lfloat_x_engine     __ftox_engine
# elif __SIZEOF_LONG_DOUBLE__ == 8
#  define _NEED_IO_FLOAT64
#  define LONG_FLOAT_MAX_DIG    DTOA_MAX_DIG
#  define __lfloat_d_engine     __dtoa_engine
#  define __lfloat_x_engine     __dtox_engine
# elif __SIZEOF_LONG_DOUBLE__ > 8
#  define _NEED_IO_FLOAT_LARGE
#  define LONG_FLOAT_MAX_DIG    LDTOA_MAX_DIG
#  define __lfloat_d_engine     __ldtoa_engine
#  define __lfloat_x_engine     __ldtox_engine
# endif
#endif

#ifdef _NEED_IO_DOUBLE
# if __SIZEOF_DOUBLE__ == 4
#  define _NEED_IO_FLOAT32
#  define FLOAT_MAX_DIG         FTOA_MAX_DIG
#  define __float_d_engine      __ftoa_engine
#  define __float_x_engine      __ftox_engine
# elif __SIZEOF_DOUBLE__ == 8
#  define _NEED_IO_FLOAT64
#  define FLOAT_MAX_DIG         DTOA_MAX_DIG
#  define __float_d_engine      __dtoa_engine
#  define __float_x_engine      __dtox_engine
# endif
# define PRINTF_FLOAT_ARG(ap)   (va_arg(ap, double))
#endif

#ifdef _NEED_IO_FLOAT
# define _NEED_IO_FLOAT32
# define PRINTF_FLOAT_ARG(ap) (asfloat(va_arg(ap, uint32_t)))
# define FLOAT_MAX_DIG   FTOA_MAX_DIG
# define __float_d_engine __ftoa_engine
# define __float_x_engine __ftox_engine
#endif

#ifdef _NEED_IO_FLOAT_LARGE
#define DTOA_DIGITS     LDTOA_MAX_DIG
#elif defined(_NEED_IO_FLOAT64)
#define DTOA_DIGITS     DTOA_MAX_DIG
#elif defined(_NEED_IO_FLOAT32)
#define DTOA_DIGITS     FTOA_MAX_DIG
#else
#error No float requirement set
#endif

struct dtoa {
    int32_t     exp;
    uint8_t     flags;
    char        digits[DTOA_DIGITS];
};

#ifdef _NEED_IO_FLOAT_LARGE
int
__ldtoa_engine(long double x, struct dtoa *dtoa, int max_digits, bool fmode, int max_decimals);

int
__ldtox_engine(long double x, struct dtoa *dtoa, int prec, unsigned char case_convert);

long double
__atold_engine(_u128 m10, int e10);
#endif

#ifdef _NEED_IO_FLOAT64
int
__dtoa_engine(FLOAT64 x, struct dtoa *dtoa, int max_digits, bool fmode, int max_decimals);

int
__dtox_engine(FLOAT64 x, struct dtoa *dtoa, int prec, unsigned char case_convert);

FLOAT64
__atod_engine(uint64_t m10, int e10);
#endif

#ifdef _NEED_IO_FLOAT32
int __ftoa_engine (float val, struct dtoa *ftoa, int max_digits, bool fmode, int max_decimals);

int
__ftox_engine(float x, struct dtoa *dtoa, int prec, unsigned char case_convert);

float
__atof_engine(uint32_t m10, int e10);
#endif

#endif	/* !_DTOA_H_ */

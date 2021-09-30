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

#ifndef	_DTOA_ENGINE_H_
#define	_DTOA_ENGINE_H_

#include <stdlib.h>
#include <stdint.h>
#include <float.h>

#ifndef DBL_MAX_10_EXP
#error DBL_MAX_10_EXP
#endif

#ifndef DBL_MIN_10_EXP
#error DBL_MIN_10_EXP
#endif

#ifndef DBL_DIG
#error DBL_DIG
#endif

#ifdef PICOLIBC_FLOAT_PRINTF_SCANF

#define DTOA_MAX_10_EXP FLT_MAX_10_EXP
#define DTOA_MIN_10_EXP FLT_MIN_10_EXP
#define DTOA_DIG	9
#define DTOA_MAX_DIG 	9

#define __dtoa_scale_up __ftoa_scale_up
#define __dtoa_scale_down __ftoa_scale_down
#define __dtoa_round __ftoa_round
#define __atod_engine __atof_engine
#define FLOAT float
#define UINTFLOAT uint32_t

#else

#define DTOA_MAX_10_EXP DBL_MAX_10_EXP
#define DTOA_MIN_10_EXP DBL_MIN_10_EXP
#define DTOA_DIG 	17
#define DTOA_MAX_DIG	17
#define FLOAT double
#define UINTFLOAT uint64_t

#endif

#define	DTOA_MINUS	1
#define	DTOA_ZERO	2
#define	DTOA_INF	4
#define	DTOA_NAN	8
#define	DTOA_CARRY	16	/* Carry was to most significant position. */

struct dtoa {
	int32_t	exp;
	uint8_t	flags;
	char	digits[DTOA_MAX_DIG + 1];
};

int
__dtoa_engine(FLOAT x, struct dtoa *dtoa, int max_digits, int max_decimals);

extern NEWLIB_THREAD_LOCAL char __ecvt_buf[DTOA_MAX_DIG + 1];

extern const FLOAT __dtoa_scale_up[];
extern const FLOAT __dtoa_scale_down[];
extern const FLOAT __dtoa_round[];

#if DTOA_MAX_10_EXP >= 1 && DTOA_MAX_10_EXP < 2
#define DTOA_SCALE_UP_NUM 1
#endif
#if DTOA_MAX_10_EXP >= 2 && DTOA_MAX_10_EXP < 4
#define DTOA_SCALE_UP_NUM 2
#endif
#if DTOA_MAX_10_EXP >= 4 && DTOA_MAX_10_EXP < 8
#define DTOA_SCALE_UP_NUM 3
#endif
#if DTOA_MAX_10_EXP >= 8 && DTOA_MAX_10_EXP < 16
#define DTOA_SCALE_UP_NUM 4
#endif
#if DTOA_MAX_10_EXP >= 16 && DTOA_MAX_10_EXP < 32
#define DTOA_SCALE_UP_NUM 5
#endif
#if DTOA_MAX_10_EXP >= 32 && DTOA_MAX_10_EXP < 64
#define DTOA_SCALE_UP_NUM 6
#endif
#if DTOA_MAX_10_EXP >= 64 && DTOA_MAX_10_EXP < 128
#define DTOA_SCALE_UP_NUM 7
#endif
#if DTOA_MAX_10_EXP >= 128 && DTOA_MAX_10_EXP < 256
#define DTOA_SCALE_UP_NUM 8
#endif
#if DTOA_MAX_10_EXP >= 256 && DTOA_MAX_10_EXP < 512
#define DTOA_SCALE_UP_NUM 9
#endif
#if DTOA_MAX_10_EXP >= 512 && DTOA_MAX_10_EXP < 1024
#define DTOA_SCALE_UP_NUM 10
#endif
#if DTOA_MAX_10_EXP >= 1024 && DTOA_MAX_10_EXP < 2048
#define DTOA_SCALE_UP_NUM 11
#endif
#if DTOA_MAX_10_EXP >= 2048 && DTOA_MAX_10_EXP < 4096
#define DTOA_SCALE_UP_NUM 12
#endif
#if DTOA_MAX_10_EXP >= 4096 && DTOA_MAX_10_EXP < 8192
#define DTOA_SCALE_UP_NUM 13
#endif
#if DTOA_MAX_10_EXP >= 8192 && DTOA_MAX_10_EXP < 16384
#define DTOA_SCALE_UP_NUM 14
#endif
#if DTOA_MAX_10_EXP >= 16384 && DTOA_MAX_10_EXP < 32768
#define DTOA_SCALE_UP_NUM 15
#endif
#if DTOA_MAX_10_EXP >= 32768 && DTOA_MAX_10_EXP < 65536
#define DTOA_SCALE_UP_NUM 16
#endif
#if DTOA_MAX_10_EXP >= 65536 && DTOA_MAX_10_EXP < 131072
#define DTOA_SCALE_UP_NUM 17
#endif
#if DTOA_MIN_10_EXP <= -1 && DTOA_MIN_10_EXP > -2
#define DTOA_SCALE_DOWN_NUM 1
#endif
#if DTOA_MIN_10_EXP <= -2 && DTOA_MIN_10_EXP > -4
#define DTOA_SCALE_DOWN_NUM 2
#endif
#if DTOA_MIN_10_EXP <= -4 && DTOA_MIN_10_EXP > -8
#define DTOA_SCALE_DOWN_NUM 3
#endif
#if DTOA_MIN_10_EXP <= -8 && DTOA_MIN_10_EXP > -16
#define DTOA_SCALE_DOWN_NUM 4
#endif
#if DTOA_MIN_10_EXP <= -16 && DTOA_MIN_10_EXP > -32
#define DTOA_SCALE_DOWN_NUM 5
#endif
#if DTOA_MIN_10_EXP <= -32 && DTOA_MIN_10_EXP > -64
#define DTOA_SCALE_DOWN_NUM 6
#endif
#if DTOA_MIN_10_EXP <= -64 && DTOA_MIN_10_EXP > -128
#define DTOA_SCALE_DOWN_NUM 7
#endif
#if DTOA_MIN_10_EXP <= -128 && DTOA_MIN_10_EXP > -256
#define DTOA_SCALE_DOWN_NUM 8
#endif
#if DTOA_MIN_10_EXP <= -256 && DTOA_MIN_10_EXP > -512
#define DTOA_SCALE_DOWN_NUM 9
#endif
#if DTOA_MIN_10_EXP <= -512 && DTOA_MIN_10_EXP > -1024
#define DTOA_SCALE_DOWN_NUM 10
#endif
#if DTOA_MIN_10_EXP <= -1024 && DTOA_MIN_10_EXP > -2048
#define DTOA_SCALE_DOWN_NUM 11
#endif
#if DTOA_MIN_10_EXP <= -2048 && DTOA_MIN_10_EXP > -4096
#define DTOA_SCALE_DOWN_NUM 12
#endif
#if DTOA_MIN_10_EXP <= -4096 && DTOA_MIN_10_EXP > -8192
#define DTOA_SCALE_DOWN_NUM 13
#endif
#if DTOA_MIN_10_EXP <= -8192 && DTOA_MIN_10_EXP > -16384
#define DTOA_SCALE_DOWN_NUM 14
#endif
#if DTOA_MIN_10_EXP <= -16384 && DTOA_MIN_10_EXP > -32768
#define DTOA_SCALE_DOWN_NUM 15
#endif
#if DTOA_MIN_10_EXP <= -32768 && DTOA_MIN_10_EXP > -65536
#define DTOA_SCALE_DOWN_NUM 16
#endif
#if DTOA_MIN_10_EXP <= -65536 && DTOA_MIN_10_EXP > -131072
#define DTOA_SCALE_DOWN_NUM 17
#endif

#define DTOA_ROUND_NUM (DTOA_DIG + 1)

#endif	/* !_DTOA_ENGINE_H_ */

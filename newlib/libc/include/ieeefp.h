/*
Copyright (c) 1991, 1993
The Regents of the University of California.  All rights reserved.
c) UNIX System Laboratories, Inc.
All or some portions of this file are derived from material licensed
to the University of California by American Telephone and Telegraph
Co. or Unix System Laboratories, Inc. and are reproduced herein with
the permission of UNIX System Laboratories, Inc.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. Neither the name of the University nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
 */
#ifndef _IEEE_FP_H_
#define _IEEE_FP_H_

#include "_ansi.h"

#include <machine/ieeefp.h>
#include <float.h>
#include <stdint.h>

_BEGIN_STD_C

#ifndef _LDBL_EQ_DBL

#ifndef LDBL_MANT_DIG
#error "LDBL_MANT_DIG not defined - should be found in float.h"

#elif LDBL_MANT_DIG == DBL_MANT_DIG
#error "double and long double are the same size but LDBL_EQ_DBL is not defined"

#elif LDBL_MANT_DIG == 53
/* This happens when doubles are 32-bits and long doubles are 64-bits.  */
#define	EXT_EXPBITS	11
#define EXT_FRACHBITS	20
#define	EXT_FRACLBITS	32
#define __ieee_ext_field_type unsigned long

#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 65) && LDBL_MAX_EXP == 16384
/* 80-bit floats, as on x86 */
#define	EXT_EXPBITS	15
#define EXT_FRACHBITS	32
#define	EXT_FRACLBITS	32
#ifdef __IEEE_BIG_ENDIAN
#define EXT_FRACGAP     16
#endif
#define __ieee_ext_field_type unsigned int

#elif (LDBL_MANT_DIG == 113 || LDBL_MANT_DIG == 112) && LDBL_MAX_EXP == 16384
/* 128-bit IEEE floats, as on risc-v */
#define	EXT_EXPBITS	15
#define EXT_FRACHBITS	48
#define	EXT_FRACLBITS	64
#define __ieee_ext_field_type unsigned long long

#elif LDBL_MANT_DIG == 106 && DBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
/* 128-bit double-double, as on powerpc */
#define EXT_EXPBITS     11
#define EXT_FRACHBITS   DBL_MANT_DIG
#define EXT_FRACLBITS   DBL_MANT_DIG
#define EXT_FRACGAP     EXT_EXPBITS
#define __ieee_ext_field_type unsigned long long

#else
#error Unsupported value for LDBL_MANT_DIG
#endif

#define	EXT_EXP_INFNAN	   ((1 << EXT_EXPBITS) - 1) /* 32767 */
#define	EXT_EXP_BIAS	   ((1 << (EXT_EXPBITS - 1)) - 1) /* 16383 */
#define	EXT_FRACBITS	   (EXT_FRACLBITS + EXT_FRACHBITS)

typedef struct ieee_ext
{
#ifdef __IEEE_LITTLE_ENDIAN
  __ieee_ext_field_type	 ext_fracl : EXT_FRACLBITS;
#ifdef EXT_FRACGAP
  __ieee_ext_field_type  ext_gap   : EXT_FRACGAP;
#endif
  __ieee_ext_field_type	 ext_frach : EXT_FRACHBITS;
  __ieee_ext_field_type	 ext_exp   : EXT_EXPBITS;
  __ieee_ext_field_type	 ext_sign  : 1;
#endif
#ifdef __IEEE_BIG_ENDIAN
  __ieee_ext_field_type	 ext_sign  : 1;
  __ieee_ext_field_type	 ext_exp   : EXT_EXPBITS;
#ifdef EXT_FRACGAP
  __ieee_ext_field_type  ext_gap   : EXT_FRACGAP;
#endif
  __ieee_ext_field_type	 ext_frach : EXT_FRACHBITS;
  __ieee_ext_field_type	 ext_fracl : EXT_FRACLBITS;
#endif
} ieee_ext;

typedef union ieee_ext_u
{
  long double		extu_ld;
  struct ieee_ext	extu_ext;
} ieee_ext_u;

#endif /* ! _LDBL_EQ_DBL */


/* FLOATING ROUNDING */

typedef int fp_rnd;
#define FP_RN 0 	/* Round to nearest 		*/
#define FP_RM 1		/* Round down 			*/
#define FP_RP 2		/* Round up 			*/
#define FP_RZ 3		/* Round to zero (trunate) 	*/

fp_rnd fpgetround (void);
fp_rnd fpsetround (fp_rnd);

/* EXCEPTIONS */

typedef int fp_except;
#define FP_X_INV 0x10	/* Invalid operation 		*/
#define FP_X_DX  0x80	/* Divide by zero		*/
#define FP_X_OFL 0x04	/* Overflow exception		*/
#define FP_X_UFL 0x02	/* Underflow exception		*/
#define FP_X_IMP 0x01	/* imprecise exception		*/

fp_except fpgetmask (void);
fp_except fpsetmask (fp_except);
fp_except fpgetsticky (void);
fp_except fpsetsticky (fp_except);

/* INTEGER ROUNDING */

typedef int fp_rdi;
#define FP_RDI_TOZ 0	/* Round to Zero 		*/
#define FP_RDI_RD  1	/* Follow float mode		*/

fp_rdi fpgetroundtoi (void);
fp_rdi fpsetroundtoi (fp_rdi);

#define __IEEE_DBL_EXPBIAS 1023
#define __IEEE_FLT_EXPBIAS 127

#define __IEEE_DBL_EXPLEN 11
#define __IEEE_FLT_EXPLEN 8


#define __IEEE_DBL_FRACLEN (64 - (__IEEE_DBL_EXPLEN + 1))
#define __IEEE_FLT_FRACLEN (32 - (__IEEE_FLT_EXPLEN + 1))

#define __IEEE_DBL_MAXPOWTWO	((double)(1L << 32 - 2) * (1L << (32-11) - 32 + 1))
#define __IEEE_FLT_MAXPOWTWO	((float)(1L << (32-8) - 1))

#define __IEEE_DBL_NAN_EXP 0x7ff
#define __IEEE_FLT_NAN_EXP 0xff

#ifdef __ieeefp_isnanf
#define isnanf(x)	__ieeefp_isnanf(x)
#endif

#ifdef __ieeefp_isinff
#define isinff(x)	__ieeefp_isinff(x)
#endif

#ifdef __ieeefp_finitef
#define finitef(x)	__ieeefp_finitef(x)
#endif

#ifdef _DOUBLE_IS_32BITS
#undef __IEEE_DBL_EXPBIAS
#define __IEEE_DBL_EXPBIAS __IEEE_FLT_EXPBIAS

#undef __IEEE_DBL_EXPLEN
#define __IEEE_DBL_EXPLEN __IEEE_FLT_EXPLEN

#undef __IEEE_DBL_FRACLEN
#define __IEEE_DBL_FRACLEN __IEEE_FLT_FRACLEN

#undef __IEEE_DBL_MAXPOWTWO
#define __IEEE_DBL_MAXPOWTWO __IEEE_FLT_MAXPOWTWO

#undef __IEEE_DBL_NAN_EXP
#define __IEEE_DBL_NAN_EXP __IEEE_FLT_NAN_EXP

#undef __ieee_double_shape_type
#define __ieee_double_shape_type __ieee_float_shape_type

#endif /* _DOUBLE_IS_32BITS */

_END_STD_C

#endif /* _IEEE_FP_H_ */

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

static const char pstr_nfinity[] = "nfinity";
static const char pstr_an[] = "an";

#if defined(STRTOD) || defined(STRTOF) || defined(STRTOLD)
# define CHECK_WIDTH()   1
# define CHECK_RANGE(flt) do {                                          \
        int __class = fpclassify(flt);                                  \
        if (__class == FP_INFINITE || __class == FP_SUBNORMAL || __class == FP_ZERO) \
            errno = ERANGE;                                             \
    } while (0);
# ifdef _WANT_IO_C99_FORMATS
#  define _NEED_IO_C99_FORMATS
# endif
# if defined(STRTOD)
#  define _NEED_IO_DOUBLE
#  define SCANF_LEVEL           SCANF_DBL
#  define CHECK_LONG()          1
#  define CHECK_LONG_LONG()     0
# elif defined(STRTOLD)
#  define _NEED_IO_LONG_DOUBLE
#  define SCANF_LEVEL           SCANF_DBL
#  define CHECK_LONG()          0
#  define CHECK_LONG_LONG()     1
# elif defined(STRTOF)
#  define _NEED_IO_FLOAT
#  define SCANF_LEVEL           SCANF_FLT
#  define CHECK_LONG()          0
#  define CHECK_LONG_LONG()     0
# endif

#define FLT_STREAM const char

static inline int scanf_getc(const char *s, int *lenp)
{
    int l = *lenp;
    int c = s[l];
    *lenp = l + 1;
    return c;
}

static inline void scanf_ungetc(int c, const char *s, int *lenp)
{
    (void) c;
    (void) s;
    *lenp = *lenp - 1;
}

#undef EOF
#define EOF -1

#else

# define CHECK_WIDTH()   (--width != 0)
# define CHECK_RANGE(flt)

# ifdef _NEED_IO_FLOAT

#  define CHECK_LONG()          0
#  define CHECK_LONG_LONG()     0

# elif defined(_NEED_IO_DOUBLE)

#  ifdef _NEED_IO_LONG_DOUBLE
#   define CHECK_LONG()         (flags & FL_LONG)
#   define CHECK_LONG_LONG()    (flags & FL_LONGLONG)
#  else
#   define CHECK_LONG()         (flags & (FL_LONG|FL_LONGLONG))
#   define CHECK_LONG_LONG()    0
#  endif
# else
#  define CHECK_LONG()          0
#  define CHECK_LONG_LONG()     0
# endif
#endif

#include "scanf_private.h"

#include "dtoa.h"

#ifdef _NEED_IO_LONG_DOUBLE
# define FLOAT                  long double
# define FLOAT_MIN              __LDBL_MIN__
# define FLOAT_MANT_DIG         __LDBL_MANT_DIG__
# define FLOAT_MAX_EXP          __LDBL_MAX_EXP__
# define FLOAT_MIN_EXP          __LDBL_MIN_EXP__
# define ASFLOAT(x)             aslongdouble(x)
# if __SIZEOF_LONG_DOUBLE__ > 8
#  define TOFLOAT(x)            _u128_to_ld(x)
#  define UINTFLOAT             _u128
# elif __SIZEOF_LONG_DOUBLE__ == 8
#  define TOFLOAT(x)            ((long double) (x))
#  define UINTFLOAT             uint64_t
# elif __SIZEOF_LONG_DOUBLE__ == 4
#  define TOFLOAT(x)            ((long double) (x))
#  define UINTFLOAT             uint32_t
# endif
#elif defined(_NEED_IO_DOUBLE)
# define FLOAT                  double
# define FLOAT_MIN              __DBL_MIN__
# define FLOAT_MANT_DIG         __DBL_MANT_DIG__
# define FLOAT_MAX_EXP          __DBL_MAX_EXP__
# define FLOAT_MIN_EXP          __DBL_MIN_EXP__
# define ASFLOAT(x)             _asdouble(x)
# define TOFLOAT(x)             ((double) (x))
# if __SIZEOF_DOUBLE__ == 4
#  define UINTFLOAT             uint32_t
# elif __SIZEOF_DOUBLE__ == 8
#  define UINTFLOAT             uint64_t
# endif
#elif defined(_NEED_IO_FLOAT)
# define FLOAT                  float
# define FLOAT_MIN              __FLT_MIN__
# define FLOAT_MANT_DIG         __FLT_MANT_DIG__
# define FLOAT_MAX_EXP          __FLT_MAX_EXP__
# define FLOAT_MIN_EXP          __FLT_MIN_EXP__
# define ASFLOAT(x)             _asfloat(x)
# define TOFLOAT(x)             ((float) (x))
# define UINTFLOAT              uint32_t
#endif

#if FLOAT_MANT_DIG > 53
#define U32_TO_UF(x)    to_u128(x)
#define U64_TO_UF(x)    to_u128(x)
#define U128_TO_UF(x)   (x)
#define UF_TO_U32(x)    from_u128(x)
#define UF_TO_U64(x)    from_u128(x)
#define UF_TO_U128(x)   (x)
#define UF_IS_ZERO(x)   _u128_is_zero(x)
#define UF_TIMES_BASE(a,b)      _u128_times_base(a,b)
#define UF_PLUS_DIGIT(a,b)      _u128_plus_64(a,b)
#define UF_RSHIFT(a,b)          _u128_rshift(a,b)
#define UF_LSHIFT(a,b)          _u128_lshift(a,b)
#define UF_LSHIFT_64(a,b)       _u128_lshift_64(a,b)
#define UF_LT(a,b)              _u128_lt(a,b)
#define UF_GE(a,b)              _u128_ge(a,b)
#define UF_OR_64(a,b)           _u128_or_64(a,b)
#define UF_AND_64(a,b)          _u128_and_64(a,b)
#define UF_AND(a,b)             _u128_and(a,b)
#define UF_NOT(a)               _u128_not(a)
#define UF_OR(a,b)              _u128_or(a,b)
#else
#define U32_TO_UF(x)    (x)
#define U64_TO_UF(x)    (x)
#define U128_TO_UF(x)   from_u128(x)
#define UF_TO_U32(x)    (x)
#define UF_TO_U64(x)    (x)
#define UF_TO_U128(x)   to_u128(x)
#define UF_IS_ZERO(x)   ((x) == 0)
#define UF_TIMES_BASE(a,b)      ((a) * (b))
#define UF_PLUS_DIGIT(a,b)      ((a) + (b))
#define UF_RSHIFT(a,b)          ((a) >> (b))
#define UF_LSHIFT(a,b)          ((a) << (b))
#define UF_LSHIFT_64(a,b)       ((UINTFLOAT) (a) << (b))
#define UF_LT(a,b)              ((a) < (b))
#define UF_GE(a,b)              ((a) >= (b))
#define UF_OR_64(a,b)           ((a) | (b))
#define UF_AND_64(a,b)          ((a) & (b))
#define UF_AND(a,b)             ((a) & (b))
#define UF_NOT(a)               (~(a))
#define UF_OR(a,b)              ((a) | (b))
#endif

static unsigned char
conv_flt (FLT_STREAM *stream, int *lenp, width_t width, void *addr, uint16_t flags)
{
    UINTFLOAT uint;
    unsigned int overflow = 0;
    int uintdigits = 0;
    FLOAT flt;
    int i;
    const char *p;
    int exp;

    i = scanf_getc (stream, lenp);		/* after scanf_ungetc()	*/

    switch ((unsigned char)i) {
    case '-':
        flags |= FL_MINUS;
	__PICOLIBC_FALLTHROUGH;
    case '+':
	if (!CHECK_WIDTH() || (i = scanf_getc (stream, lenp)) < 0)
	    return 0;
    }

    switch (TOLOWER (i)) {
    case 'n':
	p = pstr_an;
        flt = (FLOAT) NAN;
        goto operate_pstr;

    case 'i':
	p = pstr_nfinity;
        flt = (FLOAT) INFINITY;
    operate_pstr:
        {
	    unsigned char c;

	    while ((c = *p++) != 0) {
		if (CHECK_WIDTH()) {
                    if ((i = scanf_getc (stream, lenp)) >= 0) {
                        if (TOLOWER(i) == c)
                            continue;
                        scanf_ungetc (i, stream, lenp);
                    }
		    if (p == pstr_nfinity + 3)
			break;
		    return 0;
		}
	    }
        }
	break;

    default:
        exp = 0;
	uint = U32_TO_UF(0);
#define uintdigitsmax_10_float  8
#define uintdigitsmax_16_float  7
#if __SIZEOF_DOUBLE__ == 8
#define uintdigitsmax_10_double 16
#define uintdigitsmax_16_double 15
#elif __SIZEOF_DOUBLE__ ==4
#define uintdigitsmax_10_double 8
#define uintdigitsmax_16_double 7
#endif
#if __SIZEOF_LONG_DOUBLE__ > 8
#define uintdigitsmax_10_long_double    32
#define uintdigitsmax_16_long_double    30
#elif __SIZEOF_LONG_DOUBLE__ == 8
#define uintdigitsmax_10_long_double    16
#define uintdigitsmax_16_long_double    15
#elif __SIZEOF_LONG_DOUBLE__ == 4
#define uintdigitsmax_10_long_double    8
#define uintdigitsmax_16_long_double    7
#endif

#ifdef _NEED_IO_C99_FORMATS
        int base = 10;
#else
#define base 10
#endif

#define uintdigitsmax                                                   \
        ((base) == 10 ?                                                 \
            (CHECK_LONG_LONG() ? uintdigitsmax_10_long_double :         \
             (CHECK_LONG() ?                                            \
              uintdigitsmax_10_double :                                 \
              uintdigitsmax_10_float)) :                                \
            (CHECK_LONG_LONG() ? uintdigitsmax_16_long_double :         \
             (CHECK_LONG() ?                                            \
              uintdigitsmax_16_double :                                 \
              uintdigitsmax_16_float)))

	do {

	    unsigned char c;

#ifdef _NEED_IO_C99_FORMATS
            if ((flags & FL_FHEX) && (c = TOLOWER(i) - 'a') <= 5)
                c += 10;
            else
#endif
                c = i - '0';

	    if (c < base) {
		flags |= FL_ANY;
		if (flags & FL_OVFL) {
                    overflow |= (c != 0);
		    if (!(flags & FL_DOT))
			exp += 1;
		} else {
		    if (flags & FL_DOT)
			exp -= 1;
                    uint = UF_PLUS_DIGIT(UF_TIMES_BASE(uint, base), c);
		    if (!UF_IS_ZERO(uint)) {
			uintdigits++;
                        if (uintdigits > uintdigitsmax)
                            flags |= FL_OVFL;
		    }
	        }

	    } else if (c == (('.'-'0') & 0xff) && !(flags & FL_DOT)) {
		flags |= FL_DOT;
#ifdef _NEED_IO_C99_FORMATS
            } else if (TOLOWER(i) == 'x' && (flags & FL_ANY) && UF_IS_ZERO(uint)) {
                flags |= FL_FHEX;
                base = 16;
#endif
	    } else {
		break;
	    }
	} while (CHECK_WIDTH() && (i = scanf_getc (stream, lenp)) >= 0);

	if (!(flags & FL_ANY))
	    return 0;

#ifdef _NEED_IO_C99_FORMATS
        int exp_match = (flags & FL_FHEX) ? 'p' : 'e';
#else
#define exp_match 'e'
#endif
	if (TOLOWER(i) == exp_match)
	{
            int esign, edig;
	    int expacc;

	    if (!CHECK_WIDTH())
                goto no_exp;

            esign = scanf_getc (stream, lenp);

	    switch ((unsigned char)esign) {
            case '-':
		flags |= FL_MEXP;
		__PICOLIBC_FALLTHROUGH;
            case '+':
                if (!CHECK_WIDTH()) {
                    scanf_ungetc(esign, stream, lenp);
                    goto no_exp;
                }
		edig = scanf_getc (stream, lenp);		/* test EOF will below	*/
                break;
            default:
                edig = esign;
                esign = EOF;
                break;
	    }

	    if (!isdigit (edig))
            {
                scanf_ungetc(edig, stream, lenp);
                if (esign != EOF)
                    scanf_ungetc(esign, stream, lenp);
                goto no_exp;
            }

            i = edig;
	    expacc = 0;
#define MAX_POSSIBLE_EXP        (FLOAT_MAX_EXP + FLOAT_MANT_DIG * 4)
	    do {
                if (expacc < MAX_POSSIBLE_EXP)
                    expacc = expacc * 10 + (i - '0');
	    } while (CHECK_WIDTH() && isdigit (i = scanf_getc(stream, lenp)));
	    if (flags & FL_MEXP)
		expacc = -expacc;
#ifdef _NEED_IO_C99_FORMATS
            if (flags & FL_FHEX)
                exp *= 4;
#endif
            exp += expacc;
	}

    no_exp:
	if (width)
            scanf_ungetc (i, stream, lenp);

	if (UF_IS_ZERO(uint)) {
	    flt = (FLOAT) 0;
	    break;
	}

#ifdef _NEED_IO_C99_FORMATS
        if (flags & FL_FHEX)
        {
            int sig_bits;
            int min_exp;

            if (CHECK_LONG_LONG()) {
                sig_bits = __LDBL_MANT_DIG__;
                min_exp = __LDBL_MIN_EXP__ - 1;
            } else if (CHECK_LONG()) {
                sig_bits = __DBL_MANT_DIG__;
                min_exp = __DBL_MIN_EXP__ - 1;
            } else {
                sig_bits = __FLT_MANT_DIG__;
                min_exp = __FLT_MIN_EXP__ - 1;
            }

            /* We're using two guard bits, one for the
             * 'half' value and the second to indicate whether
             * there is any non-zero value beyond that
             */
            exp += (sig_bits + 2 - 1);

            /* Make significand larger than 1.0 */
            while (UF_LT(uint, UF_LSHIFT_64(1, (sig_bits + 2 - 1)))) {
                uint = UF_LSHIFT(uint, 1);
                exp--;
            }

            /* Now shift right until the exponent is in range and the
             * value is less than 2.0. Capture any value > 0.5 in the
             * LSB of uint.  This may generate a denorm.
             */
            while (UF_GE(uint, UF_LSHIFT_64(1, (sig_bits + 2))) || exp < min_exp) {
                /* squash extra bits into  the second guard bit */
                uint = UF_OR_64(UF_RSHIFT(uint, 1), UF_AND_64(uint, 1));
                exp++;
            }

            /* Mix in the overflow bit computed while scanning the
             * string
             */
            uint = UF_OR_64(uint, overflow);

            /* Round even */
            if (UF_AND_64(uint, 3) == 3 || UF_AND_64(uint, 6) == 6) {
                uint = UF_PLUS_DIGIT(uint, 4);
                if (UF_GE(uint, UF_LSHIFT_64(1, FLOAT_MANT_DIG + 2))) {
                    uint = UF_RSHIFT(uint, 1);
                    exp++;
                }
            }

            /* remove guard bits */
            uint = UF_RSHIFT(uint, 2);

            /* align from target to FLOAT */
            uint = UF_LSHIFT(uint, FLOAT_MANT_DIG - sig_bits);

            if (min_exp != (FLOAT_MIN_EXP - 1)) {

                /* Convert from target denorm to FLOAT value */
                while (UF_LT(uint, UF_LSHIFT_64(1, (FLOAT_MANT_DIG - 1))) && exp > FLOAT_MIN_EXP) {
                    uint = UF_LSHIFT(uint, 1);
                    exp--;
                }
            }

            if (exp >= FLOAT_MAX_EXP) {
                flt = (FLOAT) INFINITY;
            } else {
#if FLOAT_MANT_DIG <= 53 || __LDBL_IS_IEC_60559__ != 0 || defined(__m68k__)
                if (UF_LT(uint, UF_LSHIFT_64(1, (FLOAT_MANT_DIG-1)))) {
                    exp = 0;
                } else {
                    exp += (FLOAT_MAX_EXP - 1);
#if FLOAT_MANT_DIG > 53 && (defined(__x86_64__) || defined(__i386__))
                    /*
                     * Intel 80-bit format is weird -- there's an
                     * integer bit so denorms can have the 'real'
                     * exponent value. That means we don't mask off
                     * the integer bit
                     */
#define EXP_SHIFT       FLOAT_MANT_DIG
#elif FLOAT_MANT_DIG > 53 && defined(__m68k__)
#define EXP_SHIFT   FLOAT_MANT_DIG + 16
#else
                    uint = UF_AND(uint, UF_NOT(UF_LSHIFT_64(1, (FLOAT_MANT_DIG-1))));
#define EXP_SHIFT       (FLOAT_MANT_DIG-1)
#endif
                }
                uint = UF_OR(uint, UF_LSHIFT_64(exp, EXP_SHIFT));
                flt = ASFLOAT(uint);
#else /* FLOAT_MANT_DIG <= 53 || __LDBL_IS_IEC_60559__ != 0 */
                flt = scalbnl(TOFLOAT(uint), exp - (FLOAT_MANT_DIG-1));
#endif
                CHECK_RANGE(flt);
            }
        }
        else
#endif
        {
#ifdef _NEED_IO_FLOAT_LARGE
            if (CHECK_LONG_LONG() && __SIZEOF_LONG_DOUBLE__ > 8)
            {
                flt = (FLOAT) __atold_engine(UF_TO_U128(uint), exp);
            }
            else
#endif
#ifdef _NEED_IO_FLOAT64
            if ((CHECK_LONG() && __SIZEOF_DOUBLE__ == 8) ||
                     (CHECK_LONG_LONG() && __SIZEOF_LONG_DOUBLE__ == 8))
            {
		if (uintdigits + exp <= -324) {
                    // Number is less than 1e-324, which should be rounded down to 0; return +/-0.0.
                    flt = (FLOAT) 0.0;
		} else if (uintdigits + exp >= 310) {
                    // Number is larger than 1e+309, which should be rounded to +/-Infinity.
                    flt = (FLOAT) INFINITY;
		} else {
                    flt = (FLOAT) __atod_engine(UF_TO_U64(uint), exp);
                }
            }
            else
#endif
            {
		if (uintdigits + exp <= -46) {
                    // Number is less than 1e-46, which should be rounded down to 0; return 0.0.
                    flt = (FLOAT) 0.0f;
		} else if (uintdigits + exp >= 40) {
                    // Number is larger than 1e+39, which should be rounded to +/-Infinity.
                    flt = (FLOAT) INFINITY;
		} else {
                    flt = (FLOAT) __atof_engine(UF_TO_U32(uint), exp);
                }
            }
        }
        CHECK_RANGE(flt)
	break;
    } /* switch */

    if (flags & FL_MINUS)
	flt = -flt;
    if (addr) {
	if (CHECK_LONG_LONG())
            *((long double *) addr) = (long double) flt;
	else if (CHECK_LONG())
	    *((double *) addr) = (double) flt;
        else
	    *((float *) addr) = (float) flt;
    }
    return 1;
}

#undef base

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

#include "scanf_private.h"
#include "../../libm/common/math_config.h"

static const char pstr_nfinity[] = "nfinity";
static const char pstr_an[] = "an";

#if defined(STRTOD) || defined(STRTOF)
# define CHECK_WIDTH()   1
# define CHECK_RANGE(flt) do {                                  \
        if (flt == (FLOAT) 0.0 || flt == (FLOAT) INFINITY)      \
            errno = ERANGE;                                     \
    } while (0);
# ifdef STRTOD
#  define CHECK_LONG()    1
# else
#  define CHECK_LONG()    0
#  define PICOLIBC_FLOAT_PRINTF_SCANF
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
    *lenp = *lenp - 1;
}

#undef EOF
#define EOF -1

#else
# define CHECK_WIDTH()   (--width != 0)
# define CHECK_RANGE(flt)
# ifdef PICOLIBC_FLOAT_PRINTF_SCANF
#  define CHECK_LONG()    0
# else
#  define CHECK_LONG()    (flags & FL_LONG)
# endif
#endif

#include "dtoa_engine.h"

static unsigned char
conv_flt (FLT_STREAM *stream, int *lenp, width_t width, void *addr, uint16_t flags)
{
    UINTFLOAT uint;
    int uintdigits = 0;
    FLOAT flt;
    int i;
    const char *p;
    int exp;

    i = scanf_getc (stream, lenp);		/* after scanf_ungetc()	*/

    switch ((unsigned char)i) {
    case '-':
        flags |= FL_MINUS;
	/* FALLTHROUGH */
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
                        if ((unsigned char)TOLOWER(i) == c)
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
	uint = 0;
#ifdef _WANT_IO_C99_FORMATS
        int base = 10;
        int uintdigitsmax = 8;
#else
#define base 10
#define uintdigitsmax 8
#endif
	do {

	    unsigned char c;

#ifdef _WANT_IO_C99_FORMATS
            if ((flags & FL_FHEX) && (c = TOLOWER(i) - 'a') <= 5)
                c += 10;
            else
#endif
                c = i - '0';

	    if (c < base) {
		flags |= FL_ANY;
		if (flags & FL_OVFL) {
		    if (!(flags & FL_DOT))
			exp += 1;
		} else {
		    if (flags & FL_DOT)
			exp -= 1;
		    uint = uint * base + c;
		    if (uint) {
			uintdigits++;
			if (CHECK_LONG()) {
			    if (uintdigits > 16)
				flags |= FL_OVFL;
			}
			else
			{
			    if (uintdigits > uintdigitsmax)
				flags |= FL_OVFL;
			}
		    }
	        }

	    } else if (c == (('.'-'0') & 0xff) && !(flags & FL_DOT)) {
		flags |= FL_DOT;
#ifdef _WANT_IO_C99_FORMATS
            } else if (TOLOWER(i) == 'x' && (flags & FL_ANY) && uint == 0) {
                flags |= FL_FHEX;
                base = 16;
                uintdigitsmax = 7;
#endif
	    } else {
		break;
	    }
	} while (CHECK_WIDTH() && (i = scanf_getc (stream, lenp)) >= 0);

	if (!(flags & FL_ANY))
	    return 0;

#ifdef _WANT_IO_C99_FORMATS
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
		/* FALLTHROUGH */
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

	    if (!ISDIGIT (edig))
            {
                scanf_ungetc(edig, stream, lenp);
                if (esign != EOF)
                    scanf_ungetc(esign, stream, lenp);
                goto no_exp;
            }

            i = edig;
	    expacc = 0;
	    do {
		expacc = expacc * 10 + (i - '0');
	    } while (CHECK_WIDTH() && ISDIGIT (i = scanf_getc(stream, lenp)));
	    if (flags & FL_MEXP)
		expacc = -expacc;
#ifdef _WANT_IO_C99_FORMATS
            if (flags & FL_FHEX)
                exp *= 4;
#endif
            exp += expacc;
	}

    no_exp:
	if (width)
            scanf_ungetc (i, stream, lenp);

	if (uint == 0) {
	    flt = 0;
	    break;
	}

#ifdef _WANT_IO_C99_FORMATS
        if (flags & FL_FHEX)
        {
#define MAX_EXP_32      127
#define SIG_BITS_32     23
#define MAX_EXP_64      1023
#define SIG_BITS_64     52
            if (CHECK_LONG()) {
                int64_t fi = _asint64((double) uint);
                int64_t s = _significand64(fi);
                exp += _exponent64(fi);
                if (exp > MAX_EXP_64 + MAX_EXP_64)
                    flt = (double) INFINITY;
                else if (exp < -SIG_BITS_64)
                    flt = 0.0;
                else {
                    if (exp < 0) {
                        int shift = -exp;
                        s >>= shift;
                        exp = 0;
                    }
                    flt = _asdouble(((int64_t)exp << SIG_BITS_64) | s);
                }
            }
            else
            {
                int32_t fi = _asint32((float) uint);
                int32_t s = _significand32(fi);
                exp += _exponent32(fi);
                if (exp > MAX_EXP_32 + MAX_EXP_32)
                    flt = (FLOAT) INFINITY;
                else if (exp < -SIG_BITS_32)
                    flt = (FLOAT) 0.0;
                else {
                    if (exp < 0) {
                        int shift = -exp;
                        s >>= shift;
                        exp = 0;
                    }
                    flt = (FLOAT) _asfloat(((int32_t)exp << SIG_BITS_32) | s);
                }
            }
        }
        else
#endif
            if (CHECK_LONG())
            {
		if ((uintdigits + exp <= -324) || (uint == 0)) {
                    // Number is less than 1e-324, which should be rounded down to 0; return +/-0.0.
                    flt = 0.0;
		} else if (uintdigits + exp >= 310) {
                    // Number is larger than 1e+309, which should be rounded to +/-Infinity.
                    flt = (FLOAT) INFINITY;
		} else {
                    flt = __atod_engine(uint, exp);
                }
            }
            else
            {
		if ((uintdigits + exp <= -46) || (uint == 0)) {
                    // Number is less than 1e-46, which should be rounded down to 0; return 0.0.
                    flt = (FLOAT) 0.0f;
		} else if (uintdigits + exp >= 40) {
                    // Number is larger than 1e+39, which should be rounded to +/-Infinity.
                    flt = (FLOAT) INFINITY;
		} else {
                    flt = (FLOAT) __atof_engine(uint, exp);
                }
            }
        CHECK_RANGE(flt)
	break;
    } /* switch */

    if (flags & FL_MINUS)
	flt = -flt;
    if (addr) {
	if (CHECK_LONG())
	    *((double *) addr) = (double) flt;
	else
	    *((float *) addr) = flt;
    }
    return 1;
}

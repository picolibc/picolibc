/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * Copyright (c) 2012-2014 ARM Ltd
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the company may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ARM LTD ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ARM LTD BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef VFSCANF_LOCAL
#define VFSCANF_LOCAL

#ifndef NO_FLOATING_POINT
#define FLOATING_POINT
#endif

#ifdef STRING_ONLY
#undef _newlib_flockfile_start
#undef _newlib_flockfile_exit
#undef _newlib_flockfile_end
#define _newlib_flockfile_start(x) {}
#define _newlib_flockfile_exit(x) {}
#define _newlib_flockfile_end(x) {}
#define _ungetc_r _sungetc_r
#define __srefill_r __ssrefill_r
#endif

#ifdef FLOATING_POINT
#include <math.h>
#include <float.h>

/* Currently a test is made to see if long double processing is warranted.
   This could be changed in the future should the _ldtoa_r code be
   preferred over _dtoa_r.  */
#define _NO_LONGDBL

#include "floatio.h"

#if ((MAXEXP+MAXFRACT+3) > MB_LEN_MAX)
/* "3 = sign + decimal point + NUL".  */
# define BUF (MAXEXP+MAXFRACT+3)
#else
# define BUF MB_LEN_MAX
#endif

/* An upper bound for how long a long prints in decimal.  4 / 13 approximates
   log (2).  Add one char for roundoff compensation and one for the sign.  */
#define MAX_LONG_LEN ((CHAR_BIT * sizeof (long)  - 1) * 4 / 13 + 2)
#else
#define	BUF	40
#endif


#define _NO_LONGLONG
#undef _WANT_IO_C99_FORMATS
#undef _WANT_IO_POS_ARGS

#define _NO_POS_ARGS

/* Macros for converting digits to letters and vice versa.  */
#define	to_digit(c)	((c) - '0')
#define is_digit(c)	((unsigned)to_digit (c) <= 9)
#define	to_char(n)	((n) + '0')

/*
 * Flags used during conversion.
 */

#define	SHORT		0x01	/* "h": short.  */
#define	LONG		0x02	/* "l": long or double.  */
#define	LONGDBL		0x04	/* "L/ll": long double or long long.  */
#define CHAR		0x08	/* "hh": 8 bit integer.  */
#define	SUPPRESS	0x10	/* Suppress assignment.  */
#define	POINTER		0x20	/* Weird %p pointer (`fake hex').  */
#define	NOSKIP		0x40	/* Do not skip blanks */

/* The following are used in numeric conversions only:
   SIGNOK, NDIGITS, DPTOK, and EXPOK are for floating point;
   SIGNOK, NDIGITS, PFXOK, and NZDIGITS are for integral.  */

#define	SIGNOK		0x80	/* "+/-" is (still) legal.  */
#define	NDIGITS		0x100	/* No digits detected.  */

#define	DPTOK		0x200	/* (Float) decimal point is still legal.  */
#define	EXPOK		0x400	/* (Float) exponent (e+3, etc) still legal.  */

#define	PFXOK		0x200	/* "0x" prefix is (still) legal.  */
#define	NZDIGITS	0x400	/* No zero digits detected.  */
#define	NNZDIGITS	0x800	/* No non-zero digits detected.  */

/* Conversion types.  */

#define	CT_CHAR		0	/* "%c" conversion.  */
#define	CT_CCL		1	/* "%[...]" conversion.  */
#define	CT_STRING	2	/* "%s" conversion.  */
#define	CT_INT		3	/* Integer, i.e., strtol.  */
#define	CT_UINT		4	/* Unsigned integer, i.e., strtoul.  */
#define	CT_FLOAT	5	/* Floating, i.e., strtod.  */

#define u_char unsigned char
#define u_long unsigned long

/* Macro to support positional arguments.  */
#define GET_ARG(n, ap, type) (va_arg ((ap), type))

#define MATCH_FAILURE	1
#define INPUT_FAILURE	2


/* All data needed to decode format string are kept in below struct.  */
struct _scan_data_t
{
  int flags;            /* Flags.  */
  int base;             /* Base.  */
  size_t width;         /* Width.  */
  int nassigned;        /* Number of assignments so far.  */
  int nread;            /* Number of chars read so far.  */
  char *ccltab;         /* Table used for [ format.  */
  int code;             /* Current conversion specifier.  */
  char buf[BUF];        /* Internal buffer for scan.  */
  /* Internal buffer for scan.  */
  int (*pfn_ungetc)(struct _reent*, int, FILE*);
  /* Internal buffer for scan.  */
  int (*pfn_refill)(struct _reent*, FILE*);
};

extern int
_scanf_chars (struct _reent *rptr,
	      struct _scan_data_t *pdata,
	      FILE *fp, va_list *ap);
extern int
_scanf_i (struct _reent *rptr,
	  struct _scan_data_t *pdata,
	  FILE *fp, va_list *ap);
/* Make _scanf_float weak symbol, so it won't be linked in if target program
   does not need it.  */
extern int
_scanf_float (struct _reent *rptr,
	      struct _scan_data_t *pdata,
	      FILE *fp, va_list *ap) _ATTRIBUTE((__weak__));

#endif

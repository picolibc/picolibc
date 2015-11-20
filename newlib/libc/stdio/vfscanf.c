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
FUNCTION
<<vfscanf>>, <<vscanf>>, <<vsscanf>>---format argument list

INDEX
	vfscanf
INDEX
	_vfscanf_r
INDEX
	vscanf
INDEX
	_vscanf_r
INDEX
	vsscanf
INDEX
	_vsscanf_r

ANSI_SYNOPSIS
	#include <stdio.h>
	#include <stdarg.h>
	int vscanf(const char *<[fmt]>, va_list <[list]>);
	int vfscanf(FILE *<[fp]>, const char *<[fmt]>, va_list <[list]>);
	int vsscanf(const char *<[str]>, const char *<[fmt]>, va_list <[list]>);

	int _vscanf_r(struct _reent *<[reent]>, const char *<[fmt]>,
                       va_list <[list]>);
	int _vfscanf_r(struct _reent *<[reent]>, FILE *<[fp]>, const char *<[fmt]>,
                       va_list <[list]>);
	int _vsscanf_r(struct _reent *<[reent]>, const char *<[str]>,
                       const char *<[fmt]>, va_list <[list]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	#include <varargs.h>
	int vscanf( <[fmt]>, <[ist]>)
	char *<[fmt]>;
	va_list <[list]>;

	int vfscanf( <[fp]>, <[fmt]>, <[list]>)
	FILE *<[fp]>;
	char *<[fmt]>;
	va_list <[list]>;

	int vsscanf( <[str]>, <[fmt]>, <[list]>)
	char *<[str]>;
	char *<[fmt]>;
	va_list <[list]>;

	int _vscanf_r( <[reent]>, <[fmt]>, <[ist]>)
	struct _reent *<[reent]>;
	char *<[fmt]>;
	va_list <[list]>;

	int _vfscanf_r( <[reent]>, <[fp]>, <[fmt]>, <[list]>)
	struct _reent *<[reent]>;
	FILE *<[fp]>;
	char *<[fmt]>;
	va_list <[list]>;

	int _vsscanf_r( <[reent]>, <[str]>, <[fmt]>, <[list]>)
	struct _reent *<[reent]>;
	char *<[str]>;
	char *<[fmt]>;
	va_list <[list]>;

DESCRIPTION
<<vscanf>>, <<vfscanf>>, and <<vsscanf>> are (respectively) variants
of <<scanf>>, <<fscanf>>, and <<sscanf>>.  They differ only in
allowing their caller to pass the variable argument list as a
<<va_list>> object (initialized by <<va_start>>) rather than
directly accepting a variable number of arguments.

RETURNS
The return values are consistent with the corresponding functions:
<<vscanf>> returns the number of input fields successfully scanned,
converted, and stored; the return value does not include scanned
fields which were not stored.

If <<vscanf>> attempts to read at end-of-file, the return value
is <<EOF>>.

If no fields were stored, the return value is <<0>>.

The routines <<_vscanf_r>>, <<_vfscanf_f>>, and <<_vsscanf_r>> are
reentrant versions which take an additional first parameter which points to the
reentrancy structure.

PORTABILITY
These are GNU extensions.

Supporting OS subroutines required:
*/

#include <_ansi.h>
#include <reent.h>
#include <newlib.h>
#include <ctype.h>
#include <wctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <wchar.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include "local.h"
#include "../stdlib/local.h"

#ifdef INTEGER_ONLY
#define VFSCANF vfiscanf
#define _VFSCANF_R _vfiscanf_r
#define __SVFSCANF __svfiscanf
#ifdef STRING_ONLY
#  define __SVFSCANF_R __ssvfiscanf_r
#else
#  define __SVFSCANF_R __svfiscanf_r
#endif
#else
#define VFSCANF vfscanf
#define _VFSCANF_R _vfscanf_r
#define __SVFSCANF __svfscanf
#ifdef STRING_ONLY
#  define __SVFSCANF_R __ssvfscanf_r
#else
#  define __SVFSCANF_R __svfscanf_r
#endif
#ifndef NO_FLOATING_POINT
#define FLOATING_POINT
#endif
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
#define _fread_r _sfread_r
#endif

#ifdef FLOATING_POINT
#include <math.h>
#include <float.h>
#include <locale.h>

/* Currently a test is made to see if long double processing is warranted.
   This could be changed in the future should the _ldtoa_r code be
   preferred over _dtoa_r.  */
#define _NO_LONGDBL
#if defined _WANT_IO_LONG_DOUBLE && (LDBL_MANT_DIG > DBL_MANT_DIG)
#undef _NO_LONGDBL
#endif

#include "floatio.h"

#define BUF (MAXEXP+MAXFRACT+MB_LEN_MAX+2) /* decimal point + sign + NUL */

/* An upper bound for how long a long prints in decimal.  4 / 13 approximates
   log (2).  Add one char for roundoff compensation and one for the sign.  */
#define MAX_LONG_LEN ((CHAR_BIT * sizeof (long)  - 1) * 4 / 13 + 2)
#else
#define	BUF	40
#endif

#define _NO_LONGLONG
#if defined _WANT_IO_LONG_LONG \
	&& (defined __GNUC__ || __STDC_VERSION__ >= 199901L)
# undef _NO_LONGLONG
#endif

#define _NO_POS_ARGS
#ifdef _WANT_IO_POS_ARGS
# undef _NO_POS_ARGS
# ifdef NL_ARGMAX
#  define MAX_POS_ARGS NL_ARGMAX
# else
#  define MAX_POS_ARGS 32
# endif

static void * get_arg (int, va_list *, int *, void **);
#endif /* _WANT_IO_POS_ARGS */

/*
 * Flags used during conversion.
 */

#define	LONG		0x01	/* l: long or double */
#define	LONGDBL		0x02	/* L/ll: long double or long long */
#define	SHORT		0x04	/* h: short */
#define CHAR		0x08	/* hh: 8 bit integer */
#define	SUPPRESS	0x10	/* suppress assignment */
#define	POINTER		0x20	/* weird %p pointer (`fake hex') */
#define	NOSKIP		0x40	/* do not skip blanks */

/*
 * The following are used in numeric conversions only:
 * SIGNOK, NDIGITS, DPTOK, and EXPOK are for floating point;
 * SIGNOK, NDIGITS, PFXOK, and NZDIGITS are for integral.
 */

#define	SIGNOK		0x80	/* +/- is (still) legal */
#define	NDIGITS		0x100	/* no digits detected */

#define	DPTOK		0x200	/* (float) decimal point is still legal */
#define	EXPOK		0x400	/* (float) exponent (e+3, etc) still legal */

#define	PFXOK		0x200	/* 0x prefix is (still) legal */
#define	NZDIGITS	0x400	/* no zero digits detected */
#define	NNZDIGITS	0x800	/* no non-zero digits detected */

/*
 * Conversion types.
 */

#define	CT_CHAR		0	/* %c conversion */
#define	CT_CCL		1	/* %[...] conversion */
#define	CT_STRING	2	/* %s conversion */
#define	CT_INT		3	/* integer, i.e., strtol or strtoul */
#define	CT_FLOAT	4	/* floating, i.e., strtod */

#define u_char unsigned char
#define u_long unsigned long

#ifndef _NO_LONGLONG
typedef unsigned long long u_long_long;
#endif

/*
 * vfscanf
 */

#define BufferEmpty (fp->_r <= 0 && __srefill_r(rptr, fp))

#ifndef STRING_ONLY

#ifndef _REENT_ONLY

int
_DEFUN(VFSCANF, (fp, fmt, ap),
       register FILE *fp _AND
       _CONST char *fmt _AND
       va_list ap)
{
  struct _reent *reent = _REENT;

  CHECK_INIT(reent, fp);
  return __SVFSCANF_R (reent, fp, fmt, ap);
}

int
_DEFUN(__SVFSCANF, (fp, fmt0, ap),
       register FILE *fp _AND
       char _CONST *fmt0 _AND
       va_list ap)
{
  return __SVFSCANF_R (_REENT, fp, fmt0, ap);
}

#endif /* !_REENT_ONLY */

int
_DEFUN(_VFSCANF_R, (data, fp, fmt, ap),
       struct _reent *data _AND
       register FILE *fp   _AND
       _CONST char *fmt    _AND
       va_list ap)
{
  CHECK_INIT(data, fp);
  return __SVFSCANF_R (data, fp, fmt, ap);
}
#endif /* !STRING_ONLY */

#if defined (STRING_ONLY) && defined (INTEGER_ONLY)
/* When dealing with the sscanf family, we don't want to use the
 * regular ungetc which will drag in file I/O items we don't need.
 * So, we create our own trimmed-down version.  */
int
_DEFUN(_sungetc_r, (data, fp, ch),
	struct _reent *data _AND
	int c               _AND
	register FILE *fp)
{
  if (c == EOF)
    return (EOF);

  /* After ungetc, we won't be at eof anymore */
  fp->_flags &= ~__SEOF;
  c = (unsigned char) c;

  /*
   * If we are in the middle of ungetc'ing, just continue.
   * This may require expanding the current ungetc buffer.
   */

  if (HASUB (fp))
    {
      if (fp->_r >= fp->_ub._size && __submore (data, fp))
        {
          return EOF;
        }
      *--fp->_p = c;
      fp->_r++;
      return c;
    }

  /*
   * If we can handle this by simply backing up, do so,
   * but never replace the original character.
   * (This makes sscanf() work when scanning `const' data.)
   */

  if (fp->_bf._base != NULL && fp->_p > fp->_bf._base && fp->_p[-1] == c)
    {
      fp->_p--;
      fp->_r++;
      return c;
    }

  /*
   * Create an ungetc buffer.
   * Initially, we will use the `reserve' buffer.
   */

  fp->_ur = fp->_r;
  fp->_up = fp->_p;
  fp->_ub._base = fp->_ubuf;
  fp->_ub._size = sizeof (fp->_ubuf);
  fp->_ubuf[sizeof (fp->_ubuf) - 1] = c;
  fp->_p = &fp->_ubuf[sizeof (fp->_ubuf) - 1];
  fp->_r = 1;
  return c;
}

/* String only version of __srefill_r for sscanf family.  */
int
_DEFUN(__ssrefill_r, (ptr, fp),
       struct _reent * ptr _AND
       register FILE * fp)
{
  /*
   * Our only hope of further input is the ungetc buffer.
   * If there is anything in that buffer to read, return.
   */
  if (HASUB (fp))
    {
      FREEUB (ptr, fp);
      if ((fp->_r = fp->_ur) != 0)
        {
          fp->_p = fp->_up;
	  return 0;
        }
    }

  /* Otherwise we are out of character input.  */
  fp->_p = fp->_bf._base;
  fp->_r = 0;
  fp->_flags |= __SEOF;
  return EOF;
}

size_t
_DEFUN(_sfread_r, (ptr, buf, size, count, fp),
       struct _reent * ptr _AND
       _PTR buf _AND
       size_t size _AND
       size_t count _AND
       FILE * fp)
{
  register size_t resid;
  register char *p;
  register int r;
  size_t total;

  if ((resid = count * size) == 0)
    return 0;

  total = resid;
  p = buf;

  while (resid > (r = fp->_r))
    {
      _CAST_VOID memcpy ((_PTR) p, (_PTR) fp->_p, (size_t) r);
      fp->_p += r;
      fp->_r = 0;
      p += r;
      resid -= r;
      if (__ssrefill_r (ptr, fp))
        {
          /* no more input: return partial result */
          return (total - resid) / size;
        }
    }
  _CAST_VOID memcpy ((_PTR) p, (_PTR) fp->_p, resid);
  fp->_r -= resid;
  fp->_p += resid;
  return count;
}
#else /* !STRING_ONLY || !INTEGER_ONLY */
int _EXFUN (_sungetc_r, (struct _reent *, int, register FILE *));
int _EXFUN (__ssrefill_r, (struct _reent *, register FILE *));
size_t _EXFUN (_sfread_r, (struct _reent *, _PTR buf, size_t, size_t, FILE *));
#endif /* !STRING_ONLY || !INTEGER_ONLY */

int
_DEFUN(__SVFSCANF_R, (rptr, fp, fmt0, ap),
       struct _reent *rptr _AND
       register FILE *fp   _AND
       char _CONST *fmt0   _AND
       va_list ap)
{
  register u_char *fmt = (u_char *) fmt0;
  register int c;		/* character from format, or conversion */
  register size_t width;	/* field width, or 0 */
  register char *p;		/* points into all kinds of strings */
  register int n;		/* handy integer */
  register int flags;		/* flags as defined above */
  register char *p0;		/* saves original value of p when necessary */
  int nassigned;		/* number of fields assigned */
  int nread;			/* number of characters consumed from fp */
#ifndef _NO_POS_ARGS
  int N;			/* arg number */
  int arg_index = 0;		/* index into args processed directly */
  int numargs = 0;		/* number of varargs read */
  void *args[MAX_POS_ARGS];	/* positional args read */
  int is_pos_arg;		/* is current format positional? */
#endif
  int base = 0;			/* base argument to strtol/strtoul */
  int nbytes = 1;               /* number of bytes read from fmt string */
  wchar_t wc;                   /* wchar to use to read format string */
  wchar_t *wcp;                 /* handy wide character pointer */
  size_t mbslen;                /* length of converted multibyte sequence */
#ifdef _MB_CAPABLE
  mbstate_t state;              /* value to keep track of multibyte state */
#endif

  #define CCFN_PARAMS	_PARAMS((struct _reent *, const char *, char **, int))
  u_long (*ccfn)CCFN_PARAMS=0;	/* conversion function (strtol/strtoul) */
  char ccltab[256];		/* character class table for %[...] */
  char buf[BUF];		/* buffer for numeric conversions */
  unsigned char *lptr;          /* literal pointer */

  char *cp;
  short *sp;
  int *ip;
#ifdef FLOATING_POINT
  float *flp;
  _LONG_DOUBLE *ldp;
  double *dp;
#endif
  long *lp;
#ifndef _NO_LONGLONG
  long long *llp;
#endif

  /* `basefix' is used to avoid `if' tests in the integer scanner */
  static _CONST short basefix[17] =
    {10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

  /* Macro to support positional arguments */
#ifndef _NO_POS_ARGS
# define GET_ARG(n, ap, type)					\
  ((type) (is_pos_arg						\
	   ? (n < numargs					\
	      ? args[n]						\
	      : get_arg (n, &ap, &numargs, args))		\
	   : (arg_index++ < numargs				\
	      ? args[n]						\
	      : (numargs < MAX_POS_ARGS				\
		 ? args[numargs++] = va_arg (ap, void *)	\
		 : va_arg (ap, void *)))))
#else
# define GET_ARG(n, ap, type) (va_arg (ap, type))
#endif

  _newlib_flockfile_start (fp);

  ORIENT (fp, -1);

  nassigned = 0;
  nread = 0;
#ifdef _MB_CAPABLE
  memset (&state, 0, sizeof (state));
#endif

  for (;;)
    {
#ifndef _MB_CAPABLE
      wc = *fmt;
#else
      nbytes = __mbtowc (rptr, &wc, (char *) fmt, MB_CUR_MAX,
			 __locale_charset (), &state);
      if (nbytes < 0) {
	wc = 0xFFFD; /* Unicode replacement character */
	nbytes = 1;
	memset (&state, 0, sizeof (state));
      }
#endif
      fmt += nbytes;

      if (wc == 0)
	goto all_done;
      if (nbytes == 1 && isspace (wc))
	{
	  for (;;)
	    {
	      if (BufferEmpty || !isspace (*fp->_p))
		break;
	      nread++, fp->_r--, fp->_p++;
	    }
	  continue;
	}
      if (wc != '%')
	goto literal;
      width = 0;
      flags = 0;
#ifndef _NO_POS_ARGS
      N = arg_index;
      is_pos_arg = 0;
#endif

      /*
       * switch on the format.  continue if done; break once format
       * type is derived.
       */

    again:
      c = *fmt++;

      switch (c)
	{
	case '%':
	literal:
          lptr = fmt - nbytes;
          for (n = 0; n < nbytes; ++n)
            {
	      if (BufferEmpty)
	        goto input_failure;
	      if (*fp->_p != *lptr)
	        goto match_failure;
	      fp->_r--, fp->_p++;
	      nread++;
              ++lptr;
            }
	  continue;

	case '*':
	  flags |= SUPPRESS;
	  goto again;
	case 'l':
#if defined _WANT_IO_C99_FORMATS || !defined _NO_LONGLONG
	  if (*fmt == 'l')	/* Check for 'll' = long long (SUSv3) */
	    {
	      ++fmt;
	      flags |= LONGDBL;
	    }
	  else
#endif
	    flags |= LONG;
	  goto again;
	case 'L':
	  flags |= LONGDBL;
	  goto again;
	case 'h':
#ifdef _WANT_IO_C99_FORMATS
	  if (*fmt == 'h')	/* Check for 'hh' = char int (SUSv3) */
	    {
	      ++fmt;
	      flags |= CHAR;
	    }
	  else
#endif
	    flags |= SHORT;
	  goto again;
#ifdef _WANT_IO_C99_FORMATS
	case 'j': /* intmax_t */
	  if (sizeof (intmax_t) == sizeof (long))
	    flags |= LONG;
	  else
	    flags |= LONGDBL;
	  goto again;
	case 't': /* ptrdiff_t */
	  if (sizeof (ptrdiff_t) < sizeof (int))
	    /* POSIX states ptrdiff_t is 16 or more bits, as
	       is short.  */
	    flags |= SHORT;
	  else if (sizeof (ptrdiff_t) == sizeof (int))
	    /* no flag needed */;
	  else if (sizeof (ptrdiff_t) <= sizeof (long))
	    flags |= LONG;
	  else
	    /* POSIX states that at least one programming
	       environment must support ptrdiff_t no wider than
	       long, but that means other environments can
	       have ptrdiff_t as wide as long long.  */
	    flags |= LONGDBL;
	  goto again;
	case 'z': /* size_t */
	  if (sizeof (size_t) < sizeof (int))
	    /* POSIX states size_t is 16 or more bits, as is short.  */
	    flags |= SHORT;
	  else if (sizeof (size_t) == sizeof (int))
	    /* no flag needed */;
	  else if (sizeof (size_t) <= sizeof (long))
	    flags |= LONG;
	  else
	    /* POSIX states that at least one programming
	       environment must support size_t no wider than
	       long, but that means other environments can
	       have size_t as wide as long long.  */
	    flags |= LONGDBL;
	  goto again;
#endif /* _WANT_IO_C99_FORMATS */

	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	  width = width * 10 + c - '0';
	  goto again;

#ifndef _NO_POS_ARGS
	case '$':
	  if (width <= MAX_POS_ARGS)
	    {
	      N = width - 1;
	      is_pos_arg = 1;
	      width = 0;
	      goto again;
	    }
	  rptr->_errno = EINVAL;
	  goto input_failure;
#endif /* !_NO_POS_ARGS */

	  /*
	   * Conversions. Those marked `compat' are for
	   * 4.[123]BSD compatibility.
	   *
	   * (According to ANSI, E and X formats are supposed to
	   * the same as e and x.  Sorry about that.)
	   */

	case 'D':		/* compat */
	  flags |= LONG;
	  /* FALLTHROUGH */
	case 'd':
	  c = CT_INT;
	  ccfn = (u_long (*)CCFN_PARAMS)_strtol_r;
	  base = 10;
	  break;

	case 'i':
	  c = CT_INT;
	  ccfn = (u_long (*)CCFN_PARAMS)_strtol_r;
	  base = 0;
	  break;

	case 'O':		/* compat */
	  flags |= LONG;
	  /* FALLTHROUGH */
	case 'o':
	  c = CT_INT;
	  ccfn = _strtoul_r;
	  base = 8;
	  break;

	case 'u':
	  c = CT_INT;
	  ccfn = _strtoul_r;
	  base = 10;
	  break;

	case 'X':
	case 'x':
	  flags |= PFXOK;	/* enable 0x prefixing */
	  c = CT_INT;
	  ccfn = _strtoul_r;
	  base = 16;
	  break;

#ifdef FLOATING_POINT
# ifdef _WANT_IO_C99_FORMATS
	case 'a':
	case 'A':
	case 'F':
# endif
	case 'E':
	case 'G':
	case 'e':
	case 'f':
	case 'g':
	  c = CT_FLOAT;
	  break;
#endif

#ifdef _WANT_IO_C99_FORMATS
	case 'S':
	  flags |= LONG;
	  /* FALLTHROUGH */
#endif

	case 's':
	  c = CT_STRING;
	  break;

	case '[':
	  fmt = (u_char *) __sccl (ccltab, (unsigned char *) fmt);
	  flags |= NOSKIP;
	  c = CT_CCL;
	  break;

#ifdef _WANT_IO_C99_FORMATS
	case 'C':
	  flags |= LONG;
	  /* FALLTHROUGH */
#endif

	case 'c':
	  flags |= NOSKIP;
	  c = CT_CHAR;
	  break;

	case 'p':		/* pointer format is like hex */
	  flags |= POINTER | PFXOK;
	  c = CT_INT;
	  ccfn = _strtoul_r;
	  base = 16;
	  break;

	case 'n':
	  if (flags & SUPPRESS)	/* ??? */
	    continue;
#ifdef _WANT_IO_C99_FORMATS
	  if (flags & CHAR)
	    {
	      cp = GET_ARG (N, ap, char *);
	      *cp = nread;
	    }
	  else
#endif
	  if (flags & SHORT)
	    {
	      sp = GET_ARG (N, ap, short *);
	      *sp = nread;
	    }
	  else if (flags & LONG)
	    {
	      lp = GET_ARG (N, ap, long *);
	      *lp = nread;
	    }
#ifndef _NO_LONGLONG
	  else if (flags & LONGDBL)
	    {
	      llp = GET_ARG (N, ap, long long*);
	      *llp = nread;
	    }
#endif
	  else
	    {
	      ip = GET_ARG (N, ap, int *);
	      *ip = nread;
	    }
	  continue;

	  /*
	   * Disgusting backwards compatibility hacks.	XXX
	   */
	case '\0':		/* compat */
	  _newlib_flockfile_exit (fp);
	  return EOF;

	default:		/* compat */
	  if (isupper (c))
	    flags |= LONG;
	  c = CT_INT;
	  ccfn = (u_long (*)CCFN_PARAMS)_strtol_r;
	  base = 10;
	  break;
	}

      /*
       * We have a conversion that requires input.
       */
      if (BufferEmpty)
	goto input_failure;

      /*
       * Consume leading white space, except for formats that
       * suppress this.
       */
      if ((flags & NOSKIP) == 0)
	{
	  while (isspace (*fp->_p))
	    {
	      nread++;
	      if (--fp->_r > 0)
		fp->_p++;
	      else
	      if (__srefill_r (rptr, fp))
		goto input_failure;
	    }
	  /*
	   * Note that there is at least one character in the
	   * buffer, so conversions that do not set NOSKIP ca
	   * no longer result in an input failure.
	   */
	}

      /*
       * Do the conversion.
       */
      switch (c)
	{

	case CT_CHAR:
	  /* scan arbitrary characters (sets NOSKIP) */
	  if (width == 0)
	    width = 1;
#if !defined(_ELIX_LEVEL) || _ELIX_LEVEL >= 2
          if (flags & LONG)
            {
              mbstate_t state;
              memset (&state, 0, sizeof (mbstate_t));
              if ((flags & SUPPRESS) == 0)
                wcp = GET_ARG (N, ap, wchar_t *);
              else
                wcp = NULL;
              n = 0;
              while (width != 0)
                {
                  if (n == MB_CUR_MAX)
                    goto input_failure;
                  buf[n++] = *fp->_p;
                  fp->_r -= 1;
                  fp->_p += 1;
                  if ((mbslen = _mbrtowc_r (rptr, wcp, buf, n, &state))
                                                         == (size_t)-1)
                    goto input_failure; /* Invalid sequence */
                  if (mbslen == 0 && !(flags & SUPPRESS))
                    *wcp = L'\0';
                  if (mbslen != (size_t)-2) /* Incomplete sequence */
                    {
                      nread += n;
                      width -= 1;
                      if (!(flags & SUPPRESS))
                        wcp += 1;
                      n = 0;
                    }
                  if (BufferEmpty)
	            {
                      if (n != 0)
                        goto input_failure;
                      break;
                    }
                }
              if (!(flags & SUPPRESS))
                nassigned++;
            }
          else
#endif
        	  if (flags & SUPPRESS)
	    {
	      size_t sum = 0;
	      for (;;)
		{
		  if ((n = fp->_r) < (int)width)
		    {
		      sum += n;
		      width -= n;
		      fp->_p += n;
		      if (__srefill_r (rptr, fp))
			{
			  if (sum == 0)
			    goto input_failure;
			  break;
			}
		    }
		  else
		    {
		      sum += width;
		      fp->_r -= width;
		      fp->_p += width;
		      break;
		    }
		}
	      nread += sum;
	    }
	  else
	    {
	      size_t r = _fread_r (rptr, (_PTR) GET_ARG (N, ap, char *), 1, width, fp);

	      if (r == 0)
		goto input_failure;
	      nread += r;
	      nassigned++;
	    }
	  break;

	case CT_CCL:
	  /* scan a (nonempty) character class (sets NOSKIP) */
	  if (width == 0)
	    width = ~0;		/* `infinity' */
	  /* take only those things in the class */
	  if (flags & SUPPRESS)
	    {
	      n = 0;
	      while (ccltab[*fp->_p])
		{
		  n++, fp->_r--, fp->_p++;
		  if (--width == 0)
		    break;
		  if (BufferEmpty)
		    {
		      if (n == 0)
			goto input_failure;
		      break;
		    }
		}
	      if (n == 0)
		goto match_failure;
	    }
	  else
	    {
	      p0 = p = GET_ARG (N, ap, char *);
	      while (ccltab[*fp->_p])
		{
		  fp->_r--;
		  *p++ = *fp->_p++;
		  if (--width == 0)
		    break;
		  if (BufferEmpty)
		    {
		      if (p == p0)
			goto input_failure;
		      break;
		    }
		}
	      n = p - p0;
	      if (n == 0)
		goto match_failure;
	      *p = 0;
	      nassigned++;
	    }
	  nread += n;
	  break;

	case CT_STRING:
	  /* like CCL, but zero-length string OK, & no NOSKIP */
	  if (width == 0)
            width = (size_t)~0;
#if !defined(_ELIX_LEVEL) || _ELIX_LEVEL >= 2
          if (flags & LONG)
            {
              /* Process %S and %ls placeholders */
              mbstate_t state;
              memset (&state, 0, sizeof (mbstate_t));
              if ((flags & SUPPRESS) == 0)
                wcp = GET_ARG (N, ap, wchar_t *);
              else
                wcp = &wc;
              n = 0;
              while (!isspace (*fp->_p) && width != 0)
                {
                  if (n == MB_CUR_MAX)
                    goto input_failure;
                  buf[n++] = *fp->_p;
                  fp->_r -= 1;
                  fp->_p += 1;
                  if ((mbslen = _mbrtowc_r (rptr, wcp, buf, n, &state))
                                                        == (size_t)-1)
                    goto input_failure;
                  if (mbslen == 0)
                    *wcp = L'\0';
                  if (mbslen != (size_t)-2) /* Incomplete sequence */
                    {
                      if (iswspace(*wcp))
                        {
                          while (n != 0)
                            _ungetc_r (rptr, (unsigned char) buf[--n], fp);
                          break;
                        }
                      nread += n;
                      width -= 1;
                      if ((flags & SUPPRESS) == 0)
                        wcp += 1;
                      n = 0;
                    }
                  if (BufferEmpty)
                    {
                      if (n != 0)
                        goto input_failure;
                      break;
                    }
                }
              if (!(flags & SUPPRESS))
                {
                  *wcp = L'\0';
                  nassigned++;
                }
            }
          else
#endif
        	  if (flags & SUPPRESS)
	    {
	      n = 0;
	      while (!isspace (*fp->_p))
		{
		  n++, fp->_r--, fp->_p++;
		  if (--width == 0)
		    break;
		  if (BufferEmpty)
		    break;
		}
	      nread += n;
	    }
	  else
	    {
	      p0 = p = GET_ARG (N, ap, char *);
	      while (!isspace (*fp->_p))
		{
		  fp->_r--;
		  *p++ = *fp->_p++;
		  if (--width == 0)
		    break;
		  if (BufferEmpty)
		    break;
		}
	      *p = 0;
	      nread += p - p0;
	      nassigned++;
	    }
	  continue;

	case CT_INT:
	{
	  /* scan an integer as if by strtol/strtoul */
	  unsigned width_left = 0;
	  int skips = 0;
#ifdef hardway
	  if (width == 0 || width > sizeof (buf) - 1)
#else
	  /* size_t is unsigned, hence this optimisation */
	  if (width - 1 > sizeof (buf) - 2)
#endif
	    {
	      width_left = width - (sizeof (buf) - 1);
	      width = sizeof (buf) - 1;
	    }
	  flags |= SIGNOK | NDIGITS | NZDIGITS | NNZDIGITS;
	  for (p = buf; width; width--)
	    {
	      c = *fp->_p;
	      /*
	       * Switch on the character; `goto ok' if we
	       * accept it as a part of number.
	       */
	      switch (c)
		{
		  /*
		   * The digit 0 is always legal, but is special.
		   * For %i conversions, if no digits (zero or nonzero)
		   * have been scanned (only signs), we will have base==0.
		   * In that case, we should set it to 8 and enable 0x
		   * prefixing. Also, if we have not scanned zero digits
		   * before this, do not turn off prefixing (someone else
		   * will turn it off if we have scanned any nonzero digits).
		   */
		case '0':
		  if (! (flags & NNZDIGITS))
		    goto ok;
		  if (base == 0)
		    {
		      base = 8;
		      flags |= PFXOK;
		    }
		  if (flags & NZDIGITS)
		    {
		      flags &= ~(SIGNOK | NZDIGITS | NDIGITS);
		      goto ok;
		    }
		  flags &= ~(SIGNOK | PFXOK | NDIGITS);
		  if (width_left)
		    {
		      width_left--;
		      width++;
		    }
		  ++skips;
		  goto skip;

		  /* 1 through 7 always legal */
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		  base = basefix[base];
		  flags &= ~(SIGNOK | PFXOK | NDIGITS | NNZDIGITS);
		  goto ok;

		  /* digits 8 and 9 ok iff decimal or hex */
		case '8':
		case '9':
		  base = basefix[base];
		  if (base <= 8)
		    break;	/* not legal here */
		  flags &= ~(SIGNOK | PFXOK | NDIGITS | NNZDIGITS);
		  goto ok;

		  /* letters ok iff hex */
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		  /* no need to fix base here */
		  if (base <= 10)
		    break;	/* not legal here */
		  flags &= ~(SIGNOK | PFXOK | NDIGITS | NNZDIGITS);
		  goto ok;

		  /* sign ok only as first character */
		case '+':
		case '-':
		  if (flags & SIGNOK)
		    {
		      flags &= ~SIGNOK;
		      goto ok;
		    }
		  break;

		  /* x ok iff flag still set & single 0 seen */
		case 'x':
		case 'X':
		  if ((flags & (PFXOK | NZDIGITS)) == PFXOK)
		    {
		      base = 16;/* if %i */
		      flags &= ~PFXOK;
		      /* We must reset the NZDIGITS and NDIGITS
			 flags that would have been unset by seeing
			 the zero that preceded the X or x.  */
		      flags |= NZDIGITS | NDIGITS;
		      goto ok;
		    }
		  break;
		}

	      /*
	       * If we got here, c is not a legal character
	       * for a number.  Stop accumulating digits.
	       */
	      break;
	    ok:
	      /*
	       * c is legal: store it and look at the next.
	       */
	      *p++ = c;
	    skip:
	      if (--fp->_r > 0)
		fp->_p++;
	      else
	      if (__srefill_r (rptr, fp))
		break;		/* EOF */
	    }
	  /*
	   * If we had only a sign, it is no good; push back the sign.
	   * If the number ends in `x', it was [sign] '0' 'x', so push back
	   * the x and treat it as [sign] '0'.
	   * Use of ungetc here and below assumes ASCII encoding; we are only
	   * pushing back 7-bit characters, so casting to unsigned char is
	   * not necessary.
	   */
	  if (flags & NDIGITS)
	    {
	      if (p > buf)
		_ungetc_r (rptr, *--p, fp); /* [-+xX] */
	      if (p == buf)
		goto match_failure;
	    }
	  if ((flags & SUPPRESS) == 0)
	    {
	      u_long res;

	      *p = 0;
	      res = (*ccfn) (rptr, buf, (char **) NULL, base);
	      if (flags & POINTER)
		{
		  void **vp = GET_ARG (N, ap, void **);
#ifndef _NO_LONGLONG
		  if (sizeof (uintptr_t) > sizeof (u_long))
		    {
		      u_long_long resll;
		      resll = _strtoull_r (rptr, buf, (char **) NULL, base);
		      *vp = (void *) (uintptr_t) resll;
		    }
		  else
#endif /* !_NO_LONGLONG */
		    *vp = (void *) (uintptr_t) res;
		}
#ifdef _WANT_IO_C99_FORMATS
	      else if (flags & CHAR)
		{
		  cp = GET_ARG (N, ap, char *);
		  *cp = res;
		}
#endif
	      else if (flags & SHORT)
		{
		  sp = GET_ARG (N, ap, short *);
		  *sp = res;
		}
	      else if (flags & LONG)
		{
		  lp = GET_ARG (N, ap, long *);
		  *lp = res;
		}
#ifndef _NO_LONGLONG
	      else if (flags & LONGDBL)
		{
		  u_long_long resll;
		  if (ccfn == _strtoul_r)
		    resll = _strtoull_r (rptr, buf, (char **) NULL, base);
		  else
		    resll = _strtoll_r (rptr, buf, (char **) NULL, base);
		  llp = GET_ARG (N, ap, long long*);
		  *llp = resll;
		}
#endif
	      else
		{
		  ip = GET_ARG (N, ap, int *);
		  *ip = res;
		}
	      nassigned++;
	    }
	  nread += p - buf + skips;
	  break;
	}
#ifdef FLOATING_POINT
	case CT_FLOAT:
	{
	  /* scan a floating point number as if by strtod */
	  /* This code used to assume that the number of digits is reasonable.
	     However, ANSI / ISO C makes no such stipulation; we have to get
	     exact results even when there is an unreasonable amount of
	     leading zeroes.  */
	  long leading_zeroes = 0;
	  long zeroes, exp_adjust;
	  char *exp_start = NULL;
	  unsigned width_left = 0;
	  char nancount = 0;
	  char infcount = 0;
	  const char *decpt = _localeconv_r (rptr)->decimal_point;
#ifdef _MB_CAPABLE
	  int decptpos = 0;
#endif
#ifdef hardway
	  if (width == 0 || width > sizeof (buf) - 1)
#else
	  /* size_t is unsigned, hence this optimisation */
	  if (width - 1 > sizeof (buf) - 2)
#endif
	    {
	      width_left = width - (sizeof (buf) - 1);
	      width = sizeof (buf) - 1;
	    }
	  flags |= SIGNOK | NDIGITS | DPTOK | EXPOK;
	  zeroes = 0;
	  exp_adjust = 0;
	  for (p = buf; width; )
	    {
	      c = *fp->_p;
	      /*
	       * This code mimicks the integer conversion
	       * code, but is much simpler.
	       */
	      switch (c)
		{
		case '0':
		  if (flags & NDIGITS)
		    {
		      flags &= ~SIGNOK;
		      zeroes++;
		      if (width_left)
			{
			  width_left--;
			  width++;
			}
		      goto fskip;
		    }
		  /* Fall through.  */
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		  if (nancount + infcount == 0)
		    {
		      flags &= ~(SIGNOK | NDIGITS);
		      goto fok;
		    }
		  break;

		case '+':
		case '-':
		  if (flags & SIGNOK)
		    {
		      flags &= ~SIGNOK;
		      goto fok;
		    }
		  break;
		case 'n':
		case 'N':
		  if (nancount == 0 && zeroes == 0
		      && (flags & (NDIGITS | DPTOK | EXPOK)) ==
				  (NDIGITS | DPTOK | EXPOK))
		    {
		      flags &= ~(SIGNOK | DPTOK | EXPOK | NDIGITS);
		      nancount = 1;
		      goto fok;
		    }
		  if (nancount == 2)
		    {
		      nancount = 3;
		      goto fok;
		    }
		  if (infcount == 1 || infcount == 4)
		    {
		      infcount++;
		      goto fok;
		    }
		  break;
		case 'a':
		case 'A':
		  if (nancount == 1)
		    {
		      nancount = 2;
		      goto fok;
		    }
		  break;
		case 'i':
		case 'I':
		  if (infcount == 0 && zeroes == 0
		      && (flags & (NDIGITS | DPTOK | EXPOK)) ==
				  (NDIGITS | DPTOK | EXPOK))
		    {
		      flags &= ~(SIGNOK | DPTOK | EXPOK | NDIGITS);
		      infcount = 1;
		      goto fok;
		    }
		  if (infcount == 3 || infcount == 5)
		    {
		      infcount++;
		      goto fok;
		    }
		  break;
		case 'f':
		case 'F':
		  if (infcount == 2)
		    {
		      infcount = 3;
		      goto fok;
		    }
		  break;
		case 't':
		case 'T':
		  if (infcount == 6)
		    {
		      infcount = 7;
		      goto fok;
		    }
		  break;
		case 'y':
		case 'Y':
		  if (infcount == 7)
		    {
		      infcount = 8;
		      goto fok;
		    }
		  break;
		case 'e':
		case 'E':
		  /* no exponent without some digits */
		  if ((flags & (NDIGITS | EXPOK)) == EXPOK
		      || ((flags & EXPOK) && zeroes))
		    {
		      if (! (flags & DPTOK))
			{
			  exp_adjust = zeroes - leading_zeroes;
			  exp_start = p;
			}
		      flags =
			(flags & ~(EXPOK | DPTOK)) |
			SIGNOK | NDIGITS;
		      zeroes = 0;
		      goto fok;
		    }
		  break;
		default:
#ifndef _MB_CAPABLE
		  if ((unsigned char) c == (unsigned char) decpt[0]
		      && (flags & DPTOK))
		    {
		      flags &= ~(SIGNOK | DPTOK);
		      leading_zeroes = zeroes;
		      goto fok;
		    }
		  break;
#else
		  if (flags & DPTOK)
		    {
		      while ((unsigned char) c
			     == (unsigned char) decpt[decptpos])
			{
			  if (decpt[++decptpos] == '\0')
			    {
			      /* We read the complete decpt seq. */
			      flags &= ~(SIGNOK | DPTOK);
			      leading_zeroes = zeroes;
			      p = stpncpy (p, decpt, decptpos);
			      decptpos = 0;
			      goto fskip;
			    }
			  ++nread;
			  if (--fp->_r > 0)
			    fp->_p++;
			  else if (__srefill_r (rptr, fp))
			    break;		/* EOF */
			  c = *fp->_p;
			}
		      if (decptpos > 0)
			{
			  /* We read part of a multibyte decimal point,
			     but the rest is invalid or we're at EOF,
			     so back off. */
			  while (decptpos-- > 0)
			    {
			      _ungetc_r (rptr, (unsigned char) decpt[decptpos],
					 fp);
			      --nread;
			    }
			}
		    }
		  break;
#endif
		}
	      break;
	    fok:
	      *p++ = c;
	    fskip:
	      width--;
	      ++nread;
	      if (--fp->_r > 0)
		fp->_p++;
	      else
	      if (__srefill_r (rptr, fp))
		break;		/* EOF */
	    }
	  if (zeroes)
	    flags &= ~NDIGITS;
	  /* We may have a 'N' or possibly even [sign] 'N' 'a' as the
	     start of 'NaN', only to run out of chars before it was
	     complete (or having encountered a non-matching char).  So
	     check here if we have an outstanding nancount, and if so
	     put back the chars we did swallow and treat as a failed
	     match.

	     FIXME - we still don't handle NAN([0xdigits]).  */
	  if (nancount - 1U < 2U) /* nancount && nancount < 3 */
	    {
	      /* Newlib's ungetc works even if we called __srefill in
		 the middle of a partial parse, but POSIX does not
		 guarantee that in all implementations of ungetc.  */
	      while (p > buf)
		{
		  _ungetc_r (rptr, *--p, fp); /* [-+nNaA] */
		  --nread;
		}
	      goto match_failure;
	    }
	  /* Likewise for 'inf' and 'infinity'.	 But be careful that
	     'infinite' consumes only 3 characters, leaving the stream
	     at the second 'i'.	 */
	  if (infcount - 1U < 7U) /* infcount && infcount < 8 */
	    {
	      if (infcount >= 3) /* valid 'inf', but short of 'infinity' */
		while (infcount-- > 3)
		  {
		    _ungetc_r (rptr, *--p, fp); /* [iInNtT] */
		    --nread;
		  }
	      else
		{
		  while (p > buf)
		    {
		      _ungetc_r (rptr, *--p, fp); /* [-+iInN] */
		      --nread;
		    }
		  goto match_failure;
		}
	    }
	  /*
	   * If no digits, might be missing exponent digits
	   * (just give back the exponent) or might be missing
	   * regular digits, but had sign and/or decimal point.
	   */
	  if (flags & NDIGITS)
	    {
	      if (flags & EXPOK)
		{
		  /* no digits at all */
		  while (p > buf)
		    {
		      _ungetc_r (rptr, *--p, fp); /* [-+.] */
		      --nread;
		    }
		  goto match_failure;
		}
	      /* just a bad exponent (e and maybe sign) */
	      c = *--p;
	      --nread;
	      if (c != 'e' && c != 'E')
		{
		  _ungetc_r (rptr, c, fp); /* [-+] */
		  c = *--p;
		  --nread;
		}
	      _ungetc_r (rptr, c, fp); /* [eE] */
	    }
	  if ((flags & SUPPRESS) == 0)
	    {
	      double res = 0;
#ifdef _NO_LONGDBL
#define QUAD_RES res;
#else  /* !_NO_LONG_DBL */
	      long double qres = 0;
#define QUAD_RES qres;
#endif /* !_NO_LONG_DBL */
	      long new_exp = 0;

	      *p = 0;
	      if ((flags & (DPTOK | EXPOK)) == EXPOK)
		{
		  exp_adjust = zeroes - leading_zeroes;
		  new_exp = -exp_adjust;
		  exp_start = p;
		}
	      else if (exp_adjust)
                new_exp = _strtol_r (rptr, (exp_start + 1), NULL, 10) - exp_adjust;
	      if (exp_adjust)
		{

		  /* If there might not be enough space for the new exponent,
		     truncate some trailing digits to make room.  */
		  if (exp_start >= buf + sizeof (buf) - MAX_LONG_LEN)
		    exp_start = buf + sizeof (buf) - MAX_LONG_LEN - 1;
                 sprintf (exp_start, "e%ld", new_exp);
		}

	      /* FIXME: Is that still true?
	         Current _strtold routine is markedly slower than
	         _strtod_r.  Only use it if we have a long double
	         result.  */
#ifndef _NO_LONGDBL /* !_NO_LONGDBL */
	      if (flags & LONGDBL)
		qres = _strtold_r (rptr, buf, NULL);
	      else
#endif
	        res = _strtod_r (rptr, buf, NULL);

	      if (flags & LONG)
		{
		  dp = GET_ARG (N, ap, double *);
		  *dp = res;
		}
	      else if (flags & LONGDBL)
		{
		  ldp = GET_ARG (N, ap, _LONG_DOUBLE *);
		  *ldp = QUAD_RES;
		}
	      else
		{
		  flp = GET_ARG (N, ap, float *);
		  if (isnan (res))
		    *flp = nanf (NULL);
		  else
		    *flp = res;
		}
	      nassigned++;
	    }
	  break;
	}
#endif /* FLOATING_POINT */
	}
    }
input_failure:
  /* On read failure, return EOF failure regardless of matches; errno
     should have been set prior to here.  On EOF failure (including
     invalid format string), return EOF if no matches yet, else number
     of matches made prior to failure.  */
  _newlib_flockfile_exit (fp);
  return nassigned && !(fp->_flags & __SERR) ? nassigned : EOF;
match_failure:
all_done:
  /* Return number of matches, which can be 0 on match failure.  */
  _newlib_flockfile_end (fp);
  return nassigned;
}

#ifndef _NO_POS_ARGS
/* Process all intermediate arguments.  Fortunately, with scanf, all
   intermediate arguments are sizeof(void*), so we don't need to scan
   ahead in the format string.  */
static void *
get_arg (int n, va_list *ap, int *numargs_p, void **args)
{
  int numargs = *numargs_p;
  while (n >= numargs)
    args[numargs++] = va_arg (*ap, void *);
  *numargs_p = numargs;
  return args[n];
}
#endif /* !_NO_POS_ARGS */

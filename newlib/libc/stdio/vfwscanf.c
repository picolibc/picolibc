/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * and/or other materials related to such
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
<<vfwscanf>>, <<vwscanf>>, <<vswscanf>>---scan and format argument list from wide character input

INDEX
	vfwscanf
INDEX
	_vfwscanf
INDEX
	vwscanf
INDEX
	_vwscanf
INDEX
	vswscanf
INDEX
	_vswscanf

SYNOPSIS
	#include <stdio.h>
	#include <stdarg.h>
	int vwscanf(const wchar_t *__restrict <[fmt]>, va_list <[list]>);
	int vfwscanf(FILE *__restrict <[fp]>,
                     const wchar_t *__restrict <[fmt]>, va_list <[list]>);
	int vswscanf(const wchar_t *__restrict <[str]>,
                     const wchar_t *__restrict <[fmt]>, va_list <[list]>);

	int _vwscanf(struct _reent *<[reent]>, const wchar_t *<[fmt]>,
                       va_list <[list]>);
	int _vfwscanf(struct _reent *<[reent]>, FILE *<[fp]>,
                      const wchar_t *<[fmt]>, va_list <[list]>);
	int _vswscanf(struct _reent *<[reent]>, const wchar_t *<[str]>,
                       const wchar_t *<[fmt]>, va_list <[list]>);

DESCRIPTION
<<vwscanf>>, <<vfwscanf>>, and <<vswscanf>> are (respectively) variants
of <<wscanf>>, <<fwscanf>>, and <<swscanf>>.  They differ only in
allowing their caller to pass the variable argument list as a
<<va_list>> object (initialized by <<va_start>>) rather than
directly accepting a variable number of arguments.

RETURNS
The return values are consistent with the corresponding functions:
<<vwscanf>> returns the number of input fields successfully scanned,
converted, and stored; the return value does not include scanned
fields which were not stored.

If <<vwscanf>> attempts to read at end-of-file, the return value
is <<EOF>>.

If no fields were stored, the return value is <<0>>.

The routines <<_vwscanf>>, <<_vfwscanf>>, and <<_vswscanf>> are
reentrant versions which take an additional first parameter which points
to the reentrancy structure.

PORTABILITY
C99, POSIX-1.2008
*/

#define _DEFAULT_SOURCE
#include <_ansi.h>
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

#ifdef INTEGER_ONLY
#define VFWSCANF vfiwscanf
#ifdef STRING_ONLY
#  define _SVFWSCANF _ssvfiwscanf
#else
#  define _SVFWSCANF _svfiwscanf
#endif
#else
#define VFWSCANF vfwscanf
#ifdef STRING_ONLY
#  define _SVFWSCANF _ssvfwscanf
#else
#  define _SVFWSCANF _svfwscanf
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
#define ungetwc sungetwc
#define _srefill _ssrefill
#define fgetwc sfgetwc
#endif

#ifdef FLOATING_POINT
#include <math.h>
#include <float.h>
#include <locale.h>
#ifdef __HAVE_LOCALE_INFO_EXTENDED__
#include "../locale/setlocale.h"
#endif

/* Currently a test is made to see if long double processing is warranted.
   This could be changed in the future should the __ldtoa code be
   preferred over __dtoa.  */
#define _NO_LONGDBL
#if defined _WANT_IO_LONG_DOUBLE && (LDBL_MANT_DIG > DBL_MANT_DIG)
#undef _NO_LONGDBL
#endif

#include "floatio.h"

#if ((MAXEXP+MAXFRACT+3) > MB_LEN_MAX)
#  define BUF (MAXEXP+MAXFRACT+3)        /* 3 = sign + decimal point + NUL */
#else
#  define BUF MB_LEN_MAX
#endif

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

typedef struct {
    va_list ap;
} my_va_list;

static void * get_arg (int, my_va_list *, int *, void **);
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
#define	MALLOC		0x80	/* handle 'm' modifier */

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
#define HAVESIGN        0x10000 /* sign detected */

/*
 * Conversion types.
 */

#define	CT_CHAR		0	/* %c conversion */
#define	CT_CCL		1	/* %[...] conversion */
#define	CT_STRING	2	/* %s conversion */
#define	CT_INT		3	/* integer, i.e., wcstol or wcstoul */
#define	CT_FLOAT	4	/* floating, i.e., wcstod */

#define INCCL(_c)       \
	(cclcompl ? (wmemchr(ccls, (_c), ccle - ccls) == NULL) : \
	(wmemchr(ccls, (_c), ccle - ccls) != NULL))

/*
 * vfwscanf
 */

#ifndef STRING_ONLY

int
VFWSCANF (
       register FILE *fp,
       const wchar_t *fmt,
       va_list ap)
{
  CHECK_INIT(data, fp);
  return _SVFWSCANF (fp, fmt, ap);
}
#endif /* !STRING_ONLY */

#ifdef STRING_ONLY
/* When dealing with the swscanf family, we don't want to use the
 * regular ungetwc which will drag in file I/O items we don't need.
 * So, we create our own trimmed-down version.  */
static wint_t
sungetwc (
	wint_t wc,
	register FILE *fp)
{
  if (wc == WEOF)
    return (WEOF);

  /* After ungetc, we won't be at eof anymore */
  fp->_flags &= ~__SEOF;

  /* All ungetwc usage in scanf un-gets the current character, so
   * just back up over the string if we aren't at the start
   */
  if (fp->_bf._base != NULL && fp->_p > fp->_bf._base)
    {
      fp->_p -= sizeof (wchar_t);
      fp->_r += sizeof (wchar_t);
    }

  return wc;
}

extern int _ssrefill ( register FILE * fp);

static size_t
sfgetwc (
       FILE * fp)
{
  wchar_t wc;

  if (fp->_r <= 0 && _ssrefill ( fp))
    return (WEOF);
  wc = *(wchar_t *) fp->_p;
  fp->_p += sizeof (wchar_t);
  fp->_r -= sizeof (wchar_t);
  return (wc);
}
#endif /* STRING_ONLY */

int
_SVFWSCANF (
       register FILE *fp,
       wchar_t const *fmt0,
       va_list ap)
{
  register wchar_t *fmt = (wchar_t *) fmt0;
  register wint_t c;            /* character from format, or conversion */
  register size_t width;	/* field width, or 0 */
  register wchar_t *p = NULL;	/* points into all kinds of strings */
  register int n;		/* handy integer */
  register int flags;		/* flags as defined above */
  register wchar_t *p0;		/* saves original value of p when necessary */
  int nassigned;		/* number of fields assigned */
  int nread;			/* number of characters consumed from fp */
#ifndef _NO_POS_ARGS
  int N;			/* arg number */
  int arg_index = 0;		/* index into args processed directly */
  int numargs = 0;		/* number of varargs read */
  my_va_list my_ap;
  va_copy(my_ap.ap, ap);
  void *args[MAX_POS_ARGS];	/* positional args read */
  int is_pos_arg;		/* is current format positional? */
#endif
  int base = 0;			/* base argument to wcstol/wcstoul */

  mbstate_t mbs;                /* value to keep track of multibyte state */

  #define CCFN_PARAMS	(const wchar_t *, wchar_t **, int)
  unsigned long (*ccfn)CCFN_PARAMS=0;	/* conversion function (wcstol/wcstoul) */
  wchar_t buf[BUF];		/* buffer for numeric conversions */
  const wchar_t *ccls;          /* character class start */
  const wchar_t *ccle;          /* character class end */
  int cclcompl = 0;             /* ccl is complemented? */
  wint_t wi;                    /* handy wint_t */
  char *mbp = NULL;             /* multibyte string pointer for %c %s %[ */
  size_t nconv;                 /* number of bytes in mb. conversion */
  char mbbuf[MB_LEN_MAX];	/* temporary mb. character buffer */

  char *cp;
  short *sp;
  int *ip;
#ifdef FLOATING_POINT
  float *flp;
  _LONG_DOUBLE *ldp;
  double *dp;
  wchar_t decpt;
#endif
  long *lp;
#ifndef _NO_LONGLONG
  long long *llp;
#endif
#ifdef _WANT_IO_C99_FORMATS
#define _WANT_IO_POSIX_EXTENSIONS
#endif
#ifdef _WANT_IO_POSIX_EXTENSIONS
  /* POSIX requires that fwscanf frees all allocated strings from 'm'
     conversions in case it returns EOF.  m_ptr is used to keep track.
     It will be allocated on the stack the first time an 'm' conversion
     takes place, and it will be free'd on return from the function.
     This implementation tries to save space by only allocating 8
     pointer slots at a time.  Most scenarios should never have to call
     realloc again.  This implementation allows only up to 65528 'm'
     conversions per fwscanf invocation for now.  That should be enough
     for almost all scenarios, right? */
  struct m_ptrs {
    void ***m_arr;		/* Array of pointer args to 'm' conversion */
    uint16_t m_siz;		/* Number of slots in m_arr */
    uint16_t m_cnt;		/* Number of valid entries in m_arr */
  } *m_ptr = NULL, m_store;
  #define init_m_ptr()							\
    do									\
      {									\
	if (!m_ptr)							\
	  {								\
	    m_ptr = &m_store;		                                \
	    m_ptr->m_arr = NULL;					\
	    m_ptr->m_siz = 0;						\
	    m_ptr->m_cnt = 0;						\
	  }								\
      }									\
    while (0)
  #define push_m_ptr(arg)						\
    do									\
      {									\
	if (m_ptr->m_cnt >= m_ptr->m_siz)				\
	  {								\
	    void ***n = NULL;						\
									\
	    if (m_ptr->m_siz + 8 > 0 && m_ptr->m_siz + 8 < UINT16_MAX)	\
	      n = (void ***) realloc (m_ptr->m_arr,			\
				      (m_ptr->m_siz + 8) *		\
				      sizeof (void **));		\
	    if (!n)							\
	      {								\
		nassigned = EOF;					\
		goto match_failure;					\
	      }								\
	    m_ptr->m_arr = n;						\
	    m_ptr->m_siz += 8;						\
	  }								\
	m_ptr->m_arr[m_ptr->m_cnt++] = (void **) (arg);				\
      }									\
    while (0)
  #define alloc_m_ptr(_type, _p, _p0, _p_p, _w)				\
    ({									\
      _p_p = GET_ARG (N, ap, _type **);					\
      if (!_p_p)							\
	goto match_failure;						\
      _p0 = (_type *) malloc ((_w) * sizeof (_type));			\
      if (!_p0)								\
	{								\
	  nassigned = EOF;						\
	  goto match_failure;						\
	}								\
      *_p_p = _p0;							\
      push_m_ptr (_p_p);						\
      _p = _p0;								\
      _w;								\
    })
  /* For char output, check if there's room for at least MB_CUR_MAX
     characters. */
  #define realloc_m_ptr(_type, _p, _p0, _p_p, _w)			\
    ({									\
      size_t _nw = (_w);						\
      ptrdiff_t _dif = _p - _p0;					\
      if (_p_p &&							\
	  ((sizeof (_type) == 1 && (size_t) _dif >= _nw - MB_CUR_MAX)   \
	   || (size_t) _dif >= _nw))                                    \
	{								\
	  _p0 = (_type *) realloc (_p0, (_nw << 1) * sizeof (_type));	\
	  if (!_p0)							\
	    {								\
	      nassigned = EOF;						\
	      goto match_failure;					\
	    }								\
	  _p = _p0 + _dif;						\
	  *_p_p = _p0;							\
	  _nw <<= 1;							\
	}								\
      _nw;								\
    })
  #define shrink_m_ptr(_type, _p_p, _w, _cw)				\
    ({									\
	size_t _nw = (_w);						\
	if (_p_p && _nw < _cw)						\
	  {								\
	    _type *_np_p = (_type *)					\
			   realloc (*_p_p, _nw * sizeof (_type));	\
	    if (_np_p)							\
	      *_p_p = _np_p;						\
	  }								\
    })
  #define free_m_ptr()							\
    do									\
      {									\
	if (m_ptr)							\
	  {								\
	    if (nassigned == EOF)					\
	      {								\
                unsigned i;                                             \
		for (i = 0; i < m_ptr->m_cnt; ++i)			\
		  {							\
		    free (*m_ptr->m_arr[i]);				\
		    *m_ptr->m_arr[i] = NULL;				\
		  }							\
	      }								\
	    if (m_ptr->m_arr)						\
	      free (m_ptr->m_arr);					\
	  }								\
      }									\
    while (0)
#endif

  /* `basefix' is used to avoid `if' tests in the integer scanner */
  static const short basefix[17] =
    {10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

  /* Macro to support positional arguments */
#ifndef _NO_POS_ARGS
# define GET_ARG(n, ap, type)					\
  ((type) (is_pos_arg						\
	   ? (n < numargs					\
	      ? args[n]						\
	      : get_arg (n, &my_ap, &numargs, args))      \
	   : (arg_index++ < numargs				\
	      ? args[n]						\
	      : (numargs < MAX_POS_ARGS				\
		 ? args[numargs++] = va_arg (my_ap.ap, void *)	\
		 : va_arg (my_ap.ap, void *)))))
#else
# define GET_ARG(n, ap, type) (va_arg (ap, type))
#endif

#ifdef FLOATING_POINT
#ifdef _MB_CAPABLE
#ifdef __HAVE_LOCALE_INFO_EXTENDED__
	  decpt = *__get_current_numeric_locale ()->wdecimal_point;
#else
	  {
	    size_t nconv;

	    memset (&mbs, '\0', sizeof (mbs));
	    nconv = mbrtowc (&decpt,
				localeconv ()->decimal_point,
				MB_CUR_MAX, &mbs);
	    if (nconv == (size_t) -1 || nconv == (size_t) -2)
	      decpt = L'.';
	  }
#endif /* !__HAVE_LOCALE_INFO_EXTENDED__ */
#else
	  decpt = (wchar_t) *localeconv ()->decimal_point;
#endif /* !_MB_CAPABLE */
#endif /* FLOATING_POINT */

  _newlib_flockfile_start (fp);

  if (ORIENT (fp, 1) != 1)
    {
      nassigned = EOF;
      goto all_done;
    }

  nassigned = 0;
  nread = 0;
  ccls = ccle = NULL;
  for (;;)
    {
      c = *fmt++;
      if (c == L'\0')
	goto all_done;
      if (iswspace (c))
	{
	  while ((c = fgetwc ( fp)) != WEOF && iswspace(c))
	    ;
	  if (c != WEOF)
	    ungetwc ( c, fp);
	  continue;
	}
      if (c != L'%')
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
	case L'%':
	literal:
	  if ((wi = fgetwc ( fp)) == WEOF)
	    goto input_failure;
	  if (wi != c)
	    {
	      ungetwc ( wi, fp);
	      goto input_failure;
	    }
	  nread++;
	  continue;

	case L'*':
	  if ((flags & (CHAR | SHORT | LONG | LONGDBL | SUPPRESS | MALLOC))
	      || width)
	    goto match_failure;
	  flags |= SUPPRESS;
	  goto again;
	case L'l':
	  if (flags & (CHAR | SHORT | LONG | LONGDBL))
	    goto match_failure;
#if defined _WANT_IO_C99_FORMATS || !defined _NO_LONGLONG
	  if (*fmt == L'l')	/* Check for 'll' = long long (SUSv3) */
	    {
	      ++fmt;
	      flags |= LONGDBL;
	    }
	  else
#endif
	    flags |= LONG;
	  goto again;
	case L'L':
	  if (flags & (CHAR | SHORT | LONG | LONGDBL))
	    goto match_failure;
	  flags |= LONGDBL;
	  goto again;
	case L'h':
#ifdef _WANT_IO_C99_FORMATS
	  if (flags & (CHAR | SHORT | LONG | LONGDBL))
	    goto match_failure;
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
	case L'j': /* intmax_t */
	  if (flags & (CHAR | SHORT | LONG | LONGDBL))
	    goto match_failure;
	  if (sizeof (intmax_t) == sizeof (long))
	    flags |= LONG;
	  else
	    flags |= LONGDBL;
	  goto again;
	case L't': /* ptrdiff_t */
	  if (flags & (CHAR | SHORT | LONG | LONGDBL))
	    goto match_failure;
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
	case L'z': /* size_t */
	  if (flags & (CHAR | SHORT | LONG | LONGDBL))
	    goto match_failure;
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
#ifdef _WANT_IO_POSIX_EXTENSIONS
	case 'm':
	  if (flags & (CHAR | SHORT | LONG | LONGDBL | MALLOC))
	    goto match_failure;
	  init_m_ptr ();
	  flags |= MALLOC;
	  goto again;
#endif

	case L'0':
	case L'1':
	case L'2':
	case L'3':
	case L'4':
	case L'5':
	case L'6':
	case L'7':
	case L'8':
	case L'9':
	  if (flags & (CHAR | SHORT | LONG | LONGDBL | MALLOC))
	    goto match_failure;
	  width = width * 10 + c - L'0';
	  goto again;

#ifndef _NO_POS_ARGS
	case L'$':
	  if (flags & (CHAR | SHORT | LONG | LONGDBL | SUPPRESS | MALLOC))
	    goto match_failure;
	  if (width <= MAX_POS_ARGS)
	    {
	      N = width - 1;
	      is_pos_arg = 1;
	      width = 0;
	      goto again;
	    }
	  _REENT_ERRNO(rptr) = EINVAL;
	  goto input_failure;
#endif /* !_NO_POS_ARGS */

	case L'd':
	  c = CT_INT;
	  ccfn = (unsigned long (*)CCFN_PARAMS)wcstol;
	  base = 10;
	  break;

	case L'i':
	  c = CT_INT;
	  ccfn = (unsigned long (*)CCFN_PARAMS)wcstol;
	  base = 0;
	  break;

	case L'o':
	  c = CT_INT;
	  ccfn = wcstoul;
	  base = 8;
	  break;

	case L'u':
	  c = CT_INT;
	  ccfn = wcstoul;
	  base = 10;
	  break;

	case L'X':
	case L'x':
	  flags |= PFXOK;	/* enable 0x prefixing */
	  c = CT_INT;
	  ccfn = wcstoul;
	  base = 16;
	  break;

#ifdef FLOATING_POINT
# ifdef _WANT_IO_C99_FORMATS
	case L'A':
	case L'a':
	case L'F':
# endif
	case L'E':
	case L'G':
	case L'e':
	case L'f':
	case L'g':
	  c = CT_FLOAT;
	  break;
#endif

#ifdef _WANT_IO_C99_FORMATS
	case L'S':
	  flags |= LONG;
          __PICOLIBC_FALLTHROUGH;
#endif

	case L's':
	  c = CT_STRING;
	  break;

	case L'[':
	  if (*fmt == '^')
	    {
	      cclcompl = 1;
	      ++fmt;
	    }
	  else
	    cclcompl = 0;
	  ccls = fmt;
	  if (*fmt == ']')
	    fmt++;
	  while (*fmt != '\0' && *fmt != ']')
	    fmt++;
	  ccle = fmt;
	  fmt++;
	  flags |= NOSKIP;
	  c = CT_CCL;
	  break;

#ifdef _WANT_IO_C99_FORMATS
	case 'C':
	  flags |= LONG;
          __PICOLIBC_FALLTHROUGH;
#endif

	case 'c':
	  flags |= NOSKIP;
	  c = CT_CHAR;
	  break;

	case 'p':		/* pointer format is like hex */
	  flags |= POINTER | PFXOK;
	  c = CT_INT;
	  ccfn = wcstoul;
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

	default:
	  goto match_failure;
	}

      /*
       * Consume leading white space, except for formats that
       * suppress this.
       */
      if ((flags & NOSKIP) == 0)
	{
	  while ((wi = fgetwc ( fp)) != WEOF && iswspace (wi))
	    nread++;
	  if (wi == WEOF)
	    goto input_failure;
	  ungetwc ( wi, fp);
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
          if (flags & LONG)
	    {
#ifdef _WANT_IO_POSIX_EXTENSIONS
	      wchar_t **p_p = NULL;
	      wchar_t *p0 = NULL;
	      size_t p_siz = 0;
#endif

	      if (flags & SUPPRESS)
		;
#ifdef _WANT_IO_POSIX_EXTENSIONS
	      else if (flags & MALLOC)
		p_siz = alloc_m_ptr (wchar_t, p, p0, p_p, 32);
#endif
	      else
		p = GET_ARG(N, ap, wchar_t *);
	      n = 0;
	      while (width-- != 0 && (wi = fgetwc ( fp)) != WEOF)
		{
		  if (!(flags & SUPPRESS))
		    {
#ifdef _WANT_IO_POSIX_EXTENSIONS
		      /* Check before ++ because we never add a \0 */
		      p_siz = realloc_m_ptr (wchar_t, p, p0, p_p, p_siz);
#endif
		      *p++ = (wchar_t) wi;
		    }
		  n++;
		}
	      if (n == 0)
		goto input_failure;
	      nread += n;
#ifdef _WANT_IO_POSIX_EXTENSIONS
	      shrink_m_ptr (wchar_t, p_p, p - p0, p_siz);
#endif
	      if (!(flags & SUPPRESS))
		nassigned++;
	    }
	  else
	    {
#ifdef _WANT_IO_POSIX_EXTENSIONS
	      char **mbp_p = NULL;
	      char *mbp0 = NULL;
	      size_t mbp_siz = 0;
#endif

	      if (flags & SUPPRESS)
		mbp = mbbuf;
#ifdef _WANT_IO_POSIX_EXTENSIONS
	      else if (flags & MALLOC)
		mbp_siz = alloc_m_ptr (char, mbp, mbp0, mbp_p, 32);
#endif
	      else
		mbp = GET_ARG(N, ap, char *);
	      n = 0;
	      memset ((void *)&mbs, '\0', sizeof (mbstate_t));
	      while (width != 0 && (wi = fgetwc ( fp)) != WEOF)
		{
		  nconv = wcrtomb (mbp, wi, &mbs);
		  if (nconv == (size_t) -1)
		    goto input_failure;
		  /* Ignore high surrogate in width counting */
		  if (nconv != 0 || mbs.__count != -4)
		    width--;
		  if (!(flags & SUPPRESS))
		    {
#ifdef _WANT_IO_POSIX_EXTENSIONS
		      mbp_siz = realloc_m_ptr (char, mbp, mbp0, mbp_p, mbp_siz);
#endif
		      mbp += nconv;
		    }
		  n++;
		}
	      if (n == 0)
		goto input_failure;
	      nread += n;
#ifdef _WANT_IO_POSIX_EXTENSIONS
	      shrink_m_ptr (char, mbp_p, mbp - mbp0, mbp_siz);
#endif
	      if (!(flags & SUPPRESS))
		nassigned++;
	    }
	  break;

	case CT_CCL:
	  /* scan a (nonempty) character class (sets NOSKIP) */
	  if (width == 0)
	    width = SIZE_MAX;		/* `infinity' */
	  /* take only those things in the class */
	  if ((flags & SUPPRESS) && (flags & LONG))
	    {
	      n = 0;
	      while ((wi = fgetwc ( fp)) != WEOF
		     && width-- != 0 && INCCL (wi))
		n++;
	      if (wi != WEOF)
		ungetwc ( wi, fp);
	      if (n == 0)
		goto match_failure;
	    }
	  else if (flags & LONG)
	    {
#ifdef _WANT_IO_POSIX_EXTENSIONS
	      wchar_t **p_p = NULL;
	      size_t p_siz = 0;

	      if (flags & MALLOC)
		p_siz = alloc_m_ptr (wchar_t, p, p0, p_p, 32);
	      else
#endif
		p0 = p = GET_ARG(N, ap, wchar_t *);
	      while ((wi = fgetwc ( fp)) != WEOF
		     && width-- != 0 && INCCL (wi))
		{
		  *p++ = (wchar_t) wi;
#ifdef _WANT_IO_POSIX_EXTENSIONS
		  p_siz = realloc_m_ptr (wchar_t, p, p0, p_p, p_siz);
#endif
		}
	      if (wi != WEOF)
		ungetwc ( wi, fp);
	      n = p - p0;
	      if (n == 0)
		goto match_failure;
	      *p = L'\0';
#ifdef _WANT_IO_POSIX_EXTENSIONS
	      shrink_m_ptr (wchar_t, p_p, n + 1, p_siz);
#endif
	      nassigned++;
	    }
	  else
	    {
#ifdef _WANT_IO_POSIX_EXTENSIONS
	      char **mbp_p = NULL;
	      char *mbp0 = NULL;
	      size_t mbp_siz = 0;
#endif

	      if (flags & SUPPRESS)
		mbp = mbbuf;
#ifdef _WANT_IO_POSIX_EXTENSIONS
	      else if (flags & MALLOC)
		mbp_siz = alloc_m_ptr (char, mbp, mbp0, mbp_p, 32);
#endif
	      else
		mbp = GET_ARG(N, ap, char *);
	      n = 0;
	      memset ((void *) &mbs, '\0', sizeof (mbstate_t));
	      while ((wi = fgetwc ( fp)) != WEOF
		     && width != 0 && INCCL (wi))
		{
		  nconv = wcrtomb (mbp, wi, &mbs);
		  if (nconv == (size_t) -1)
		    goto input_failure;
		  /* Ignore high surrogate in width counting */
		  if (nconv != 0 || mbs.__count != -4)
		    width--;
		  if (!(flags & SUPPRESS))
		    {
		      mbp += nconv;
#ifdef _WANT_IO_POSIX_EXTENSIONS
		      mbp_siz = realloc_m_ptr (char, mbp, mbp0, mbp_p, mbp_siz);
#endif
		    }
		  n++;
		}
	      if (wi != WEOF)
		ungetwc ( wi, fp);
	      if (!(flags & SUPPRESS))
		{
		  *mbp = 0;
#ifdef _WANT_IO_POSIX_EXTENSIONS
		  shrink_m_ptr (char, mbp_p, mbp - mbp0 + 1, mbp_siz);
#endif
		  nassigned++;
		}
	    }
	  nread += n;
	  break;

	case CT_STRING:
	  /* like CCL, but zero-length string OK, & no NOSKIP */
	  if (width == 0)
            width = SIZE_MAX;
	  if ((flags & SUPPRESS) && (flags & LONG))
	    {
	      while ((wi = fgetwc ( fp)) != WEOF
		     && width-- != 0 && !iswspace (wi))
		nread++;
	      if (wi != WEOF)
		ungetwc ( wi, fp);
	    }
	  else if (flags & LONG)
	    {
#ifdef _WANT_IO_POSIX_EXTENSIONS
              wchar_t **p_p = NULL;
              size_t p_siz = 0;

              if (flags & MALLOC)
                p_siz = alloc_m_ptr (wchar_t, p, p0, p_p, 32);
              else
#endif
		p0 = p = GET_ARG(N, ap, wchar_t *);
	      while ((wi = fgetwc ( fp)) != WEOF
		     && width-- != 0 && !iswspace (wi))
		{
		  *p++ = (wchar_t) wi;
#ifdef _WANT_IO_POSIX_EXTENSIONS
		  p_siz = realloc_m_ptr (wchar_t, p, p0, p_p, p_siz);
#endif
		  nread++;
		}
	      if (wi != WEOF)
		ungetwc ( wi, fp);
	      *p = L'\0';
#ifdef _WANT_IO_POSIX_EXTENSIONS
	      shrink_m_ptr (wchar_t, p_p, p - p0 + 1, p_siz);
#endif
	      nassigned++;
	    }
	  else
	    {
#ifdef _WANT_IO_POSIX_EXTENSIONS
	      char **mbp_p = NULL;
	      char *mbp0 = NULL;
	      size_t mbp_siz = 0;
#endif

	      if (flags & SUPPRESS)
		mbp = mbbuf;
#ifdef _WANT_IO_POSIX_EXTENSIONS
	      else if (flags & MALLOC)
		mbp_siz = alloc_m_ptr (char, mbp, mbp0, mbp_p, 32);
#endif
	      else
		mbp = GET_ARG(N, ap, char *);
	      memset ((void *) &mbs, '\0', sizeof (mbstate_t));
	      while ((wi = fgetwc ( fp)) != WEOF
		     && width != 0 && !iswspace (wi))
		{
		  nconv = wcrtomb(mbp, wi, &mbs);
		  if (nconv == (size_t)-1)
		    goto input_failure;
		  /* Ignore high surrogate in width counting */
		  if (nconv != 0 || mbs.__count != -4)
		    width--;
		  if (!(flags & SUPPRESS))
		    {
		      mbp += nconv;
#ifdef _WANT_IO_POSIX_EXTENSIONS
		      mbp_siz = realloc_m_ptr (char, mbp, mbp0, mbp_p, mbp_siz);
#endif
		    }
		  nread++;
		}
	      if (wi != WEOF)
		ungetwc ( wi, fp);
	      if (!(flags & SUPPRESS))
		{
		  *mbp = 0;
#ifdef _WANT_IO_POSIX_EXTENSIONS
		  shrink_m_ptr (char, mbp_p, mbp - mbp0 + 1, mbp_siz);
#endif
		  nassigned++;
		}
	    }
	  continue;

	case CT_INT:
	{
	  /* scan an integer as if by wcstol/wcstoul */
	  if (width == 0 || width > sizeof (buf) / sizeof (*buf) - 1)
	    width = sizeof(buf) / sizeof (*buf) - 1;
	  flags |= SIGNOK | NDIGITS | NZDIGITS;
	  for (p = buf; width; width--)
	    {
	      c = fgetwc ( fp);
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
		case L'0':
		  if (base == 0)
		    {
		      base = 8;
		      flags |= PFXOK;
		    }
		  if (flags & NZDIGITS)
		    flags &= ~(SIGNOK | NZDIGITS | NDIGITS);
		  else
		    flags &= ~(SIGNOK | PFXOK | NDIGITS);
		  goto ok;

		  /* 1 through 7 always legal */
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		  base = basefix[base];
		  flags &= ~(SIGNOK | PFXOK | NDIGITS);
		  goto ok;

		  /* digits 8 and 9 ok iff decimal or hex */
		case L'8':
		case L'9':
		  base = basefix[base];
		  if (base <= 8)
		    break;	/* not legal here */
		  flags &= ~(SIGNOK | PFXOK | NDIGITS);
		  goto ok;

		  /* letters ok iff hex */
		case L'A':
		case L'B':
		case L'C':
		case L'D':
		case L'E':
		case L'F':
		case L'a':
		case L'b':
		case L'c':
		case L'd':
		case L'e':
		case L'f':
		  /* no need to fix base here */
		  if (base <= 10)
		    break;	/* not legal here */
		  flags &= ~(SIGNOK | PFXOK | NDIGITS);
		  goto ok;

		  /* sign ok only as first character */
		case L'+':
		case L'-':
		  if (flags & SIGNOK)
		    {
		      flags &= ~SIGNOK;
		      flags |= HAVESIGN;
		      goto ok;
		    }
		  break;

		  /* x ok iff flag still set & single 0 seen */
		case L'x':
		case L'X':
		  if ((flags & PFXOK) && p == buf + 1 + !!(flags & HAVESIGN))
		    {
		      base = 16;/* if %i */
		      flags &= ~PFXOK;
		      goto ok;
		    }
		  break;
		}

	      /*
	       * If we got here, c is not a legal character
	       * for a number.  Stop accumulating digits.
	       */
	      if (c != WEOF)
		ungetwc ( c, fp);
	      break;
	    ok:
	      /*
	       * c is legal: store it and look at the next.
	       */
	      *p++ = (wchar_t) c;
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
		ungetwc ( *--p, fp); /* [-+xX] */
	      goto match_failure;
	    }
	  c = p[-1];
	  if (c == L'x' || c == L'X')
	    {
	      --p;
	      ungetwc ( c, fp);
	    }
	  if ((flags & SUPPRESS) == 0)
	    {
	      unsigned long res;

	      *p = 0;
	      res = (*ccfn) (buf, (wchar_t **) NULL, base);
	      if (flags & POINTER)
		{
		  void **vp = GET_ARG (N, ap, void **);
#ifndef _NO_LONGLONG
		  if (sizeof (uintptr_t) > sizeof (unsigned long))
		    {
		      unsigned long long resll;
		      resll = wcstoull (buf, (wchar_t **) NULL, base);
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
		  unsigned long long resll;
		  if (ccfn == wcstoul)
		    resll = wcstoull (buf, (wchar_t **) NULL, base);
		  else
		    resll = wcstoll (buf, (wchar_t **) NULL, base);
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
	  nread += p - buf;
	  break;
	}
#ifdef FLOATING_POINT
	case CT_FLOAT:
	{
	  /* scan a floating point number as if by wcstod */
	  /* This code used to assume that the number of digits is reasonable.
	     However, ANSI / ISO C makes no such stipulation; we have to get
	     exact results even when there is an unreasonable amount of
	     leading zeroes.  */
	  long leading_zeroes = 0;
	  long zeroes, exp_adjust;
	  wchar_t *exp_start = NULL;
	  unsigned width_left = 0;
	  char nancount = 0;
	  char infcount = 0;
#ifdef hardway
	  if (width == 0 || width > sizeof (buf) / sizeof (*buf) - 1)
#else
	  /* size_t is unsigned, hence this optimisation */
	  if (width - 1 > sizeof (buf) / sizeof (*buf) - 2)
#endif
	    {
	      width_left = width - (sizeof (buf) / sizeof (*buf) - 1);
	      width = sizeof (buf) / sizeof (*buf) - 1;
	    }
	  flags |= SIGNOK | NDIGITS | DPTOK | EXPOK;
	  zeroes = 0;
	  exp_adjust = 0;
	  for (p = buf; width; )
	    {
	      c = fgetwc ( fp);
	      /*
	       * This code mimicks the integer conversion
	       * code, but is much simpler.
	       */
	      switch (c)
		{
		case L'0':
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
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
		  if (nancount + infcount == 0)
		    {
		      flags &= ~(SIGNOK | NDIGITS);
		      goto fok;
		    }
		  break;

		case L'+':
		case L'-':
		  if (flags & SIGNOK)
		    {
		      flags &= ~SIGNOK;
		      goto fok;
		    }
		  break;
		case L'n':
		case L'N':
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
		case L'a':
		case L'A':
		  if (nancount == 1)
		    {
		      nancount = 2;
		      goto fok;
		    }
		  break;
		case L'i':
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
		case L'f':
		case L'F':
		  if (infcount == 2)
		    {
		      infcount = 3;
		      goto fok;
		    }
		  break;
		case L't':
		case L'T':
		  if (infcount == 6)
		    {
		      infcount = 7;
		      goto fok;
		    }
		  break;
		case L'y':
		case L'Y':
		  if (infcount == 7)
		    {
		      infcount = 8;
		      goto fok;
		    }
		  break;
		case L'e':
		case L'E':
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
		  if ((wchar_t) c == decpt && (flags & DPTOK))
		    {
		      flags &= ~(SIGNOK | DPTOK);
		      leading_zeroes = zeroes;
		      goto fok;
		    }
		  break;
		}
	      if (c != WEOF)
		ungetwc ( c, fp);
	      break;
	    fok:
	      *p++ = c;
	    fskip:
	      width--;
	      ++nread;
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
		  ungetwc ( *--p, fp); /* [-+nNaA] */
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
		    ungetwc ( *--p, fp); /* [iInNtT] */
		    --nread;
		  }
	      else
		{
		  while (p > buf)
		    {
		      ungetwc ( *--p, fp); /* [-+iInN] */
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
		      ungetwc ( *--p, fp); /* [-+.] */
		      --nread;
		    }
		  goto match_failure;
		}
	      /* just a bad exponent (e and maybe sign) */
	      c = *--p;
	      --nread;
	      if (c != L'e' && c != L'E')
		{
		  ungetwc ( c, fp); /* [-+] */
		  c = *--p;
		  --nread;
		}
	      ungetwc ( c, fp); /* [eE] */
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
                new_exp = wcstol ((exp_start + 1), NULL, 10) - exp_adjust;
	      if (exp_adjust)
		{

		  /* If there might not be enough space for the new exponent,
		     truncate some trailing digits to make room.  */
		  if (exp_start >= buf + sizeof (buf) / sizeof (*buf)
				   - MAX_LONG_LEN)
		    exp_start = buf + sizeof (buf) / sizeof (*buf)
				- MAX_LONG_LEN - 1;
                 swprintf (exp_start, MAX_LONG_LEN, L"e%ld", new_exp);
		}

	      /* FIXME: We don't have wcstold yet. */
#if 0//ndef _NO_LONGDBL /* !_NO_LONGDBL */
	      if (flags & LONGDBL)
		qres = wcstold (buf, NULL);
	      else
#endif
	        res = wcstod (buf, NULL);

	      if (flags & LONG)
		{
		  dp = GET_ARG (N, ap, double *);
		  *dp = res;
		}
	      else if (flags & LONGDBL)
		{
		  ldp = GET_ARG (N, ap, _LONG_DOUBLE *);
		  *ldp = (_LONG_DOUBLE) QUAD_RES;
		}
	      else
		{
		  flp = GET_ARG (N, ap, float *);
		  if (isnan (res))
		    *flp = nanf ("");
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
  nassigned = nassigned && !(fp->_flags & __SERR) ? nassigned : EOF;
match_failure:
all_done:
  /* Return number of matches, which can be 0 on match failure.  */
  _newlib_flockfile_end (fp);
#ifdef _WANT_IO_POSIX_EXTENSIONS
  free_m_ptr ();
#endif
  return nassigned;
}

#ifndef _NO_POS_ARGS
/* Process all intermediate arguments.  Fortunately, with wscanf, all
   intermediate arguments are sizeof(void*), so we don't need to scan
   ahead in the format string.  */
static void *
get_arg (int n, my_va_list *my_ap, int *numargs_p, void **args)
{
  int numargs = *numargs_p;
  while (n >= numargs)
    args[numargs++] = va_arg (my_ap->ap, void *);
  *numargs_p = numargs;
  return args[n];
}
#endif /* !_NO_POS_ARGS */

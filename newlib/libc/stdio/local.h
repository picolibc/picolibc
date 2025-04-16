/*
 * Copyright (c) 1990, 2007 The Regents of the University of California.
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
 *
 *	%W% (UofMD/Berkeley) %G%
 */

/*
 * Information local to this implementation of stdio,
 * in particular, macros and private variables.
 */

#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <wchar.h>
#ifdef __SCLE
# include <io.h>
#endif
#include "fvwrite.h"

/* The following macros are supposed to replace calls to _flockfile/_funlockfile
   and __sfp_lock_acquire/__sfp_lock_release.  In case of multi-threaded
   environments using pthreads, it's not sufficient to lock the stdio functions
   against concurrent threads accessing the same data, the locking must also be
   secured against thread cancellation.

   The below macros have to be used in pairs.  The _newlib_XXX_start macro
   starts with a opening curly brace, the _newlib_XXX_end macro ends with a
   closing curly brace, so the start macro and the end macro mark the code
   start and end of a critical section.  In case the code leaves the critical
   section before reaching the end of the critical section's code end, use
   the appropriate _newlib_XXX_exit macro. */

#if !defined (__SINGLE_THREAD) && defined (_POSIX_THREADS)
#define _STDIO_WITH_THREAD_CANCELLATION_SUPPORT
#endif

#if defined(__SINGLE_THREAD) || defined(__IMPL_UNLOCKED__)

# define _newlib_flockfile_start(_fp)
# define _newlib_flockfile_exit(_fp)
# define _newlib_flockfile_end(_fp)
# define _newlib_sfp_lock_start()
# define _newlib_sfp_lock_exit()
# define _newlib_sfp_lock_end()

#elif defined(_STDIO_WITH_THREAD_CANCELLATION_SUPPORT)
#include <pthread.h>

/* Start a stream oriented critical section: */
# define _newlib_flockfile_start(_fp) \
	{ \
	  int __oldfpcancel; \
	  pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &__oldfpcancel); \
	  if (!(_fp->_flags2 & __SNLK)) \
	    _flockfile (_fp)

/* Exit from a stream oriented critical section prematurely: */
# define _newlib_flockfile_exit(_fp) \
	  if (!(_fp->_flags2 & __SNLK)) \
	    _funlockfile (_fp); \
	  pthread_setcancelstate (__oldfpcancel, &__oldfpcancel);

/* End a stream oriented critical section: */
# define _newlib_flockfile_end(_fp) \
	  if (!(_fp->_flags2 & __SNLK)) \
	    _funlockfile (_fp); \
	  pthread_setcancelstate (__oldfpcancel, &__oldfpcancel); \
	}

/* Start a stream list oriented critical section: */
# define _newlib_sfp_lock_start() \
	{ \
	  int __oldsfpcancel; \
	  pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &__oldsfpcancel); \
	  __sfp_lock_acquire ()

/* Exit from a stream list oriented critical section prematurely: */
# define _newlib_sfp_lock_exit() \
	  __sfp_lock_release (); \
	  pthread_setcancelstate (__oldsfpcancel, &__oldsfpcancel);

/* End a stream list oriented critical section: */
# define _newlib_sfp_lock_end() \
	  __sfp_lock_release (); \
	  pthread_setcancelstate (__oldsfpcancel, &__oldsfpcancel); \
	}

#else

# define _newlib_flockfile_start(_fp) \
	{ \
		if (!(_fp->_flags2 & __SNLK)) \
		  _flockfile (_fp)

# define _newlib_flockfile_exit(_fp) \
		if (!(_fp->_flags2 & __SNLK)) \
		  _funlockfile(_fp); \

# define _newlib_flockfile_end(_fp) \
		if (!(_fp->_flags2 & __SNLK)) \
		  _funlockfile(_fp); \
	}

# define _newlib_sfp_lock_start() \
	{ \
		__sfp_lock_acquire ()

# define _newlib_sfp_lock_exit() \
		__sfp_lock_release ();

# define _newlib_sfp_lock_end() \
		__sfp_lock_release (); \
	}

#endif

extern wint_t __fgetwc (FILE *);
extern wint_t __fputwc (wchar_t, FILE *);
extern unsigned char *__sccl (char *, unsigned char *fmt);
extern int    _svfscanf (FILE *, const char *,va_list);
extern int    _ssvfscanf (FILE *, const char *,va_list);
extern int    _svfiscanf (FILE *, const char *,va_list);
extern int    _ssvfiscanf (FILE *, const char *,va_list);
extern int    _svfwscanf (FILE *, const wchar_t *,va_list);
extern int    _ssvfwscanf (FILE *, const wchar_t *,va_list);
extern int    _svfiwscanf (FILE *, const wchar_t *,va_list);
extern int    _ssvfiwscanf (FILE *, const wchar_t *,va_list);
int	      svfprintf ( FILE *, const char *,
				  va_list)
                    __picolibc_format(__printf__, 2, 0);
int	      svfiprintf ( FILE *, const char *,
				  va_list)
                    __picolibc_format(__printf__, 2, 0);
int	      svfwprintf ( FILE *, const wchar_t *,
				  va_list);
int	      svfiwprintf ( FILE *, const wchar_t *,
				  va_list);
extern FILE  *__sfp (void);
extern int    __sflags (const char*, int*);
extern int    _sflush (FILE *);
#ifdef _STDIO_BSD_SEMANTICS
extern int    _sflushw (FILE *);
#endif
extern int    _srefill (FILE *);
extern ssize_t __sread (void *, char *,
					       size_t);
extern ssize_t __seofread (void *,
						  char *,
						  size_t);
extern ssize_t __swrite (void *,
						const char *,
						size_t);
extern _fpos_t __sseek (void *, _fpos_t, int);
extern int    __sclose (void *);
extern int    __stextmode (int);
extern void   __sinit (void);
extern void   _smakebuf ( FILE *);
extern int    _swhatbuf ( FILE *, size_t *, int *);
extern int __submore (FILE *);

extern int __sprint (FILE *, register struct __suio *);
extern int __ssprint (FILE *, register struct __suio *);
extern int __ssputs (FILE *fp, const char *buf, size_t len);
extern int __ssputws (FILE *fp,	const wchar_t *buf, size_t len);
extern int __sfputs (FILE *, const char *buf, size_t len);
extern int __sfputws (FILE *, const wchar_t *buf, size_t len);
extern int sungetc (int c, register FILE *fp);
extern int _ssrefill (register FILE * fp);
extern size_t _sfread (void *buf, size_t size, size_t count, FILE * fp);

#ifdef __LARGE64_FILES
extern _fpos64_t __sseek64 (void *, _fpos64_t, int);
extern ssize_t __swrite64 (void *,
						  const char *,
						  size_t);
#endif

extern void (*_stdio_cleanup)(void);

/* Called by the main entry point fns to ensure stdio has been initialized.  */

#define CHECK_INIT() \
  do								\
    {								\
      if (!_stdio_cleanup)			                \
	__sinit ();				                \
    }								\
  while (0)

/* Return true and set errno and stream error flag iff the given FILE
   cannot be written now.  */

#define	cantwrite(ptr, fp)                                     \
  ((((fp)->_flags & __SWR) == 0 || (fp)->_bf._base == NULL) && \
   _swsetup( fp))

/* Test whether the given stdio file has an active ungetc buffer;
   release such a buffer, without restoring ordinary unread data.  */

#define	HASUB(fp) ((fp)->_ub._base != NULL)
#define	FREEUB(ptr, fp) {                    \
	if ((fp)->_ub._base != (fp)->_ubuf) \
		free((char *)(fp)->_ub._base); \
	(fp)->_ub._base = NULL; \
}

/* Test for an fgetline() buffer.  */

#define	HASLB(fp) ((fp)->_lb._base != NULL)
#define	FREELB(ptr, fp) { free((char *)(fp)->_lb._base);	\
      (fp)->_lb._base = NULL; }

#ifdef __WIDE_ORIENT
/*
 * Set the orientation for a stream. If o > 0, the stream has wide-
 * orientation. If o < 0, the stream has byte-orientation.
 */
#if defined(__GNUCLIKE_PRAGMA_DIAGNOSTIC) && !defined(__clang__)
#pragma GCC diagnostic ignored "-Wunused-value"
#endif
#define ORIENT(fp,ori)			\
  (					\
    (					\
      ((fp)->_flags & __SORD) ?		\
	0				\
      :					\
	(				\
	  ((fp)->_flags |= __SORD),	\
	  (ori > 0) ?			\
	    ((fp)->_flags2 |= __SWID)	\
	  :				\
	    ((fp)->_flags2 &= ~__SWID)	\
	)				\
    ),					\
    ((fp)->_flags2 & __SWID) ? 1 : -1	\
  )
#else
#define ORIENT(fp,ori) (-1)
#endif

/* Same thing as the functions in stdio.h, but these are to be called
   from inside the wide-char functions. */
int	__swbufw (int, FILE *);
#ifdef __GNUC__
__elidable_inline int __swputc(int _c, FILE *_p) {
#ifdef __SCLE
	if ((_p->_flags & __SCLE) && _c == '\n')
	  __swputc ('\r', _p);
#endif
	if (--_p->_w >= 0 || (_p->_w >= _p->_lbfsize && (char)_c != '\n'))
		return (*_p->_p++ = _c);
	else
		return (__swbufw(_c, _p));
}
#else
#define       __swputc_raw(__c, __p) \
	(--(__p)->_w < 0 ? \
		(__p)->_w >= (__p)->_lbfsize ? \
			(*(__p)->_p = (__c)), *(__p)->_p != '\n' ? \
				(int)*(__p)->_p++ : \
				__swbufw('\n', __p) : \
			__swbufw((int)(__c), __p) : \
		(*(__p)->_p = (__c), (int)*(__p)->_p++))
#ifdef __SCLE
#define __swputc(__c, __p) \
        ((((__p)->_flags & __SCLE) && ((__c) == '\n')) \
          ? __swputc_raw('\r', (__p)) : 0 , \
        __swputc_raw((__c), (__p)))
#else
#define __swputc(__c, __p) __swputc_raw(__c, __p)
#endif
#endif

/* WARNING: _dcvt is defined in the stdlib directory, not here!  */

char *_dcvt (char *, double, int, int, char, int);
char *_sicvt (char *, short, char);
char *_icvt (char *, int, char);
char *_licvt (char *, long, char);
#ifdef __GNUC__
char *_llicvt (char *, long long, char);
#endif

#define CVT_BUF_SIZE 128

#define	NDYNAMIC 4	/* add four more whenever necessary */

#define __sfp_lock_acquire() __LIBC_LOCK()
#define __sfp_lock_release() __LIBC_UNLOCK()
#define __sinit_lock_acquire() __LIBC_LOCK()
#define __sinit_lock_release() __LIBC_UNLOCK()

/* Types used in positional argument support in vfprinf/vfwprintf.
   The implementation is char/wchar_t dependent but the class and state
   tables are only defined once in vfprintf.c. */
typedef enum __packed {
  ZERO,   /* '0' */
  DIGIT,  /* '1-9' */
  DOLLAR, /* '$' */
  MODFR,  /* spec modifier */
  SPEC,   /* format specifier */
  DOT,    /* '.' */
  STAR,   /* '*' */
  FLAG,   /* format flag */
  OTHER,  /* all other chars */
  MAX_CH_CLASS /* place-holder */
} __CH_CLASS;

typedef enum __packed {
  START,  /* start */
  SFLAG,  /* seen a flag */
  WDIG,   /* seen digits in width area */
  WIDTH,  /* processed width */
  SMOD,   /* seen spec modifier */
  SDOT,   /* seen dot */
  VARW,   /* have variable width specifier */
  VARP,   /* have variable precision specifier */
  PREC,   /* processed precision */
  VWDIG,  /* have digits in variable width specification */
  VPDIG,  /* have digits in variable precision specification */
  DONE,   /* done */
  MAX_STATE, /* place-holder */
} __STATE;

typedef enum __packed {
  NOOP,  /* do nothing */
  NUMBER, /* build a number from digits */
  SKIPNUM, /* skip over digits */
  GETMOD,  /* get and process format modifier */
  GETARG,  /* get and process argument */
  GETPW,   /* get variable precision or width */
  GETPWB,  /* get variable precision or width and pushback fmt char */
  GETPOS,  /* get positional parameter value */
  PWPOS,   /* get positional parameter value for variable width or precision */
} __ACTION;

extern const __CH_CLASS __chclass[256];
extern const __STATE __state_table[MAX_STATE][MAX_CH_CLASS];
extern const __ACTION __action_table[MAX_STATE][MAX_CH_CLASS];

#if defined(__NANO_FORMATTED_IO) && defined(__strong_reference)
#define __nano_reference(a,b) __strong_reference(a,b)
#else
#define __nano_reference(a,b)
#endif

#if defined(__LONG_DOUBLE_128__) && defined(__strong_reference)
#if defined(__GNUCLIKE_PRAGMA_DIAGNOSTIC) && !defined(__clang__)
#pragma GCC diagnostic ignored "-Wmissing-attributes"
#endif
#define __ieee128_reference(a,b) __strong_reference(a,b)
#else
#define __ieee128_reference(a,b)
#endif

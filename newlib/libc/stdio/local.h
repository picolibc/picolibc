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

#include <_ansi.h>
#include <reent.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#ifdef __SCLE
# include <io.h>
#endif

/* The following define determines if the per-reent stdin, stdout and stderr
   streams are closed during _reclaim_reent().  The stdin, stdout and stderr
   streams are initialized to use file descriptors 0, 1 and 2 respectively.  In
   case _STDIO_CLOSE_PER_REENT_STD_STREAMS is defined these file descriptors
   will be closed via close() provided the owner of the reent structure
   triggerd the on demand reent initilization, see CHECK_INIT(). */
#if !defined(__tirtos__)
#define _STDIO_CLOSE_PER_REENT_STD_STREAMS
#endif

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

#if !defined (__SINGLE_THREAD__) && defined (_POSIX_THREADS) \
    && !defined (__rtems__)
#define _STDIO_WITH_THREAD_CANCELLATION_SUPPORT
#endif

#if defined(__SINGLE_THREAD__) || defined(__IMPL_UNLOCKED__)

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

#else /* !__SINGLE_THREAD__ && !__IMPL_UNLOCKED__ && !_STDIO_WITH_THREAD_CANCELLATION_SUPPORT */

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

#endif /* __SINGLE_THREAD__ || __IMPL_UNLOCKED__ */

extern wint_t __fgetwc (struct _reent *, FILE *);
extern wint_t __fputwc (struct _reent *, wchar_t, FILE *);
extern u_char *__sccl (char *, u_char *fmt);
extern int    __svfscanf_r (struct _reent *,FILE *, const char *,va_list);
extern int    __ssvfscanf_r (struct _reent *,FILE *, const char *,va_list);
extern int    __svfiscanf_r (struct _reent *,FILE *, const char *,va_list);
extern int    __ssvfiscanf_r (struct _reent *,FILE *, const char *,va_list);
extern int    __svfwscanf_r (struct _reent *,FILE *, const wchar_t *,va_list);
extern int    __ssvfwscanf_r (struct _reent *,FILE *, const wchar_t *,va_list);
extern int    __svfiwscanf_r (struct _reent *,FILE *, const wchar_t *,va_list);
extern int    __ssvfiwscanf_r (struct _reent *,FILE *, const wchar_t *,va_list);
int	      _svfprintf_r (struct _reent *, FILE *, const char *, 
				  va_list)
               			_ATTRIBUTE ((__format__ (__printf__, 3, 0)));
int	      _svfiprintf_r (struct _reent *, FILE *, const char *, 
				  va_list)
               			_ATTRIBUTE ((__format__ (__printf__, 3, 0)));
int	      _svfwprintf_r (struct _reent *, FILE *, const wchar_t *, 
				  va_list);
int	      _svfiwprintf_r (struct _reent *, FILE *, const wchar_t *, 
				  va_list);
extern FILE  *__sfp (struct _reent *);
extern int    __sflags (struct _reent *,const char*, int*);
extern int    __sflush_r (struct _reent *,FILE *);
#ifdef _STDIO_BSD_SEMANTICS
extern int    __sflushw_r (struct _reent *,FILE *);
#endif
extern int    __srefill_r (struct _reent *,FILE *);
extern _READ_WRITE_RETURN_TYPE __sread (struct _reent *, void *, char *,
					       _READ_WRITE_BUFSIZE_TYPE);
extern _READ_WRITE_RETURN_TYPE __seofread (struct _reent *, void *,
						  char *,
						  _READ_WRITE_BUFSIZE_TYPE);
extern _READ_WRITE_RETURN_TYPE __swrite (struct _reent *, void *,
						const char *,
						_READ_WRITE_BUFSIZE_TYPE);
extern _fpos_t __sseek (struct _reent *, void *, _fpos_t, int);
extern int    __sclose (struct _reent *, void *);
extern int    __stextmode (int);
extern void   __sinit (struct _reent *);
extern void   __smakebuf_r (struct _reent *, FILE *);
extern int    __swhatbuf_r (struct _reent *, FILE *, size_t *, int *);
extern int    _fwalk_reent (struct _reent *, int (*)(struct _reent *, FILE *));
extern int __submore (struct _reent *, FILE *);

#ifdef __LARGE64_FILES
extern _fpos64_t __sseek64 (struct _reent *, void *, _fpos64_t, int);
extern _READ_WRITE_RETURN_TYPE __swrite64 (struct _reent *, void *,
						  const char *,
						  _READ_WRITE_BUFSIZE_TYPE);
#endif

/* Called by the main entry point fns to ensure stdio has been initialized.  */

#if defined(_REENT_SMALL) && !defined(_REENT_GLOBAL_STDIO_STREAMS)
#define CHECK_INIT(ptr, fp) \
  do								\
    {								\
      struct _reent *_check_init_ptr = (ptr);			\
      if ((_check_init_ptr) && !(_check_init_ptr)->__cleanup)	\
	__sinit (_check_init_ptr);				\
      if ((fp) == (FILE *)&__sf_fake_stdin)			\
	(fp) = _stdin_r(_check_init_ptr);			\
      else if ((fp) == (FILE *)&__sf_fake_stdout)		\
	(fp) = _stdout_r(_check_init_ptr);			\
      else if ((fp) == (FILE *)&__sf_fake_stderr)		\
	(fp) = _stderr_r(_check_init_ptr);			\
    }								\
  while (0)
#else /* !_REENT_SMALL || _REENT_GLOBAL_STDIO_STREAMS */
#define CHECK_INIT(ptr, fp) \
  do								\
    {								\
      struct _reent *_check_init_ptr = (ptr);			\
      if ((_check_init_ptr) && !(_check_init_ptr)->__cleanup)	\
	__sinit (_check_init_ptr);				\
    }								\
  while (0)
#endif /* !_REENT_SMALL || _REENT_GLOBAL_STDIO_STREAMS */

/* Return true and set errno and stream error flag iff the given FILE
   cannot be written now.  */

#define	cantwrite(ptr, fp)                                     \
  ((((fp)->_flags & __SWR) == 0 || (fp)->_bf._base == NULL) && \
   __swsetup_r(ptr, fp))

/* Test whether the given stdio file has an active ungetc buffer;
   release such a buffer, without restoring ordinary unread data.  */

#define	HASUB(fp) ((fp)->_ub._base != NULL)
#define	FREEUB(ptr, fp) {                    \
	if ((fp)->_ub._base != (fp)->_ubuf) \
		_free_r(ptr, (char *)(fp)->_ub._base); \
	(fp)->_ub._base = NULL; \
}

/* Test for an fgetline() buffer.  */

#define	HASLB(fp) ((fp)->_lb._base != NULL)
#define	FREELB(ptr, fp) { _free_r(ptr,(char *)(fp)->_lb._base); \
      (fp)->_lb._base = NULL; }

#ifdef _WIDE_ORIENT
/*
 * Set the orientation for a stream. If o > 0, the stream has wide-
 * orientation. If o < 0, the stream has byte-orientation.
 */
#define ORIENT(fp,ori)					\
  do								\
    {								\
      if (!((fp)->_flags & __SORD))	\
	{							\
	  (fp)->_flags |= __SORD;				\
	  if (ori > 0)						\
	    (fp)->_flags2 |= __SWID;				\
	  else							\
	    (fp)->_flags2 &= ~__SWID;				\
	}							\
    }								\
  while (0)
#else
#define ORIENT(fp,ori)
#endif

/* WARNING: _dcvt is defined in the stdlib directory, not here!  */

char *_dcvt (struct _reent *, char *, double, int, int, char, int);
char *_sicvt (char *, short, char);
char *_icvt (char *, int, char);
char *_licvt (char *, long, char);
#ifdef __GNUC__
char *_llicvt (char *, long long, char);
#endif

#define CVT_BUF_SIZE 128

#define	NDYNAMIC 4	/* add four more whenever necessary */

#ifdef __SINGLE_THREAD__
#define __sfp_lock_acquire()
#define __sfp_lock_release()
#define __sinit_lock_acquire()
#define __sinit_lock_release()
#else
void __sfp_lock_acquire (void);
void __sfp_lock_release (void);
void __sinit_lock_acquire (void);
void __sinit_lock_release (void);
#endif

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

/* This header file provides the reentrancy.  */

/* WARNING: All identifiers here must begin with an underscore.  This file is
   included by stdio.h and others and we therefore must only use identifiers
   in the namespace allotted to us.  */

#ifndef _SYS_REENT_H_
#ifdef __cplusplus
extern "C" {
#endif
#define _SYS_REENT_H_

#include <_ansi.h>
#include <sys/_types.h>

#ifndef __Long
#if __LONG_MAX__ == 2147483647L
#define __Long long
typedef unsigned __Long __ULong;
#elif __INT_MAX__ == 2147483647
#define __Long int
typedef unsigned __Long __ULong;
#endif
#endif

#ifndef __Long
#define __Long __int32_t
typedef __uint32_t __ULong;
#endif

/*
 * If _REENT_SMALL is defined, we make struct _reent as small as possible,
 * by having nearly everything possible allocated at first use.
 */

struct _glue 
{
  struct _glue *_next;
  int _niobs;
  struct __sFILE *_iobs;
};

struct _Bigint 
{
  struct _Bigint *_next;
  int _k, _maxwds, _sign, _wds;
  __ULong _x[1];
};

/* needed by reentrant structure */
struct __tm
{
  int   __tm_sec;
  int   __tm_min;
  int   __tm_hour;
  int   __tm_mday;
  int   __tm_mon;
  int   __tm_year;
  int   __tm_wday;
  int   __tm_yday;
  int   __tm_isdst;
};

/*
 * atexit() support.  For _REENT_SMALL, we limit to 32 max.
 */

#define	_ATEXIT_SIZE 32	/* must be at least 32 to guarantee ANSI conformance */

#ifndef _REENT_SMALL
struct _atexit {
	struct	_atexit *_next;			/* next in list */
	int	_ind;				/* next index in this table */
	void	(*_fns[_ATEXIT_SIZE])(void);	/* the table itself */
};
#else
struct _atexit {
	int	_ind;				/* next index in this table */
	void	(*_fns[_ATEXIT_SIZE])(void);	/* the table itself */
};
#endif

/*
 * Stdio buffers.
 *
 * This and __sFILE are defined here because we need them for struct _reent,
 * but we don't want stdio.h included when stdlib.h is.
 */

struct __sbuf {
	unsigned char *_base;
	int	_size;
};

/*
 * We need fpos_t for the following, but it doesn't have a leading "_",
 * so we use _fpos_t instead.
 */

typedef long _fpos_t;		/* XXX must match off_t in <sys/types.h> */
				/* (and must be `long' for now) */

/*
 * Stdio state variables.
 *
 * The following always hold:
 *
 *	if (_flags&(__SLBF|__SWR)) == (__SLBF|__SWR),
 *		_lbfsize is -_bf._size, else _lbfsize is 0
 *	if _flags&__SRD, _w is 0
 *	if _flags&__SWR, _r is 0
 *
 * This ensures that the getc and putc macros (or inline functions) never
 * try to write or read from a file that is in `read' or `write' mode.
 * (Moreover, they can, and do, automatically switch from read mode to
 * write mode, and back, on "r+" and "w+" files.)
 *
 * _lbfsize is used only to make the inline line-buffered output stream
 * code as compact as possible.
 *
 * _ub, _up, and _ur are used when ungetc() pushes back more characters
 * than fit in the current _bf, or when ungetc() pushes back a character
 * that does not match the previous one in _bf.  When this happens,
 * _ub._base becomes non-nil (i.e., a stream has ungetc() data iff
 * _ub._base!=NULL) and _up and _ur save the current values of _p and _r.
 */

#ifdef _REENT_SMALL
/*
 * struct __sFILE_fake is the start of a struct __sFILE, with only the
 * minimal fields allocated.  In __sinit() we really allocate the 3
 * standard streams, etc., and point away from this fake.
 */
struct __sFILE_fake {
  unsigned char *_p;	/* current position in (some) buffer */
  int	_r;		/* read space left for getc() */
  int	_w;		/* write space left for putc() */
  short	_flags;		/* flags, below; this FILE is free if 0 */
  short	_file;		/* fileno, if Unix descriptor, else -1 */
  struct __sbuf _bf;	/* the buffer (at least 1 byte, if !NULL) */
  int	_lbfsize;	/* 0 or -_bf._size, for inline putc */

  struct _reent *_data;
};
/* CHECK_INIT() comes from stdio/local.h; be sure to include that.  */
# define _REENT_SMALL_CHECK_INIT(fp) CHECK_INIT(fp)
#else
# define _REENT_SMALL_CHECK_INIT(fp) /* nothing */
#endif

struct __sFILE {
  unsigned char *_p;	/* current position in (some) buffer */
  int	_r;		/* read space left for getc() */
  int	_w;		/* write space left for putc() */
  short	_flags;		/* flags, below; this FILE is free if 0 */
  short	_file;		/* fileno, if Unix descriptor, else -1 */
  struct __sbuf _bf;	/* the buffer (at least 1 byte, if !NULL) */
  int	_lbfsize;	/* 0 or -_bf._size, for inline putc */

#ifdef _REENT_SMALL
  struct _reent *_data;
#endif

  /* operations */
  _PTR	_cookie;	/* cookie passed to io functions */

  _READ_WRITE_RETURN_TYPE _EXFUN((*_read),(_PTR _cookie, char *_buf, int _n));
  _READ_WRITE_RETURN_TYPE _EXFUN((*_write),(_PTR _cookie, const char *_buf,
					    int _n));
  _fpos_t _EXFUN((*_seek),(_PTR _cookie, _fpos_t _offset, int _whence));
  int	_EXFUN((*_close),(_PTR _cookie));

  /* separate buffer for long sequences of ungetc() */
  struct __sbuf _ub;	/* ungetc buffer */
  unsigned char *_up;	/* saved _p when _p is doing ungetc data */
  int	_ur;		/* saved _r when _r is counting ungetc data */

  /* tricks to meet minimum requirements even when malloc() fails */
  unsigned char _ubuf[3];	/* guarantee an ungetc() buffer */
  unsigned char _nbuf[1];	/* guarantee a getc() buffer */

  /* separate buffer for fgetline() when line crosses buffer boundary */
  struct __sbuf _lb;	/* buffer for fgetline() */

  /* Unix stdio files get aligned to block boundaries on fseek() */
  int	_blksize;	/* stat.st_blksize (may be != _bf._size) */
  int	_offset;	/* current lseek offset */

#ifndef _REENT_SMALL
  struct _reent *_data;		/* Here for binary compatibility? Remove? */
#endif
};

/*
 * rand48 family support
 *
 * Copyright (c) 1993 Martin Birgmeier
 * All rights reserved.
 *
 * You may redistribute unmodified or modified versions of this source
 * code provided that the above copyright notice and this and the
 * following conditions are retained.
 *
 * This software is provided ``as is'', and comes with no warranties
 * of any kind. I shall in no event be liable for anything that happens
 * to anyone/anything when using this software.
 */
#define        _RAND48_SEED_0  (0x330e)
#define        _RAND48_SEED_1  (0xabcd)
#define        _RAND48_SEED_2  (0x1234)
#define        _RAND48_MULT_0  (0xe66d)
#define        _RAND48_MULT_1  (0xdeec)
#define        _RAND48_MULT_2  (0x0005)
#define        _RAND48_ADD     (0x000b)
struct _rand48 {
  unsigned short _seed[3];
  unsigned short _mult[3];
  unsigned short _add;
#ifdef _REENT_SMALL
  /* Put this in here as well, for good luck.  */
  __extension__ unsigned long long _rand_next;
#endif
};

/* How big the some arrays are.  */
#define _REENT_EMERGENCY_SIZE 25
#define _REENT_ASCTIME_SIZE 26

/*
 * struct _reent
 *
 * This structure contains *all* globals needed by the library.
 * It's raison d'etre is to facilitate threads by making all library routines
 * reentrant.  IE: All state information is contained here.
 */

#ifdef _REENT_SMALL

struct _mprec
{
  /* used by mprec routines */
  struct _Bigint *_result;
  int _result_k;
  struct _Bigint *_p5s;
  struct _Bigint **_freelist;
};


struct _misc_reent
{
  /* miscellaneous reentrant data */
  char *_strtok_last;
  int _mblen_state;
  int _wctomb_state;
  int _mbtowc_state;
};

/* This version of _reent is layed our with "int"s in pairs, to help
 * ports with 16-bit int's but 32-bit pointers, align nicely.  */
struct _reent
{

  /* FILE is a big struct and may change over time.  To try to achieve binary
     compatibility with future versions, put stdin,stdout,stderr here.
     These are pointers into member __sf defined below.  */
  struct __sFILE *_stdin, *_stdout, *_stderr;	/* XXX */

  int _errno;			/* local copy of errno */

  int  _inc;			/* used by tmpnam */

  char *_emergency;
 
  int __sdidinit;		/* 1 means stdio has been init'd */

  int _current_category;	/* used by setlocale */
  _CONST char *_current_locale;

  struct _mprec *_mp;

  void _EXFUN((*__cleanup),(struct _reent *));

  int _gamma_signgam;

  /* used by some fp conversion routines */
  int _cvtlen;			/* should be size_t */
  char *_cvtbuf;

  struct _rand48 *_r48;
  struct __tm *_localtime_buf;
  char *_asctime_buf;

  /* signal info */
  void (**(_sig_func))(int);

  /* atexit stuff */
  struct _atexit _atexit;

  struct _glue __sglue;			/* root of glue chain */
  struct __sFILE *__sf;			/* file descriptors */
  struct __sFILE_fake __sf_fake;	/* fake initial stdin/out/err */
  struct _misc_reent *_misc;            /* strtok, multibyte states */
};

#define _REENT_INIT(var) \
  { &var.__sf_fake, &var.__sf_fake, &var.__sf_fake, 0, 0, _NULL, 0, 0, \
    "C", _NULL, _NULL, 0, 0, _NULL, _NULL, _NULL, _NULL, _NULL, \
    { 0, _NULL }, { _NULL, 0, _NULL }, 0, _NULL }

#define _REENT_INIT_PTR(var) \
  { var->_stdin = &var->__sf_fake; \
    var->_stdout = &var->__sf_fake; \
    var->_stderr = &var->__sf_fake; \
    var->_errno = 0; \
    var->_inc = 0; \
    var->_emergency = _NULL; \
    var->__sdidinit = 0; \
    var->_current_category = 0; \
    var->_current_locale = "C"; \
    var->_mp = _NULL; \
    var->__cleanup = _NULL; \
    var->_gamma_signgam = 0; \
    var->_cvtlen = 0; \
    var->_cvtbuf = _NULL; \
    var->_r48 = _NULL; \
    var->_localtime_buf = _NULL; \
    var->_asctime_buf = _NULL; \
    var->_sig_func = _NULL; \
    var->_atexit._ind = 0; \
    var->_atexit._fns = _NULL}; \
    var->__sglue._next = _NULL; \
    var->__sglue._niobs = 0; \
    var->__sglue._iobs = _NULL; \
    var->__sf = 0; \
    var->_misc = _NULL; \
  }

  /* signal info */
  void (**(_sig_func))(int);
/* Only built the assert() calls if we are built with debugging.  */
#if DEBUG 
#include <assert.h>
#else
#define assert(x) ((void)0)
#endif

/* Generic _REENT check macro.  */
#define _REENT_CHECK(var, what, type, size, init) do { \
  struct _reent *_r = (var); \
  if (_r->what == NULL) { \
    _r->what = (type)malloc(size); \
    assert(_r->what); \
    init; \
  } \
} while (0)

#define _REENT_CHECK_TM(var) \
  _REENT_CHECK(var, _localtime_buf, struct __tm *, sizeof *((var)->_localtime_buf), \
    /* nothing */)

#define _REENT_CHECK_ASCTIME_BUF(var) \
  _REENT_CHECK(var, _asctime_buf, char *, _REENT_ASCTIME_SIZE, \
    memset((var)->_asctime_buf, 0, _REENT_ASCTIME_SIZE))

/* Handle the dynamically allocated rand48 structure. */
#define _REENT_INIT_RAND48(var) do { \
  struct _reent *_r = (var); \
  _r->_r48->_seed[0] = _RAND48_SEED_0; \
  _r->_r48->_seed[1] = _RAND48_SEED_1; \
  _r->_r48->_seed[2] = _RAND48_SEED_2; \
  _r->_r48->_mult[0] = _RAND48_MULT_0; \
  _r->_r48->_mult[1] = _RAND48_MULT_1; \
  _r->_r48->_mult[2] = _RAND48_MULT_2; \
  _r->_r48->_add = _RAND48_ADD; \
} while (0)
#define _REENT_CHECK_RAND48(var) \
  _REENT_CHECK(var, _r48, struct _rand48 *, sizeof *((var)->_r48), _REENT_INIT_RAND48((var)))

#define _REENT_INIT_MP(var) do { \
  struct _reent *_r = (var); \
  _r->_mp->_result_k = 0; \
  _r->_mp->_result = _r->_mp->_p5s = _NULL; \
  _r->_mp->_freelist = _NULL; \
} while (0)
#define _REENT_CHECK_MP(var) \
  _REENT_CHECK(var, _mp, struct _mprec *, sizeof *((var)->_mp), _REENT_INIT_MP(var))

#define _REENT_CHECK_EMERGENCY(var) \
  _REENT_CHECK(var, _emergency, char *, _REENT_EMERGENCY_SIZE, /* nothing */)

#define _REENT_INIT_MISC(var) do { \
  struct _reent *_r = (var); \
  _r->_misc->_strtok_last = _NULL; \
  _r->_misc->_mblen_state = 0; \
  _r->_misc->_wctomb_state = 0; \
  _r->_misc->_mbtowc_state = 0; \
} while (0)
#define _REENT_CHECK_MISC(var) \
  _REENT_CHECK(var, _misc, struct _misc_reent *, sizeof *((var)->_misc), _REENT_INIT_MISC(var))

#define _REENT_SIGNGAM(ptr)	((ptr)->_gamma_signgam)
#define _REENT_RAND_NEXT(ptr)	((ptr)->_r48->_rand_next)
#define _REENT_RAND48_SEED(ptr)	((ptr)->_r48->_seed)
#define _REENT_RAND48_MULT(ptr)	((ptr)->_r48->_mult)
#define _REENT_RAND48_ADD(ptr)	((ptr)->_r48->_add)
#define _REENT_MP_RESULT(ptr)	((ptr)->_mp->_result)
#define _REENT_MP_RESULT_K(ptr)	((ptr)->_mp->_result_k)
#define _REENT_MP_P5S(ptr)	((ptr)->_mp->_p5s)
#define _REENT_MP_FREELIST(ptr)	((ptr)->_mp->_freelist)
#define _REENT_ASCTIME_BUF(ptr)	((ptr)->_asctime_buf)
#define _REENT_TM(ptr)		((ptr)->_localtime_buf)
#define _REENT_EMERGENCY(ptr)	((ptr)->_emergency)
#define _REENT_STRTOK_LAST(ptr)	((ptr)->_misc->_strtok_last)
#define _REENT_MBLEN_STATE(ptr)	((ptr)->_misc->_mblen_state)
#define _REENT_MBTOWC_STATE(ptr)((ptr)->_misc->_mbtowc_state)
#define _REENT_WCTOMB_STATE(ptr)((ptr)->_misc->_wctomb_state)

#else /* !_REENT_SMALL */

struct _reent
{
  int _errno;			/* local copy of errno */

  /* FILE is a big struct and may change over time.  To try to achieve binary
     compatibility with future versions, put stdin,stdout,stderr here.
     These are pointers into member __sf defined below.  */
  struct __sFILE *_stdin, *_stdout, *_stderr;

  int  _inc;			/* used by tmpnam */
  char _emergency[_REENT_EMERGENCY_SIZE];
 
  int _current_category;	/* used by setlocale */
  _CONST char *_current_locale;

  int __sdidinit;		/* 1 means stdio has been init'd */

  void _EXFUN((*__cleanup),(struct _reent *));

  /* used by mprec routines */
  struct _Bigint *_result;
  int _result_k;
  struct _Bigint *_p5s;
  struct _Bigint **_freelist;

  /* used by some fp conversion routines */
  int _cvtlen;			/* should be size_t */
  char *_cvtbuf;

  union
    {
      struct
        {
          unsigned int _unused_rand;
          char * _strtok_last;
          char _asctime_buf[26];
          struct __tm _localtime_buf;
          int _gamma_signgam;
          __extension__ unsigned long long _rand_next;
          struct _rand48 _r48;
          int _mblen_state;
          int _mbtowc_state;
          int _wctomb_state;
        } _reent;
  /* Two next two fields were once used by malloc.  They are no longer
     used. They are used to preserve the space used before so as to
     allow addition of new reent fields and keep binary compatibility.   */ 
      struct
        {
#define _N_LISTS 30
          unsigned char * _nextf[_N_LISTS];
          unsigned int _nmalloc[_N_LISTS];
        } _unused;
    } _new;

  /* atexit stuff */
  struct _atexit *_atexit;	/* points to head of LIFO stack */
  struct _atexit _atexit0;	/* one guaranteed table, required by ANSI */

  /* signal info */
  void (**(_sig_func))(int);

  /* These are here last so that __sFILE can grow without changing the offsets
     of the above members (on the off chance that future binary compatibility
     would be broken otherwise).  */
  struct _glue __sglue;			/* root of glue chain */
  struct __sFILE __sf[3];		/* first three file descriptors */
};

#define _REENT_INIT(var) \
  { 0, &var.__sf[0], &var.__sf[1], &var.__sf[2], 0, "", 0, "C", \
    0, _NULL, _NULL, 0, _NULL, _NULL, 0, _NULL, { {0, _NULL, "", \
    { 0,0,0,0,0,0,0,0}, 0, 1, \
    {{_RAND48_SEED_0, _RAND48_SEED_1, _RAND48_SEED_2}, \
     {_RAND48_MULT_0, _RAND48_MULT_1, _RAND48_MULT_2}, _RAND48_ADD}, \
    0, 0, 0} } }

#define _REENT_INIT_PTR(var) \
  { int i; \
    char *tmp_ptr; \
    var->_errno = 0; \
    var->_stdin = &var->__sf[0]; \
    var->_stdout = &var->__sf[1]; \
    var->_stderr = &var->__sf[2]; \
    var->_inc = 0; \
    for (i = 0; i < _REENT_EMERGENCY_SIZE; ++i) \
      var->_emergency[i] = 0; \
    var->_current_category = 0; \
    var->_current_locale = "C"; \
    var->__sdidinit = 0; \
    var->__cleanup = _NULL; \
    var->_result = _NULL; \
    var->_result_k = 0; \
    var->_p5s = _NULL; \
    var->_freelist = _NULL; \
    var->_cvtlen = 0; \
    var->_cvtbuf = _NULL; \
    var->_new._reent._unused_rand = 0; \
    var->_new._reent._strtok_last = _NULL; \
    var->_new._reent._asctime_buf[0] = 0; \
    tmp_ptr = (char *)&var->_new._reent._localtime_buf; \
    for (i = 0; i < sizeof(struct __tm); ++i) \
      tmp_ptr[i] = 0; \
    var->_new._reent._gamma_signgam = 0; \
    var->_new._reent._rand_next = 1; \
    var->_new._reent._r48._seed[0] = _RAND48_SEED_0; \
    var->_new._reent._r48._seed[1] = _RAND48_SEED_1; \
    var->_new._reent._r48._seed[2] = _RAND48_SEED_2; \
    var->_new._reent._r48._mult[0] = _RAND48_MULT_0; \
    var->_new._reent._r48._mult[1] = _RAND48_MULT_1; \
    var->_new._reent._r48._mult[2] = _RAND48_MULT_2; \
    var->_new._reent._r48._add = _RAND48_ADD; \
    var->_new._reent._mblen_state = 0; \
    var->_new._reent._mbtowc_state = 0; \
    var->_new._reent._wctomb_state = 0; \
  }

#define _REENT_CHECK_RAND48(ptr)	/* nothing */
#define _REENT_CHECK_MP(ptr)		/* nothing */
#define _REENT_CHECK_TM(ptr)		/* nothing */
#define _REENT_CHECK_ASCTIME_BUF(ptr)	/* nothing */
#define _REENT_CHECK_EMERGENCY(ptr)	/* nothing */
#define _REENT_CHECK_MISC(ptr)	        /* nothing */

#define _REENT_SIGNGAM(ptr)	((ptr)->_new._reent._gamma_signgam)
#define _REENT_RAND_NEXT(ptr)	((ptr)->_new._reent._rand_next)
#define _REENT_RAND48_SEED(ptr)	((ptr)->_new._reent._r48._seed)
#define _REENT_RAND48_MULT(ptr)	((ptr)->_new._reent._r48._mult)
#define _REENT_RAND48_ADD(ptr)	((ptr)->_new._reent._r48._add)
#define _REENT_MP_RESULT(ptr)	((ptr)->_result)
#define _REENT_MP_RESULT_K(ptr)	((ptr)->_result_k)
#define _REENT_MP_P5S(ptr)	((ptr)->_p5s)
#define _REENT_MP_FREELIST(ptr)	((ptr)->_freelist)
#define _REENT_ASCTIME_BUF(ptr)	(&(ptr)->_new._reent._asctime_buf)
#define _REENT_TM(ptr)		(&(ptr)->_new._reent._localtime_buf)
#define _REENT_EMERGENCY(ptr)	((ptr)->_emergency)
#define _REENT_STRTOK_LAST(ptr)	((ptr)->_new._reent._strtok_last)
#define _REENT_MBLEN_STATE(ptr)	((ptr)->_new._reent._mblen_state)
#define _REENT_MBTOWC_STATE(ptr)((ptr)->_new._reent._mbtowc_state)
#define _REENT_WCTOMB_STATE(ptr)((ptr)->_new._reent._wctomb_state)

#endif /* !_REENT_SMALL */

#define _NULL 0

/*
 * All references to struct _reent are via this pointer.
 * Internally, newlib routines that need to reference it should use _REENT.
 */

#ifndef __ATTRIBUTE_IMPURE_PTR__
#define __ATTRIBUTE_IMPURE_PTR__
#endif

extern struct _reent *_impure_ptr __ATTRIBUTE_IMPURE_PTR__;

void _reclaim_reent _PARAMS ((struct _reent *));

/* #define _REENT_ONLY define this to get only reentrant routines */

#ifndef _REENT_ONLY
#define _REENT _impure_ptr
#endif /* !_REENT_ONLY */

#ifdef __cplusplus
}
#endif
#endif /* _SYS_REENT_H_ */

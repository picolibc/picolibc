/*
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
 *
 *	@(#)stdio.h	5.3 (Berkeley) 3/15/86
 */

/*
 * NB: to fit things in six character monocase externals, the
 * stdio code uses the prefix `__s' for stdio objects, typically
 * followed by a three-character attempt at a mnemonic.
 */

#ifndef _STDIO_H_
#define	_STDIO_H_

#include "_ansi.h"

#define	_FSTDIO			/* ``function stdio'' */

#define __need_size_t
#define __need_ssize_t
#define __need_NULL
#include <sys/cdefs.h>
#include <stddef.h>

#include <stdarg.h>
#include <_ansi.h>
#include <sys/_types.h>

#ifndef __machine_flock_t_defined
#include <sys/lock.h>
typedef _LOCK_RECURSIVE_T _flock_t;
#endif

/*
 * Stdio buffers.
 */

struct __sbuf {
	unsigned char *_base;
	int	_size;
};

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

#define _REENT_SMALL_CHECK_INIT(ptr) /* nothing */

struct __sFILE {
  unsigned char *_p;	/* current position in (some) buffer */
  int	_r;		/* read space left for getc() */
  int	_w;		/* write space left for putc() */
  short	_flags;		/* flags, below; this FILE is free if 0 */
  short	_file;		/* fileno, if Unix descriptor, else -1 */
  struct __sbuf _bf;	/* the buffer (at least 1 byte, if !NULL) */
  int	_lbfsize;	/* 0 or -_bf._size, for inline putc */

  /* operations */
  void *	_cookie;	/* cookie passed to io functions */

  _ssize_t (*_read) (void *,
					   char *, size_t);
  _ssize_t (*_write) (void *,
					    const char *,
					    size_t);
  _fpos_t (*_seek) (void *, _fpos_t, int);
  int (*_close) (void *);

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
  _off_t _offset;	/* current lseek offset */

#ifndef __SINGLE_THREAD__
  _flock_t _lock;	/* for thread-safety locking */
#endif
  _mbstate_t _mbstate;	/* for wide char stdio functions. */
  int   _flags2;        /* for future use */
};

#ifdef __CUSTOM_FILE_IO__

/* Get custom _FILE definition.  */
#include <sys/custom_file.h>

#else /* !__CUSTOM_FILE_IO__ */
#ifdef __LARGE64_FILES
struct __sFILE64 {
  unsigned char *_p;	/* current position in (some) buffer */
  int	_r;		/* read space left for getc() */
  int	_w;		/* write space left for putc() */
  short	_flags;		/* flags, below; this FILE is free if 0 */
  short	_file;		/* fileno, if Unix descriptor, else -1 */
  struct __sbuf _bf;	/* the buffer (at least 1 byte, if !NULL) */
  int	_lbfsize;	/* 0 or -_bf._size, for inline putc */

  /* operations */
  void *	_cookie;	/* cookie passed to io functions */

  _ssize_t (*_read) (void *,
					   char *, size_t);
  _ssize_t (*_write) (void *,
					    const char *,
					    size_t);
  _fpos_t (*_seek) (void *, _fpos_t, int);
  int (*_close) (void *);

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
  int   _flags2;        /* for future use */

  _off64_t _offset;     /* current lseek offset */
  _fpos64_t (*_seek64) (void *, _fpos64_t, int);

#ifndef __SINGLE_THREAD__
  _flock_t _lock;	/* for thread-safety locking */
#endif
  _mbstate_t _mbstate;	/* for wide char stdio functions. */
};
typedef struct __sFILE64 __FILE;
#else
typedef struct __sFILE   __FILE;
#endif /* __LARGE64_FILES */
#endif /* !__CUSTOM_FILE_IO__ */

extern __FILE __sf[3];

extern NEWLIB_THREAD_LOCAL __FILE *_tls_stdin;
#define _REENT_STDIN(_ptr) (_tls_stdin)
extern NEWLIB_THREAD_LOCAL __FILE *_tls_stdout;
#define _REENT_STDOUT(_ptr) (_tls_stdout)
extern NEWLIB_THREAD_LOCAL __FILE *_tls_stderr;
#define _REENT_STDERR(_ptr) (_tls_stderr)

extern void (*__stdio_exit_handler) (void);

struct _glue
{
  struct _glue *_next;
  int _niobs;
  __FILE *_iobs;
};

extern struct _glue __sglue;

extern int _fwalk_sglue (int (*)(__FILE *), struct _glue *);

#include <sys/_types.h>

_BEGIN_STD_C

#if !defined(__FILE_defined)
typedef __FILE FILE;
# define __FILE_defined
#endif

typedef _fpos_t fpos_t;
#ifdef __LARGE64_FILES
typedef _fpos64_t fpos64_t;
#endif

#ifndef _OFF_T_DECLARED
typedef __off_t off_t;
#define	_OFF_T_DECLARED
#endif

#ifndef _SSIZE_T_DECLARED
typedef _ssize_t ssize_t;
#define	_SSIZE_T_DECLARED
#endif

#include <sys/stdio.h>

#define	__SLBF	0x0001		/* line buffered */
#define	__SNBF	0x0002		/* unbuffered */
#define	__SRD	0x0004		/* OK to read */
#define	__SWR	0x0008		/* OK to write */
	/* RD and WR are never simultaneously asserted */
#define	__SRW	0x0010		/* open for reading & writing */
#define	__SEOF	0x0020		/* found EOF */
#define	__SERR	0x0040		/* found error */
#define	__SMBF	0x0080		/* _buf is from malloc */
#define	__SAPP	0x0100		/* fdopen()ed in append mode - so must  write to end */
#define	__SSTR	0x0200		/* this is an sprintf/snprintf string */
#define	__SOPT	0x0400		/* do fseek() optimisation */
#define	__SNPT	0x0800		/* do not do fseek() optimisation */
#define	__SOFF	0x1000		/* set iff _offset is in fact correct */
#define	__SORD	0x2000		/* true => stream orientation (byte/wide) decided */
#if defined(__CYGWIN__)
#  define __SCLE  0x4000        /* convert line endings CR/LF <-> NL */
#endif
#define	__SL64	0x8000		/* is 64-bit offset large file */

/* _flags2 flags */
#define	__SNLK  0x0001		/* stdio functions do not lock streams themselves */
#define	__SWID	0x2000		/* true => stream orientation wide, false => byte, only valid if __SORD in _flags is true */

/*
 * The following three definitions are for ANSI C, which took them
 * from System V, which stupidly took internal interface macros and
 * made them official arguments to setvbuf(), without renaming them.
 * Hence, these ugly _IOxxx names are *supposed* to appear in user code.
 *
 * Although these happen to match their counterparts above, the
 * implementation does not rely on that (so these could be renumbered).
 */
#define	_IOFBF	0		/* setvbuf should set fully buffered */
#define	_IOLBF	1		/* setvbuf should set line buffered */
#define	_IONBF	2		/* setvbuf should set unbuffered */

#define	EOF	(-1)

#ifdef __BUFSIZ__
#define	BUFSIZ		__BUFSIZ__
#else
#define	BUFSIZ		1024
#endif

#ifdef __FOPEN_MAX__
#define FOPEN_MAX	__FOPEN_MAX__
#else
#define	FOPEN_MAX	20
#endif

#ifdef __FILENAME_MAX__
#define FILENAME_MAX    __FILENAME_MAX__
#else
#define	FILENAME_MAX	1024
#endif

#ifdef __L_tmpnam__
#define L_tmpnam	__L_tmpnam__
#else
#define	L_tmpnam	FILENAME_MAX
#endif

#if __BSD_VISIBLE || __XSI_VISIBLE
#define P_tmpdir        "/tmp"
#endif

#ifndef SEEK_SET
#define	SEEK_SET	0	/* set file offset to offset */
#endif
#ifndef SEEK_CUR
#define	SEEK_CUR	1	/* set file offset to current plus offset */
#endif
#ifndef SEEK_END
#define	SEEK_END	2	/* set file offset to EOF plus offset */
#endif

#define	TMP_MAX		26

#define	stdin	_REENT_STDIN(_REENT)
#define	stdout	_REENT_STDOUT(_REENT)
#define	stderr	_REENT_STDERR(_REENT)

#define _stdin_r(x)	_REENT_STDIN(x)
#define _stdout_r(x)	_REENT_STDOUT(x)
#define _stderr_r(x)	_REENT_STDERR(x)

/*
 * Functions defined in ANSI C standard.
 */

#ifndef __VALIST
#ifdef __GNUC__
#define __VALIST __gnuc_va_list
#else
#define __VALIST char*
#endif
#endif

#if __POSIX_VISIBLE
char *	ctermid (char *);
#endif
#if __GNU_VISIBLE || (__XSI_VISIBLE && __XSI_VISIBLE < 600)
char *	cuserid (char *);
#endif
FILE *	tmpfile (void);
char *	tmpnam (char *);
#if __BSD_VISIBLE || __XSI_VISIBLE || __POSIX_VISIBLE >= 200112
void	free (void *) _NOTHROW;
char *	tempnam (const char *, const char *) __malloc_like __result_use_check;
#endif
int	fclose (FILE *);
int	fflush (FILE *);
FILE *	freopen (const char *__restrict, const char *__restrict, FILE *__restrict);
void	setbuf (FILE *__restrict, char *__restrict);
int	setvbuf (FILE *__restrict, char *__restrict, int, size_t);
int	fprintf (FILE *__restrict, const char *__restrict, ...)
               _ATTRIBUTE ((__format__ (__printf__, 2, 3)));
int	fscanf (FILE *__restrict, const char *__restrict, ...)
               _ATTRIBUTE ((__format__ (__scanf__, 2, 3)));
int	printf (const char *__restrict, ...)
               _ATTRIBUTE ((__format__ (__printf__, 1, 2)));
int	scanf (const char *__restrict, ...)
               _ATTRIBUTE ((__format__ (__scanf__, 1, 2)));
int	sscanf (const char *__restrict, const char *__restrict, ...)
               _ATTRIBUTE ((__format__ (__scanf__, 2, 3)));
int	vfprintf (FILE *__restrict, const char *__restrict, va_list)
               _ATTRIBUTE ((__format__ (__printf__, 2, 0)));
int	vprintf (const char *, va_list)
               _ATTRIBUTE ((__format__ (__printf__, 1, 0)));
int	vsprintf (char *__restrict, const char *__restrict, va_list)
               _ATTRIBUTE ((__format__ (__printf__, 2, 0)));
int	fgetc (FILE *);
char *  fgets (char *__restrict, int, FILE *__restrict);
int	fputc (int, FILE *);
int	fputs (const char *__restrict, FILE *__restrict);
int	getc (FILE *);
int	getchar (void);
char *  gets (char *);
int	putc (int, FILE *);
int	putchar (int);
int	puts (const char *);
int	ungetc (int, FILE *);
size_t	fread (void *__restrict, size_t _size, size_t _n, FILE *__restrict);
size_t	fwrite (const void *__restrict , size_t _size, size_t _n, FILE *);
#ifdef _LIBC
int	fgetpos (FILE *, _fpos_t *);
#else
int	fgetpos (FILE *__restrict, fpos_t *__restrict);
#endif
int	fseek (FILE *, long, int);
#ifdef _LIBC
int	fsetpos (FILE *, const _fpos_t *);
#else
int	fsetpos (FILE *, const fpos_t *);
#endif
long	ftell ( FILE *);
void	rewind (FILE *);
void	clearerr (FILE *);
int	feof (FILE *);
int	ferror (FILE *);
void    perror (const char *);
#ifndef _REENT_ONLY
FILE *	fopen (const char *__restrict _name, const char *__restrict _type);
int	sprintf (char *__restrict, const char *__restrict, ...)
               _ATTRIBUTE ((__format__ (__printf__, 2, 3)));
int	remove (const char *);
int	rename (const char *, const char *);
#ifdef _LIBC
int	_rename (const char *, const char *);
#endif
#endif
#if __LARGEFILE_VISIBLE || __POSIX_VISIBLE >= 200112
#ifdef _LIBC
int	fseeko (FILE *, _off_t, int);
_off_t	ftello (FILE *);
#else
int	fseeko (FILE *, off_t, int);
off_t	ftello (FILE *);
#endif
#endif
#if __GNU_VISIBLE
int	fcloseall (void);
#endif
#ifndef _REENT_ONLY
#if __ISO_C_VISIBLE >= 1999
int	snprintf (char *__restrict, size_t, const char *__restrict, ...)
               _ATTRIBUTE ((__format__ (__printf__, 3, 4)));
int	vsnprintf (char *__restrict, size_t, const char *__restrict, va_list)
               _ATTRIBUTE ((__format__ (__printf__, 3, 0)));
int	vfscanf (FILE *__restrict, const char *__restrict, va_list)
               _ATTRIBUTE ((__format__ (__scanf__, 2, 0)));
int	vscanf (const char *, va_list)
               _ATTRIBUTE ((__format__ (__scanf__, 1, 0)));
int	vsscanf (const char *__restrict, const char *__restrict, va_list)
               _ATTRIBUTE ((__format__ (__scanf__, 2, 0)));
#endif
#if __GNU_VISIBLE
int	asprintf (char **__restrict, const char *__restrict, ...)
               _ATTRIBUTE ((__format__ (__printf__, 2, 3)));
int	vasprintf (char **, const char *, va_list)
               _ATTRIBUTE ((__format__ (__printf__, 2, 0)));
#endif
#if __MISC_VISIBLE /* Newlib-specific */
int	asiprintf (char **, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 2, 3)));
char *	asniprintf (char *, size_t *, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 3, 4)));
char *	asnprintf (char *__restrict, size_t *__restrict, const char *__restrict, ...)
               _ATTRIBUTE ((__format__ (__printf__, 3, 4)));
#ifndef diprintf
int	diprintf (int, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 2, 3)));
#endif
int	fiprintf (FILE *, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 2, 3)));
#define __i_fprintf fiprintf
int	fiscanf (FILE *, const char *, ...)
               _ATTRIBUTE ((__format__ (__scanf__, 2, 3)));
int	iprintf (const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 1, 2)));
int	iscanf (const char *, ...)
               _ATTRIBUTE ((__format__ (__scanf__, 1, 2)));
int	siprintf (char *, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 2, 3)));
#define __i_sprintf siprintf
int	siscanf (const char *, const char *, ...)
               _ATTRIBUTE ((__format__ (__scanf__, 2, 3)));
int	sniprintf (char *, size_t, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 3, 4)));
#define  __i_snprintf sniprintf
int	vasiprintf (char **, const char *, va_list)
               _ATTRIBUTE ((__format__ (__printf__, 2, 0)));
char *	vasniprintf (char *, size_t *, const char *, va_list)
               _ATTRIBUTE ((__format__ (__printf__, 3, 0)));
char *	vasnprintf (char *, size_t *, const char *, va_list)
               _ATTRIBUTE ((__format__ (__printf__, 3, 0)));
int	vdiprintf (int, const char *, va_list)
               _ATTRIBUTE ((__format__ (__printf__, 2, 0)));
int	vfiprintf (FILE *, const char *, va_list)
               _ATTRIBUTE ((__format__ (__printf__, 2, 0)));
int	vfiscanf (FILE *, const char *, va_list)
               _ATTRIBUTE ((__format__ (__scanf__, 2, 0)));
int	viprintf (const char *, va_list)
               _ATTRIBUTE ((__format__ (__printf__, 1, 0)));
int	viscanf (const char *, va_list)
               _ATTRIBUTE ((__format__ (__scanf__, 1, 0)));
int	vsiprintf (char *, const char *, va_list)
               _ATTRIBUTE ((__format__ (__printf__, 2, 0)));
int	vsiscanf (const char *, const char *, va_list)
               _ATTRIBUTE ((__format__ (__scanf__, 2, 0)));
int	vsniprintf (char *, size_t, const char *, va_list)
               _ATTRIBUTE ((__format__ (__printf__, 3, 0)));
#endif /* __MISC_VISIBLE */
#endif /* !_REENT_ONLY */

/*
 * Routines in POSIX 1003.1:2001.
 */

#if __POSIX_VISIBLE
#ifndef _REENT_ONLY
FILE *	fdopen (int, const char *);
#endif
int	fileno (FILE *);
#endif
#if __MISC_VISIBLE || __POSIX_VISIBLE >= 199209
int	pclose (FILE *);
FILE *  popen (const char *, const char *);
#endif

#if __BSD_VISIBLE
void    setbuffer (FILE *, char *, int);
int	setlinebuf (FILE *);
#endif

#if __MISC_VISIBLE || (__XSI_VISIBLE && __POSIX_VISIBLE < 200112)
int	getw (FILE *);
int	putw (int, FILE *);
#endif
#if __MISC_VISIBLE || __POSIX_VISIBLE
int	getc_unlocked (FILE *);
int	getchar_unlocked (void);
void	flockfile (FILE *);
int	ftrylockfile (FILE *);
void	funlockfile (FILE *);
int	putc_unlocked (int, FILE *);
int	putchar_unlocked (int);
#endif

/*
 * Routines in POSIX 1003.1:200x.
 */

#if __POSIX_VISIBLE >= 200809
# ifndef _REENT_ONLY
#  ifndef dprintf
int	dprintf (int, const char *__restrict, ...)
               _ATTRIBUTE ((__format__ (__printf__, 2, 3)));
#  endif
FILE *	fmemopen (void *__restrict, size_t, const char *__restrict);
/* getdelim - see __getdelim for now */
/* getline - see __getline for now */
FILE *	open_memstream (char **, size_t *);
int	vdprintf (int, const char *__restrict, va_list)
               _ATTRIBUTE ((__format__ (__printf__, 2, 0)));
# endif
#endif
#if __ATFILE_VISIBLE
int	renameat (int, const char *, int, const char *);
# ifdef __CYGWIN__
int	renameat2 (int, const char *, int, const char *, unsigned int);
# endif
#endif

/* Other extensions.  */

int	fpurge (FILE *);
ssize_t __getdelim (char **, size_t *, int, FILE *);
ssize_t __getline (char **, size_t *, FILE *);

#if __MISC_VISIBLE
void	clearerr_unlocked (FILE *);
int	feof_unlocked (FILE *);
int	ferror_unlocked (FILE *);
int	fileno_unlocked (FILE *);
int	fflush_unlocked (FILE *);
int	fgetc_unlocked (FILE *);
int	fputc_unlocked (int, FILE *);
size_t	fread_unlocked (void *__restrict, size_t _size, size_t _n, FILE *__restrict);
size_t	fwrite_unlocked (const void *__restrict , size_t _size, size_t _n, FILE *);
#endif

#if __GNU_VISIBLE
char *  fgets_unlocked (char *__restrict, int, FILE *__restrict);
int	fputs_unlocked (const char *__restrict, FILE *__restrict);
#endif

#ifdef __LARGE64_FILES
#if !defined(__CYGWIN__) || defined(_LIBC)
FILE *	fdopen64 (int, const char *);
FILE *  fopen64 (const char *, const char *);
FILE *  freopen64 (const char *, const char *, FILE *);
_off64_t ftello64 (FILE *);
_off64_t fseeko64 (FILE *, _off64_t, int);
int     fgetpos64 (FILE *, _fpos64_t *);
int     fsetpos64 (FILE *, const _fpos64_t *);
FILE *  tmpfile64 (void);

#endif /* !__CYGWIN__ */
#endif /* __LARGE64_FILES */

/*
 * Routines internal to the implementation.
 */

int	_srget ( FILE *);
int	_swbuf ( int, FILE *);

/*
 * Stdio function-access interface.
 */

#if __BSD_VISIBLE
# ifdef __LARGE64_FILES
FILE	*funopen (const void *__cookie,
		int (*__readfn)(void *__c, char *__buf,
				size_t __n),
		int (*__writefn)(void *__c, const char *__buf,
				 size_t __n),
		_fpos64_t (*__seekfn)(void *__c, _fpos64_t __off, int __whence),
		int (*__closefn)(void *__c));
FILE	*funopen ( const void *__cookie,
		int (*__readfn)(void *__c, char *__buf,
				size_t __n),
		int (*__writefn)(void *__c, const char *__buf,
				 size_t __n),
		_fpos64_t (*__seekfn)(void *__c, _fpos64_t __off, int __whence),
		int (*__closefn)(void *__c));
# else
FILE	*funopen (const void *__cookie,
		int (*__readfn)(void *__cookie, char *__buf,
				size_t __n),
		int (*__writefn)(void *__cookie, const char *__buf,
				 size_t __n),
		fpos_t (*__seekfn)(void *__cookie, fpos_t __off, int __whence),
		int (*__closefn)(void *__cookie));
FILE	*funopen ( const void *__cookie,
		int (*__readfn)(void *__cookie, char *__buf,
				size_t __n),
		int (*__writefn)(void *__cookie, const char *__buf,
				 size_t __n),
		fpos_t (*__seekfn)(void *__cookie, fpos_t __off, int __whence),
		int (*__closefn)(void *__cookie));
# endif /* !__LARGE64_FILES */

# define	fropen(__cookie, __fn) funopen(__cookie, __fn, NULL, NULL, NULL)
# define	fwopen(__cookie, __fn) funopen(__cookie, NULL, __fn, NULL, NULL)
#endif /* __BSD_VISIBLE */

#if __GNU_VISIBLE
typedef ssize_t cookie_read_function_t(void *__cookie, char *__buf, size_t __n);
typedef ssize_t cookie_write_function_t(void *__cookie, const char *__buf,
					size_t __n);
# ifdef __LARGE64_FILES
typedef int cookie_seek_function_t(void *__cookie, _off64_t *__off,
				   int __whence);
# else
typedef int cookie_seek_function_t(void *__cookie, off_t *__off, int __whence);
# endif /* !__LARGE64_FILES */
typedef int cookie_close_function_t(void *__cookie);
typedef struct
{
  /* These four struct member names are dictated by Linux; hopefully,
     they don't conflict with any macros.  */
  cookie_read_function_t  *read;
  cookie_write_function_t *write;
  cookie_seek_function_t  *seek;
  cookie_close_function_t *close;
} cookie_io_functions_t;
FILE *fopencookie (void *__cookie,
		const char *__mode, cookie_io_functions_t __functions);
FILE *fopencookie ( void *__cookie,
		const char *__mode, cookie_io_functions_t __functions);
#endif /* __GNU_VISIBLE */

#ifndef __CUSTOM_FILE_IO__
/*
 * The __sfoo macros are here so that we can
 * define function versions in the C library.
 */
#define       _sgetc_raw( __f) (--(__f)->_r < 0 ? _srget( __f) : (int)(*(__f)->_p++))

#ifdef __SCLE
/*  For a platform with CR/LF, additional logic is required by
  __sgetc_r which would otherwise simply be a macro; therefore we
  use an inlined function.  The function is only meant to be inlined
  in place as used and the function body should never be emitted.

  There are two possible means to this end when compiling with GCC,
  one when compiling with a standard C99 compiler, and for other
  compilers we're just stuck.  At the moment, this issue only
  affects the Cygwin target, so we'll most likely be using GCC. */

_ELIDABLE_INLINE int _sgetc( FILE *__p);

_ELIDABLE_INLINE int _sgetc( FILE *__p)
  {
    int __c = _sgetc_raw( __p);
    if ((__p->_flags & __SCLE) && (__c == '\r'))
      {
      int __c2 = _sgetc_raw( __p);
      if (__c2 == '\n')
        __c = __c2;
      else
        ungetc(__c2, __p);
      }
    return __c;
  }
#else
#define _sgetc( __p) _sgetc_raw( __p)
#endif

#ifdef __GNUC__
_ELIDABLE_INLINE int _sputc( int _c, FILE *_p) {
#ifdef __SCLE
	if ((_p->_flags & __SCLE) && _c == '\n')
	  _sputc ( '\r', _p);
#endif
	if (--_p->_w >= 0 || (_p->_w >= _p->_lbfsize && (char)_c != '\n'))
                return (*_p->_p++ = (unsigned char) _c);
	else
		return (_swbuf( _c, _p));
}
#else
/*
 * This has been tuned to generate reasonable code on the vax using pcc
 */
#define       _sputc_raw( __c, __p) \
	(--(__p)->_w < 0 ? \
		(__p)->_w >= (__p)->_lbfsize ? \
			(*(__p)->_p = (__c)), *(__p)->_p != '\n' ? \
				(int)*(__p)->_p++ : \
				_swbuf( '\n', __p) : \
			_swbuf( (int)(__c), __p) : \
		(*(__p)->_p = (__c), (int)*(__p)->_p++))
#ifdef __SCLE
#define _sputc( __c, __p) \
        ((((__p)->_flags & __SCLE) && ((__c) == '\n')) \
          ? _sputc_raw( '\r', (__p)) : 0 , \
        _sputc_raw( (__c), (__p)))
#else
#define _sputc( __c, __p) _sputc_raw( __c, __p)
#endif
#endif

#define	__sfeof(p)	((int)(((p)->_flags & __SEOF) != 0))
#define	__sferror(p)	((int)(((p)->_flags & __SERR) != 0))
#define	__sclearerr(p)	((void)((p)->_flags &= ~(__SERR|__SEOF)))
#define	__sfileno(p)	((p)->_file)

#ifndef __cplusplus
#ifndef _REENT_SMALL
#define	feof(p)		__sfeof(p)
#define	ferror(p)	__sferror(p)
#define	clearerr(p)	__sclearerr(p)

#if __MISC_VISIBLE
#define	feof_unlocked(p)	__sfeof(p)
#define	ferror_unlocked(p)	__sferror(p)
#define	clearerr_unlocked(p)	__sclearerr(p)
#endif /* __MISC_VISIBLE */
#endif /* _REENT_SMALL */

#if 0 /* __POSIX_VISIBLE - FIXME: must initialize stdio first, use fn */
#define	fileno(p)	__sfileno(p)
#endif

static __inline int
_getchar_unlocked(void)
{
    return (_sgetc(stdin)); 
}

static __inline int
_putchar_unlocked(int _c)
{
	return (_sputc( _c, stdout));
}

#ifdef __SINGLE_THREAD__
#define	getc(_p)	_sgetc( _p)
#define	putc(_c, _p)	_sputc( _c, _p)
#define	getchar()	_getchar_unlocked()
#define	putchar(_c)	_putchar_unlocked(_c)
#endif /* __SINGLE_THREAD__ */

#if __MISC_VISIBLE || __POSIX_VISIBLE
#define	getchar_unlocked()	_getchar_unlocked()
#define	putchar_unlocked(_c)	_putchar_unlocked(_c)
#endif
#endif /* __cplusplus */

#if __MISC_VISIBLE
/* fast always-buffered version, true iff error */
#define	fast_putc(x,p) (--(p)->_w < 0 ? \
	_swbuf( (int)(x), p) == EOF : (*(p)->_p = (x), (p)->_p++, 0))
#endif

#if __GNU_VISIBLE || (__XSI_VISIBLE && __XSI_VISIBLE < 600)
#define	L_cuserid	9		/* posix says it goes in stdio.h :( */
#endif
#if __POSIX_VISIBLE
#define L_ctermid       16
#endif

#else /* __CUSTOM_FILE_IO__ */

#define	getchar()	getc(stdin)
#define	putchar(x)	putc(x, stdout)

#if __MISC_VISIBLE || __POSIX_VISIBLE
#define	getchar_unlocked()	getc_unlocked(stdin)
#define	putchar_unlocked(x)	putc_unlocked(x, stdout)
#endif

#endif /* !__CUSTOM_FILE_IO__ */

_END_STD_C

#if __SSP_FORTIFY_LEVEL > 0
#include <ssp/stdio.h>
#endif

#endif /* _STDIO_H_ */

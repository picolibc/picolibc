/*
 * wchar.h
 *
 * Defines of all functions for supporting wide characters. Actually it
 * just includes all those headers, which is not a good thing to do from a
 * processing time point of view, but it does mean that everything will be
 * in sync.
 *
 * This file is part of the Mingw32 package.
 *
 *  THIS SOFTWARE IS NOT COPYRIGHTED
 *
 *  This source code is offered for use in the public domain. You may
 *  use, modify or distribute it freely.
 *
 *  This code is distributed in the hope that it will be useful but
 *  WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 *  DISCLAIMED. This includes but is not limited to warranties of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $Revision$
 * $Author$
 * $Date$
 *
 */

#ifndef	_WCHAR_H_
#define	_WCHAR_H_

/* All the headers include this file. */
#include <_mingw.h>

#define __need_size_t
#define __need_wint_t
#define __need_wchar_t
#define __need_NULL
#ifndef RC_INVOKED
#include <stddef.h>
#endif /* Not RC_INVOKED */

/*
 * FIXME: 
 * MSDN says that isw* char classifications are in wchar.h.
 * ISO C says they are wctype.h and ISO C++ expects them there as well.
 * Including wctype.h here in C++ will cause the std symbols to be in
 * global namespace as well as in std.  That is rude. 
 */
#if !(defined  __STRICT_ANSI__ && defined __cplusplus)
#include <wctype.h>
#endif

#define WCHAR_MIN	0
#define WCHAR_MAX	((wchar_t)-1)

#ifndef RC_INVOKED

__BEGIN_CSTD_NAMESPACE
#ifndef _FILE_DEFINED
#define	_FILE_DEFINED
typedef struct _iobuf
{
	char*	_ptr;
	int	_cnt;
	char*	_base;
	int	_flag;
	int	_file;
	int	_charbuf;
	int	_bufsiz;
	char*	_tmpfname;
} FILE;
#endif	/* Not _FILE_DEFINED */

#ifndef _TIME_T_DEFINED
typedef long time_t;
#define _TIME_T_DEFINED
#endif

#ifndef _TM_DEFINED
struct tm {
        int tm_sec;     /* seconds after the minute - [0,59] */
        int tm_min;     /* minutes after the hour - [0,59] */
        int tm_hour;    /* hours since midnight - [0,23] */
        int tm_mday;    /* day of the month - [1,31] */
        int tm_mon;     /* months since January - [0,11] */
        int tm_year;    /* years since 1900 */
        int tm_wday;    /* days since Sunday - [0,6] */
        int tm_yday;    /* days since January 1 - [0,365] */
        int tm_isdst;   /* daylight savings time flag */
        };
#define _TM_DEFINED
#endif
__END_CSTD_NAMESPACE

#ifndef _WSTDIO_DEFINED
__BEGIN_CSTD_NAMESPACE
/*  also in stdio.h - keep in sync */
int	fwprintf (FILE*, const wchar_t*, ...);
int	wprintf (const wchar_t*, ...);
int	swprintf (wchar_t*, const wchar_t*, ...);
int	vfwprintf (FILE*, const wchar_t*, __VALIST);
int	vwprintf (const wchar_t*, __VALIST);
int	vswprintf (wchar_t*, const wchar_t*, __VALIST);
int	fwscanf (FILE*, const wchar_t*, ...);
int	wscanf (const wchar_t*, ...);
int	swscanf (const wchar_t*, const wchar_t*, ...);
wint_t	fgetwc (FILE*);
wint_t	fputwc (wchar_t, FILE*);
wint_t	ungetwc (wchar_t, FILE*);
#ifdef __MSVCRT__ 
wchar_t* fgetws (wchar_t*, int, FILE*);
int	fputws (const wchar_t*, FILE*);
wint_t	getwc (FILE*);
wint_t	getwchar (void);
wint_t	putwc (wint_t, FILE*);
wint_t	putwchar (wint_t);
#endif

__END_CSTD_NAMESPACE
__BEGIN_CGLOBAL_NAMESPACE

#ifdef __MSVCRT__ 
#ifndef __STRICT_ANSI__
wchar_t* _getws (wchar_t*);
int	_putws (const wchar_t*);
FILE*	_wfdopen(int, wchar_t *);
FILE*	_wfopen (const wchar_t*, const wchar_t*);
FILE*	_wfreopen (const wchar_t*, const wchar_t*, FILE*);
FILE*	_wfsopen (const wchar_t*, const wchar_t*, int);
wchar_t* _wtmpnam (wchar_t*);
wchar_t* _wtempnam (const wchar_t*, const wchar_t*);
int	_wrename (const wchar_t*, const wchar_t*);
int	_wremove (const wchar_t*);
void	_wperror (const wchar_t*);
FILE*	_wpopen (const wchar_t*, const wchar_t*);
#endif	/* Not __STRICT_ANSI__ */
#endif	/* __MSVCRT__ */

/* C99 names, but non-standard behaviour */
int	_snwprintf (wchar_t*, size_t, const wchar_t*, ...);
int	_vsnwprintf (wchar_t*, size_t, const wchar_t*, __VALIST);
#ifndef __NO_ISOCEXT  /* externs in libmingwex.a */
int snwprintf(wchar_t* s, size_t n, const wchar_t*  format, ...);
extern __inline__ int
vsnwprintf (wchar_t* s, size_t n, const wchar_t* format, __VALIST arg)
  { return _vsnwprintf ( s, n, format, arg);}
#endif
__END_CGLOBAL_NAMESPACE

#define _WSTDIO_DEFINED
#endif /* _WSTDIO_DEFINED */

#ifndef _WSTRING_DEFINED
/*
 * Unicode versions of the standard calls.
 * Also in stdio.h, since MSDN puts them in both headers well */ 
 */
__BEGIN_CSTD_NAMESPACE
wchar_t* wcscat (wchar_t*, const wchar_t*);
wchar_t* wcschr (const wchar_t*, wchar_t);
int	wcscmp (const wchar_t*, const wchar_t*);
int	wcscoll (const wchar_t*, const wchar_t*);
wchar_t* wcscpy (wchar_t*, const wchar_t*);
size_t	wcscspn (const wchar_t*, const wchar_t*);
/* Note: No wcserror in CRTDLL. */
size_t	wcslen (const wchar_t*);
wchar_t* wcsncat (wchar_t*, const wchar_t*, size_t);
int	wcsncmp(const wchar_t*, const wchar_t*, size_t);
wchar_t* wcsncpy(wchar_t*, const wchar_t*, size_t);
wchar_t* wcspbrk(const wchar_t*, const wchar_t*);
wchar_t* wcsrchr(const wchar_t*, wchar_t);
size_t	wcsspn(const wchar_t*, const wchar_t*);
wchar_t* wcsstr(const wchar_t*, const wchar_t*);
wchar_t* wcstok(wchar_t*, const wchar_t*);
size_t	wcsxfrm(wchar_t*, const wchar_t*, size_t);
__END_CSTD_NAMESPACE

#ifndef __STRICT_ANSI__
__BEGIN_CGLOBAL_NAMESPACE
/*
 * Unicode versions of non-ANSI functions provided by CRTDLL.
 */

/* NOTE: _wcscmpi not provided by CRTDLL, this define is for portability */
#define		_wcscmpi	_wcsicmp

wchar_t* _wcsdup (const wchar_t*);
int	_wcsicmp (const wchar_t*, const wchar_t*);
int	_wcsicoll (const wchar_t*, const wchar_t*);
wchar_t* _wcslwr (wchar_t*);
int	_wcsnicmp (const wchar_t*, const wchar_t*, size_t);
wchar_t* _wcsnset (wchar_t*, wchar_t, size_t);
wchar_t* _wcsrev (wchar_t*);
wchar_t* _wcsset (wchar_t*, wchar_t);
wchar_t* _wcsupr (wchar_t*);

#ifdef __MSVCRT__
int  _wcsncoll(const wchar_t*, const wchar_t*, size_t);
int  _wcsnicoll(const wchar_t*, const wchar_t*, size_t);
#endif

#ifndef __NO_OLDNAMES
/* NOTE: There is no _wcscmpi, but this is for compatibility. */
int	wcscmpi	(const wchar_t*, const wchar_t*);
wchar_t* wcsdup (wchar_t*);
int	wcsicmp (const wchar_t*, const wchar_t*);
int	wcsicoll (const wchar_t*, const wchar_t*);
wchar_t* wcslwr (wchar_t*);
int	wcsnicmp (const wchar_t*, const wchar_t*, size_t);
wchar_t* wcsnset (wchar_t*, wchar_t, size_t);
wchar_t* wcsrev (wchar_t*);
wchar_t* wcsset (wchar_t*, wchar_t);
wchar_t* wcsupr (wchar_t*);
#endif	/* Not _NO_OLDNAMES */

__END_CGLOBAL_NAMESPACE
#endif	/* Not strict ANSI */

#define _WSTRING_DEFINED
#endif  /* _WSTRING_DEFINED */

#ifndef __STRICT_ANSI__
__BEGIN_CGLOBAL_NAMESPACE
/*
 * non_ANSI wide char functions from io.h, direct.h, sys/stat.h
 * and locale.h
 */
#ifndef	_FSIZE_T_DEFINED
typedef	unsigned long	_fsize_t;
#define _FSIZE_T_DEFINED
#endif

#ifndef _WFINDDATA_T_DEFINED
struct _wfinddata_t {
    	unsigned	attrib;
    	__CSTD time_t	time_create;	/* -1 for FAT file systems */
    	__CSTD time_t	time_access;	/* -1 for FAT file systems */
    	__CSTD time_t	time_write;
    	_fsize_t	size;
    	wchar_t		name[260];	/* may include spaces. */
};
struct _wfinddatai64_t {
    unsigned		attrib;
    __CSTD time_t	time_create;
    __CSTD time_t	time_access;
    __CSTD time_t	time_write;
    __int64		size;
    wchar_t		name[260];
};
#define _WFINDDATA_T_DEFINED
#endif

#if !defined (_WIO_DEFINED)
/* Wide character versions from io.h  */
#if defined (__MSVCRT__)
int 		_waccess(const wchar_t*, int);
int 		_wchmod(const wchar_t*, int);
int 		_wcreat(const wchar_t*, int);
long 		_wfindfirst(const wchar_t*, struct _wfinddata_t*);
int 		_wfindnext(long, struct _wfinddata_t *);
int 		_wunlink(const wchar_t*);
int 		_wopen(const wchar_t*, int, ...);
int 		_wsopen(const wchar_t*, int, int, ...);
wchar_t * 	_wmktemp(wchar_t*);
long  _wfindfirsti64(const wchar_t*, struct _wfinddatai64_t*);
int  _wfindnexti64(long, struct _wfinddatai64_t*);

#ifndef __NO_OLDNAMES
/* Where do these live? Not in libmoldname.a nor in libmsvcrt.a */
#if 0
int 		waccess(const wchar_t *, int);
int 		wchmod(const wchar_t *, int);
int 		wcreat(const wchar_t *, int);
long 		wfindfirst(wchar_t *, struct _wfinddata_t *);
int 		wfindnext(long, struct _wfinddata_t *);
int 		wunlink(const wchar_t *);
int 		wrename(const wchar_t *, const wchar_t *);
int 		wopen(const wchar_t *, int, ...);
int 		wsopen(const wchar_t *, int, int, ...);
wchar_t * 	wmktemp(wchar_t *);
#endif
#endif
#endif /* defined (__MSVCRT__) */

#define _WIO_DEFINED
#endif /* _WIO_DEFINED */

#ifndef _WDIRECT_DEFINED
/* Also in direct.h */
#ifdef __MSVCRT__ 
int	  _wchdir (const wchar_t*);
wchar_t*  _wgetcwd (wchar_t*, int);
wchar_t*  _wgetdcwd (int, wchar_t*, int);
int	  _wmkdir (const wchar_t*);
int	  _wrmdir (const wchar_t*);
#endif	/* __MSVCRT__ */
#define _WDIRECT_DEFINED
#endif /* _WDIRECT_DEFINED */

#ifndef _STAT_DEFINED
/*
 * The structure manipulated and returned by stat and fstat.
 *
 * NOTE: If called on a directory the values in the time fields are not only
 * invalid, they will cause localtime et. al. to return NULL. And calling
 * asctime with a NULL pointer causes an Invalid Page Fault. So watch it!
 */
struct _stat
{
	_dev_t	st_dev;		/* Equivalent to drive number 0=A 1=B ... */
	_ino_t	st_ino;		/* Always zero ? */
	_mode_t	st_mode;	/* See above constants */
	short	st_nlink;	/* Number of links. */
	short	st_uid;		/* User: Maybe significant on NT ? */
	short	st_gid;		/* Group: Ditto */
	_dev_t	st_rdev;	/* Seems useless (not even filled in) */
	_off_t	st_size;	/* File size in bytes */
	__CSTD time_t	st_atime;	/* Accessed date (always 00:00 hrs local
					 * on FAT) */
	__CSTD time_t	st_mtime;	/* Modified time */
	__CSTD time_t	st_ctime;	/* Creation time */
};

struct stat
{
	_dev_t	st_dev;		/* Equivalent to drive number 0=A 1=B ... */
	_ino_t	st_ino;		/* Always zero ? */
	_mode_t	st_mode;	/* See above constants */
	short	st_nlink;	/* Number of links. */
	short	st_uid;		/* User: Maybe significant on NT ? */
	short	st_gid;		/* Group: Ditto */
	_dev_t	st_rdev;	/* Seems useless (not even filled in) */
	_off_t	st_size;	/* File size in bytes */
	__CSTD time_t	 st_atime;	/* Accessed date (always 00:00 hrs local
					 * on FAT) */
	__CSTD time_t	st_mtime;	/* Modified time */
	__CSTD time_t	st_ctime;	/* Creation time */
};
#if defined (__MSVCRT__) 
struct _stati64 {
    _dev_t st_dev;
    _ino_t st_ino;
    unsigned short st_mode;
    short st_nlink;
    short st_uid;
    short st_gid;
    _dev_t st_rdev;
    __int64 st_size;
    __CSTD time_t st_atime;
    __CSTD time_t st_mtime;
    __CSTD time_t st_ctime;
    };
#endif  /* __MSVCRT__ */

#define _STAT_DEFINED
#endif /* _STAT_DEFINED */

#if !defined ( _WSTAT_DEFINED)
/* also declared in sys/stat.h */
#if defined __MSVCRT__
int	_wstat (const wchar_t*, struct _stat*);
int	_wstati64 (const wchar_t*, struct _stati64*);
#endif  /* __MSVCRT__ */
#define _WSTAT_DEFINED
#endif /* ! _WSTAT_DEFIND  */

#ifndef _WLOCALE_DEFINED  /* also declared in locale.h */
wchar_t* _wsetlocale (int, const wchar_t*);
#define _WLOCALE_DEFINED
#endif
__END_CGLOBAL_NAMESPACE
#endif /* __STRICT_ANSI__ */

#ifndef _WTIME_DEFINED
__BEGIN_CGLOBAL_NAMESPACE
#ifdef __MSVCRT__
#ifndef __STRICT_ANSI__
/* wide function prototypes, also declared in time.h */
wchar_t*	_wasctime (const struct __CSTD tm*);
wchar_t*	_wctime (const __CSTD time_t*);
wchar_t*	_wstrdate (wchar_t*);
wchar_t*	_wstrtime (wchar_t*);
#endif /* __MSVCRT__ */
#endif /* __STRICT_ANSI__ */
__END_CGLOBAL_NAMESPACE
__BEGIN_CSTD_NAMESPACE
size_t		wcsftime (wchar_t*, size_t, const wchar_t*, const struct tm*);
__END_CSTD_NAMESPACE
#define _WTIME_DEFINED
#endif /* _WTIME_DEFINED */ 

#ifndef _WSTDLIB_DEFINED /* also declared in stdlib.h */
__BEGIN_CSTD_NAMESPACE
long	wcstol	(const wchar_t*, wchar_t**, int);
unsigned long	wcstoul (const wchar_t*, wchar_t**, int);
double	wcstod	(const wchar_t*, wchar_t**);
#if !defined __NO_ISOCEXT /* extern stub in static libmingwex.a */
extern __inline__ float wcstof( const wchar_t *nptr, wchar_t **endptr)
{  return (wcstod(nptr, endptr)); }
#endif /* __NO_ISOCEXT */
__END_CSTD_NAMESPACE
#define  _WSTDLIB_DEFINED
#endif

__BEGIN_CSTD_NAMESPACE

/* These are resolved by -lmsvcp60 */
/* If you don't have msvcp60.dll in your windows system directory, you can
   easily obtain it with a search from your favorite search engine. */
typedef int mbstate_t;
typedef wchar_t _Wint_t;

wint_t  btowc(int);
size_t  mbrlen(const char *, size_t, mbstate_t *);
size_t  mbrtowc(wchar_t *, const char *, size_t, mbstate_t *);
size_t  mbsrtowcs(wchar_t *, const char **, size_t, mbstate_t *);

size_t  wcrtomb(char *, wchar_t, mbstate_t *);
size_t  wcsrtombs(char *, const wchar_t **, size_t, mbstate_t *);
int  	wctob(wint_t);

#ifndef __NO_ISOCEXT /* these need static lib libmingwex.a */
extern __inline__ int fwide(FILE* stream, int mode) {return -1;} /* limited to byte orientation */ 
extern __inline__ int mbsinit(const mbstate_t* ps) {return 1;}
wchar_t* wmemset(wchar_t* s, wchar_t c, size_t n);
wchar_t* wmemchr(const wchar_t* s, wchar_t c, size_t n);
int wmemcmp(const wchar_t* s1, const wchar_t * s2, size_t n);
wchar_t* wmemcpy(wchar_t* __restrict__ s1, const wchar_t* __restrict__ s2,
		 size_t n);
wchar_t* wmemmove(wchar_t* s1, const wchar_t* s2, size_t n);
long long wcstoll(const wchar_t* __restrict__ nptr,
		  wchar_t** __restrict__ endptr, int base);
unsigned long long wcstoull(const wchar_t* __restrict__ nptr,
			    wchar_t ** __restrict__ endptr, int base);

#endif /* __NO_ISOCEXT */

__END_CSTD_NAMESPACE

#endif /* Not RC_INVOKED */

#endif /* not _WCHAR_H_ */


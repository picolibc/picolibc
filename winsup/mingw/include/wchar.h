/*
 * wchar.h
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is a part of the mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within the package.
 *
 * Defines of all functions for supporting wide characters. Actually it
 * just includes all those headers, which is not a good thing to do from a
 * processing time point of view, but it does mean that everything will be
 * in sync.
 *
 */

#ifndef	_WCHAR_H_
#define	_WCHAR_H_

/* All the headers include this file. */
#include <_mingw.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <stdint.h>

#define __need_size_t
#define __need_wint_t
#define __need_wchar_t
#ifndef RC_INVOKED
#include <stddef.h>
#endif /* Not RC_INVOKED */

#define WCHAR_MIN	0
#define WCHAR_MAX	((wchar_t)-1)

#ifndef RC_INVOKED

#ifdef __cplusplus 
extern "C" {
#endif

#ifndef	__STRICT_ANSI__

#ifndef	_FSIZE_T_DEFINED
typedef	unsigned long	_fsize_t;
#define _FSIZE_T_DEFINED
#endif

#ifndef _WFINDDATA_T_DEFINED
struct _wfinddata_t {
	unsigned	attrib;
	time_t		time_create;	/* -1 for FAT file systems */
	time_t		time_access;	/* -1 for FAT file systems */
	time_t		time_write;
	_fsize_t	size;
	wchar_t		name[FILENAME_MAX];	/* may include spaces. */
};
struct _wfinddatai64_t {
	unsigned    attrib;
	time_t      time_create;
	time_t      time_access;
	time_t      time_write;
	__int64     size;
	wchar_t     name[FILENAME_MAX];
};
struct __wfinddata64_t {
        unsigned    attrib;
        __time64_t  time_create;    
        __time64_t  time_access;
        __time64_t  time_write;
        _fsize_t    size;
        wchar_t     name[FILENAME_MAX];
};
#define _WFINDDATA_T_DEFINED
#endif

/* Wide character versions. Also defined in io.h. */
/* CHECK: I believe these only exist in MSVCRT, and not in CRTDLL. Also
   applies to other wide character versions? */
#if !defined (_WIO_DEFINED)
#if defined (__MSVCRT__)
_CRTIMP int __cdecl	 _waccess (const wchar_t*, int);
_CRTIMP int __cdecl	_wchmod (const wchar_t*, int);
_CRTIMP int __cdecl	_wcreat (const wchar_t*, int);
_CRTIMP long __cdecl	_wfindfirst (const wchar_t*, struct _wfinddata_t *);
_CRTIMP int __cdecl	_wfindnext (long, struct _wfinddata_t *);
_CRTIMP int __cdecl	_wunlink (const wchar_t*);
_CRTIMP int __cdecl	_wopen (const wchar_t*, int, ...);
_CRTIMP int __cdecl	_wsopen (const wchar_t*, int, int, ...);
_CRTIMP wchar_t* __cdecl _wmktemp (wchar_t*);
_CRTIMP long __cdecl	_wfindfirsti64 (const wchar_t*, struct _wfinddatai64_t*);
_CRTIMP int __cdecl 	_wfindnexti64 (long, struct _wfinddatai64_t*);
#if __MSVCRT_VERSION__ >= 0x0601
_CRTIMP intptr_t __cdecl _wfindfirst64(const wchar_t*, struct __wfinddata64_t*); 
_CRTIMP intptr_t __cdecl _wfindnext64(intptr_t, struct __wfinddata64_t*);
#endif
#endif /* defined (__MSVCRT__) */
#define _WIO_DEFINED
#endif /* _WIO_DEFINED */

#ifndef _WSTDIO_DEFINED
/* also in stdio.h - keep in sync */
_CRTIMP int __cdecl	fwprintf (FILE*, const wchar_t*, ...);
_CRTIMP int __cdecl	wprintf (const wchar_t*, ...);
_CRTIMP int __cdecl	swprintf (wchar_t*, const wchar_t*, ...);
_CRTIMP int __cdecl	_snwprintf (wchar_t*, size_t, const wchar_t*, ...);
_CRTIMP int __cdecl	vfwprintf (FILE*, const wchar_t*, __VA_LIST);
_CRTIMP int __cdecl	vwprintf (const wchar_t*, __VA_LIST);
_CRTIMP int __cdecl	vswprintf (wchar_t*, const wchar_t*, __VA_LIST);
_CRTIMP int __cdecl	_vsnwprintf (wchar_t*, size_t, const wchar_t*, __VA_LIST);
_CRTIMP int __cdecl	fwscanf (FILE*, const wchar_t*, ...);
_CRTIMP int __cdecl	wscanf (const wchar_t*, ...);
_CRTIMP int __cdecl	swscanf (const wchar_t*, const wchar_t*, ...);
_CRTIMP wint_t __cdecl	fgetwc (FILE*);
_CRTIMP wint_t __cdecl	fputwc (wchar_t, FILE*);
_CRTIMP wint_t __cdecl	ungetwc (wchar_t, FILE*);

#ifndef __NO_ISOCEXT  /* externs in libmingwex.a */
int __cdecl snwprintf (wchar_t* s, size_t n, const wchar_t*  format, ...);
__CRT_INLINE int __cdecl
vsnwprintf (wchar_t* s, size_t n, const wchar_t* format, __VALIST arg)
  { return _vsnwprintf ( s, n, format, arg);}
int __cdecl vwscanf (const wchar_t * __restrict__, __VALIST);
int __cdecl vfwscanf (FILE * __restrict__,
		       const wchar_t * __restrict__, __VALIST);
int __cdecl vswscanf (const wchar_t * __restrict__,
		       const wchar_t * __restrict__, __VALIST);
#endif

#ifdef __MSVCRT__ 
_CRTIMP wchar_t* __cdecl fgetws (wchar_t*, int, FILE*);
_CRTIMP int __cdecl	fputws (const wchar_t*, FILE*);
_CRTIMP wint_t __cdecl	getwc (FILE*);
_CRTIMP wint_t __cdecl  getwchar (void);
_CRTIMP wchar_t* __cdecl _getws (wchar_t*);
_CRTIMP wint_t __cdecl	putwc (wint_t, FILE*);
_CRTIMP int __cdecl	_putws (const wchar_t*);
_CRTIMP wint_t __cdecl	putwchar (wint_t);
_CRTIMP FILE* __cdecl	_wfdopen(int, wchar_t *);
_CRTIMP FILE* __cdecl	_wfopen (const wchar_t*, const wchar_t*);
_CRTIMP FILE* __cdecl	_wfreopen (const wchar_t*, const wchar_t*, FILE*);
_CRTIMP FILE* __cdecl   _wfsopen (const wchar_t*, const wchar_t*, int);
_CRTIMP wchar_t* __cdecl _wtmpnam (wchar_t*);
_CRTIMP wchar_t* __cdecl _wtempnam (const wchar_t*, const wchar_t*);
_CRTIMP int __cdecl	_wrename (const wchar_t*, const wchar_t*);
_CRTIMP int __cdecl	_wremove (const wchar_t*)

_CRTIMP FILE* __cdecl	_wpopen (const wchar_t*, const wchar_t*)
_CRTIMP void __cdecl	_wperror (const wchar_t*);
#endif	/* __MSVCRT__ */
#define _WSTDIO_DEFINED
#endif /* _WSTDIO_DEFINED */

#ifndef _WDIRECT_DEFINED
/* Also in direct.h */
#ifdef __MSVCRT__ 
_CRTIMP int __cdecl	  _wchdir (const wchar_t*);
_CRTIMP wchar_t* __cdecl  _wgetcwd (wchar_t*, int);
_CRTIMP wchar_t* __cdecl  _wgetdcwd (int, wchar_t*, int);
_CRTIMP int __cdecl	  _wmkdir (const wchar_t*);
_CRTIMP int __cdecl	  _wrmdir (const wchar_t*);
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
	time_t	st_atime;	/* Accessed date (always 00:00 hrs local
				 * on FAT) */
	time_t	st_mtime;	/* Modified time */
	time_t	st_ctime;	/* Creation time */
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
	time_t	st_atime;	/* Accessed date (always 00:00 hrs local
				 * on FAT) */
	time_t	st_mtime;	/* Modified time */
	time_t	st_ctime;	/* Creation time */
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
    time_t st_atime;
    time_t st_mtime;
    time_t st_ctime;
    };

struct __stat64
{
    _dev_t st_dev;
    _ino_t st_ino;
    _mode_t st_mode;
    short st_nlink;
    short st_uid;
    short st_gid;
    _dev_t st_rdev;
    _off_t st_size;
    __time64_t st_atime;
    __time64_t st_mtime;
    __time64_t st_ctime;
};
#endif  /* __MSVCRT__ */
#define _STAT_DEFINED
#endif /* _STAT_DEFINED */

#if !defined ( _WSTAT_DEFINED)
/* also declared in sys/stat.h */
#if defined __MSVCRT__
_CRTIMP int __cdecl	_wstat (const wchar_t*, struct _stat*);
_CRTIMP int __cdecl	_wstati64 (const wchar_t*, struct _stati64*);
#if __MSVCRT_VERSION__ >= 0x0601
_CRTIMP int __cdecl _wstat64 (const wchar_t*, struct __stat64*);
#endif /* __MSVCRT_VERSION__ >= 0x0601 */
#endif  /* __MSVCRT__ */
#define _WSTAT_DEFINED
#endif /* ! _WSTAT_DEFIND  */

#ifndef _WTIME_DEFINED
#ifdef __MSVCRT__
/* wide function prototypes, also declared in time.h */
_CRTIMP wchar_t* __cdecl	_wasctime (const struct tm*);
_CRTIMP wchar_t* __cdecl	_wctime (const time_t*);
_CRTIMP wchar_t* __cdecl	_wstrdate (wchar_t*);
_CRTIMP wchar_t* __cdecl	_wstrtime (wchar_t*);
#if __MSVCRT_VERSION__ >= 0x601
_CRTIMP wchar_t* __cdecl	_wctime64 (const __time64_t*);
#endif
#endif /* __MSVCRT__ */
_CRTIMP size_t __cdecl		wcsftime (wchar_t*, size_t, const wchar_t*, const struct tm*);
#define _WTIME_DEFINED
#endif /* _WTIME_DEFINED */ 

#ifndef _WLOCALE_DEFINED  /* also declared in locale.h */
_CRTIMP wchar_t* __cdecl _wsetlocale (int, const wchar_t*);
#define _WLOCALE_DEFINED
#endif

#ifndef _WSTDLIB_DEFINED /* also declared in stdlib.h */
_CRTIMP long __cdecl 		wcstol (const wchar_t*, wchar_t**, int);
_CRTIMP unsigned long __cdecl	wcstoul (const wchar_t*, wchar_t**, int);
_CRTIMP double __cdecl		wcstod (const wchar_t*, wchar_t**);
#if !defined __NO_ISOCEXT /* extern stub in static libmingwex.a */
__CRT_INLINE float __cdecl wcstof( const wchar_t *nptr, wchar_t **endptr)
{  return (wcstod(nptr, endptr)); }
long double __cdecl wcstold (const wchar_t * __restrict__, wchar_t ** __restrict__);
#endif /* __NO_ISOCEXT */
#define  _WSTDLIB_DEFINED
#endif


#ifndef	_NO_OLDNAMES

/* Wide character versions. Also declared in io.h. */
/* CHECK: Are these in the oldnames???  NO! */
#if (0)
int 		waccess (const wchar_t *, int);
int 		wchmod (const wchar_t *, int);
int 		wcreat (const wchar_t *, int);
long 		wfindfirst (wchar_t *, struct _wfinddata_t *);
int 		wfindnext (long, struct _wfinddata_t *);
int 		wunlink (const wchar_t *);
int 		wrename (const wchar_t *, const wchar_t *);
int 		wremove (const wchar_t *);
int 		wopen (const wchar_t *, int, ...);
int 		wsopen (const wchar_t *, int, int, ...);
wchar_t* 	wmktemp (wchar_t *);
#endif
#endif /* _NO_OLDNAMES */

#endif /* not __STRICT_ANSI__ */

/* These are resolved by -lmsvcp60 */
/* If you don't have msvcp60.dll in your windows system directory, you can
   easily obtain it with a search from your favorite search engine. */
typedef int mbstate_t;
typedef wchar_t _Wint_t;

wint_t __cdecl btowc(int);
size_t __cdecl mbrlen(const char *, size_t, mbstate_t *);
size_t __cdecl mbrtowc(wchar_t *, const char *, size_t, mbstate_t *);
size_t __cdecl mbsrtowcs(wchar_t *, const char **, size_t, mbstate_t *);

size_t __cdecl wcrtomb(char *, wchar_t, mbstate_t *);
size_t __cdecl wcsrtombs(char *, const wchar_t **, size_t, mbstate_t *);
int __cdecl wctob(wint_t);

#ifndef __NO_ISOCEXT /* these need static lib libmingwex.a */
__CRT_INLINE int __cdecl fwide(FILE* __UNUSED_PARAM(stream), int __UNUSED_PARAM(mode))
  {return -1;} /* limited to byte orientation */ 
__CRT_INLINE int __cdecl mbsinit(const mbstate_t* __UNUSED_PARAM(ps))
  {return 1;}
wchar_t* __cdecl wmemset(wchar_t* s, wchar_t c, size_t n);
wchar_t* __cdecl wmemchr(const wchar_t* s, wchar_t c, size_t n);
int wmemcmp(const wchar_t* s1, const wchar_t * s2, size_t n);
wchar_t* __cdecl wmemcpy(wchar_t* __restrict__ s1, const wchar_t* __restrict__ s2,
		 size_t n);
wchar_t* __cdecl wmemmove(wchar_t* s1, const wchar_t* s2, size_t n);
long long __cdecl wcstoll(const wchar_t* __restrict__ nptr,
		  wchar_t** __restrict__ endptr, int base);
unsigned long long __cdecl wcstoull(const wchar_t* __restrict__ nptr,
			    wchar_t ** __restrict__ endptr, int base);
#endif /* __NO_ISOCEXT */

#ifdef __cplusplus
}	/* end of extern "C" */
#endif

#endif /* Not RC_INVOKED */

#endif /* not _WCHAR_H_ */


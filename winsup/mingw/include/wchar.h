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
 *  DISCLAMED. This includes but is not limited to warranties of
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

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
#define _WFINDDATA_T_DEFINED
#endif

/* Wide character versions. Also defined in io.h. */
/* CHECK: I believe these only exist in MSVCRT, and not in CRTDLL. Also
   applies to other wide character versions? */
int 		_waccess(const wchar_t *, int);
int 		_wchmod(const wchar_t *, int);
int 		_wcreat(const wchar_t *, int);
long 		_wfindfirst(wchar_t *, struct _wfinddata_t *);
int 		_wfindnext(long, struct _wfinddata_t *);
int 		_wunlink(const wchar_t *);
int 		_wrename(const wchar_t *, const wchar_t *);
int 		_wremove (const wchar_t *);
int 		_wopen(const wchar_t *, int, ...);
int 		_wsopen(const wchar_t *, int, int, ...);
wchar_t * 	_wmktemp(wchar_t *);

#ifndef _WDIRECT_DEFINED

/* Also in direct.h */

int _wchdir(const wchar_t*);
wchar_t* _wgetcwd(wchar_t*, int);
wchar_t* _wgetdcwd(int, wchar_t*, int);
int _wmkdir(const wchar_t*);
int _wrmdir(const wchar_t*);

#define _WDIRECT_DEFINED
#endif

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

#define _STAT_DEFINED
#endif /* _STAT_DEFINED */

#ifndef _WSTAT_DEFINED

/* also declared in sys/stat.h */

int	_wstat(const wchar_t *, struct _stat *);

#define _WSTAT_DEFINED
#endif /* _WSTAT_DEFINED */

#ifndef _WTIME_DEFINED
#ifdef __MSVCRT__

/* wide function prototypes, also declared in time.h */

wchar_t *	_wasctime(const struct tm*);
wchar_t *	_wctime(const time_t*);
wchar_t*	_wstrdate(wchar_t*);
wchar_t*	_wstrtime(wchar_t*);

#endif /* __MSVCRT__ */

size_t		wcsftime(wchar_t*, size_t, const wchar_t*, const struct tm*);

#define _WTIME_DEFINED
#endif /* _WTIME_DEFINED */ 


#ifndef	_NO_OLDNAMES

/* Wide character versions. Also declared in wchar.h. */
/* CHECK: Are these in the olnames??? */
int 		waccess(const wchar_t *, int);
int 		wchmod(const wchar_t *, int);
int 		wcreat(const wchar_t *, int);
long 		wfindfirst(wchar_t *, struct _wfinddata_t *);
int 		wfindnext(long, struct _wfinddata_t *);
int 		wunlink(const wchar_t *);
int 		wrename(const wchar_t *, const wchar_t *);
int 		wremove (const wchar_t *);
int 		wopen(const wchar_t *, int, ...);
int 		wsopen(const wchar_t *, int, int, ...);
wchar_t * 	wmktemp(wchar_t *);

#endif /* _NO_OLDNAMES */

#endif /* not __STRICT_ANSI__ */

typedef int mbstate_t;
typedef wchar_t _Wint_t;

wint_t  btowc(int);
size_t  mbrlen(const char *, size_t, mbstate_t *);
size_t  mbrtowc(wchar_t *, const char *, size_t, mbstate_t *);
size_t  mbsrtowcs(wchar_t *, const char **, size_t, mbstate_t *);

size_t  wcrtomb(char *, wchar_t, mbstate_t *);
size_t  wcsrtombs(char *, const wchar_t **, size_t, mbstate_t *);
int  wctob(wint_t);

#ifdef __cplusplus 
}
#endif

#endif /* Not RC_INVOKED */

#endif /* not _WCHAR_H_ */


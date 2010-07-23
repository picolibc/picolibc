/*
 * io.h
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is a part of the mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within the package.
 *
 * System level I/O functions and types.
 *
 */
#ifndef	_IO_H_
#define	_IO_H_

/* All the headers include this file. */
#include <_mingw.h>

/* MSVC's io.h contains the stuff from dir.h, so I will too.
 * NOTE: This also defines off_t, the file offset type, through
 *       an inclusion of sys/types.h */

#include <sys/types.h>	/* To get time_t.  */

/*
 * Attributes of files as returned by _findfirst et al.
 */
#define	_A_NORMAL	0x00000000
#define	_A_RDONLY	0x00000001
#define	_A_HIDDEN	0x00000002
#define	_A_SYSTEM	0x00000004
#define	_A_VOLID	0x00000008
#define	_A_SUBDIR	0x00000010
#define	_A_ARCH		0x00000020


#ifndef RC_INVOKED

#ifndef _INTPTR_T_DEFINED
#define _INTPTR_T_DEFINED
#ifdef _WIN64
  typedef __int64 intptr_t;
#else
  typedef int intptr_t;
#endif
#endif

#ifndef	_FSIZE_T_DEFINED
typedef	unsigned long	_fsize_t;
#define _FSIZE_T_DEFINED
#endif

/*
 * The maximum length of a file name. You should use GetVolumeInformation
 * instead of this constant. But hey, this works.
 * Also defined in stdio.h.
 */
#ifndef FILENAME_MAX
#define	FILENAME_MAX	(260)
#endif

/*
 * The following structure is filled in by _findfirst or _findnext when
 * they succeed in finding a match.
 */
struct _finddata_t
{
	unsigned	attrib;		/* Attributes, see constants above. */
	time_t		time_create;
	time_t		time_access;	/* always midnight local time */
	time_t		time_write;
	_fsize_t	size;
	char		name[FILENAME_MAX];	/* may include spaces. */
};

struct _finddatai64_t {
    unsigned    attrib;
    time_t      time_create;
    time_t      time_access;
    time_t      time_write;
    __int64     size;
    char        name[FILENAME_MAX];
};

#if __MSVCRT_VERSION__ >= 0x0601
struct __finddata64_t {
        unsigned    attrib;
        __time64_t  time_create;
        __time64_t  time_access;
        __time64_t  time_write;
/* 8 bytes are returned so it can't be _fsize_t */
        __int64    size;
         char       name[FILENAME_MAX];
};
#endif

#if __MSVCRT_VERSION__ >= 0x0800
struct _finddata32_t {
	unsigned	attrib;
	__time32_t	time_create;
	__time32_t	time_access;
	__time32_t	time_write;
	__int32		size;
	char		name[FILENAME_MAX];
};

struct _finddata32i64_t {
	unsigned	attrib;
	__time32_t	time_create;
	__time32_t	time_access;
	__time32_t	time_write;
	__int64		size;
	char		name[FILENAME_MAX];
};

struct _finddata64i32_t {
	unsigned	attrib;
	__time64_t	time_create;
	__time64_t	time_access;
	__time64_t	time_write;
	__int32		size;
	char		name[FILENAME_MAX];
};
#endif /* __MSVCRT_VERSION__ >= 0x0800 */


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

#if __MSVCRT_VERSION__ >= 0x0601
struct __wfinddata64_t {
        unsigned    attrib;
        __time64_t  time_create;
        __time64_t  time_access;
        __time64_t  time_write;
/* 8 bytes are returned so it can't be _fsize_t */
        __int64    size;
        wchar_t     name[FILENAME_MAX];
};
#endif

#if __MSVCRT_VERSION__ >= 0x0800
struct __wfinddata32_t {
	unsigned	attrib;
	__time32_t	time_create;
	__time32_t	time_access;
	__time32_t	time_write;
	__int32		size;
	wchar_t		name[FILENAME_MAX];
};

struct _wfinddata32i64_t {
	unsigned	attrib;
	__time32_t	time_create;
	__time32_t	time_access;
	__time32_t	time_write;
	__int64		size;
	wchar_t		name[FILENAME_MAX];
};

struct _wfinddata64i32_t {
	unsigned	attrib;
	__time64_t	time_create;
	__time64_t	time_access;
	__time64_t	time_write;
	__int32		size;
	wchar_t		name[FILENAME_MAX];
};
#endif /* __MSVCRT_VERSION__ >= 0x0800 */


#define _WFINDDATA_T_DEFINED
#endif

#ifdef	__cplusplus
extern "C" {
#endif

/*
 * Functions for searching for files. _findfirst returns -1 if no match
 * is found. Otherwise it returns a handle to be used in _findnext and
 * _findclose calls. _findnext also returns -1 if no match could be found,
 * and 0 if a match was found. Call _findclose when you are finished.
 */
/*  FIXME: Should these all use intptr_t, as per recent MSDN docs?  */
#if __MSVCRT_VERSION__ >= 0x0800
/*
intptr_t _findfirst      (const char *filespec,struct _finddata_t      *fileinfo);
intptr_t _findfirst32    (const char *filespec,struct _finddata32_t    *fileinfo);
intptr_t _findfirst64    (const char *filespec,struct __finddata64_t   *fileinfo);
intptr_t _findfirsti64   (const char *filespec,struct _finddatai64_t   *fileinfo);
intptr_t _findfirst32i64 (const char *filespec,struct _finddata32i64_t *fileinfo);
intptr_t _findfirst64i32 (const char *filespec,struct _finddata64i32_t *fileinfo);

intptr_t _wfindfirst     (const wchar_t *filespec,struct _wfinddata_t *fileinfo);
intptr_t _wfindfirst32   (const wchar_t *filespec,struct __wfinddata32_t *fileinfo);
intptr_t _wfindfirst64   (const wchar_t *filespec, struct __wfinddata64_t   *fileinfo);
intptr_t _wfindfirsti64  (const wchar_t *filespec, struct _wfinddatai64_t   *fileinfo);
intptr_t _wfindfirst32i64(const wchar_t *filespec, struct _wfinddata32i64_t *fileinfo);
intptr_t _wfindfirst64i32(const wchar_t *filespec, struct _wfinddata64i32_t *fileinfo);

Time Type and File Length Type Variations of _findfirst:
Functions		_USE_32BIT_TIME_T defined?	Time type	File length type
_findfirst,		Not defined			64-bit		32-bit
_wfindfirst
_findfirst,		Defined				32-bit		32-bit
_wfindfirst

_findfirst32,		Not affected by the macro	32-bit		32-bit
_wfindfirst32		definition

_findfirst64,		Not affected by the macro	64-bit		64-bit
_wfindfirst64		definition

_findfirsti64,		Not defined			64-bit		64-bit
_wfindfirsti64
_findfirsti64,		Defined				32-bit		64-bit
_wfindfirsti64

_findfirst32i64,	Not affected by the macro	32-bit		64-bit
_wfindfirst32i64	definition

_findfirst64i32,	Not affected by the macro	64-bit		32-bit
_wfindfirst64i32	definition
*/
#endif
#if __MSVCRT_VERSION__ < 0x0800
_CRTIMP intptr_t __cdecl __MINGW_NOTHROW _findfirst (const char*, struct _finddata_t*);
_CRTIMP int __cdecl __MINGW_NOTHROW _findnext (intptr_t, struct _finddata_t*);
#endif /* __MSVCRT_VERSION__ < 0x0800 */
_CRTIMP int __cdecl __MINGW_NOTHROW _findclose (intptr_t);
#if __MSVCRT_VERSION__ >= 0x0800
_CRTIMP intptr_t __cdecl __MINGW_NOTHROW _findfirst32 (const char*, struct _finddata32_t*);
_CRTIMP int  __cdecl __MINGW_NOTHROW	_findnext32 (intptr_t, struct _finddata32_t*);
#endif /* __MSVCRT_VERSION__ >= 0x0800 */

_CRTIMP int __cdecl __MINGW_NOTHROW _chdir (const char*);
_CRTIMP char* __cdecl __MINGW_NOTHROW _getcwd (char*, int);
_CRTIMP int __cdecl __MINGW_NOTHROW _mkdir (const char*);
_CRTIMP char* __cdecl __MINGW_NOTHROW _mktemp (char*);
_CRTIMP int __cdecl __MINGW_NOTHROW _rmdir (const char*);
_CRTIMP int __cdecl __MINGW_NOTHROW _chmod (const char*, int);
#ifdef __MSVCRT__
_CRTIMP __int64 __cdecl __MINGW_NOTHROW _filelengthi64(int);
#if __MSVCRT_VERSION__ < 0x0800
_CRTIMP intptr_t __cdecl __MINGW_NOTHROW _findfirsti64(const char*, struct _finddatai64_t*);
_CRTIMP int __cdecl __MINGW_NOTHROW _findnexti64(intptr_t, struct _finddatai64_t*);
#else
_CRTIMP intptr_t __cdecl __MINGW_NOTHROW _findfirst32i64 (const char*, struct _finddata32i64_t*);
_CRTIMP intptr_t __cdecl __MINGW_NOTHROW _findfirst64i32 (const char*, struct _finddata64i32_t*);
_CRTIMP int __cdecl __MINGW_NOTHROW	_findnext32i64 (intptr_t, struct _finddata32i64_t*);
_CRTIMP int __cdecl __MINGW_NOTHROW	_findnext64i32 (intptr_t, struct _finddata64i32_t*);
#endif /* __MSVCRT_VERSION__ < 0x0800 */
_CRTIMP __int64 __cdecl __MINGW_NOTHROW _lseeki64(int, __int64, int);
_CRTIMP __int64 __cdecl __MINGW_NOTHROW _telli64(int);
/* These require newer versions of msvcrt.dll (6.1 or higher). */ 
#if __MSVCRT_VERSION__ >= 0x0601
_CRTIMP intptr_t __cdecl __MINGW_NOTHROW _findfirst64(const char*, struct __finddata64_t*);
_CRTIMP int __cdecl __MINGW_NOTHROW _findnext64(intptr_t, struct __finddata64_t*);
#endif /* __MSVCRT_VERSION__ >= 0x0601 */
#if __MSVCRT_VERSION__ >= 0x0800
#ifndef _USE_32BIT_TIME_T
_CRTALIAS intptr_t __cdecl __MINGW_NOTHROW _findfirst (const char* _v1, struct _finddata_t* _v2)	{ return(_findfirst64i32 (_v1,(struct _finddata64i32_t*)_v2)); }
_CRTALIAS int __cdecl __MINGW_NOTHROW	_findnext (intptr_t _v1, struct _finddata_t* _v2)		{ return(_findnext64i32  (_v1,(struct _finddata64i32_t*)_v2)); }
_CRTALIAS intptr_t __cdecl __MINGW_NOTHROW _findfirsti64 (const char* _v1, struct _finddatai64_t* _v2)	{ return(_findfirst64 (_v1,(struct __finddata64_t*)_v2)); }
_CRTALIAS int  __cdecl __MINGW_NOTHROW	_findnexti64 (intptr_t _v1, struct _finddatai64_t* _v2)		{ return(_findnext64  (_v1,(struct __finddata64_t*)_v2)); }
#else
_CRTALIAS intptr_t __cdecl __MINGW_NOTHROW _findfirst (const char* _v1, struct _finddata_t* _v2)	{ return(_findfirst32 (_v1,(struct _finddata32_t*)_v2)); }
_CRTALIAS int __cdecl __MINGW_NOTHROW	_findnext (intptr_t _v1, struct _finddata_t* _v2)		{ return(_findnext32  (_v1,(struct _finddata32_t*)_v2)); }
_CRTALIAS intptr_t __cdecl __MINGW_NOTHROW _findfirsti64 (const char* _v1, struct _finddatai64_t* _v2)	{ return(_findfirst32i64 (_v1,(struct _finddata32i64_t*)_v2)); }
_CRTALIAS int __cdecl __MINGW_NOTHROW	_findnexti64 (intptr_t _v1, struct _finddatai64_t* _v2)		{ return(_findnext32i64  (_v1,(struct _finddata32i64_t*)_v2)); }
#endif /* !_USE_32BIT_TIME_T */
#endif /* __MSVCRT_VERSION__ >= 0x0800 */
#ifndef __NO_MINGW_LFS
__CRT_INLINE off64_t lseek64 (int, off64_t, int);
__CRT_INLINE off64_t lseek64 (int fd, off64_t offset, int whence)
{
  return _lseeki64(fd, (__int64) offset, whence);
}
#endif

#endif /* __MSVCRT__ */

#ifndef _NO_OLDNAMES

#ifndef _UWIN
_CRTIMP int __cdecl __MINGW_NOTHROW chdir (const char*);
_CRTIMP char* __cdecl __MINGW_NOTHROW getcwd (char*, int);
_CRTIMP int __cdecl __MINGW_NOTHROW mkdir (const char*);
_CRTIMP char* __cdecl __MINGW_NOTHROW mktemp (char*);
_CRTIMP int __cdecl __MINGW_NOTHROW rmdir (const char*);
_CRTIMP int __cdecl __MINGW_NOTHROW chmod (const char*, int);
#endif /* _UWIN */

#endif /* Not _NO_OLDNAMES */

#ifdef	__cplusplus
}
#endif

#endif	/* Not RC_INVOKED */

/* TODO: Maximum number of open handles has not been tested, I just set
 * it the same as FOPEN_MAX. */
#define	HANDLE_MAX	FOPEN_MAX

/* Some defines for _access nAccessMode (MS doesn't define them, but
 * it doesn't seem to hurt to add them). */
#define	F_OK	0	/* Check for file existence */
/* Well maybe it does hurt.  On newer versions of MSVCRT, an access mode
   of 1 causes invalid parameter error. */   
#define	X_OK	1	/* MS access() doesn't check for execute permission. */
#define	W_OK	2	/* Check for write permission */
#define	R_OK	4	/* Check for read permission */

#ifndef RC_INVOKED

#ifdef	__cplusplus
extern "C" {
#endif

_CRTIMP int __cdecl __MINGW_NOTHROW _access (const char*, int);
_CRTIMP int __cdecl __MINGW_NOTHROW _chsize (int, long);
_CRTIMP int __cdecl __MINGW_NOTHROW _close (int);
_CRTIMP int __cdecl __MINGW_NOTHROW _commit(int);

/* NOTE: The only significant bit in unPermissions appears to be bit 7 (0x80),
 *       the "owner write permission" bit (on FAT). */
_CRTIMP int __cdecl __MINGW_NOTHROW _creat (const char*, int);

_CRTIMP int __cdecl __MINGW_NOTHROW _dup (int);
_CRTIMP int __cdecl __MINGW_NOTHROW _dup2 (int, int);
_CRTIMP long __cdecl __MINGW_NOTHROW _filelength (int);
_CRTIMP long __cdecl __MINGW_NOTHROW _get_osfhandle (int);
_CRTIMP int __cdecl __MINGW_NOTHROW _isatty (int);

/* In a very odd turn of events this function is excluded from those
 * files which define _STREAM_COMPAT. This is required in order to
 * build GNU libio because of a conflict with _eof in streambuf.h
 * line 107. Actually I might just be able to change the name of
 * the enum member in streambuf.h... we'll see. TODO */
#ifndef	_STREAM_COMPAT
_CRTIMP int __cdecl __MINGW_NOTHROW _eof (int);
#endif

/* LK_... locking commands defined in sys/locking.h. */
_CRTIMP int __cdecl __MINGW_NOTHROW _locking (int, int, long);

_CRTIMP long __cdecl __MINGW_NOTHROW _lseek (int, long, int);

/* Optional third argument is unsigned unPermissions. */
_CRTIMP int __cdecl __MINGW_NOTHROW _open (const char*, int, ...);

_CRTIMP int __cdecl __MINGW_NOTHROW _open_osfhandle (intptr_t, int);
_CRTIMP int __cdecl __MINGW_NOTHROW _pipe (int *, unsigned int, int);
_CRTIMP int __cdecl __MINGW_NOTHROW _read (int, void*, unsigned int);
_CRTIMP int __cdecl __MINGW_NOTHROW _setmode (int, int);
/* MS puts remove & rename (but not wide versions) in io.h as well
   as in stdio.h. */
_CRTIMP int __cdecl __MINGW_NOTHROW	remove (const char*);
_CRTIMP int __cdecl __MINGW_NOTHROW	rename (const char*, const char*);

/* SH_... flags for nShFlags defined in share.h
 * Optional fourth argument is unsigned unPermissions */
_CRTIMP int __cdecl __MINGW_NOTHROW _sopen (const char*, int, int, ...);

_CRTIMP long __cdecl __MINGW_NOTHROW _tell (int);
/* Should umask be in sys/stat.h and/or sys/types.h instead? */
_CRTIMP int __cdecl __MINGW_NOTHROW _umask (int);
_CRTIMP int __cdecl __MINGW_NOTHROW _unlink (const char*);
_CRTIMP int __cdecl __MINGW_NOTHROW _write (int, const void*, unsigned int);

/* Wide character versions. Also declared in wchar.h. */
/* Not in crtdll.dll */
#if !defined (_WIO_DEFINED)
#if defined (__MSVCRT__)
_CRTIMP int __cdecl __MINGW_NOTHROW _waccess(const wchar_t*, int);
_CRTIMP int __cdecl __MINGW_NOTHROW _wchmod(const wchar_t*, int);
_CRTIMP int __cdecl __MINGW_NOTHROW _wcreat(const wchar_t*, int);
#if __MSVCRT_VERSION__ < 0x0800
_CRTIMP long __cdecl __MINGW_NOTHROW _wfindfirst(const wchar_t*, struct _wfinddata_t*);
_CRTIMP int __cdecl __MINGW_NOTHROW _wfindnext(long, struct _wfinddata_t *);
#else
_CRTIMP long __cdecl __MINGW_NOTHROW	_wfindfirst32 (const wchar_t*, struct __wfinddata32_t*);
_CRTIMP int  __cdecl __MINGW_NOTHROW	_wfindnext32 (long, struct __wfinddata32_t*);
#endif /* __MSVCRT_VERSION__ < 0x0800 */
_CRTIMP int __cdecl __MINGW_NOTHROW _wunlink(const wchar_t*);
_CRTIMP int __cdecl __MINGW_NOTHROW _wopen(const wchar_t*, int, ...);
_CRTIMP int __cdecl __MINGW_NOTHROW _wsopen(const wchar_t*, int, int, ...);
_CRTIMP wchar_t * __cdecl __MINGW_NOTHROW _wmktemp(wchar_t*);
#if __MSVCRT_VERSION__ < 0x0800
_CRTIMP long __cdecl __MINGW_NOTHROW _wfindfirsti64(const wchar_t*, struct _wfinddatai64_t*);
_CRTIMP int __cdecl __MINGW_NOTHROW _wfindnexti64(long, struct _wfinddatai64_t*);
#else
_CRTIMP long __cdecl __MINGW_NOTHROW	_wfindfirst32i64 (const wchar_t*, struct _wfinddata32i64_t*);
_CRTIMP long __cdecl __MINGW_NOTHROW	_wfindfirst64i32 (const wchar_t*, struct _wfinddata64i32_t*);
_CRTIMP int  __cdecl __MINGW_NOTHROW	_wfindnext32i64 (long, struct _wfinddata32i64_t*);
_CRTIMP int  __cdecl __MINGW_NOTHROW	_wfindnext64i32 (long, struct _wfinddata64i32_t*);
#endif /* __MSVCRT_VERSION__ < 0x0800 */
#if __MSVCRT_VERSION__ >= 0x0601
_CRTIMP intptr_t __cdecl __MINGW_NOTHROW _wfindfirst64(const wchar_t*, struct __wfinddata64_t*); 
_CRTIMP intptr_t __cdecl __MINGW_NOTHROW _wfindnext64(intptr_t, struct __wfinddata64_t*);
#endif
#if __MSVCRT_VERSION__ >= 0x0800
#ifndef _USE_32BIT_TIME_T
_CRTALIAS long __cdecl __MINGW_NOTHROW	_wfindfirst (const wchar_t* _v1, struct _wfinddata_t* _v2)	 { return(_wfindfirst64i32 (_v1,(struct _wfinddata64i32_t*)_v2)); }
_CRTALIAS int  __cdecl __MINGW_NOTHROW	_wfindnext (long _v1, struct _wfinddata_t* _v2)			 { return(_wfindnext64i32  (_v1,(struct _wfinddata64i32_t*)_v2)); }
_CRTALIAS long __cdecl __MINGW_NOTHROW	_wfindfirsti64 (const wchar_t* _v1, struct _wfinddatai64_t* _v2) { return(_wfindfirst64 (_v1,(struct __wfinddata64_t*)_v2)); }
_CRTALIAS int  __cdecl __MINGW_NOTHROW	_wfindnexti64 (long _v1, struct _wfinddatai64_t* _v2)		 { return(_wfindnext64  (_v1,(struct __wfinddata64_t*)_v2)); }
#else
_CRTALIAS long __cdecl __MINGW_NOTHROW	_wfindfirst (const wchar_t* _v1, struct _wfinddata_t* _v2)	 { return(_wfindfirst32 (_v1,(struct __wfinddata32_t*)_v2)); }
_CRTALIAS int  __cdecl __MINGW_NOTHROW	_wfindnext (long _v1, struct _wfinddata_t* _v2)			 { return(_wfindnext32  (_v1,(struct __wfinddata32_t*)_v2)); }
_CRTALIAS long __cdecl __MINGW_NOTHROW	_wfindfirsti64 (const wchar_t* _v1, struct _wfinddatai64_t* _v2) { return(_wfindfirst32i64 (_v1,(struct _wfinddata32i64_t*)_v2)); }
_CRTALIAS int  __cdecl __MINGW_NOTHROW	_wfindnexti64 (long _v1, struct _wfinddatai64_t* _v2)		 { return(_wfindnext32i64  (_v1,(struct _wfinddata32i64_t*)_v2)); }
#endif /* !_USE_32BIT_TIME_T */
#endif /* __MSVCRT_VERSION__ >= 0x0800 */
#endif /* defined (__MSVCRT__) */
#define _WIO_DEFINED
#endif /* _WIO_DEFINED */

#ifndef	_NO_OLDNAMES
/*
 * Non-underscored versions of non-ANSI functions to improve portability.
 * These functions live in libmoldname.a.
 */

#ifndef _UWIN
_CRTIMP int __cdecl __MINGW_NOTHROW access (const char*, int);
_CRTIMP int __cdecl __MINGW_NOTHROW chsize (int, long );
_CRTIMP int __cdecl __MINGW_NOTHROW close (int);
_CRTIMP int __cdecl __MINGW_NOTHROW creat (const char*, int);
_CRTIMP int __cdecl __MINGW_NOTHROW dup (int);
_CRTIMP int __cdecl __MINGW_NOTHROW dup2 (int, int);
_CRTIMP int __cdecl __MINGW_NOTHROW eof (int);
_CRTIMP long __cdecl __MINGW_NOTHROW filelength (int);
_CRTIMP int __cdecl __MINGW_NOTHROW isatty (int);
_CRTIMP long __cdecl __MINGW_NOTHROW lseek (int, long, int);
_CRTIMP int __cdecl __MINGW_NOTHROW open (const char*, int, ...);
_CRTIMP int __cdecl __MINGW_NOTHROW read (int, void*, unsigned int);
_CRTIMP int __cdecl __MINGW_NOTHROW setmode (int, int);
_CRTIMP int __cdecl __MINGW_NOTHROW sopen (const char*, int, int, ...);
_CRTIMP long __cdecl __MINGW_NOTHROW tell (int);
_CRTIMP int __cdecl __MINGW_NOTHROW umask (int);
_CRTIMP int __cdecl __MINGW_NOTHROW unlink (const char*);
_CRTIMP int __cdecl __MINGW_NOTHROW write (int, const void*, unsigned int);
#endif /* _UWIN */

#ifdef __USE_MINGW_ACCESS
/*  Old versions of MSVCRT access() just ignored X_OK, while the version
    shipped with Vista, returns an error code.  This will restore the
    old behaviour  */
static inline int __mingw_access (const char* __fname, int __mode)
  { return  _access (__fname, __mode & ~X_OK); }
#define access(__f,__m)  __mingw_access (__f, __m)
#endif

/* Wide character versions. Also declared in wchar.h. */
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

#endif	/* Not _NO_OLDNAMES */

#ifdef	__cplusplus
}
#endif

#endif	/* Not RC_INVOKED */

#endif	/* _IO_H_ not defined */

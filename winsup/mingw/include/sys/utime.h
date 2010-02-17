/*
 * utime.h
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is a part of the mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within the package.
 *
 * Support for the utime function.
 *
 */
#ifndef	_UTIME_H_
#define	_UTIME_H_

/* All the headers include this file. */
#include <_mingw.h>

#define __need_wchar_t
#define __need_size_t
#ifndef RC_INVOKED
#include <stddef.h>
#endif	/* Not RC_INVOKED */
#include <sys/types.h>

#ifndef	RC_INVOKED

/*
 * Structure used by _utime function.
 */
struct _utimbuf
{
	time_t	actime;		/* Access time */
	time_t	modtime;	/* Modification time */
};
#if __MSVCRT_VERSION__ >= 0x0800
struct __utimbuf32
{
	__time32_t actime;
	__time32_t modtime;
};
#endif /* __MSVCRT_VERSION__ >= 0x0800 */


#ifndef	_NO_OLDNAMES
/* NOTE: Must be the same as _utimbuf above. */
struct utimbuf
{
	time_t	actime;
	time_t	modtime;
};
#endif	/* Not _NO_OLDNAMES */

#ifdef	__cplusplus
extern "C" {
#endif

#if __MSVCRT_VERSION__ < 0x0800
_CRTIMP int __cdecl __MINGW_NOTHROW	_utime (const char*, struct _utimbuf*);
#endif

#ifndef	_NO_OLDNAMES
/* FIXME for __MSVCRT_VERSION__ >= 0x0800 */
_CRTIMP int __cdecl __MINGW_NOTHROW	utime (const char*, struct utimbuf*);
#endif	/* Not _NO_OLDNAMES */

#if __MSVCRT_VERSION__ < 0x0800
_CRTIMP int __cdecl __MINGW_NOTHROW	_futime (int, struct _utimbuf*);
#endif

/* The wide character version, only available for MSVCRT versions of the
 * C runtime library. */
#ifdef __MSVCRT__
#if __MSVCRT_VERSION__ < 0x0800
_CRTIMP int __cdecl __MINGW_NOTHROW	_wutime (const wchar_t*, struct _utimbuf*);
#endif
#endif /* MSVCRT runtime */

/* These require newer versions of msvcrt.dll (6.10 or higher).  */ 
#if __MSVCRT_VERSION__ >= 0x0601
struct __utimbuf64
{
	__time64_t actime;
	__time64_t modtime;
};

_CRTIMP int __cdecl __MINGW_NOTHROW	_utime64 (const char*, struct __utimbuf64*);
_CRTIMP int __cdecl __MINGW_NOTHROW	_wutime64 (const wchar_t*, struct __utimbuf64*);
_CRTIMP int __cdecl __MINGW_NOTHROW	_futime64 (int, struct __utimbuf64*);
#endif /* __MSVCRT_VERSION__ >= 0x0601 */

#if __MSVCRT_VERSION__ >= 0x0800
_CRTIMP int __cdecl __MINGW_NOTHROW	_utime32 (const char*, struct __utimbuf32*);
_CRTIMP int __cdecl __MINGW_NOTHROW	_wutime32 (const wchar_t*, struct __utimbuf32*);
_CRTIMP int __cdecl __MINGW_NOTHROW	_futime32 (int, struct __utimbuf32*);
#ifndef _USE_32BIT_TIME_T
_CRTALIAS int __cdecl __MINGW_NOTHROW	_utime (const char* _v1, struct _utimbuf* _v2)	   { return(_utime64  (_v1,(struct __utimbuf64*)_v2)); }
_CRTALIAS int __cdecl __MINGW_NOTHROW	_wutime (const wchar_t* _v1, struct _utimbuf* _v2) { return(_wutime64 (_v1,(struct __utimbuf64*)_v2)); }
_CRTALIAS int __cdecl __MINGW_NOTHROW	_futime (int _v1, struct _utimbuf* _v2)		   { return(_futime64 (_v1,(struct __utimbuf64*)_v2)); }
#else
_CRTALIAS int __cdecl __MINGW_NOTHROW	_utime (const char* _v1, struct _utimbuf* _v2)	   { return(_utime32  (_v1,(struct __utimbuf32*)_v2)); }
_CRTALIAS int __cdecl __MINGW_NOTHROW	_wutime (const wchar_t* _v1, struct _utimbuf* _v2) { return(_wutime32 (_v1,(struct __utimbuf32*)_v2)); }
_CRTALIAS int __cdecl __MINGW_NOTHROW	_futime (int _v1, struct _utimbuf* _v2)		   { return(_futime32 (_v1,(struct __utimbuf32*)_v2)); }
#endif
#endif /* __MSVCRT_VERSION__ >= 0x0800 */

#ifdef	__cplusplus
}
#endif

#endif	/* Not RC_INVOKED */

#endif	/* Not _UTIME_H_ */

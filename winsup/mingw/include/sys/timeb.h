/*
 * timeb.h
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is a part of the mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within the package.
 *
 * Support for the UNIX System V ftime system call.
 *
 */

#ifndef	_TIMEB_H_
#define	_TIMEB_H_

/* All the headers include this file. */
#include <_mingw.h>
#include <sys/types.h>

#ifndef	RC_INVOKED

/*
 * TODO: Structure not tested.
 */
struct _timeb
{
	time_t	time;
	short	millitm;
	short	timezone;
	short	dstflag;
};

#if __MSVCRT_VERSION__ >= 0x0800
/*
 * TODO: Structure not tested.
 */
struct __timeb32
{
	__time32_t	time;
	short	millitm;
	short	timezone;
	short	dstflag;
};
#endif /* __MSVCRT_VERSION__ >= 0x0800 */

#ifndef	_NO_OLDNAMES
/*
 * TODO: Structure not tested.
 */
struct timeb
{
	time_t	time;
	short	millitm;
	short	timezone;
	short	dstflag;
};
#endif

#ifdef	__cplusplus
extern "C" {
#endif

/* TODO: Not tested. */
_CRTIMP void __cdecl __MINGW_NOTHROW	_ftime (struct _timeb*);

#ifndef	_NO_OLDNAMES
/* FIXME for __MSVCRT_VERSION__ >= 0x0800 */
_CRTIMP void __cdecl __MINGW_NOTHROW	ftime (struct timeb*);
#endif	/* Not _NO_OLDNAMES */

/* This requires newer versions of msvcrt.dll (6.10 or higher).  */ 
#if __MSVCRT_VERSION__ >= 0x0601
struct __timeb64
{
  __time64_t time;
  short millitm;
  short timezone;
  short dstflag;
};

_CRTIMP void __cdecl __MINGW_NOTHROW	_ftime64 (struct __timeb64*);
#endif /* __MSVCRT_VERSION__ >= 0x0601 */

#if __MSVCRT_VERSION__ >= 0x0800
_CRTIMP void __cdecl __MINGW_NOTHROW	_ftime32 (struct __timeb32*);
#ifndef _USE_32BIT_TIME_T
_CRTALIAS void __cdecl __MINGW_NOTHROW	_ftime (struct _timeb* _v) { return(_ftime64 ((struct __timeb64*)_v)); }
#else
_CRTALIAS void __cdecl __MINGW_NOTHROW	_ftime (struct _timeb* _v) { return(_ftime32 ((struct __timeb32*)_v)); }
#endif
#endif /* __MSVCRT_VERSION__ >= 0x0800 */

#ifdef	__cplusplus
}
#endif

#endif	/* Not RC_INVOKED */

#endif	/* Not _TIMEB_H_ */

/* 
 * assert.h
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is a part of the mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within the package.
 *
 * Define the assert macro for debug output.
 *
 */

#ifndef _ASSERT_H_
#define	_ASSERT_H_

/* All the headers include this file. */
#include <_mingw.h>

#ifndef RC_INVOKED

#ifdef	__cplusplus
extern "C" {
#endif

#ifdef NDEBUG

/*
 * If not debugging, assert does nothing.
 */
#define assert(x)	((void)0)

#else /* debugging enabled */

/*
 * CRTDLL nicely supplies a function which does the actual output and
 * call to abort.
 */
_CRTIMP void __cdecl _assert (const char*, const char*, int)
#ifdef	__GNUC__
	__attribute__ ((noreturn))
#endif
	;

/*
 * Definition of the assert macro.
 */
#define assert(e)       ((e) ? (void)0 : _assert(#e, __FILE__, __LINE__))
#endif	/* NDEBUG */

#ifdef	__cplusplus
}
#endif

#endif /* Not RC_INVOKED */

#endif /* Not _ASSERT_H_ */


/* 
 * wctype.h
 *
 * Functions for testing wide character types and converting characters.
 *
 * This file is part of the Mingw32 package.
 *
 * Contributors:
 *  Created by Mumit Khan <khan@xraylith.wisc.edu>
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
 */

#ifndef _WCTYPE_H_
#define _WCTYPE_H_

/* All the headers include this file. */
#include <_mingw.h>

#define	__need_wchar_t
#define	__need_wint_t
#ifndef RC_INVOKED
#include <stddef.h>
#endif	/* Not RC_INVOKED */

/*
 * The following flags are used to tell iswctype and _isctype what character
 * types you are looking for.
 */
#define	_UPPER		0x0001
#define	_LOWER		0x0002
#define	_DIGIT		0x0004
#define	_SPACE		0x0008
#define	_PUNCT		0x0010
#define	_CONTROL	0x0020
#define	_BLANK		0x0040
#define	_HEX		0x0080
#define	_LEADBYTE	0x8000

#define	_ALPHA		0x0103

#ifndef RC_INVOKED

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WEOF
#define	WEOF	(wchar_t)(0xFFFF)
#endif

#ifndef _WCTYPE_T_DEFINED
typedef wchar_t wctype_t;
#define _WCTYPE_T_DEFINED
#endif

/* Wide character equivalents - also in ctype.h */
int	iswalnum(wint_t);
int	iswalpha(wint_t);
int	iswascii(wint_t);
int	iswcntrl(wint_t);
int	iswctype(wint_t, wctype_t);
int	is_wctype(wint_t, wctype_t);	/* Obsolete! */
int	iswdigit(wint_t);
int	iswgraph(wint_t);
int	iswlower(wint_t);
int	iswprint(wint_t);
int	iswpunct(wint_t);
int	iswspace(wint_t);
int	iswupper(wint_t);
int	iswxdigit(wint_t);

wchar_t	towlower(wchar_t);
wchar_t	towupper(wchar_t);

int	isleadbyte (int);

/* Also in ctype.h */

#ifdef __DECLSPEC_SUPPORTED
__MINGW_IMPORT unsigned short _ctype[];
# ifdef __MSVCRT__
  __MINGW_IMPORT unsigned short* _pctype;
# else	/* CRTDLL */
  __MINGW_IMPORT unsigned short* _pctype_dll;
# define  _pctype _pctype_dll
# endif

#else		/* ! __DECLSPEC_SUPPORTED */
extern unsigned short** _imp___ctype;
#define _ctype (*_imp___ctype)
# ifdef __MSVCRT__
  extern unsigned short** _imp___pctype;
# define _pctype (*_imp___pctype)
# else	/* CRTDLL */
  extern unsigned short** _imp___pctype_dll;
# define _pctype (*_imp___pctype_dll)
# endif	/* CRTDLL */
#endif		/*  __DECLSPEC_SUPPORTED */


#if !(defined(__NO_CTYPE_INLINES) || defined(__WCTYPE_INLINES_DEFINED))
#define __WCTYPE_INLINES_DEFINED
extern __inline__ int iswalnum(wint_t wc) {return (iswctype(wc,_ALPHA|_DIGIT));}
extern __inline__ int iswalpha(wint_t wc) {return (iswctype(wc,_ALPHA));}
extern __inline__ int iswascii(wint_t wc) {return ((wc & ~0x7F) ==0);}
extern __inline__ int iswcntrl(wint_t wc) {return (iswctype(wc,_CONTROL));}
extern __inline__ int iswdigit(wint_t wc) {return (iswctype(wc,_DIGIT));}
extern __inline__ int iswgraph(wint_t wc) {return (iswctype(wc,_PUNCT|_ALPHA|_DIGIT));}
extern __inline__ int iswlower(wint_t wc) {return (iswctype(wc,_LOWER));}
extern __inline__ int iswprint(wint_t wc) {return (iswctype(wc,_BLANK|_PUNCT|_ALPHA|_DIGIT));}
extern __inline__ int iswpunct(wint_t wc) {return (iswctype(wc,_PUNCT));}
extern __inline__ int iswspace(wint_t wc) {return (iswctype(wc,_SPACE));}
extern __inline__ int iswupper(wint_t wc) {return (iswctype(wc,_UPPER));}
extern __inline__ int iswxdigit(wint_t wc) {return (iswctype(wc,_HEX));}
extern __inline__ int isleadbyte(int c) {return (_pctype[(unsigned char)(c)] & _LEADBYTE);}
#endif /* !(defined(__NO_CTYPE_INLINES) || defined(__WCTYPE_INLINES_DEFINED)) */


typedef wchar_t wctrans_t;
wint_t 		towctrans(wint_t, wctrans_t);
wctrans_t	wctrans(const char*);
wctype_t	wctype(const char*);

#ifdef __cplusplus
}
#endif

#endif	/* Not RC_INVOKED */

#endif	/* Not _WCTYPE_H_ */


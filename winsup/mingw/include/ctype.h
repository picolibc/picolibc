/* 
 * ctype.h
 *
 * Functions for testing character types and converting characters.
 *
 * This file is part of the Mingw32 package.
 *
 * Contributors:
 *  Created by Colin Peters <colin@bird.fu.is.saga-u.ac.jp>
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

#ifndef _CTYPE_H_
#define _CTYPE_H_

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
#define	_SPACE		0x0008 /* HT  LF  VT  FF  CR  SP */
#define	_PUNCT		0x0010
#define	_CONTROL	0x0020
#define	_BLANK		0x0040 /* this is SP only, not SP and HT as in C99  */
#define	_HEX		0x0080
#define	_LEADBYTE	0x8000

#define	_ALPHA		0x0103

#ifndef RC_INVOKED

__BEGIN_CSTD_NAMESPACE

int	isalnum(int);
int	isalpha(int);
int	iscntrl(int);
int	isdigit(int);
int	isgraph(int);
int	islower(int);
int	isprint(int);
int	ispunct(int);
int	isspace(int);
int	isupper(int);
int	isxdigit(int);


/* These are the ANSI versions, with correct checking of argument */
int	tolower(int);
int	toupper(int);

/*
 * NOTE: The above are not old name type wrappers, but functions exported
 * explicitly by MSVCRT/CRTDLL. However, underscored versions are also
 * exported.
 */

__END_CSTD_NAMESPACE
__BEGIN_CGLOBAL_NAMESPACE

#ifndef	__STRICT_ANSI__
/*
 *  These are the cheap non-std versions: The return values are undefined
 *  if the argument is not ASCII char or is not of appropriate case
 */ 
int	_tolower(int);
int	_toupper(int);

#if !defined (__NO_CTYPE_INLINES)
/* these reproduce behaviour of lib underscored versions  */
extern __inline__ int _tolower(int __c) {return ( __c -'A'+'a');}
extern __inline__ int _toupper(int __c) {return ( __c -'a'+'A');}
#endif

int	_isctype (int, int);

#endif /* __STRICT_ANSI__ */

__END_CGLOBAL_NAMESPACE
__BEGIN_CSTD_NAMESPACE

/* Also defined in stdlib.h */
#ifndef MB_CUR_MAX
#ifdef __DECLSPEC_SUPPORTED
# ifdef __MSVCRT__
#  define MB_CUR_MAX __mb_cur_max
   __MINGW_IMPORT int __mb_cur_max;
# else	/* not __MSVCRT */
#  define MB_CUR_MAX __mb_cur_max_dll
   __MINGW_IMPORT int __mb_cur_max_dll;
# endif	/* not __MSVCRT */

#else		/* ! __DECLSPEC_SUPPORTED */
# ifdef __MSVCRT__
   extern int* _imp____mbcur_max;
#  define MB_CUR_MAX (*_imp____mb_cur_max)
# else		/* not __MSVCRT */
   extern int*  _imp____mbcur_max_dll;
#  define MB_CUR_MAX (*_imp____mb_cur_max_dll)
# endif 	/* not __MSVCRT */
#endif  	/*  __DECLSPEC_SUPPORTED */
#endif  /* MB_CUR_MAX */


#ifdef __DECLSPEC_SUPPORTED
__MINGW_IMPORT unsigned short _ctype[];
# ifdef __MSVCRT__
  __MINGW_IMPORT unsigned short* _pctype;
# else /* CRTDLL */
  __MINGW_IMPORT unsigned short* _pctype_dll;
# define  _pctype _pctype_dll
# endif 

#else		/*  __DECLSPEC_SUPPORTED */
extern unsigned short** _imp___ctype;
#define _ctype (*_imp___ctype)
# ifdef __MSVCRT__
  extern unsigned short** _imp___pctype;
# define _pctype (*_imp___pctype)
# else /* CRTDLL */
  extern unsigned short** _imp___pctype_dll;
# define _pctype (*_imp___pctype_dll)
# endif /* CRTDLL */
#endif		/*  __DECLSPEC_SUPPORTED */

/*
 * Use inlines here rather than macros, because macros will upset 
 * C++ usage (eg, ::isalnum), and so usually get undefined
 *
 * According to standard for SB chars, these function are defined only
 * for input values representable by unsigned char or EOF.
 * Thus, there is no range test.
 * This reproduces behaviour of MSVCRT.dll lib implemention for SB chars.
 *
 * If no MB char support is needed, these can be simplified even
 * more by command line define -DMB_CUR_MAX=1.  The compiler will then
 * optimise away the constant condition.			
 */

#if ! (defined (__NO_CTYPE_INLINES) || defined (__STRICT_ANSI__ ))
/* use  simple lookup if SB locale, else  _isctype()  */
#define __ISCTYPE(__c, __mask) \
  (MB_CUR_MAX == 1 ? (_pctype[__c] & __mask) : __CGLOBAL _isctype(__c, __mask))
extern __inline__ int isalnum(int __c) {return __ISCTYPE(__c, (_ALPHA|_DIGIT));}
extern __inline__ int isalpha(int __c) {return __ISCTYPE(__c, _ALPHA);}
extern __inline__ int iscntrl(int __c) {return __ISCTYPE(__c, _CONTROL);}
extern __inline__ int isdigit(int __c) {return __ISCTYPE(__c, _DIGIT);}
extern __inline__ int isgraph(int __c) {return __ISCTYPE(__c, (_PUNCT|_ALPHA|_DIGIT));}
extern __inline__ int islower(int __c) {return __ISCTYPE(__c, _LOWER);}
extern __inline__ int isprint(int __c) {return __ISCTYPE(__c, (_BLANK|_PUNCT|_ALPHA|_DIGIT));}
extern __inline__ int ispunct(int __c) {return __ISCTYPE(__c, _PUNCT);}
extern __inline__ int isspace(int __c) {return __ISCTYPE(__c, _SPACE);}
extern __inline__ int isupper(int __c) {return __ISCTYPE(__c, _UPPER);}
extern __inline__ int isxdigit(int __c) {return __ISCTYPE(__c, _HEX);}

/* TODO? Is it worth inlining ANSI tolower, toupper? Probably only
   if we only want C-locale. */

#endif /* _NO_CTYPE_INLINES */

/* Wide character equivalents
   Also in wctype.h */

#ifndef WEOF
#define	WEOF	(wchar_t)(0xFFFF)
#endif

#ifndef _WCTYPE_T_DEFINED
typedef wchar_t wctype_t;
#define _WCTYPE_T_DEFINED
#endif

int	iswalnum(wint_t);
int	iswalpha(wint_t);
int	iswascii(wint_t);
int	iswcntrl(wint_t);
int	iswctype(wint_t, wctype_t);
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

#if ! (defined(__NO_CTYPE_INLINES) || defined(__WCTYPE_INLINES_DEFINED))
#define __WCTYPE_INLINES_DEFINED
extern __inline__ int iswalnum(wint_t __wc) {return (iswctype(__wc,_ALPHA|_DIGIT));}
extern __inline__ int iswalpha(wint_t __wc) {return (iswctype(__wc,_ALPHA));}
extern __inline__ int iswascii(wint_t __wc) {return (((unsigned)__wc & 0x7F) ==0);}
extern __inline__ int iswcntrl(wint_t __wc) {return (iswctype(__wc,_CONTROL));}
extern __inline__ int iswdigit(wint_t __wc) {return (iswctype(__wc,_DIGIT));}
extern __inline__ int iswgraph(wint_t __wc) {return (iswctype(__wc,_PUNCT|_ALPHA|_DIGIT));}
extern __inline__ int iswlower(wint_t __wc) {return (iswctype(__wc,_LOWER));}
extern __inline__ int iswprint(wint_t __wc) {return (iswctype(__wc,_BLANK|_PUNCT|_ALPHA|_DIGIT));}
extern __inline__ int iswpunct(wint_t __wc) {return (iswctype(__wc,_PUNCT));}
extern __inline__ int iswspace(wint_t __wc) {return (iswctype(__wc,_SPACE));}
extern __inline__ int iswupper(wint_t __wc) {return (iswctype(__wc,_UPPER));}
extern __inline__ int iswxdigit(wint_t __wc) {return (iswctype(__wc,_HEX));}
extern __inline__ int isleadbyte(int __c)
  {return (_pctype[(unsigned char)(__c)] & _LEADBYTE);}
#endif /* !(defined(__NO_CTYPE_INLINES) || defined(__WCTYPE_INLINES_DEFINED)) */
__END_CSTD_NAMESPACE


#ifndef	__STRICT_ANSI__
__BEGIN_CGLOBAL_NAMESPACE

int	__isascii (int);
int	__toascii (int);
int	__iscsymf (int);	/* Valid first character in C symbol */
int	__iscsym (int);		/* Valid character in C symbol (after first) */

#ifndef __NO_CTYPE_INLINES
extern __inline__ int __isascii(int __c)
  {return (((unsigned)__c & ~0x7F) == 0);} 
extern __inline__ int __toascii(int __c) {return  (__c & 0x7F);}
extern __inline__ int __iscsymf(int __c) {return (__CSTD isalpha(__c) || (__c == '_'));}
extern __inline__ int __iscsym(int __c)  {return  (__CSTD isalnum(__c) || (__c == '_'));}
#endif /* __NO_CTYPE_INLINES */

#ifndef	_NO_OLDNAMES
int	isascii (int);
int	toascii (int);
int	iscsymf (int);
int	iscsym (int);
#endif	/* Not _NO_OLDNAMES */

int	is_wctype(wint_t, __CSTD wctype_t);	/* Obsolete! */

__END_CGLOBAL_NAMESPACE
#endif	/* Not __STRICT_ANSI__ */

#endif	/* Not RC_INVOKED */

#endif	/* Not _CTYPE_H_ */


/*
Copyright (c) 1991, 1993
The Regents of the University of California.  All rights reserved.
c) UNIX System Laboratories, Inc.
All or some portions of this file are derived from material licensed
to the University of California by American Telephone and Telegraph
Co. or Unix System Laboratories, Inc. and are reproduced herein with
the permission of UNIX System Laboratories, Inc.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. Neither the name of the University nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
 */
#ifndef _CTYPE_H_
#define _CTYPE_H_

#include <sys/cdefs.h>

#if __POSIX_VISIBLE >= 200809 || __MISC_VISIBLE
#include <sys/_locale.h>
#endif

_BEGIN_STD_C

/*
 * The small ctype code does not support extended character sets. It also breaks
 * libstdc++'s ctype implementation, so just skip it for c++
 */
#if defined(__MB_EXTENDED_CHARSETS_NON_UNICODE) || defined (__cplusplus)
#undef _PICOLIBC_CTYPE_SMALL
#define _PICOLIBC_CTYPE_SMALL 0
#endif

/*
 * The default ctype style depends upon whether we are optimizing for
 * size and whether the library supports charset-specific ctype data
 */
#if !defined(_PICOLIBC_CTYPE_SMALL)
#ifdef __OPTIMIZE_SIZE__
#define _PICOLIBC_CTYPE_SMALL 1
#else
#define _PICOLIBC_CTYPE_SMALL 0
#endif
#endif

int isalnum (int c);
int isalpha (int c);
int iscntrl (int c);
int isdigit (int c);
int isgraph (int c);
int islower (int c);
int isprint (int c);
int ispunct (int c);
int isspace (int c);
int isupper (int c);
int isxdigit (int c);
int tolower (int c);
int toupper (int c);

#if __ISO_C_VISIBLE >= 1999
int isblank (int c);
#endif

#if __MISC_VISIBLE || __XSI_VISIBLE
int isascii (int c);
int toascii (int c);
#define _tolower(__c) ((__c) + ('a' - 'A'))
#define _toupper(__c) ((__c) - ('a' - 'A'))
#define isascii(__c)	((unsigned)(__c)<=0177)
#define toascii(__c)	((__c)&0177)
#endif

#if __POSIX_VISIBLE >= 200809
int isalnum_l (int c, locale_t l);
int isalpha_l (int c, locale_t l);
int isblank_l (int c, locale_t l);
int iscntrl_l (int c, locale_t l);
int isdigit_l (int c, locale_t l);
int isgraph_l (int c, locale_t l);
int islower_l (int c, locale_t l);
int isprint_l (int c, locale_t l);
int ispunct_l (int c, locale_t l);
int isspace_l (int c, locale_t l);
int isupper_l (int c, locale_t l);
int isxdigit_l(int c, locale_t l);
int tolower_l (int c, locale_t l);
int toupper_l (int c, locale_t l);
#endif

#if __MISC_VISIBLE
int isascii_l (int c, locale_t l);
int toascii_l (int c, locale_t l);
#define isascii_l(__c,__l)	((void) (__l),(unsigned)(__c)<=0177)
#define toascii_l(__c,__l)	((void) (__l),(__c)&0177)
#endif

#if _PICOLIBC_CTYPE_SMALL

#ifdef __declare_extern_inline

__declare_extern_inline(int) isblank (int c)
{
    return c == ' ' || c == '\t';
}

__declare_extern_inline(int) iscntrl (int c)
{
    return (0x00 <= c && c <= 0x1f) || c == 0x7f;
}

__declare_extern_inline(int) isdigit (int c)
{
    return '0' <= c && c <= '9';
}

__declare_extern_inline(int) isgraph (int c)
{
    return '!' <= c && c <= '~';
}

__declare_extern_inline(int) islower (int c)
{
    return 'a' <= c && c <= 'z';
}

__declare_extern_inline(int) isprint (int c)
{
    return ' ' <= c && c <= '~';
}

__declare_extern_inline(int) ispunct (int c)
{
    return (('!' <= c && c <= '/') ||
            (':' <= c && c <= '@') ||
            ('[' <= c && c <= '`') ||
            ('{' <= c && c <= '~'));
}

__declare_extern_inline(int) isspace (int c)
{
    return c == ' ' || ('\t' <= c && c <= '\r');
}

__declare_extern_inline(int) isupper (int c)
{
    return 'A' <= c && c <= 'Z';
}

__declare_extern_inline(int) isxdigit (int c)
{
    return (isdigit(c) ||
            ('A' <= c && c <= 'F') ||
            ('a' <= c && c <= 'f'));
}

__declare_extern_inline(int) isalpha (int c)
{
    return isupper(c) || islower(c);
}

__declare_extern_inline(int) isalnum (int c)
{
    return isalpha(c) || isdigit(c);
}

__declare_extern_inline(int) tolower (int c)
{
    if (isupper(c))
        c = c - 'A' + 'a';
    return c;
}

__declare_extern_inline(int) toupper (int c)
{
    if (islower(c))
        c = c - 'a' + 'A';
    return c;
}

#if __POSIX_VISIBLE >= 200809
__declare_extern_inline(int) isalnum_l (int c, locale_t l) { (void) l; return isalnum(c); }
__declare_extern_inline(int) isalpha_l (int c, locale_t l) { (void) l; return isalpha(c); }
__declare_extern_inline(int) isblank_l (int c, locale_t l) { (void) l; return isblank(c); }
__declare_extern_inline(int) iscntrl_l (int c, locale_t l) { (void) l; return iscntrl(c); }
__declare_extern_inline(int) isdigit_l (int c, locale_t l) { (void) l; return isdigit(c); }
__declare_extern_inline(int) isgraph_l (int c, locale_t l) { (void) l; return isgraph(c); }
__declare_extern_inline(int) islower_l (int c, locale_t l) { (void) l; return islower(c); }
__declare_extern_inline(int) isprint_l (int c, locale_t l) { (void) l; return isprint(c); }
__declare_extern_inline(int) ispunct_l (int c, locale_t l) { (void) l; return ispunct(c); }
__declare_extern_inline(int) isspace_l (int c, locale_t l) { (void) l; return isspace(c); }
__declare_extern_inline(int) isupper_l (int c, locale_t l) { (void) l; return isupper(c); }
__declare_extern_inline(int) isxdigit_l(int c, locale_t l) { (void) l; return isxdigit(c); }
__declare_extern_inline(int) tolower_l (int c, locale_t l) { (void) l; return tolower(c); }
__declare_extern_inline(int) toupper_l (int c, locale_t l) { (void) l; return toupper(c); }
#endif

#endif /* __declare_extern_inline */

#else  /* _PICOLIBC_CTYPE_SMALL */

#define _CTYPE_OFFSET   127

extern const char       _ctype_b[];
extern const short      _ctype_wide[];

#define _ctype_ (_ctype_b + _CTYPE_OFFSET)

#define	__CTYPE_UPPER	0x001    /* upper */
#define	__CTYPE_LOWER	0x002    /* lower */
#define	__CTYPE_DIGIT	0x004    /* digit */
#define	__CTYPE_SPACE	0x008    /* space */
#define __CTYPE_PUNCT	0x010    /* punct */
#define __CTYPE_CNTRL	0x020    /* control */
#define __CTYPE_HEX	0x040    /* hex */
#define	__CTYPE_BLANK	0x080    /* blank (but not tab) */
#define __CTYPE_TAB     0x100    /* tab (only in wide table) */

#ifdef __cplusplus
/* We need these legacy symbols to build libstdc++ */
#define _U __CTYPE_UPPER
#define _L __CTYPE_LOWER
#define _N __CTYPE_DIGIT
#define _S __CTYPE_SPACE
#define _P __CTYPE_PUNCT
#define _C __CTYPE_CNTRL
#define _X __CTYPE_HEX
#define _B __CTYPE_BLANK
#endif

#ifdef __MB_EXTENDED_CHARSETS_NON_UNICODE
const char *__locale_ctype_ptr (void);
#define __CTYPE_PTR     __locale_ctype_ptr()
#else
#define __CTYPE_PTR	_ctype_
#endif

#ifndef __cplusplus

#define __ctype_lookup(__c) (__CTYPE_PTR + 1)[(int) (__c)]

#define	isalpha(__c)	(__ctype_lookup(__c)&(__CTYPE_UPPER|__CTYPE_LOWER))
#define	isupper(__c)	((__ctype_lookup(__c)&(__CTYPE_UPPER|__CTYPE_LOWER))==__CTYPE_UPPER)
#define	islower(__c)	((__ctype_lookup(__c)&(__CTYPE_UPPER|__CTYPE_LOWER))==__CTYPE_LOWER)
#define	isdigit(__c)	(__ctype_lookup(__c)&__CTYPE_DIGIT)
#define	isxdigit(__c)	(__ctype_lookup(__c)&(__CTYPE_HEX|__CTYPE_DIGIT))
#define	isspace(__c)	(__ctype_lookup(__c)&__CTYPE_SPACE)
#define ispunct(__c)	(__ctype_lookup(__c)&__CTYPE_PUNCT)
#define isalnum(__c)	(__ctype_lookup(__c)&(__CTYPE_UPPER|__CTYPE_LOWER|__CTYPE_DIGIT))
#define isprint(__c)	(__ctype_lookup(__c)&(__CTYPE_PUNCT|__CTYPE_UPPER|__CTYPE_LOWER|__CTYPE_DIGIT|__CTYPE_BLANK))
#define	isgraph(__c)	(__ctype_lookup(__c)&(__CTYPE_PUNCT|__CTYPE_UPPER|__CTYPE_LOWER|__CTYPE_DIGIT))
#define iscntrl(__c)	(__ctype_lookup(__c)&__CTYPE_CNTRL)

#if __ISO_C_VISIBLE >= 1999 && defined(__declare_extern_inline)
__declare_extern_inline(int) isblank(int c) {
    return c == '\t' || __ctype_lookup(c) & __CTYPE_BLANK;
}
#endif

#if __POSIX_VISIBLE >= 200809

#ifdef __MB_EXTENDED_CHARSETS_NON_UNICODE
const char *__locale_ctype_ptr_l (locale_t);
#define __CTYPE_PTR_L(__l)        __locale_ctype_ptr_l(__l)
#else
#define __CTYPE_PTR_L(__l) ((void) (__l), _ctype_)
#endif

#define __ctype_lookup_l(__c, __l) ((__CTYPE_PTR_L(__l)+1)[(int)(__c)])

#define	isalpha_l(__c,__l)	(__ctype_lookup_l(__c,__l)&(__CTYPE_UPPER|__CTYPE_LOWER))
#define	isupper_l(__c,__l)	((__ctype_lookup_l(__c,__l)&(__CTYPE_UPPER|__CTYPE_LOWER))==__CTYPE_UPPER)
#define	islower_l(__c,__l)	((__ctype_lookup_l(__c,__l)&(__CTYPE_UPPER|__CTYPE_LOWER))==__CTYPE_LOWER)
#define	isdigit_l(__c,__l)	(__ctype_lookup_l(__c,__l)&__CTYPE_DIGIT)
#define	isxdigit_l(__c,__l)	(__ctype_lookup_l(__c,__l)&(__CTYPE_HEX|__CTYPE_DIGIT))
#define	isspace_l(__c,__l)	(__ctype_lookup_l(__c,__l)&__CTYPE_SPACE)
#define ispunct_l(__c,__l)	(__ctype_lookup_l(__c,__l)&__CTYPE_PUNCT)
#define isalnum_l(__c,__l)	(__ctype_lookup_l(__c,__l)&(__CTYPE_UPPER|__CTYPE_LOWER|__CTYPE_DIGIT))
#define isprint_l(__c,__l)	(__ctype_lookup_l(__c,__l)&(__CTYPE_PUNCT|__CTYPE_UPPER|__CTYPE_LOWER|__CTYPE_DIGIT|__CTYPE_BLANK))
#define	isgraph_l(__c,__l)	(__ctype_lookup_l(__c,__l)&(__CTYPE_PUNCT|__CTYPE_UPPER|__CTYPE_LOWER|__CTYPE_DIGIT))
#define iscntrl_l(__c,__l)	(__ctype_lookup_l(__c,__l)&__CTYPE_CNTRL)

#ifdef __declare_extern_inline
__declare_extern_inline(int) isblank_l(int c, locale_t l) {
    return c == '\t' || (__ctype_lookup_l(c, l) & __CTYPE_BLANK);
}
#endif

#endif /* __POSIX_VISIBLE >= 200809 */

#endif /* !__cplusplus */

#endif /* else _PICOLIBC_CTYPE_SMALL */

_END_STD_C

#endif /* _CTYPE_H_ */

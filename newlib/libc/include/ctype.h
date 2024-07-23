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
 * The small ctype code does not support locales or extended character sets. It also breaks
 * libstdc++'s ctype implementation, so just skip it for c++
 */
#if defined(__HAVE_LOCALE_INFO__) || \
    defined (_MB_EXTENDED_CHARSETS_ISO) || \
    defined (_MB_EXTENDED_CHARSETS_WINDOWS) || \
    defined (__cplusplus)
#undef _PICOLIBC_CTYPE_SMALL
#define _PICOLIBC_CTYPE_SMALL 0
#endif

/*
 * The default ctype style depends upon whether we are optimizing for
 * size and whether the library supports locale-specific ctype data
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
#define isascii_l(__c,__l)	((__l),(unsigned)(__c)<=0177)
#define toascii_l(__c,__l)	((__l),(__c)&0177)
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

#endif

#else

#define	_U	01
#define	_L	02
#define	_N	04
#define	_S	010
#define _P	020
#define _C	040
#define _X	0100
#define	_B	0200

#ifndef __CHAR_UNSIGNED__
#define ALLOW_NEGATIVE_CTYPE_INDEX
#endif

#if defined(ALLOW_NEGATIVE_CTYPE_INDEX)
extern const char	_ctype_b[];
#define _ctype_ (_ctype_b + 127)
#else
extern const char	_ctype_[];
#endif

#ifdef __HAVE_LOCALE_INFO__
const char *__locale_ctype_ptr (void);
#else
#define __locale_ctype_ptr()	_ctype_
#endif

# define __CTYPE_PTR	(__locale_ctype_ptr ())

#ifndef __cplusplus
static __inline char __ctype_lookup(int c) { return (__CTYPE_PTR + 1)[c]; }

#define	isalpha(__c)	(__ctype_lookup(__c)&(_U|_L))
#define	isupper(__c)	((__ctype_lookup(__c)&(_U|_L))==_U)
#define	islower(__c)	((__ctype_lookup(__c)&(_U|_L))==_L)
#define	isdigit(__c)	(__ctype_lookup(__c)&_N)
#define	isxdigit(__c)	(__ctype_lookup(__c)&(_X|_N))
#define	isspace(__c)	(__ctype_lookup(__c)&_S)
#define ispunct(__c)	(__ctype_lookup(__c)&_P)
#define isalnum(__c)	(__ctype_lookup(__c)&(_U|_L|_N))
#define isprint(__c)	(__ctype_lookup(__c)&(_P|_U|_L|_N|_B))
#define	isgraph(__c)	(__ctype_lookup(__c)&(_P|_U|_L|_N))
#define iscntrl(__c)	(__ctype_lookup(__c)&_C)

#if __ISO_C_VISIBLE >= 1999
#if defined(__GNUC__)
#define isblank(__c)                                            \
    __extension__ ({ __typeof__ (__c) __x = (__c);		\
            (__ctype_lookup(__x)&_B) || (int) (__x) == '\t';})
#endif
#endif

#if __POSIX_VISIBLE >= 200809
#ifdef __HAVE_LOCALE_INFO__
const char *__locale_ctype_ptr_l (locale_t);
#else
static __inline const char *
__locale_ctype_ptr_l(locale_t _l)
{
	(void)_l;
	return __locale_ctype_ptr();
}
#endif

static __inline char __ctype_lookup_l(int c, locale_t l) {
	return (__locale_ctype_ptr_l(l)+1)[(int)(c)];
}

#define	isalpha_l(__c,__l)	(__ctype_lookup_l(__c,__l)&(_U|_L))
#define	isupper_l(__c,__l)	((__ctype_lookup_l(__c,__l)&(_U|_L))==_U)
#define	islower_l(__c,__l)	((__ctype_lookup_l(__c,__l)&(_U|_L))==_L)
#define	isdigit_l(__c,__l)	(__ctype_lookup_l(__c,__l)&_N)
#define	isxdigit_l(__c,__l)	(__ctype_lookup_l(__c,__l)&(_X|_N))
#define	isspace_l(__c,__l)	(__ctype_lookup_l(__c,__l)&_S)
#define ispunct_l(__c,__l)	(__ctype_lookup_l(__c,__l)&_P)
#define isalnum_l(__c,__l)	(__ctype_lookup_l(__c,__l)&(_U|_L|_N))
#define isprint_l(__c,__l)	(__ctype_lookup_l(__c,__l)&(_P|_U|_L|_N|_B))
#define	isgraph_l(__c,__l)	(__ctype_lookup_l(__c,__l)&(_P|_U|_L|_N))
#define iscntrl_l(__c,__l)	(__ctype_lookup_l(__c,__l)&_C)

#if defined(__GNUC__)
#define isblank_l(__c, __l) \
  __extension__ ({ __typeof__ (__c) __x = (__c);		\
        (__ctype_lookup_l(__x,__l)&_B) || (int) (__x) == '\t';})
#endif

#endif /* __POSIX_VISIBLE >= 200809 */

/* Non-gcc versions will get the library versions, and will be
   slightly slower.  These macros are not NLS-aware so they are
   disabled if the system supports the extended character sets. */
# if defined(__GNUC__)
#  if !defined (_MB_EXTENDED_CHARSETS_ISO) && !defined (_MB_EXTENDED_CHARSETS_WINDOWS)
#   define toupper(__c) \
  __extension__ ({ __typeof__ (__c) __x = (__c);	\
      islower (__x) ? (int) __x - 'a' + 'A' : (int) __x;})
#   define tolower(__c) \
  __extension__ ({ __typeof__ (__c) __x = (__c);	\
      isupper (__x) ? (int) __x - 'A' + 'a' : (int) __x;})
#  else /* _MB_EXTENDED_CHARSETS* */
/* Allow a gcc warning if the user passed 'char', but defer to the
   function.  */
#   define toupper(__c) \
  __extension__ ({ __typeof__ (__c) __x = (__c);	\
      (void) __CTYPE_PTR[__x]; (toupper) (__x);})
#   define tolower(__c) \
  __extension__ ({ __typeof__ (__c) __x = (__c);	\
      (void) __CTYPE_PTR[__x]; (tolower) (__x);})
#  endif /* _MB_EXTENDED_CHARSETS* */
# endif /* __GNUC__ */

#endif /* !__cplusplus */

#endif /* else _PICOLIBC_CTYPE_SMALL */

_END_STD_C

#endif /* _CTYPE_H_ */

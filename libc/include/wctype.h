/* Copyright (c) 2002 Jeff Johnston  <jjohnstn@redhat.com> */
#ifndef _WCTYPE_H_
#define _WCTYPE_H_

#include <sys/cdefs.h>
#include <sys/_types.h>

#define __need_wint_t
#include <stddef.h>

#if __POSIX_VISIBLE >= 200809
#include <sys/_locale.h>
#endif

#ifndef WEOF
#define WEOF ((wint_t) - 1)
#endif

_BEGIN_STD_C

#ifndef _WCTYPE_DECLARED
typedef __wctype_t wctype_t;
#define _WCTYPE_DECLARED
#endif

#ifndef _WCTRANS_DECLARED
typedef int wctrans_t;
#define _WCTRANS_DECLARED
#endif

int iswalnum(wint_t) __picolibc_export;
int iswalpha(wint_t) __picolibc_export;
#if __ISO_C_VISIBLE >= 1999
int iswblank(wint_t) __picolibc_export;
#endif
int       iswcntrl(wint_t) __picolibc_export;
int       iswctype(wint_t, wctype_t) __picolibc_export;
int       iswdigit(wint_t) __picolibc_export;
int       iswgraph(wint_t) __picolibc_export;
int       iswlower(wint_t) __picolibc_export;
int       iswprint(wint_t) __picolibc_export;
int       iswpunct(wint_t) __picolibc_export;
int       iswspace(wint_t) __picolibc_export;
int       iswupper(wint_t) __picolibc_export;
int       iswxdigit(wint_t) __picolibc_export;
wint_t    towctrans(wint_t, wctrans_t) __picolibc_export;
wint_t    towupper(wint_t) __picolibc_export;
wint_t    towlower(wint_t) __picolibc_export;
wctrans_t wctrans(const char *) __picolibc_export;
wctype_t  wctype(const char *) __picolibc_export;

#if __POSIX_VISIBLE >= 200809
int       iswalnum_l(wint_t, locale_t) __picolibc_export;
int       iswalpha_l(wint_t, locale_t) __picolibc_export;
int       iswblank_l(wint_t, locale_t) __picolibc_export;
int       iswcntrl_l(wint_t, locale_t) __picolibc_export;
int       iswctype_l(wint_t, wctype_t, locale_t) __picolibc_export;
int       iswdigit_l(wint_t, locale_t) __picolibc_export;
int       iswgraph_l(wint_t, locale_t) __picolibc_export;
int       iswlower_l(wint_t, locale_t) __picolibc_export;
int       iswprint_l(wint_t, locale_t) __picolibc_export;
int       iswpunct_l(wint_t, locale_t) __picolibc_export;
int       iswspace_l(wint_t, locale_t) __picolibc_export;
int       iswupper_l(wint_t, locale_t) __picolibc_export;
int       iswxdigit_l(wint_t, locale_t) __picolibc_export;
wint_t    towctrans_l(wint_t, wctrans_t, locale_t) __picolibc_export;
wint_t    towupper_l(wint_t, locale_t) __picolibc_export;
wint_t    towlower_l(wint_t, locale_t) __picolibc_export;
wctrans_t wctrans_l(const char *, locale_t) __picolibc_export;
wctype_t  wctype_l(const char *, locale_t) __picolibc_export;
#endif

_END_STD_C

#endif /* _WCTYPE_H_ */

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
# define WEOF ((wint_t)-1)
#endif

_BEGIN_STD_C

#ifndef _WCTYPE_DECLARED
typedef int wctype_t;
#define _WCTYPE_DECLARED
#endif

#ifndef _WCTRANS_DECLARED
typedef int wctrans_t;
#define _WCTRANS_DECLARED
#endif

int	iswalnum (wint_t);
int	iswalpha (wint_t);
#if __ISO_C_VISIBLE >= 1999
int	iswblank (wint_t);
#endif
int	iswcntrl (wint_t);
int	iswctype (wint_t, wctype_t);
int	iswdigit (wint_t);
int	iswgraph (wint_t);
int	iswlower (wint_t);
int	iswprint (wint_t);
int	iswpunct (wint_t);
int	iswspace (wint_t);
int	iswupper (wint_t);
int	iswxdigit (wint_t);
wint_t	towctrans (wint_t, wctrans_t);
wint_t	towupper (wint_t);
wint_t	towlower (wint_t);
wctrans_t wctrans (const char *);
wctype_t wctype (const char *);

#if __POSIX_VISIBLE >= 200809
int	iswalnum_l (wint_t, locale_t);
int	iswalpha_l (wint_t, locale_t);
int	iswblank_l (wint_t, locale_t);
int	iswcntrl_l (wint_t, locale_t);
int	iswctype_l (wint_t, wctype_t, locale_t);
int	iswdigit_l (wint_t, locale_t);
int	iswgraph_l (wint_t, locale_t);
int	iswlower_l (wint_t, locale_t);
int	iswprint_l (wint_t, locale_t);
int	iswpunct_l (wint_t, locale_t);
int	iswspace_l (wint_t, locale_t);
int	iswupper_l (wint_t, locale_t);
int	iswxdigit_l (wint_t, locale_t);
wint_t	towctrans_l (wint_t, wctrans_t, locale_t);
wint_t	towupper_l (wint_t, locale_t);
wint_t	towlower_l (wint_t, locale_t);
wctrans_t wctrans_l (const char *, locale_t);
wctype_t wctype_l (const char *, locale_t);
#endif

_END_STD_C

#endif /* _WCTYPE_H_ */

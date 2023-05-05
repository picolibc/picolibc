/*
Copyright (c) 1990 Regents of the University of California.
All rights reserved.
 */
/* Misc. local definitions for libc/stdlib */

#ifndef _LOCAL_H_
#define _LOCAL_H_

char *	_gcvt (double , int , char *, char, int);

#include "../locale/setlocale.h"

#ifndef __machine_mbstate_t_defined
#include <wchar.h>
#endif

typedef int wctomb_f (char *, wchar_t, mbstate_t *);
typedef wctomb_f *wctomb_p;

wctomb_f __ascii_wctomb;
#ifdef _MB_CAPABLE
wctomb_f __utf8_wctomb;
wctomb_f __sjis_wctomb;
wctomb_f __eucjp_wctomb;
wctomb_f __jis_wctomb;
wctomb_p __iso_wctomb (int val);
wctomb_p __cp_wctomb (int val);
#ifdef __CYGWIN__
wctomb_f __gbk_wctomb;
wctomb_f __gb18030_wctomb;
wctomb_f __kr_wctomb;
wctomb_f __big5_wctomb;
#endif
#endif

#define __WCTOMB (__get_current_locale()->wctomb)

typedef int mbtowc_f (wchar_t *, const char *, size_t,
		      mbstate_t *);
typedef mbtowc_f *mbtowc_p;

mbtowc_f __ascii_mbtowc;
#ifdef _MB_CAPABLE
mbtowc_f __utf8_mbtowc;
mbtowc_f __sjis_mbtowc;
mbtowc_f __eucjp_mbtowc;
mbtowc_f __jis_mbtowc;
mbtowc_p __iso_mbtowc (int val);
mbtowc_p __cp_mbtowc (int val);
#ifdef __CYGWIN__
mbtowc_f __gbk_mbtowc;
mbtowc_f __gb18030_mbtowc;
mbtowc_f __kr_mbtowc;
mbtowc_f __big5_mbtowc;
#endif
#endif

#define __MBTOWC (__get_current_locale()->mbtowc)

extern wchar_t __iso_8859_conv[14][0x60];
int __iso_8859_val_index (int);
int __iso_8859_index (const char *);

extern wchar_t __cp_conv[][0x80];
int __cp_val_index (int);
int __cp_index (const char *);

#endif

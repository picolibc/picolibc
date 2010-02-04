/* wchar.h: Extra wchar defs

   Copyright 2007, 2009, 2010 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGWIN_WCHAR_H
#define _CYGWIN_WCHAR_H

#include_next <wchar.h>

#define ENCODING_LEN 31

#ifdef __cplusplus
extern "C" {
#endif

typedef int mbtowc_f (struct _reent *, wchar_t *, const char *, size_t,
		      const char *, mbstate_t *);
typedef mbtowc_f *mbtowc_p;

extern mbtowc_p __mbtowc;
extern mbtowc_f __ascii_mbtowc;
extern mbtowc_f __utf8_mbtowc;
extern mbtowc_f __iso_mbtowc;
extern mbtowc_f __cp_mbtowc;
extern mbtowc_f __sjis_mbtowc;
extern mbtowc_f __eucjp_mbtowc;
extern mbtowc_f __gbk_mbtowc;
extern mbtowc_f __kr_mbtowc;
extern mbtowc_f __big5_mbtowc;

typedef int wctomb_f (struct _reent *, char *, wchar_t, const char *,
		      mbstate_t *);
typedef wctomb_f *wctomb_p;

extern wctomb_p __wctomb;
extern wctomb_f __ascii_wctomb;
extern wctomb_f __utf8_wctomb;

extern char *__locale_charset ();

#ifdef __cplusplus
}
#endif

#ifdef __INSIDE_CYGWIN__
#ifdef __cplusplus
size_t __stdcall sys_cp_wcstombs (wctomb_p, const char *, char *, size_t,
				  const wchar_t *, size_t = (size_t) -1)
       __attribute__ ((regparm(3)));
size_t __stdcall sys_wcstombs (char *dst, size_t len, const wchar_t * src,
			       size_t nwc = (size_t) -1)
       __attribute__ ((regparm(3)));
size_t __stdcall sys_wcstombs_alloc (char **, int, const wchar_t *,
				     size_t = (size_t) -1)
       __attribute__ ((regparm(3)));

size_t __stdcall sys_cp_mbstowcs (mbtowc_p, const char *, wchar_t *, size_t,
				  const char *, size_t = (size_t) -1)
       __attribute__ ((regparm(3)));
size_t __stdcall sys_mbstowcs (wchar_t * dst, size_t dlen, const char *src,
		     size_t nms = (size_t) -1)
       __attribute__ ((regparm(3)));
size_t __stdcall sys_mbstowcs_alloc (wchar_t **, int, const char *,
				     size_t = (size_t) -1)
       __attribute__ ((regparm(3)));
#endif /* __cplusplus */
#endif /* __INSIDE_CYGWIN__ */

#endif /* _CYGWIN_WCHAR_H */

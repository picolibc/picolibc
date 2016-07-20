/* wchar.h: Extra wchar defs

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGWIN_WCHAR_H
#define _CYGWIN_WCHAR_H

#include_next <wchar.h>

/* Internal headers from newlib */
#include "../locale/setlocale.h"

#define ENCODING_LEN 31

#ifdef __cplusplus
extern "C" {
#endif

typedef int mbtowc_f (struct _reent *, wchar_t *, const char *, size_t,
		      mbstate_t *);
typedef mbtowc_f *mbtowc_p;

extern mbtowc_f __ascii_mbtowc;
extern mbtowc_f __utf8_mbtowc;
extern mbtowc_p __iso_mbtowc (int);
extern mbtowc_p __cp_mbtowc (int);

#define __MBTOWC (__get_current_locale ()->mbtowc)

typedef int wctomb_f (struct _reent *, char *, wchar_t, mbstate_t *);
typedef wctomb_f *wctomb_p;

extern wctomb_f __ascii_wctomb;
extern wctomb_f __utf8_wctomb;

#define __WCTOMB (__get_current_locale ()->wctomb)

#ifdef __cplusplus
}
#endif

#ifdef __INSIDE_CYGWIN__
#ifdef __cplusplus
size_t __reg3 sys_wcstombs (char *dst, size_t len, const wchar_t * src,
			    size_t nwc = (size_t) -1);
size_t __reg3 sys_wcstombs_no_path (char *dst, size_t len,
				    const wchar_t * src,
				    size_t nwc = (size_t) -1);
size_t __reg3 sys_wcstombs_alloc (char **, int, const wchar_t *,
				  size_t = (size_t) -1);
size_t __reg3 sys_wcstombs_alloc_no_path (char **, int, const wchar_t *,
					  size_t = (size_t) -1);

size_t __reg3 sys_cp_mbstowcs (mbtowc_p, wchar_t *, size_t, const char *,
			       size_t = (size_t) -1);
size_t __reg3 sys_mbstowcs (wchar_t * dst, size_t dlen, const char *src,
			    size_t nms = (size_t) -1);
size_t __reg3 sys_mbstowcs_alloc (wchar_t **, int, const char *,
				  size_t = (size_t) -1);
#endif /* __cplusplus */
#endif /* __INSIDE_CYGWIN__ */

#endif /* _CYGWIN_WCHAR_H */

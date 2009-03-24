/* wchar.h: Extra wchar defs

   Copyright 2007, 2009 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGWIN_WCHAR_H
#define _CYGWIN_WCHAR_H

#include_next <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int mbtowc_f (struct _reent *, wchar_t *, const char *, size_t,
		      const char *, mbstate_t *);
typedef mbtowc_f *mbtowc_p;

extern int __utf8_wctomb (struct _reent *, char *, wchar_t,
			      const char *, mbstate_t *);

extern int (*__mbtowc) (struct _reent *, wchar_t *, const char *, size_t,
                 const char *, mbstate_t *);
extern mbtowc_f __ascii_mbtowc;
extern mbtowc_f __utf8_mbtowc;
extern mbtowc_f __iso_mbtowc;
extern mbtowc_f __cp_mbtowc;

extern char *__locale_charset ();

extern mbtowc_p __set_charset_from_codepage (unsigned int cp, char *charset);

#ifdef __cplusplus
}
#endif
#endif /* _CYGWIN_WCHAR_H */

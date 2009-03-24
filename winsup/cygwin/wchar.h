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

extern "C" int __utf8_wctomb (struct _reent *, char *, wchar_t,
			      const char *, mbstate_t *);

typedef int mbtowc_f (struct _reent *, wchar_t *, const char *, size_t,
		      const char *, mbstate_t *);
typedef mbtowc_f *mbtowc_p;

extern "C" mbtowc_p __mbtowc;
extern "C" mbtowc_f __ascii_mbtowc;
extern "C" mbtowc_f __utf8_mbtowc;
extern "C" mbtowc_f __iso_mbtowc;
extern "C" mbtowc_f __cp_mbtowc;

extern "C" char *__locale_charset ();

extern "C" mbtowc_p __set_charset_from_codepage (UINT cp, char *charset);

#ifdef __cplusplus
}
#endif
#endif /* _CYGWIN_WCHAR_H */

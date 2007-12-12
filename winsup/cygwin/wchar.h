/* wchar.h: Extra wchar defs

   Copyright 2007 Red Hat, Inc.

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

#undef wcscasecmp
#define wcscasecmp cygwin_wcscasecmp
int __stdcall cygwin_wcscasecmp (const wchar_t *, const wchar_t *);

#undef wcsncasecmp
#define wcsncasecmp cygwin_wcsncasecmp
int __stdcall cygwin_wcsncasecmp (const wchar_t *, const wchar_t *, size_t);

#undef wcslwr
#define wcslwr cygwin_wcslwr
wchar_t * __stdcall cygwin_wcslwr (wchar_t *);

#undef wcsupr
#define wcsupr cygwin_wcsupr
wchar_t * __stdcall cygwin_wcsupr (wchar_t *);

#ifdef __cplusplus
}
#endif
#endif /* _CYGWIN_WCHAR_H */

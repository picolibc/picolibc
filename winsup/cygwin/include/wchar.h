/* wchar.h

   Copyright 1998 Cygnus Solutions

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _WCHAR_H
#define _WCHAR_H

#include <sys/cdefs.h>

/* Get wchar_t from <stddef.h>.  */
#define __need_wchar_t
#include <stddef.h>

__BEGIN_DECLS

int wcscmp (wchar_t *__s1, wchar_t *__s2);
int wcslen (wchar_t *__s1);

__END_DECLS

#endif /* _WCHAR_H */

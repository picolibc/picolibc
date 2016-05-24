/* dso_handle.c: Provide default __dso_handle.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

extern void *__ImageBase;
void *__dso_handle = &__ImageBase;

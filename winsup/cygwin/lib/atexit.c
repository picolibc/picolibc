/* atexit.c: atexit entry point

   Copyright 2014 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stddef.h>
#include <windows.h>

/* Statically linked replacement for the former cygwin_atexit.  We need
   the function here to be able to access the correct __dso_handle of the
   caller's DSO. */
int
atexit (void (*fn) (void))
{
  extern int __cxa_atexit(void (*)(void*), void*, void*);
  extern void *__dso_handle;
  extern void *__ImageBase;

  /* Check for being called from inside the executable.  If so, use NULL
     as __dso_handle.  This allows to link executables with GCC versions
     not providing __dso_handle in crtbegin{S}.o.  In this case our own
     __dso_handle defined in lib/dso_handle.c is used.  However, our
     __dso_handle always points to &__ImageBase, while the __dso_handle
     for executables provided by crtbegin.o usually points to NULL.
     That's what we remodel here. */
  return __cxa_atexit ((void (*)(void*))fn, NULL,
		       &__ImageBase == (void **) GetModuleHandleW (NULL)
		       ? NULL : &__dso_handle);
}

/* atexit.c: atexit entry point

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stddef.h>
#include <sys/cygwin.h>
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

  void *fixed_dso_handle = &__dso_handle;
  /* Check for being called from inside the executable.  If so, use NULL
     as __dso_handle.  This allows to link executables with GCC versions
     not providing __dso_handle in crtbegin{S}.o.  In this case our own
     __dso_handle defined in lib/dso_handle.c is used.  However, our
     __dso_handle always points to &__ImageBase, while the __dso_handle
     for executables provided by crtbegin.o usually points to NULL.
     That's what we remodel here. */
  if (&__ImageBase == (void **) GetModuleHandleW (NULL))
    fixed_dso_handle = NULL;
  /* With recent Cygwin versions starting with API version 0.280 we call
     __cxa_atexit (which is actually the cygwin__cxa_atexit wrapper in
     dcrt0.cc) with the address of __dso_handle since that's how g++ generates
     calls to __cxa_atexit as well.  However, when running an application
     built with this atexit under an older Cygwin version, the __cxa_atexit
     entry point is the one from newlib, which expects the *value* of
     __dso_handle.  So, check for the Cygwin version we're running under.
     Older version prior to 0.280 don't know CW_FIXED_ATEXIT and return -1.
     0.280 and later return 0. */
  else if (cygwin_internal (CW_FIXED_ATEXIT) != 0)
    fixed_dso_handle = __dso_handle;

  return __cxa_atexit ((void (*)(void*))fn, NULL, fixed_dso_handle);
}

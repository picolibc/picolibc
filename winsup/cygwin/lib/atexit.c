/* atexit.c: atexit entry point

   Copyright 2014 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <stddef.h>

/* Statically linked replacement for the former cygwin_atexit.  We need
   the function here to be able to access the correct __dso_handle of the
   caller's DSO. */
int
atexit (void (*fn) (void))
{
  extern int __cxa_atexit(void (*)(void*), void*, void*);
  extern void *__dso_handle;

  return __cxa_atexit ((void (*)(void*))fn, NULL, &__dso_handle);
}

/* perthread.h: Header file for cygwin synchronization primitives.

   Copyright 2000 Cygnus Solutions.

   Written by Christopher Faylor <cgf@cygnus.com>

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define PTMAGIC 0x77366377

struct _reent;
extern struct _reent reent_data;

extern DWORD *__stackbase __asm__ ("%fs:4");

extern __inline struct _reent *
get_reent ()
{
  DWORD *base = __stackbase - 1;

  if (*base != PTMAGIC)
    return &reent_data;
  return (struct _reent *) base[-1];
}

extern inline void
set_reent (struct _reent *r)
{
  DWORD *base = __stackbase - 1;

  *base = PTMAGIC;
  base[-1] = (DWORD) r;
}

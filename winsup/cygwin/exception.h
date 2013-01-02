/* exception.h

   Copyright 2010, 2011, 2012, 2013 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#pragma once

#include <exceptions.h>

extern exception_list *_except_list asm ("%fs:0");

class exception
{
  exception_list el;
  exception_list *save;
  static int handle (EXCEPTION_RECORD *, exception_list *, CONTEXT *, void *);
public:
  exception () __attribute__ ((always_inline))
  {
    save = _except_list;
    el.handler = handle;
    el.prev = _except_list;
    _except_list = &el;
  };
  ~exception () __attribute__ ((always_inline)) { _except_list = save; }
};

class cygwin_exception
{
  DWORD ebp;
  PCONTEXT ctx;
  EXCEPTION_RECORD *e;
  void dump_exception ();
public:
  cygwin_exception (DWORD in_ebp, PCONTEXT in_ctx = NULL, EXCEPTION_RECORD *in_e = NULL):
    ebp (in_ebp), ctx (in_ctx), e (in_e) {}
  void dumpstack ();
  PCONTEXT context () const {return ctx;}
};

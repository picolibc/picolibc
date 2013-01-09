/* exception.h

   Copyright 2010, 2011, 2012, 2013 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#pragma once

#include <exceptions.h>

#ifdef __x86_64__
extern exception_list *_except_list asm ("%gs:0");
#else
extern exception_list *_except_list asm ("%fs:0");
#endif

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
  PUINT_PTR framep;
  PCONTEXT ctx;
  EXCEPTION_RECORD *e;
  void dump_exception ();
public:
  cygwin_exception (PUINT_PTR in_framep, PCONTEXT in_ctx = NULL, EXCEPTION_RECORD *in_e = NULL):
    framep (in_framep), ctx (in_ctx), e (in_e) {}
  void dumpstack ();
  PCONTEXT context () const {return ctx;}
};

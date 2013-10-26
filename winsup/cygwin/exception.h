/* exception.h

   Copyright 2010, 2011, 2012, 2013 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#pragma once

#include <exceptions.h>

#ifndef __x86_64__
extern exception_list *_except_list asm ("%fs:0");
#endif

class exception
{
#ifdef __x86_64__
  static bool handler_installed; 
  static int handle (LPEXCEPTION_POINTERS);
#else
  exception_list el;
  exception_list *save;
  static int handle (EXCEPTION_RECORD *, exception_list *, CONTEXT *, void *);
#endif
public:
  exception () __attribute__ ((always_inline))
  {
#ifdef __x86_64__
    if (!handler_installed)
      {
	handler_installed = true;
	/* The unhandled exception filter goes first.  It won't work if the
	   executable is debugged, but then the vectored continue handler
	   kicks in.  For some reason the vectored continue handler doesn't
	   get called if no unhandled exception filter is installed. */
	SetUnhandledExceptionFilter (handle);
	AddVectoredContinueHandler (1, handle);
      }
#else
    save = _except_list;
    el.handler = handle;
    el.prev = _except_list;
    _except_list = &el;
#endif
  };
#ifndef __x86_64__
  ~exception () __attribute__ ((always_inline)) { _except_list = save; }
#endif
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

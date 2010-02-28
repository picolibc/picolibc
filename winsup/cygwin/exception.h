/* exception.h

   Copyright 2003, 2004, 2005, 2008, 2009 Red Hat, Inc.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include <exceptions.h>

extern exception_list *_except_list asm ("%fs:0");

class exception
{
  exception_list el;
  exception_list *save;
  static int handle (EXCEPTION_RECORD *, exception_list *, CONTEXT *, void *);
public:
#ifdef DEBUG_EXCEPTION
  exception ();
  ~exception ();
#else
  exception () __attribute__ ((always_inline))
  {
    save = _except_list;
    el.handler = handle;
    el.prev = _except_list;
    _except_list = &el;
  };
  ~exception () __attribute__ ((always_inline)) { _except_list = save; }
#endif
};

#endif /*_CYGTLS_H*/ /*gentls_offsets*/


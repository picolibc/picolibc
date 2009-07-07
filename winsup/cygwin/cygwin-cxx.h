/* cygwin-cxx.h

   Copyright 2009 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _CYGWIN_CXX_H
#define _CYGWIN_CXX_H

#ifndef __cplusplus
#error This header should not be included by C source files.
#endif

/* Files including this header must override -nostdinc++ */
#include <new>

/* This is an optional struct pointed to by per_process if it exists.  */
struct per_process_cxx_malloc
{
  void *(*oper_new) (std::size_t);
  void *(*oper_new__) (std::size_t);
  void (*oper_delete) (void *);
  void (*oper_delete__) (void *);
  void *(*oper_new_nt) (std::size_t, const std::nothrow_t &);
  void *(*oper_new___nt) (std::size_t, const std::nothrow_t &);
  void (*oper_delete_nt) (void *, const std::nothrow_t &);
  void (*oper_delete___nt) (void *, const std::nothrow_t &);
};

/* Defined in cxx.cc  */
extern struct per_process_cxx_malloc default_cygwin_cxx_malloc;

#endif /* _CYGWIN_CXX_H */

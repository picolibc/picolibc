/* hires.h: Definitions for hires clock calculations

   Copyright 2002 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef __HIRES_H__
#define __HIRES_H__

class hires
{
  int inited;
  LARGE_INTEGER primed_ft;
  LARGE_INTEGER primed_pc;
  double freq;
  void prime () __attribute__ ((regparm (1)));
 public:
  LONGLONG usecs (bool justdelta) __attribute__ ((regparm (2)));
};
#endif /*__HIRES_H__*/

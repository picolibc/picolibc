/* hires.h: Definitions for hires clock calculations

   Copyright 2002 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef __HIRES_H__
#define __HIRES_H__

#include <mmsystem.h>

class hires_base
{
 protected:
  int inited;
  virtual void prime () {}
 public:
  virtual LONGLONG usecs (bool justdelta) {return 0LL;}
  virtual ~hires_base () {}
};

class hires_us : hires_base
{
  LARGE_INTEGER primed_ft;
  LARGE_INTEGER primed_pc;
  double freq;
  void prime ();
 public:
  LONGLONG usecs (bool justdelta);
};

class hires_ms : hires_base
{
  DWORD initime_ms;
  LARGE_INTEGER initime_us;
  UINT minperiod;
  void prime ();
 public:
  LONGLONG usecs (bool justdelta);
  ~hires_ms ();
};
#endif /*__HIRES_H__*/

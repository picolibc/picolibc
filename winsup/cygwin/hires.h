/* hires.h: Definitions for hires clock calculations

   Copyright 2002 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef __HIRES_H__
#define __HIRES_H__

#include <mmsystem.h>

/* Largest delay in ms for sleep and alarm calls.
   Allow actual delay to exceed requested delay by 10 s.
   Express as multiple of 1000 (i.e. seconds) + max resolution
   The tv_sec argument in timeval structures cannot exceed
   HIRES_DELAY_MAX / 1000 - 1, so that adding fractional part
   and rounding won't exceed HIRES_DELAY_MAX */
#define HIRES_DELAY_MAX (((UINT_MAX - 10000) / 1000) * 1000) + 10

class hires_base
{
 protected:
  int inited;
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
  static UINT minperiod;
  UINT prime ();
 public:
  LONGLONG usecs (bool justdelta);
  UINT dmsecs () { return timeGetTime (); }
  UINT resolution () { return minperiod ?: prime (); }

};

extern hires_ms gtod;
#endif /*__HIRES_H__*/

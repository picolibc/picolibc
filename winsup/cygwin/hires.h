/* hires.h: Definitions for hires clock calculations

   Copyright 2002, 2003, 2004, 2005, 2009 Red Hat, Inc.

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
#define HIRES_DELAY_MAX ((((UINT_MAX - 10000) / 1000) * 1000) + 10)

class hires_base
{
 protected:
  int inited;
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
  LONGLONG initime_ns;
  void prime ();
 public:
  LONGLONG nsecs ();
  LONGLONG usecs () {return nsecs () / 10LL;}
  LONGLONG msecs () {return nsecs () / 10000LL;}
  UINT dmsecs () { return timeGetTime (); }
  UINT resolution ();
  LONGLONG uptime () {return (nsecs () - initime_ns) / 10000LL;}
};

extern hires_ms gtod;
#endif /*__HIRES_H__*/

/* cygwait.h

   Copyright 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
   Red Hat, Inc.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details.  */

#pragma once

enum cw_wait_mask
{
  cw_cancel =		0x0001,
  cw_cancel_self =	0x0002,
  cw_sig =		0x0004,
  cw_sig_eintr =	0x0008
};

extern TIMER_BASIC_INFORMATION cw_nowait;

const unsigned cw_std_mask = cw_cancel | cw_cancel_self | cw_sig;

DWORD cancelable_wait (HANDLE, PLARGE_INTEGER timeout = NULL,
		       unsigned = cw_std_mask)
  __attribute__ ((regparm (3)));

static inline DWORD __attribute__ ((always_inline))
cancelable_wait (HANDLE h, DWORD howlong, unsigned mask)
{
  PLARGE_INTEGER pli_howlong;
  LARGE_INTEGER li_howlong;
  if (howlong == INFINITE)
    pli_howlong = NULL;
  else
    {
      li_howlong.QuadPart = 10000ULL * howlong;
      pli_howlong = &li_howlong;
    }

  return cancelable_wait (h, pi_howlong, mask);
}

static inline DWORD __attribute__ ((always_inline))
cygwait (HANDLE h, DWORD howlong = INFINITE)
{
  return cancelable_wait (h, howlong, cw_cancel | cw_sig_eintr);
}

static inline DWORD __attribute__ ((always_inline))
cygwait (DWORD howlong)
{
  return cygwait ((HANDLE) NULL, howlong);
}

class set_thread_waiting
{
  void doit (bool setit, DWORD& here)
  {
    if (setit)
      {
	if (_my_tls.signal_arrived == NULL)
	  _my_tls.signal_arrived = CreateEvent (&sec_none_nih, false, false, NULL);
	here = _my_tls.signal_arrived;
	_my_tls.waiting = true;
      }
  }
public:
  set_thread_waiting (bool setit, DWORD& here) { doit (setit, here); }
  set_thread_waiting (DWORD& here) { doit (true, here); }

  ~set_thread_waiting ()
  {
    if (_my_tls.waiting)
      {
	_my_tls.waiting = false;
	ResetEvent (_my_tls.signal_arrived);
      }
  }
};

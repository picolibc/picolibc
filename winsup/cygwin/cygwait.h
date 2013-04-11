/* cygwait.h

   Copyright 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013
   Red Hat, Inc.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details.  */

#pragma once

#define WAIT_CANCELED   (WAIT_OBJECT_0 + 2)
#define WAIT_SIGNALED  (WAIT_OBJECT_0 + 1)

enum cw_wait_mask
{
  cw_cancel =		0x0001,
  cw_cancel_self =	0x0002,
  cw_sig =		0x0004,
  cw_sig_eintr =	0x0008,
  cw_sig_cont =		0x0010
};

extern LARGE_INTEGER cw_nowait_storage;
#define cw_nowait (&cw_nowait_storage)
#define cw_infinite ((PLARGE_INTEGER) NULL)

const unsigned cw_std_mask = cw_cancel | cw_cancel_self | cw_sig;

DWORD __reg3 cygwait (HANDLE, PLARGE_INTEGER timeout,
		       unsigned = cw_std_mask);

extern inline DWORD __attribute__ ((always_inline))
cygwait (HANDLE h, DWORD howlong, unsigned mask)
{
  LARGE_INTEGER li_howlong;
  PLARGE_INTEGER pli_howlong;
  if (howlong == INFINITE)
    pli_howlong = NULL;
  else
    {
      li_howlong.QuadPart = -(10000ULL * howlong);
      pli_howlong = &li_howlong;
    }
  return cygwait (h, pli_howlong, mask);
}

static inline DWORD __attribute__ ((always_inline))
cygwait (HANDLE h, DWORD howlong = INFINITE)
{
  return cygwait (h, howlong, cw_cancel | cw_sig);
}

static inline DWORD __attribute__ ((always_inline))
cygwait (DWORD howlong)
{
  return cygwait (NULL, howlong);
}

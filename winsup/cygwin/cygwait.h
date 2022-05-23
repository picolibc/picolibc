/* cygwait.h

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details.  */

#pragma once

#define WAIT_CANCELED   (WAIT_OBJECT_0 + 2)
#define WAIT_SIGNALED  (WAIT_OBJECT_0 + 1)

enum cw_wait_mask
{
  cw_cancel =		0x0001,	/* Cancellation point.  Return to caller. */
  cw_cancel_self =	0x0002,	/* Cancellation point.  Cancel self. */
  cw_sig =		0x0004,	/* Handle signals. */
  cw_sig_eintr =	0x0008,	/* Caller handles signals. */
  cw_sig_cont =		0x0010,	/* Caller handles SIGCONT. */
  cw_sig_restart =	0x0020	/* Restart even if SA_RESTART isn't set. */
};

extern LARGE_INTEGER cw_nowait_storage;
#define cw_nowait (&cw_nowait_storage)
#define cw_infinite ((PLARGE_INTEGER) NULL)

const unsigned cw_std_mask = cw_cancel | cw_cancel_self | cw_sig;

DWORD cygwait (HANDLE, PLARGE_INTEGER timeout,
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

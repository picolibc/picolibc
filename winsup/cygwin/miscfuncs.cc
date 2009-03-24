/* miscfuncs.cc: misc funcs that don't belong anywhere else

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005, 2006, 2007, 2008 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "miscfuncs.h"
#include <sys/uio.h>
#include <assert.h>
#include <alloca.h>
#include <limits.h>
#include <wchar.h>
#include <wingdi.h>
#include <winuser.h>
#include <winnls.h>
#include "cygtls.h"
#include "ntdll.h"

long tls_ix = -1;

const char case_folded_lower[] NO_COPY = {
   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
  32, '!', '"', '#', '$', '%', '&',  39, '(', ')', '*', '+', ',', '-', '.', '/',
 '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',
 '@', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '[',  92, ']', '^', '_',
 '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', 127,
 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};

const char case_folded_upper[] NO_COPY = {
   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
  32, '!', '"', '#', '$', '%', '&',  39, '(', ')', '*', '+', ',', '-', '.', '/',
 '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',
 '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[',  92, ']', '^', '_',
 '`', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '{', '|', '}', '~', 127,
 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};

const char isalpha_array[] NO_COPY = {
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,   0,   0,   0,   0,   0,
   0,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

extern "C" int __stdcall
cygwin_wcscasecmp (const wchar_t *ws, const wchar_t *wt)
{
  UNICODE_STRING us, ut;

  RtlInitUnicodeString (&us, ws);
  RtlInitUnicodeString (&ut, wt);
  return RtlCompareUnicodeString (&us, &ut, TRUE);
}

extern "C" int __stdcall
cygwin_wcsncasecmp (const wchar_t  *ws, const wchar_t *wt, size_t n)
{
  UNICODE_STRING us, ut;
  size_t ls = 0, lt = 0;

  while (ws[ls] && ls < n)
    ++ls;
  RtlInitCountedUnicodeString (&us, ws, ls * sizeof (WCHAR));
  while (wt[lt] && lt < n)
    ++lt;
  RtlInitCountedUnicodeString (&ut, wt, lt * sizeof (WCHAR));
  return RtlCompareUnicodeString (&us, &ut, TRUE);
}

extern "C" int __stdcall
cygwin_strcasecmp (const char *cs, const char *ct)
{
  UNICODE_STRING us, ut;
  ULONG len;

  len = (strlen (cs) + 1) * sizeof (WCHAR);
  RtlInitEmptyUnicodeString (&us, (PWCHAR) alloca (len), len);
  us.Length = sys_mbstowcs (us.Buffer, us.MaximumLength, cs) * sizeof (WCHAR);
  len = (strlen (ct) + 1) * sizeof (WCHAR);
  RtlInitEmptyUnicodeString (&ut, (PWCHAR) alloca (len), len);
  ut.Length = sys_mbstowcs (ut.Buffer, ut.MaximumLength, ct) * sizeof (WCHAR);
  return RtlCompareUnicodeString (&us, &ut, TRUE);
}

extern "C" int __stdcall
cygwin_strncasecmp (const char *cs, const char *ct, size_t n)
{
  UNICODE_STRING us, ut;
  ULONG len;
  size_t ls = 0, lt = 0;

  while (cs[ls] && ls < n)
    ++ls;
  len = (ls + 1) * sizeof (WCHAR);
  RtlInitEmptyUnicodeString (&us, (PWCHAR) alloca (len), len);
  us.Length = sys_mbstowcs (us.Buffer, ls + 1, cs, ls) * sizeof (WCHAR);
  while (ct[lt] && lt < n)
    ++lt;
  len = (lt + 1) * sizeof (WCHAR);
  RtlInitEmptyUnicodeString (&ut, (PWCHAR) alloca (len), len);
  ut.Length = sys_mbstowcs (ut.Buffer, lt + 1, ct, lt)  * sizeof (WCHAR);
  return RtlCompareUnicodeString (&us, &ut, TRUE);
}

extern "C" char * __stdcall
cygwin_strlwr (char *string)
{
  UNICODE_STRING us;
  size_t len = (strlen (string) + 1) * sizeof (WCHAR);

  us.MaximumLength = len; us.Buffer = (PWCHAR) alloca (len);
  us.Length = sys_mbstowcs (us.Buffer, len, string) * sizeof (WCHAR)
	      - sizeof (WCHAR);
  RtlDowncaseUnicodeString (&us, &us, FALSE);
  sys_wcstombs (string, len / sizeof (WCHAR), us.Buffer);
  return string;
}

extern "C" char * __stdcall
cygwin_strupr (char *string)
{
  UNICODE_STRING us;
  size_t len = (strlen (string) + 1) * sizeof (WCHAR);

  us.MaximumLength = len; us.Buffer = (PWCHAR) alloca (len);
  us.Length = sys_mbstowcs (us.Buffer, len, string) * sizeof (WCHAR)
	      - sizeof (WCHAR);
  RtlUpcaseUnicodeString (&us, &us, FALSE);
  sys_wcstombs (string, len / sizeof (WCHAR), us.Buffer);
  return string;
}

int __stdcall
check_invalid_virtual_addr (const void *s, unsigned sz)
{
  MEMORY_BASIC_INFORMATION mbuf;
  const void *end;

  for (end = (char *) s + sz; s < end;
       s = (char *) mbuf.BaseAddress + mbuf.RegionSize)
    if (!VirtualQuery (s, &mbuf, sizeof mbuf))
      return EINVAL;
  return 0;
}

static char __attribute__ ((noinline))
dummytest (volatile char *p)
{
  return *p;
}

ssize_t
check_iovec (const struct iovec *iov, int iovcnt, bool forwrite)
{
  if (iovcnt <= 0 || iovcnt > IOV_MAX)
    {
      set_errno (EINVAL);
      return -1;
    }

  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;

  size_t tot = 0;

  while (iovcnt != 0)
    {
      if (iov->iov_len > SSIZE_MAX || (tot += iov->iov_len) > SSIZE_MAX)
	{
	  set_errno (EINVAL);
	  return -1;
	}

      volatile char *p = ((char *) iov->iov_base) + iov->iov_len - 1;
      if (!iov->iov_len)
	/* nothing to do */;
      else if (!forwrite)
	*p  = dummytest (p);
      else
	dummytest (p);

      iov++;
      iovcnt--;
    }

  assert (tot <= SSIZE_MAX);

  return (ssize_t) tot;
}

extern "C" int
low_priority_sleep (DWORD secs)
{
  HANDLE thisthread = GetCurrentThread ();
  int curr_prio = GetThreadPriority (thisthread);
  bool staylow;
  if (secs != INFINITE)
    staylow = false;
  else
    {
      secs = 0;
      staylow = true;
    }

  if (!secs)
    {
      for (int i = 0; i < 3; i++)
	SwitchToThread ();
    }
  else
    {
      int new_prio;
      if (GetCurrentThreadId () == cygthread::main_thread_id)
	new_prio = THREAD_PRIORITY_LOWEST;
      else
	new_prio = GetThreadPriority (hMainThread);

      if (curr_prio != new_prio)
	/* Force any threads in normal priority to be scheduled */
	SetThreadPriority (thisthread, new_prio);
      Sleep (secs);

      if (!staylow && curr_prio != new_prio)
	SetThreadPriority (thisthread, curr_prio);
    }

  return curr_prio;
}

/* Get a default value for the nice factor.  When changing these values,
   have a look into the below function nice_to_winprio.  The values must
   match the layout of the static "priority" array. */
int
winprio_to_nice (DWORD prio)
{
  switch (prio)
    {
      case REALTIME_PRIORITY_CLASS:
	return -20;
      case HIGH_PRIORITY_CLASS:
	return -16;
      case ABOVE_NORMAL_PRIORITY_CLASS:
	return -8;
      case NORMAL_PRIORITY_CLASS:
	return 0;
      case BELOW_NORMAL_PRIORITY_CLASS:
	return 8;
      case IDLE_PRIORITY_CLASS:
	return 16;
    }
  return 0;
}

/* Get a Win32 priority matching the incoming nice factor.  The incoming
   nice is limited to the interval [-NZERO,NZERO-1]. */
DWORD
nice_to_winprio (int &nice)
{
  static const DWORD priority[] NO_COPY =
    {
      REALTIME_PRIORITY_CLASS,		/*  0 */
      HIGH_PRIORITY_CLASS,		/*  1 */
      HIGH_PRIORITY_CLASS,
      HIGH_PRIORITY_CLASS,
      HIGH_PRIORITY_CLASS,
      HIGH_PRIORITY_CLASS,
      HIGH_PRIORITY_CLASS,
      HIGH_PRIORITY_CLASS,		/*  7 */
      ABOVE_NORMAL_PRIORITY_CLASS,	/*  8 */
      ABOVE_NORMAL_PRIORITY_CLASS,
      ABOVE_NORMAL_PRIORITY_CLASS,
      ABOVE_NORMAL_PRIORITY_CLASS,
      ABOVE_NORMAL_PRIORITY_CLASS,
      ABOVE_NORMAL_PRIORITY_CLASS,
      ABOVE_NORMAL_PRIORITY_CLASS,
      ABOVE_NORMAL_PRIORITY_CLASS,	/* 15 */
      NORMAL_PRIORITY_CLASS,		/* 16 */
      NORMAL_PRIORITY_CLASS,
      NORMAL_PRIORITY_CLASS,
      NORMAL_PRIORITY_CLASS,
      NORMAL_PRIORITY_CLASS,
      NORMAL_PRIORITY_CLASS,
      NORMAL_PRIORITY_CLASS,
      NORMAL_PRIORITY_CLASS,		/* 23 */
      BELOW_NORMAL_PRIORITY_CLASS,	/* 24 */
      BELOW_NORMAL_PRIORITY_CLASS,
      BELOW_NORMAL_PRIORITY_CLASS,
      BELOW_NORMAL_PRIORITY_CLASS,
      BELOW_NORMAL_PRIORITY_CLASS,
      BELOW_NORMAL_PRIORITY_CLASS,
      BELOW_NORMAL_PRIORITY_CLASS,
      BELOW_NORMAL_PRIORITY_CLASS,	/* 31 */
      IDLE_PRIORITY_CLASS,		/* 32 */
      IDLE_PRIORITY_CLASS,
      IDLE_PRIORITY_CLASS,
      IDLE_PRIORITY_CLASS,
      IDLE_PRIORITY_CLASS,
      IDLE_PRIORITY_CLASS,
      IDLE_PRIORITY_CLASS,
      IDLE_PRIORITY_CLASS		/* 39 */
    };
  if (nice < -NZERO)
    nice = -NZERO;
  else if (nice > NZERO - 1)
    nice = NZERO - 1;
  DWORD prio = priority[nice + NZERO];
  if (!wincap.has_extended_priority_class ()
      && (prio == BELOW_NORMAL_PRIORITY_CLASS
	  || prio == ABOVE_NORMAL_PRIORITY_CLASS))
    prio = NORMAL_PRIORITY_CLASS;
  return prio;
}

#undef CreatePipe
bool
create_pipe (PHANDLE hr,PHANDLE hw, LPSECURITY_ATTRIBUTES sa, DWORD n)
{
  for (int i = 0; i < 10; i++)
    if (CreatePipe (hr, hw, sa, n))
      return true;
    else if (GetLastError () == ERROR_PIPE_BUSY && i < 9)
      Sleep (10);
    else
      break;
  return false;
}

/* backslashify: Convert all forward slashes in src path to back slashes
   in dst path.  Add a trailing slash to dst when trailing_slash_p arg
   is set to 1. */

void
backslashify (const char *src, char *dst, bool trailing_slash_p)
{
  const char *start = src;

  while (*src)
    {
      if (*src == '/')
	*dst++ = '\\';
      else
	*dst++ = *src;
      ++src;
    }
  if (trailing_slash_p
      && src > start
      && !isdirsep (src[-1]))
    *dst++ = '\\';
  *dst++ = 0;
}

/* slashify: Convert all back slashes in src path to forward slashes
   in dst path.  Add a trailing slash to dst when trailing_slash_p arg
   is set to 1. */

void
slashify (const char *src, char *dst, bool trailing_slash_p)
{
  const char *start = src;

  while (*src)
    {
      if (*src == '\\')
	*dst++ = '/';
      else
	*dst++ = *src;
      ++src;
    }
  if (trailing_slash_p
      && src > start
      && !isdirsep (src[-1]))
    *dst++ = '/';
  *dst++ = 0;
}

/* miscfuncs.cc: misc funcs that don't belong anywhere else

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012 Red Hat, Inc.

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
#include <sys/param.h>
#include <wchar.h>
#include <wingdi.h>
#include <winuser.h>
#include <winnls.h>
#include "cygtls.h"
#include "ntdll.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "pinfo.h"
#include "exception.h"

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

/* Try hard to schedule another thread. */
void
yield ()
{
  int prio = GetThreadPriority (GetCurrentThread ());
  SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_IDLE);
  for (int i = 0; i < 2; i++)
    {
      /* MSDN implies that SleepEx(0,...) will force scheduling of other
	 threads.  Unlike SwitchToThread() the documentation does not mention
	 other cpus so, presumably (hah!), this + using a lower priority will
	 stall this thread temporarily and cause another to run.  */
      SleepEx (0, false);
    }
  SetThreadPriority (GetCurrentThread (), prio);
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
  return prio;
}

/* Minimal overlapped pipe I/O implementation for signal and commune stuff. */

BOOL WINAPI
CreatePipeOverlapped (PHANDLE hr, PHANDLE hw, LPSECURITY_ATTRIBUTES sa)
{
  int ret = fhandler_pipe::create (sa, hr, hw, 0, NULL,
				   FILE_FLAG_OVERLAPPED);
  if (ret)
    SetLastError (ret);
  return ret == 0;
}

BOOL WINAPI
ReadPipeOverlapped (HANDLE h, PVOID buf, DWORD len, LPDWORD ret_len,
		    DWORD timeout)
{
  OVERLAPPED ov;
  BOOL ret;

  memset (&ov, 0, sizeof ov);
  ov.hEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
  ret = ReadFile (h, buf, len, NULL, &ov);
  if (ret || GetLastError () == ERROR_IO_PENDING)
    {
      if (!ret && WaitForSingleObject (ov.hEvent, timeout) != WAIT_OBJECT_0)
	CancelIo (h);
      ret = GetOverlappedResult (h, &ov, ret_len, FALSE);
    }
  CloseHandle (ov.hEvent);
  return ret;
}

BOOL WINAPI
WritePipeOverlapped (HANDLE h, PCVOID buf, DWORD len, LPDWORD ret_len,
		     DWORD timeout)
{
  OVERLAPPED ov;
  BOOL ret;

  memset (&ov, 0, sizeof ov);
  ov.hEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
  ret = WriteFile (h, buf, len, NULL, &ov);
  if (ret || GetLastError () == ERROR_IO_PENDING)
    {
      if (!ret && WaitForSingleObject (ov.hEvent, timeout) != WAIT_OBJECT_0)
	CancelIo (h);
      ret = GetOverlappedResult (h, &ov, ret_len, FALSE);
    }
  CloseHandle (ov.hEvent);
  return ret;
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

/* CygwinCreateThread.

   Replacement function for CreateThread.  What we do here is to remove
   parameters we don't use and instead to add parameters we need to make
   the function pthreads compatible. */

struct thread_wrapper_arg
{
  LPTHREAD_START_ROUTINE func;
  PVOID arg;
  PBYTE stackaddr;
  PBYTE stackbase;
  ULONG guardsize;
};

DWORD WINAPI
thread_wrapper (VOID *arg)
{
  /* Just plain paranoia. */
  if (!arg)
    return ERROR_INVALID_PARAMETER;

  /* Fetch thread wrapper info and free from cygheap. */
  thread_wrapper_arg wrapper_arg = *(thread_wrapper_arg *) arg;
  cfree (arg);

  PTEB teb = NtCurrentTeb ();

  if (!wrapper_arg.stackaddr)
    {
      /* The simple case: Windows-provided stack. */

      /* If guardsize is exactly the page_size, we can assume that the
	 application will behave Windows conformant in terms of stack usage.
	 We can especially assume that it never allocates more than one
	 page at a time (alloca/_chkstk).  Therefore, this is the default
	 case which allows a Windows compatible stack setup with a
	 reserved region, a guard page, and a commited region.  We don't
	 need to set up a POSIX guardpage since Windows already handles
	 stack overflow: Trying to extend the stack into the last three
	 pages of the stack results in a SEGV. */
      if (wrapper_arg.guardsize != wincap.page_size ())
	{
	  /* However, if guardsize is set to something other than the
	     page size, we commit the entire stack, remove the Windows
	     guardpage and, if guardsize is > 0, set up a POSIX guardpage. */
	  DWORD old_prot;
	  ULONG stacksize = (uintptr_t) teb->Tib.StackBase
			    - (uintptr_t) teb->DeallocationStack;

	  VirtualAlloc (teb->DeallocationStack, stacksize,
			MEM_COMMIT, PAGE_READWRITE);
	  VirtualProtect (teb->DeallocationStack, stacksize,
			  PAGE_READWRITE, &old_prot);
	  VirtualProtect (teb->DeallocationStack, wrapper_arg.guardsize,
			  PAGE_NOACCESS, &old_prot);
	  teb->Tib.StackLimit = (PVOID) ((PBYTE) teb->DeallocationStack
					 + wrapper_arg.guardsize);
	}
      wrapper_arg.func (wrapper_arg.arg);
    }
  else
    {
      /* The tricky case: Application-provided stack. */

      /* Remove _cygtls from this stack since it won't be used anymore. */
      _my_tls.remove (0);

      /* Set stack values in TEB */
      teb->Tib.StackBase = wrapper_arg.stackbase;
      teb->Tib.StackLimit = wrapper_arg.stackaddr;
      /* Set DeallocationStack value.  If we have an application-provided
	 stack, we set DeallocationStack to NULL, so NtTerminateThread does
	 not deallocate any stack. Store the OS-provided DeallocationStack
	 address in wrapper_arg.stackaddr.  The below assembler code will
	 release the OS stack after switching to our new stack. */
      wrapper_arg.stackaddr = (PBYTE) teb->DeallocationStack;
      teb->DeallocationStack = NULL;

      /* Initialize new _cygtls. */
      _my_tls.init_thread (wrapper_arg.stackbase - CYGTLS_PADSIZE,
			   (DWORD (*)(void*, void*)) wrapper_arg.func);

      /* Copy exception list over to new stack.  I'm not quite sure how the
	 exception list is extended by Windows itself.  What's clear is that it
	 always grows downwards and that it starts right at the stackbase.
	 Therefore we first count the number of exception records and place
	 the copy at the stackbase, too, so there's still a lot of room to
	 extend the list up to where our _cygtls region starts. */
      _exception_list *old_start = (_exception_list *) teb->Tib.ExceptionList;
      unsigned count = 0;
      teb->Tib.ExceptionList = NULL;
      for (_exception_list *e_ptr = old_start;
	   e_ptr && e_ptr != (_exception_list *) -1;
	   e_ptr = e_ptr->prev)
	++count;
      if (count)
	{
	  _exception_list *new_start = (_exception_list *) wrapper_arg.stackbase
							   - count;
	  teb->Tib.ExceptionList = (struct _EXCEPTION_REGISTRATION_RECORD *)
				   new_start;
	  while (true)
	    {
	      new_start->handler = old_start->handler;
	      if (old_start->prev == (_exception_list *) -1)
		{
		  new_start->prev = (_exception_list *) -1;
		  break;
		}
	      new_start->prev = new_start + 1;
	      new_start = new_start->prev;
	      old_start = old_start->prev;
	    }
	}

      __asm__ ("\n\
	       movl  %[WRAPPER_ARG], %%ebx # Load &wrapper_arg into ebx  \n\
	       movl  (%%ebx), %%eax        # Load thread func into eax   \n\
	       movl  4(%%ebx), %%ecx       # Load thread arg into ecx    \n\
	       movl  8(%%ebx), %%edx       # Load stackaddr into edx     \n\
	       movl  12(%%ebx), %%ebx      # Load stackbase into ebx     \n\
	       subl  %[CYGTLS], %%ebx      # Subtract CYGTLS_PADSIZE     \n\
	       subl  $4, %%ebx             # Subtract another 4 bytes    \n\
	       movl  %%ebx, %%esp          # Set esp                     \n\
	       xorl  %%ebp, %%ebp          # Set ebp to 0                \n\
	       # Make gcc 3.x happy and align the stack so that it is    \n\
	       # 16 byte aligned right before the final call opcode.     \n\
	       andl  $-16, %%esp           # 16 byte align               \n\
	       addl  $-12, %%esp           # 12 bytes + 4 byte arg = 16  \n\
	       # Now we moved to the new stack.  Save thread func address\n\
	       # and thread arg on new stack                             \n\
	       pushl %%ecx                 # Push thread arg onto stack  \n\
	       pushl %%eax                 # Push thread func onto stack \n\
	       # Now it's safe to release the OS stack.                  \n\
	       pushl $0x8000               # dwFreeType: MEM_RELEASE     \n\
	       pushl $0x0                  # dwSize:     0               \n\
	       pushl %%edx                 # lpAddress:  stackaddr       \n\
	       call _VirtualFree@12        # Shoot                       \n\
	       # All set.  We can pop the thread function address from   \n\
	       # the stack and call it.  The thread arg is still on the  \n\
	       # stack in the expected spot.                             \n\
	       popl  %%eax                 # Pop thread_func address     \n\
	       call  *%%eax                # Call thread func            \n"
	       : : [WRAPPER_ARG] "r" (&wrapper_arg),
		   [CYGTLS] "i" (CYGTLS_PADSIZE));
    }
  /* Never return from here. */
  ExitThread (0);
}

HANDLE WINAPI
CygwinCreateThread (LPTHREAD_START_ROUTINE thread_func, PVOID thread_arg,
		    PVOID stackaddr, ULONG stacksize, ULONG guardsize,
		    DWORD creation_flags, LPDWORD thread_id)
{
  ULONG real_stacksize = 0;
  ULONG real_guardsize = 0;
  thread_wrapper_arg *wrapper_arg;

  wrapper_arg = (thread_wrapper_arg *) ccalloc (HEAP_STR, 1,
						sizeof *wrapper_arg);
  if (!wrapper_arg)
    {
      SetLastError (ERROR_OUTOFMEMORY);
      return NULL;
    }
  wrapper_arg->func = thread_func;
  wrapper_arg->arg = thread_arg;

  if (stackaddr)
    {
      /* If the application provided the stack, just use it. */
      wrapper_arg->stackaddr = (PBYTE) stackaddr;
      wrapper_arg->stackbase = (PBYTE) stackaddr + stacksize;
    }
  else
    {
      /* If not, we let CreateThread reserve the stack. */
      real_stacksize = roundup2 (stacksize, wincap.page_size ());
      real_guardsize = roundup2 (guardsize, wincap.page_size ());
      /* Add the guardsize to the stacksize */
      real_stacksize += real_guardsize;
      /* If we use the default Windows guardpage method, we have to take
	 the 2 pages dead zone into account. */
      if (real_guardsize == wincap.page_size ())
	  real_stacksize += 2 * wincap.page_size ();
      /* Now roundup the result to the next allocation boundary. */
      real_stacksize = roundup2 (real_stacksize,
				 wincap.allocation_granularity ());
      wrapper_arg->guardsize = real_guardsize;
    }
  /* Use the STACK_SIZE_PARAM_IS_A_RESERVATION parameter.  This doesn't
     work on Windows 2000, but either we deallocate the OS stack in
     thread_wrapper anyway, or the reservation is rounded to the next Meg.
     Nothing to worry about.
     Note that we reserve a 256K stack as minimum size, not 64K, otherwise
     the thread creation might crash the process due to a stack overflow. */
  return CreateThread (&sec_none_nih, stackaddr ? 4 * PTHREAD_STACK_MIN
						: real_stacksize,
		       thread_wrapper, wrapper_arg,
		       creation_flags | STACK_SIZE_PARAM_IS_A_RESERVATION,
		       thread_id);
}

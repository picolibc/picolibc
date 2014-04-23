/* miscfuncs.cc: misc funcs that don't belong anywhere else

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
   2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014 Red Hat, Inc.

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
#include "cygtls.h"
#include "ntdll.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "pinfo.h"
#include "exception.h"
#include "sigproc.h"

long tls_ix = -1;

const unsigned char case_folded_lower[] = {
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

const unsigned char case_folded_upper[] = {
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

const char isalpha_array[] = {
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

int __reg2
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
  if (iovcnt < 0 || iovcnt > IOV_MAX)
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

/* Try hard to schedule another thread.  
   Remember not to call this in a lock condition or you'll potentially
   suffer starvation.  */
void
yield ()
{
  int prio = GetThreadPriority (GetCurrentThread ());
  SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_IDLE);
  /* MSDN implies that SleepEx will force scheduling of other threads.
     Unlike SwitchToThread() the documentation does not mention other
     cpus so, presumably (hah!), this + using a lower priority will
     stall this thread temporarily and cause another to run.
     (stackoverflow and others seem to confirm that setting this thread
     to a lower priority and calling Sleep with a 0 paramenter will
     have this desired effect)  */
  Sleep (0L);
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
  static const DWORD priority[] =
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
WritePipeOverlapped (HANDLE h, LPCVOID buf, DWORD len, LPDWORD ret_len,
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

bool
NT_readline::init (POBJECT_ATTRIBUTES attr, PCHAR in_buf, ULONG in_buflen)
{
  NTSTATUS status;
  IO_STATUS_BLOCK io;

  status = NtOpenFile (&fh, SYNCHRONIZE | FILE_READ_DATA, attr, &io,
                       FILE_SHARE_VALID_FLAGS,
                       FILE_SYNCHRONOUS_IO_NONALERT
                       | FILE_OPEN_FOR_BACKUP_INTENT);
  if (!NT_SUCCESS (status))
    {
      paranoid_printf ("NtOpenFile(%S) failed, status %y",
		       attr->ObjectName, status);
      return false;
    }
  buf = in_buf;
  buflen = in_buflen;
  got = end = buf;
  len = 0;
  line = 1;
  return true;
}

PCHAR 
NT_readline::gets ()
{
  IO_STATUS_BLOCK io;

  while (true)
    {
      /* len == 0 indicates we have to read from the file. */
      if (!len)
	{
	  if (!NT_SUCCESS (NtReadFile (fh, NULL, NULL, NULL, &io, got,
				       (buflen - 2) - (got - buf), NULL, NULL)))
	    return NULL;
	  len = io.Information;
	  /* Set end marker. */
	  got[len] = got[len + 1] = '\0';
	  /* Set len to the absolute len of bytes in buf. */
	  len += got - buf;
	  /* Reset got to start reading at the start of the buffer again. */
	  got = end = buf;
	}
      else
	{
	  got = end + 1;
	  ++line;
	}
      /* Still some valid full line? */
      if (got < buf + len)
	{
	  if ((end = strchr (got, '\n')))
	    {
	      end[end[-1] == '\r' ? -1 : 0] = '\0';
	      return got;
	    }
	  /* Last line missing a \n at EOF? */
	  if (len < buflen - 2)
	    {
	      len = 0;
	      return got;
	    }
	}
      /* We have to read once more.  Move remaining bytes to the start of
         the buffer and reposition got so that it points to the end of
         the remaining bytes. */
      len = buf + len - got;
      memmove (buf, got, len);
      got = buf + len;
      buf[len] = buf[len + 1] = '\0';
      len = 0;
    }
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

/* Return an address from the import jmp table of main program.  */
void * __reg1
__import_address (void *imp)
{
  if (*((uint16_t *) imp) != 0x25ff)
    return NULL;
  myfault efault;
  if (efault.faulted ())
    return NULL;
  const char *ptr = (const char *) imp;
#ifdef __x86_64__
  const uintptr_t *jmpto = (uintptr_t *) (ptr + 6 + *(int32_t *)(ptr + 2));
#else
  const uintptr_t *jmpto = (uintptr_t *) *((uintptr_t *) (ptr + 2));
#endif
  return (void *) *jmpto;
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
  PBYTE stacklimit;
};

static DWORD WINAPI
thread_wrapper (PVOID arg)
{
  /* Just plain paranoia. */
  if (!arg)
    return ERROR_INVALID_PARAMETER;

  /* The process is now threaded.  Note the fact for later usage. */
  __isthreaded = 1;

  /* Fetch thread wrapper info and free from cygheap. */
  thread_wrapper_arg wrapper_arg = *(thread_wrapper_arg *) arg;
  cfree (arg);

  /* Remove _cygtls from this stack since it won't be used anymore. */
  _my_tls.remove (0);

  /* Set stack values in TEB */
  PTEB teb = NtCurrentTeb ();
  teb->Tib.StackBase = wrapper_arg.stackbase;
  teb->Tib.StackLimit = wrapper_arg.stacklimit ?: wrapper_arg.stackaddr;
  /* Set DeallocationStack value.  If we have an application-provided stack,
     we set DeallocationStack to NULL, so NtTerminateThread does not deallocate
     any stack.  If we created the stack in CygwinCreateThread, we set
     DeallocationStack to the stackaddr of our own stack, so it's automatically
     deallocated when the thread is terminated. */
  PBYTE dealloc_addr = (PBYTE) teb->DeallocationStack;
  teb->DeallocationStack = wrapper_arg.stacklimit ? wrapper_arg.stackaddr
						  : NULL;
  /* Store the OS-provided DeallocationStack address in wrapper_arg.stackaddr.
     The below assembler code will release the OS stack after switching to our
     new stack. */
  wrapper_arg.stackaddr = dealloc_addr;

  /* Initialize new _cygtls. */
  _my_tls.init_thread (wrapper_arg.stackbase - CYGTLS_PADSIZE,
		       (DWORD (*)(void*, void*)) wrapper_arg.func);
#ifndef __x86_64__
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
#endif
#ifdef __x86_64__
  __asm__ ("\n\
	   movq  %[WRAPPER_ARG], %%rbx	# Load &wrapper_arg into rbx	\n\
	   movq  (%%rbx), %%r12		# Load thread func into r12	\n\
	   movq  8(%%rbx), %%r13	# Load thread arg into r13	\n\
	   movq  16(%%rbx), %%rcx	# Load stackaddr into rcx	\n\
	   movq  24(%%rbx), %%rsp	# Load stackbase into rsp	\n\
	   subq  %[CYGTLS], %%rsp	# Subtract CYGTLS_PADSIZE	\n\
	   				# (here we are 16 bytes aligned)\n\
	   subq  $32, %%rsp		# Subtract another 32 bytes	\n\
	   				# (shadow space for arg regs)	\n\
	   xorq  %%rbp, %%rbp		# Set rbp to 0			\n\
	   # We moved to the new stack.					\n\
	   # Now it's safe to release the OS stack.			\n\
	   movl  $0x8000, %%r8d		# dwFreeType: MEM_RELEASE	\n\
	   xorl  %%edx, %%edx		# dwSize:     0			\n\
	   # dwAddress is already in the correct arg register rcx	\n\
	   call  VirtualFree						\n\
	   # All set.  We can copy the thread arg from the safe		\n\
	   # register r13 and then just call the function.		\n\
	   movq  %%r13, %%rcx		# Move thread arg to 1st arg reg\n\
	   call  *%%r12			# Call thread func		\n"
	   : : [WRAPPER_ARG] "r" (&wrapper_arg),
	       [CYGTLS] "i" (CYGTLS_PADSIZE));
#else
  __asm__ ("\n\
	   movl  %[WRAPPER_ARG], %%ebx	# Load &wrapper_arg into ebx	\n\
	   movl  (%%ebx), %%eax		# Load thread func into eax	\n\
	   movl  4(%%ebx), %%ecx	# Load thread arg into ecx	\n\
	   movl  8(%%ebx), %%edx	# Load stackaddr into edx	\n\
	   movl  12(%%ebx), %%ebx	# Load stackbase into ebx	\n\
	   subl  %[CYGTLS], %%ebx	# Subtract CYGTLS_PADSIZE	\n\
	   subl  $4, %%ebx		# Subtract another 4 bytes	\n\
	   movl  %%ebx, %%esp		# Set esp			\n\
	   xorl  %%ebp, %%ebp		# Set ebp to 0			\n\
	   # Make gcc 3.x happy and align the stack so that it is	\n\
	   # 16 byte aligned right before the final call opcode.	\n\
	   andl  $-16, %%esp		# 16 byte align			\n\
	   addl  $-12, %%esp		# 12 bytes + 4 byte arg = 16	\n\
	   # Now we moved to the new stack.  Save thread func address	\n\
	   # and thread arg on new stack				\n\
	   pushl %%ecx			# Push thread arg onto stack	\n\
	   pushl %%eax			# Push thread func onto stack	\n\
	   # Now it's safe to release the OS stack.			\n\
	   pushl $0x8000		# dwFreeType: MEM_RELEASE	\n\
	   pushl $0x0			# dwSize:     0			\n\
	   pushl %%edx			# lpAddress:  stackaddr		\n\
	   call _VirtualFree@12		# Shoot				\n\
	   # All set.  We can pop the thread function address from	\n\
	   # the stack and call it.  The thread arg is still on the	\n\
	   # stack in the expected spot.				\n\
	   popl  %%eax			# Pop thread_func address	\n\
	   call  *%%eax			# Call thread func		\n"
	   : : [WRAPPER_ARG] "r" (&wrapper_arg),
	       [CYGTLS] "i" (CYGTLS_PADSIZE));
#endif
  /* Never return from here. */
  ExitThread (0);
}

#ifdef __x86_64__
/* The memory region used for thread stacks */
#define THREAD_STORAGE_LOW	0x080000000L
#define THREAD_STORAGE_HIGH	0x100000000L
/* We provide the stacks always in 1 Megabyte slots */
#define THREAD_STACK_SLOT	0x100000L	/* 1 Meg */
/* Maximum stack size returned from the pool. */
#define THREAD_STACK_MAX	0x20000000L	/* 512 Megs */

class thread_allocator
{
  UINT_PTR current;
public:
  thread_allocator () : current (THREAD_STORAGE_HIGH) {}
  PVOID alloc (SIZE_T size)
  {
    SIZE_T real_size = roundup2 (size, THREAD_STACK_SLOT);
    BOOL overflow = FALSE;
    PVOID real_stackaddr = NULL;

    /* If an application requests a monster stack, we fulfill this request
       from outside of our pool, top down. */
    if (real_size > THREAD_STACK_MAX)
      return VirtualAlloc (NULL, real_size, MEM_RESERVE | MEM_TOP_DOWN,
			   PAGE_READWRITE);
    /* Simple round-robin.  Keep looping until VirtualAlloc succeeded, or
       until we overflowed and hit the current address. */
    for (UINT_PTR addr = current - real_size;
	 !real_stackaddr && (!overflow || addr >= current);
	 addr -= THREAD_STACK_SLOT)
      {
	if (addr < THREAD_STORAGE_LOW)
	  {
	    addr = THREAD_STORAGE_HIGH - real_size;
	    overflow = TRUE;
	  }
	real_stackaddr = VirtualAlloc ((PVOID) addr, real_size,
				       MEM_RESERVE, PAGE_READWRITE);
	if (!real_stackaddr)
	  {
	    /* So we couldn't grab this space.  Let's check the state.
	       If this area is free, simply try the next lower 1 Meg slot.
	       Otherwise, shift the next try down to the AllocationBase
	       of the current address, minus the requested slot size.
	       Add THREAD_STACK_SLOT since that's subtracted in the next
	       run of the loop anyway. */
	    MEMORY_BASIC_INFORMATION mbi;
	    VirtualQuery ((PVOID) addr, &mbi, sizeof mbi);
	    if (mbi.State != MEM_FREE)
	      addr = (UINT_PTR) mbi.AllocationBase - real_size
						    + THREAD_STACK_SLOT;
	  }
      }
    /* If we got an address, remember it for the next allocation attempt. */
    if (real_stackaddr)
      current = (UINT_PTR) real_stackaddr;
    return real_stackaddr;
  }
};

thread_allocator thr_alloc NO_COPY;
#endif

HANDLE WINAPI
CygwinCreateThread (LPTHREAD_START_ROUTINE thread_func, PVOID thread_arg,
		    PVOID stackaddr, ULONG stacksize, ULONG guardsize,
		    DWORD creation_flags, LPDWORD thread_id)
{
  PVOID real_stackaddr = NULL;
  ULONG real_stacksize = 0;
  ULONG real_guardsize = 0;
  thread_wrapper_arg *wrapper_arg;
  HANDLE thread = NULL;

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
      PBYTE real_stacklimit;

      /* If not, we have to create the stack here. */
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
      /* Reserve stack. */
#ifdef __x86_64__
      real_stackaddr = thr_alloc.alloc (real_stacksize);
#else
      /* FIXME? If the TOP_DOWN method tends to collide too much with
	 other stuff, we should provide our own mechanism to find a
	 suitable place for the stack.  Top down from the start of
	 the Cygwin DLL comes to mind. */
      real_stackaddr = VirtualAlloc (NULL, real_stacksize,
				     MEM_RESERVE | MEM_TOP_DOWN,
				     PAGE_READWRITE);
#endif
      if (!real_stackaddr)
	return NULL;
      /* Set up committed region.  Two cases: */
      if (real_guardsize != wincap.page_size ())
	{
	  /* If guardsize is set to something other than the page size, we
	     commit the entire stack and, if guardsize is > 0, we set up a
	     POSIX guardpage.  We don't set up a Windows guardpage. */
	  if (!VirtualAlloc (real_stackaddr, real_guardsize, MEM_COMMIT,
			     PAGE_NOACCESS))
	    goto err;
	  real_stacklimit = (PBYTE) real_stackaddr + real_guardsize;
	  if (!VirtualAlloc (real_stacklimit, real_stacksize - real_guardsize,
			     MEM_COMMIT, PAGE_READWRITE))
	    goto err;
	}
      else
	{
	  /* If guardsize is exactly the page_size, we can assume that the
	     application will behave Windows conformant in terms of stack usage.
	     We can especially assume that it never allocates more than one
	     page at a time (alloca/_chkstk).  Therefore, this is the default
	     case which allows a Windows compatible stack setup with a
	     reserved region, a guard page, and a commited region.  We don't
	     need to set up a POSIX guardpage since Windows already handles
	     stack overflow: Trying to extend the stack into the last three
	     pages of the stack results in a SEGV.
	     We always commit 64K here, starting with the guardpage. */
	  real_stacklimit = (PBYTE) real_stackaddr + real_stacksize
				- wincap.allocation_granularity ();
	  if (!VirtualAlloc (real_stacklimit, wincap.page_size (), MEM_COMMIT,
			     PAGE_READWRITE | PAGE_GUARD))
	    goto err;
	  real_stacklimit += wincap.page_size ();
	  if (!VirtualAlloc (real_stacklimit, wincap.allocation_granularity ()
					 - wincap.page_size (), MEM_COMMIT,
			     PAGE_READWRITE))
	    goto err;
      	}
      wrapper_arg->stackaddr = (PBYTE) real_stackaddr;
      wrapper_arg->stackbase = (PBYTE) real_stackaddr + real_stacksize;
      wrapper_arg->stacklimit = real_stacklimit;
    }
  /* Use the STACK_SIZE_PARAM_IS_A_RESERVATION parameter so only the
     minimum size for a thread stack is reserved by the OS.  Note that we
     reserve a 256K stack, not 64K, otherwise the thread creation might
     crash the process due to a stack overflow. */
  thread = CreateThread (&sec_none_nih, 4 * PTHREAD_STACK_MIN,
			 thread_wrapper, wrapper_arg,
			 creation_flags | STACK_SIZE_PARAM_IS_A_RESERVATION,
			 thread_id);

err:
  if (!thread && real_stackaddr)
    {
      /* Don't report the wrong error even though VirtualFree is very unlikely
	 to fail. */
      DWORD err = GetLastError ();
      VirtualFree (real_stackaddr, 0, MEM_RELEASE);
      SetLastError (err);
    }
  return thread;
}

#ifdef __x86_64__
// TODO: The equivalent newlib functions only work for SYSV ABI so far.
#undef RtlFillMemory
#undef RtlCopyMemory
extern "C" void NTAPI RtlFillMemory (PVOID, SIZE_T, BYTE);
extern "C" void NTAPI RtlCopyMemory (PVOID, const VOID *, SIZE_T);

extern "C" void *
memset (void *s, int c, size_t n)
{
  RtlFillMemory (s, n, c);
  return s;
}

extern "C" void *
memcpy(void *__restrict dest, const void *__restrict src, size_t n)
{
  RtlCopyMemory (dest, src, n);
  return dest;
}
#endif

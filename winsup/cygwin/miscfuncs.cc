/* miscfuncs.cc: misc funcs that don't belong anywhere else

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "miscfuncs.h"
#include <sys/uio.h>
#include "ntdll.h"
#include "path.h"
#include "fhandler.h"
#include "tls_pbuf.h"

/* not yet prototyped in w32api */
extern "C" HRESULT SetThreadDescription (HANDLE hThread, PCWSTR lpThreadDescription);

/* Get handle count of an object. */
ULONG
get_obj_handle_count (HANDLE h)
{
  OBJECT_BASIC_INFORMATION obi;
  NTSTATUS status;
  ULONG hdl_cnt = 0;

  status = NtQueryObject (h, ObjectBasicInformation, &obi, sizeof obi, NULL);
  if (!NT_SUCCESS (status))
    debug_printf ("NtQueryObject: %y", status);
  else
    hdl_cnt = obi.HandleCount;
  return hdl_cnt;
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

  __try
    {

      size_t tot = 0;

      while (iovcnt > 0)
	{
	  if (iov->iov_len > SSIZE_MAX || (tot += iov->iov_len) > SSIZE_MAX)
	    {
	      set_errno (EINVAL);
	      __leave;
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

      if (tot <= SSIZE_MAX)
	return (ssize_t) tot;

      set_errno (EINVAL);
    }
  __except (EFAULT)
  __endtry
  return -1;
}

/* Try hard to schedule another thread.
   Remember not to call this in a lock condition or you'll potentially
   suffer starvation.  */
void
yield ()
{
  /* MSDN implies that Sleep will force scheduling of other threads.
     Unlike SwitchToThread() the documentation does not mention other
     cpus so, presumably (hah!), this + using a lower priority will
     stall this thread temporarily and cause another to run.
     (stackoverflow and others seem to confirm that setting this thread
     to a lower priority and calling Sleep with a 0 paramenter will
     have this desired effect)

     CV 2017-03-08: Drop lowering the priority.  It leads to potential
		    starvation and it should not be necessary anymore
		    since Server 2003.  See the MSDN Sleep man page. */
  Sleep (0L);
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

BOOL
CreatePipeOverlapped (PHANDLE hr, PHANDLE hw, LPSECURITY_ATTRIBUTES sa)
{
  int ret = fhandler_pipe::create (sa, hr, hw, 0, NULL,
				   FILE_FLAG_OVERLAPPED);
  if (ret)
    SetLastError (ret);
  return ret == 0;
}

BOOL
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

BOOL
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

/* Signal the thread name to any attached debugger

   (See "How to: Set a Thread Name in Native Code"
   https://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx) */

#define MS_VC_EXCEPTION 0x406D1388

static void
SetThreadNameExc (DWORD dwThreadID, const char* threadName)
{
  if (!IsDebuggerPresent ())
    return;

  ULONG_PTR info[] =
    {
      0x1000,                 /* type, must be 0x1000 */
      (ULONG_PTR) threadName, /* pointer to threadname */
      dwThreadID,             /* thread ID (+ flags on x86_64) */
    };

  __try
    {
      RaiseException (MS_VC_EXCEPTION, 0, sizeof (info) / sizeof (ULONG_PTR),
		      info);
    }
  __except (NO_ERROR)
  __endtry
}

void
SetThreadName (DWORD dwThreadID, const char* threadName)
{
  HANDLE hThread = OpenThread (THREAD_SET_LIMITED_INFORMATION, FALSE, dwThreadID);
  if (hThread)
    {
      /* SetThreadDescription only exists in a wide-char version, so we must
	 convert threadname to wide-char.  The encoding of threadName is
	 unclear, so use UTF8 until we know better. */
      int bufsize = MultiByteToWideChar (CP_UTF8, 0, threadName, -1, NULL, 0);
      WCHAR buf[bufsize];
      bufsize = MultiByteToWideChar (CP_UTF8, 0, threadName, -1, buf, bufsize);
      HRESULT hr = SetThreadDescription (hThread, buf);
      if (hr != S_OK)
	{
	  debug_printf ("SetThreadDescription() failed. %08x %08x\n",
			GetLastError (), hr);
	}
      CloseHandle (hThread);
    }

  /* also use the older, exception-based method of setting threadname for the
     benefit of things which don't known about GetThreadDescription. */
  SetThreadNameExc (dwThreadID, threadName);
}

#define add_size(p,s) ((p) = ((__typeof__(p))((PBYTE)(p)+(s))))

static WORD num_cpu_per_group = 0;
static WORD group_count = 0;

WORD
__get_cpus_per_group (void)
{
  tmp_pathbuf tp;

  if (num_cpu_per_group)
    return num_cpu_per_group;

  num_cpu_per_group = 64;
  group_count = 1;

  PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX lpi =
            (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX) tp.c_get ();
  DWORD lpi_size = NT_MAX_PATH;

  /* Fake a SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX group info block if
     GetLogicalProcessorInformationEx fails for some reason. */
  if (!GetLogicalProcessorInformationEx (RelationGroup, lpi, &lpi_size))
    {
      lpi_size = sizeof *lpi;
      lpi->Relationship = RelationGroup;
      lpi->Size = lpi_size;
      lpi->Group.MaximumGroupCount = 1;
      lpi->Group.ActiveGroupCount = 1;
      lpi->Group.GroupInfo[0].MaximumProcessorCount = wincap.cpu_count ();
      lpi->Group.GroupInfo[0].ActiveProcessorCount
        = __builtin_popcountl (wincap.cpu_mask ());
      lpi->Group.GroupInfo[0].ActiveProcessorMask = wincap.cpu_mask ();
    }

  PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX plpi = lpi;
  for (DWORD size = lpi_size; size > 0;
       size -= plpi->Size, add_size (plpi, plpi->Size))
    if (plpi->Relationship == RelationGroup)
      {
        /* There are systems with a MaximumProcessorCount not reflecting the
	   actually available CPUs.  The ActiveProcessorCount is correct
	   though.  So we just use ActiveProcessorCount for now, hoping for
	   the best. */
        num_cpu_per_group = plpi->Group.GroupInfo[0].ActiveProcessorCount;

	/* Follow that lead to get the group count. */
	group_count = plpi->Group.ActiveGroupCount;
        break;
      }

  return num_cpu_per_group;
}

WORD
__get_group_count (void)
{
  if (group_count == 0)
    (void) __get_cpus_per_group (); // caller should have called this first
  return group_count;
}

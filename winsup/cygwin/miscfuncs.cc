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

/*
   Mapping of nice value or sched_priority from/to Windows priority
   ('batch' is used for SCHED_BATCH policy).

    nice_to_winprio()                                       winprio_to_nice()
  !batch      batch     Level  Windows priority class        !batch   batch
   12...19      4...19    0    IDLE_PRIORITY_CLASS            16        8
    4...11     -4....3    1    BELOW_NORMAL_PRIORITY_CLASS     8        0
   -4....3    -12...-5    2    NORMAL_PRIORITY_CLASS           0       -8
  -12...-5    -13..-19    3    ABOVE_NORMAL_PRIORITY_CLASS    -8      -16
  -13..-19         -20    4    HIGH_PRIORITY_CLASS           -16      -20
       -20           -    5    REALTIME_PRIORITY_CLASS       -20      -20

   schedprio_to_winprio()                               winprio_to_schedprio()
    1....6                0    IDLE_PRIORITY_CLASS             3
    7...12                1    BELOW_NORMAL_PRIORITY_CLASS     9
   13...18                2    NORMAL_PRIORITY_CLASS          15
   19...24                3    ABOVE_NORMAL_PRIORITY_CLASS    21
   25...30                4    HIGH_PRIORITY_CLASS            27
   31...32                5    REALTIME_PRIORITY_CLASS        32
*/

/* *_PRIORITY_CLASS -> 0...5 */
constexpr int
winprio_to_level (DWORD prio)
{
  switch (prio)
    {
      case IDLE_PRIORITY_CLASS:		return 0;
      case BELOW_NORMAL_PRIORITY_CLASS:	return 1;
      default:				return 2;
      case ABOVE_NORMAL_PRIORITY_CLASS:	return 3;
      case HIGH_PRIORITY_CLASS:		return 4;
      case REALTIME_PRIORITY_CLASS:	return 5;
    }
}

/* 0...5 -> *_PRIORITY_CLASS */
constexpr DWORD
level_to_winprio (int level)
{
  switch (level)
    {
      case 0:  return IDLE_PRIORITY_CLASS;
      case 1:  return BELOW_NORMAL_PRIORITY_CLASS;
      default: return NORMAL_PRIORITY_CLASS;
      case 3:  return ABOVE_NORMAL_PRIORITY_CLASS;
      case 4:  return HIGH_PRIORITY_CLASS;
      case 5:  return REALTIME_PRIORITY_CLASS;
    }
}

/* *_PRIORITY_CLASS -> nice value */
constexpr int
winprio_to_nice_impl (DWORD prio, bool batch = false)
{
  int level = winprio_to_level (prio);
  if (batch && level < 5)
    level++;
  return (level < 5 ? NZERO - 1 - 3 - level * 8 : -NZERO);
}

/* nice value -> *_PRIORITY_CLASS */
constexpr DWORD
nice_to_winprio_impl (int nice, bool batch = false)
{
  int level = (nice > -NZERO ? (NZERO - 1 - nice) / 8 : 5);
  if (batch && level > 0)
    level--;
  return level_to_winprio (level);
}

/* *_PRIORITY_CLASS -> sched_priority */
constexpr int
winprio_to_schedprio_impl (DWORD prio)
{
  int level = winprio_to_level (prio);
  return (level < 5 ? 3 + level * 6 : 32);
}

/* sched_priority -> *_PRIORITY_CLASS */
constexpr DWORD
schedprio_to_winprio_impl (int schedprio)
{
  int level = (schedprio <= 1 ? 0 : (schedprio < 32 ? (schedprio - 1) / 6 : 5));
  return level_to_winprio (level);
}

/* Check consistency at compile time. */
constexpr bool
check_nice_schedprio_winprio_mapping ()
{
  for (int nice = -NZERO; nice < NZERO; nice++)
    for (int batch = 0; batch <= 1; batch++) {
      DWORD prio = nice_to_winprio_impl (nice, !!batch);
      int nice2 = winprio_to_nice_impl (prio, !!batch);
      DWORD prio2 = nice_to_winprio_impl (nice2, !!batch);
      if (prio != prio2)
	return false;
    }
  for (int schedprio = 1; schedprio <= 32; schedprio++)
    {
      DWORD prio = schedprio_to_winprio_impl (schedprio);
      int schedprio2 = winprio_to_schedprio_impl (prio);
      DWORD prio2 = schedprio_to_winprio_impl (schedprio2);
      if (prio != prio2)
	return false;
    }
  return true;
}

static_assert (check_nice_schedprio_winprio_mapping());
static_assert (nice_to_winprio_impl(NZERO-1, false) == IDLE_PRIORITY_CLASS);
static_assert (nice_to_winprio_impl(0, true) == BELOW_NORMAL_PRIORITY_CLASS);
static_assert (winprio_to_nice_impl(BELOW_NORMAL_PRIORITY_CLASS, true) == 0);
static_assert (nice_to_winprio_impl(0, false) == NORMAL_PRIORITY_CLASS);
static_assert (winprio_to_nice_impl(NORMAL_PRIORITY_CLASS, false) == 0);
static_assert (nice_to_winprio_impl(-NZERO, false) == REALTIME_PRIORITY_CLASS);
static_assert (schedprio_to_winprio_impl(1) == IDLE_PRIORITY_CLASS);
static_assert (schedprio_to_winprio_impl(15) == NORMAL_PRIORITY_CLASS);
static_assert (winprio_to_schedprio_impl(NORMAL_PRIORITY_CLASS) == 15);
static_assert (schedprio_to_winprio_impl(32) == REALTIME_PRIORITY_CLASS);

/* Get a default value for the nice factor. */
int
winprio_to_nice (DWORD prio, bool batch /* = false */)
{
  return winprio_to_nice_impl (prio, batch);
}

/* Get a Win32 priority matching the incoming nice factor.  The incoming
   nice is limited to the interval [-NZERO,NZERO-1]. */
DWORD
nice_to_winprio (int &nice, bool batch /* = false */)
{
  if (nice < -NZERO)
    nice = -NZERO;
  else if (nice > NZERO - 1)
    nice = NZERO - 1;

  return nice_to_winprio_impl (nice, batch);
}

/* Get a default sched_priority from a Win32 priority. */
int
winprio_to_schedprio (DWORD prio)
{
  return winprio_to_schedprio_impl (prio);
}

/* Get a Win32 priority matching the sched_priority. */
DWORD
schedprio_to_winprio (int schedprio)
{
  return schedprio_to_winprio_impl (schedprio);
}

/* Set Win32 priority or return false on failure.  Also return
   false and revert to the original priority if a different (lower)
   priority is set instead.  Always revert to original priority if
   set==false. */
bool
set_and_check_winprio (HANDLE proc, DWORD prio, bool set /* = true */)
{
  DWORD prev_prio = GetPriorityClass (proc);
  if (!prev_prio)
    return false;
  if (prev_prio == prio)
    return true;

  if (!SetPriorityClass (proc, prio))
    return false;

  /* Windows silently sets a lower priority (HIGH_PRIORITY_CLASS) if
     the new priority (REALTIME_PRIORITY_CLASS) requires administrator
     privileges. */
  DWORD curr_prio = GetPriorityClass (proc);
  bool ret = (curr_prio == prio);

  if (set)
    {
      if (ret)
	debug_printf ("Changed priority from 0x%x to 0x%x", prev_prio, curr_prio);
      else
	debug_printf ("Failed to set priority 0x%x, revert from 0x%x to 0x%x",
	  prio, curr_prio, prev_prio);
    }
  if (!(set && ret))
    SetPriorityClass (proc, prev_prio);

  return ret;
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
      if (IS_ERROR (hr))
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

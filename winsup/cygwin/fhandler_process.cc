/* fhandler_process.cc: fhandler for /proc/<pid> virtual filesystem

   Copyright 2002 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/cygwin.h>
#include <ntdef.h>
#include "cygerrno.h"
#include "security.h"
#include "fhandler.h"
#include "pinfo.h"
#include "path.h"
#include "shared_info.h"
#include "dtable.h"
#include "cygheap.h"
#include "ntdll.h"
#include <sys/param.h>
#include <assert.h>
#include <sys/sysmacros.h>

#define _COMPILING_NEWLIB
#include <dirent.h>

static const int PROCESS_PPID = 2;
static const int PROCESS_EXENAME = 3;
static const int PROCESS_WINPID = 4;
static const int PROCESS_WINEXENAME = 5;
static const int PROCESS_STATUS = 6;
static const int PROCESS_UID = 7;
static const int PROCESS_GID = 8;
static const int PROCESS_PGID = 9;
static const int PROCESS_SID = 10;
static const int PROCESS_CTTY = 11;
static const int PROCESS_STAT = 12;
static const int PROCESS_STATM = 13;
static const int PROCESS_CMDLINE = 14;

static const char * const process_listing[] =
{
  ".",
  "..",
  "ppid",
  "exename",
  "winpid",
  "winexename",
  "status",
  "uid",
  "gid",
  "pgid",
  "sid",
  "ctty",
  "stat",
  "statm",
  "cmdline",
  NULL
};

static const int PROCESS_LINK_COUNT =
  (sizeof (process_listing) / sizeof (const char *)) - 1;

static off_t format_process_stat (_pinfo *p, char *destbuf, size_t maxsize);
static off_t format_process_status (_pinfo *p, char *destbuf, size_t maxsize);
static off_t format_process_statm (_pinfo *p, char *destbuf, size_t maxsize);
static int get_process_state (DWORD dwProcessId);
static bool get_mem_values (DWORD dwProcessId, unsigned long *vmsize,
			    unsigned long *vmrss, unsigned long *vmtext,
			    unsigned long *vmdata, unsigned long *vmlib,
			    unsigned long *vmshare);

/* Returns 0 if path doesn't exist, >0 if path is a directory,
 * <0 if path is a file.
 */
int
fhandler_process::exists ()
{
  const char *path = get_name ();
  debug_printf ("exists (%s)", path);
  path += proc_len + 1;
  while (*path != 0 && !SLASH_P (*path))
    path++;
  if (*path == 0)
    return 2;

  for (int i = 0; process_listing[i]; i++)
    if (pathmatch (path + 1, process_listing[i]))
      return -1;
  return 0;
}

fhandler_process::fhandler_process ():
  fhandler_proc (FH_PROCESS)
{
}

int
fhandler_process::fstat (struct __stat64 *buf, path_conv *pc)
{
  const char *path = get_name ();
  int file_type = exists ();
  (void) fhandler_base::fstat (buf, pc);
  path += proc_len + 1;
  pid = atoi (path);
  pinfo p (pid);
  if (!p)
    {
      set_errno (ENOENT);
      return -1;
    }

  buf->st_mode &= ~_IFMT & NO_W;

  switch (file_type)
    {
    case 0:
      set_errno (ENOENT);
      return -1;
    case 1:
      buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
      return 0;
    case 2:
      buf->st_ctime = buf->st_mtime = p->start_time;
      buf->st_ctim.tv_nsec = buf->st_mtim.tv_nsec = 0;
      time_as_timestruc_t (&buf->st_atim);
      buf->st_uid = p->uid;
      buf->st_gid = p->gid;
      buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
      buf->st_nlink = PROCESS_LINK_COUNT;
      return 0;
    default:
    case -1:
      buf->st_uid = p->uid;
      buf->st_gid = p->gid;
      buf->st_mode |= S_IFREG | S_IRUSR | S_IRGRP | S_IROTH;
      return 0;
    }
}

struct dirent *
fhandler_process::readdir (DIR * dir)
{
  if (dir->__d_position >= PROCESS_LINK_COUNT)
    {
      set_errno (ENMFILE);
      return NULL;
    }
  strcpy (dir->__d_dirent->d_name, process_listing[dir->__d_position++]);
  syscall_printf ("%p = readdir (%p) (%s)", &dir->__d_dirent, dir,
		  dir->__d_dirent->d_name);
  return dir->__d_dirent;
}

int
fhandler_process::open (path_conv *pc, int flags, mode_t mode)
{
  int process_file_no = -1;

  int res = fhandler_virtual::open (pc, flags, mode);
  if (!res)
    goto out;

  set_nohandle (true);

  const char *path;
  path = get_name () + proc_len + 1;
  pid = atoi (path);
  while (*path != 0 && !SLASH_P (*path))
    path++;

  if (*path == 0)
    {
      if ((flags & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL))
	{
	  set_errno (EEXIST);
	  res = 0;
	  goto out;
	}
      else if (flags & O_WRONLY)
	{
	  set_errno (EISDIR);
	  res = 0;
	  goto out;
	}
      else
	{
	  flags |= O_DIROPEN;
	  goto success;
	}
    }

  process_file_no = -1;
  for (int i = 0; process_listing[i]; i++)
    {
      if (path_prefix_p
	  (process_listing[i], path + 1, strlen (process_listing[i])))
	process_file_no = i;
    }
  if (process_file_no == -1)
    {
      if (flags & O_CREAT)
	{
	  set_errno (EROFS);
	  res = 0;
	  goto out;
	}
      else
	{
	  set_errno (ENOENT);
	  res = 0;
	  goto out;
	}
    }
  if (flags & O_WRONLY)
    {
      set_errno (EROFS);
      res = 0;
      goto out;
    }

  fileid = process_file_no;
  if (!fill_filebuf ())
	{
	  res = 0;
	  goto out;
	}

  if (flags & O_APPEND)
    position = filesize;
  else
    position = 0;

success:
  res = 1;
  set_flags ((flags & ~O_TEXT) | O_BINARY);
  set_open_status ();
out:
  syscall_printf ("%d = fhandler_proc::open (%p, %d)", res, flags, mode);
  return res;
}

bool
fhandler_process::fill_filebuf ()
{
  pinfo p (pid);

  if (!p)
    {
      set_errno (ENOENT);
      return false;
    }

  switch (fileid)
    {
    case PROCESS_UID:
    case PROCESS_GID:
    case PROCESS_PGID:
    case PROCESS_SID:
    case PROCESS_CTTY:
    case PROCESS_PPID:
      {
	filebuf = (char *) realloc (filebuf, bufalloc = 40);
	int num;
	switch (fileid)
	  {
	  case PROCESS_PPID:
	    num = p->ppid;
	    break;
	  case PROCESS_UID:
	    num = p->uid;
	    break;
	  case PROCESS_PGID:
	    num = p->pgid;
	    break;
	  case PROCESS_SID:
	    num = p->sid;
	    break;
	  case PROCESS_GID:
	    num = p->gid;
	    break;
	  case PROCESS_CTTY:
	    num = p->ctty;
	    break;
	  default: // what's this here for?
	    num = 0;
	    break;
	  }
	__small_sprintf (filebuf, "%d\n", num);
	filesize = strlen (filebuf);
	break;
      }
    case PROCESS_CMDLINE:
      {
	if (filebuf)
	  free (filebuf);
	filebuf = p->cmdline (filesize);
	if (!*filebuf)
	  filebuf = strdup ("<defunct>");
	break;
      }
    case PROCESS_EXENAME:
      {
	filebuf = (char *) realloc (filebuf, bufalloc = MAX_PATH);
	if (p->process_state & (PID_ZOMBIE | PID_EXITED))
	  strcpy (filebuf, "<defunct>");
	else
	  {
	    mount_table->conv_to_posix_path (p->progname, filebuf, 1);
	    int len = strlen (filebuf);
	    if (len > 4)
	      {
		char *s = filebuf + len - 4;
		if (strcasecmp (s, ".exe") == 0)
		  *s = 0;
	      }
	  }
	filesize = strlen (filebuf);
	break;
      }
    case PROCESS_WINPID:
      {
	filebuf = (char *) realloc (filebuf, bufalloc = 40);
	__small_sprintf (filebuf, "%d\n", p->dwProcessId);
	filesize = strlen (filebuf);
	break;
      }
    case PROCESS_WINEXENAME:
      {
	int len = strlen (p->progname);
	filebuf = (char *) realloc (filebuf, bufalloc = (len + 2));
	strcpy (filebuf, p->progname);
	filebuf[len] = '\n';
	filesize = len + 1;
	break;
      }
    case PROCESS_STATUS:
      {
	filebuf = (char *) realloc (filebuf, bufalloc = 2048);
	filesize = format_process_status (*p, filebuf, bufalloc);
	break;
      }
    case PROCESS_STAT:
      {
	filebuf = (char *) realloc (filebuf, bufalloc = 2048);
	filesize = format_process_stat (*p, filebuf, bufalloc);
	break;
      }
    case PROCESS_STATM:
      {
	filebuf = (char *) realloc (filebuf, bufalloc = 2048);
	filesize = format_process_statm (*p, filebuf, bufalloc);
	break;
      }
    }

  return true;
}

static
off_t
format_process_stat (_pinfo *p, char *destbuf, size_t maxsize)
{
  char cmd[MAX_PATH];
  int state = 'R';
  unsigned long fault_count = 0UL,
		utime = 0UL, stime = 0UL,
		start_time = 0UL,
		vmsize = 0UL, vmrss = 0UL, vmmaxrss = 0UL;
  int priority = 0;
  if (p->process_state & (PID_ZOMBIE | PID_EXITED))
    strcpy (cmd, "<defunct>");
  else
    {
      strcpy (cmd, p->progname);
      char *last_slash = strrchr (cmd, '\\');
      if (last_slash != NULL)
	strcpy (cmd, last_slash + 1);
      int len = strlen (cmd);
      if (len > 4)
	{
	  char *s = cmd + len - 4;
	  if (strcasecmp (s, ".exe") == 0)
	    *s = 0;
	 }
    }
  /*
   * Note: under Windows, a _process_ is always running - it's only _threads_
   * that get suspended. Therefore the default state is R (runnable).
   */
  if (p->process_state & PID_ZOMBIE)
    state = 'Z';
  else if (p->process_state & PID_STOPPED)
    state = 'T';
  else if (wincap.is_winnt ())
    state = get_process_state (p->dwProcessId);
 if (wincap.is_winnt ())
    {
      NTSTATUS ret;
      HANDLE hProcess;
      VM_COUNTERS vmc;
      KERNEL_USER_TIMES put;
      PROCESS_BASIC_INFORMATION pbi;
      QUOTA_LIMITS ql;
      SYSTEM_TIME_OF_DAY_INFORMATION stodi;
      SYSTEM_PROCESSOR_TIMES spt;
      hProcess = OpenProcess (PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,
			      FALSE, p->dwProcessId);
      if (hProcess != NULL)
	{
	  ret = NtQueryInformationProcess (hProcess,
					   ProcessVmCounters,
					   (PVOID) &vmc,
					   sizeof vmc, NULL);
	  if (ret == STATUS_SUCCESS)
	    ret = NtQueryInformationProcess (hProcess,
					     ProcessTimes,
					     (PVOID) &put,
					     sizeof put, NULL);
	  if (ret == STATUS_SUCCESS)
	    ret = NtQueryInformationProcess (hProcess,
					     ProcessBasicInformation,
					     (PVOID) &pbi,
					     sizeof pbi, NULL);
	  if (ret == STATUS_SUCCESS)
	    ret = NtQueryInformationProcess (hProcess,
					     ProcessQuotaLimits,
					     (PVOID) &ql,
					     sizeof ql, NULL);
	  CloseHandle (hProcess);
	}
      else
	{
	  DWORD error = GetLastError ();
	  __seterrno_from_win_error (error);
	  debug_printf ("OpenProcess: ret = %d",
			error);
	  return 0;
	}
      if (ret == STATUS_SUCCESS)
	ret = NtQuerySystemInformation (SystemTimeOfDayInformation,
					(PVOID) &stodi,
					sizeof stodi, NULL);
      if (ret == STATUS_SUCCESS)
	ret = NtQuerySystemInformation (SystemProcessorTimes,
					(PVOID) &spt,
					sizeof spt, NULL);
      if (ret != STATUS_SUCCESS)
	{
	  __seterrno_from_win_error (RtlNtStatusToDosError (ret));
	  debug_printf ("NtQueryInformationProcess: ret = %d, "
		       "Dos(ret) = %d",
		       ret, RtlNtStatusToDosError (ret));
	  return 0;
	}
       fault_count = vmc.PageFaultCount;
       utime = put.UserTime.QuadPart * HZ / 10000000ULL;
       stime = put.KernelTime.QuadPart * HZ / 10000000ULL;
       if (stodi.CurrentTime.QuadPart > put.CreateTime.QuadPart)
	 start_time = (spt.KernelTime.QuadPart + spt.UserTime.QuadPart -
		       stodi.CurrentTime.QuadPart + put.CreateTime.QuadPart) * HZ / 10000000ULL;
       else
	 /*
	  * sometimes stodi.CurrentTime is a bit behind
	  * Note: some older versions of procps are broken and can't cope
	  * with process start times > time(NULL).
	  */
	 start_time = (spt.KernelTime.QuadPart + spt.UserTime.QuadPart) * HZ / 10000000ULL;
       priority = pbi.BasePriority;
       unsigned page_size = getpagesize ();
       vmsize = vmc.VirtualSize;
       vmrss = vmc.WorkingSetSize / page_size;
       vmmaxrss = ql.MaximumWorkingSetSize / page_size;
    }
  else
    {
      start_time = (GetTickCount () / 1000 - time (NULL) + p->start_time) * HZ;
    }
  return __small_sprintf (destbuf, "%d (%s) %c "
				   "%d %d %d %d %d "
				   "%lu %lu %lu %lu %lu %lu %lu "
				   "%ld %ld %ld %ld %ld %ld "
				   "%lu %lu "
				   "%ld "
				   "%lu",
			  p->pid, cmd,
			  state,
			  p->ppid, p->pgid, p->sid, makedev (FH_TTYS, p->ctty),
			  -1, 0, fault_count, fault_count, 0, 0, utime, stime,
			  utime, stime, priority, 0, 0, 0,
			  start_time, vmsize,
			  vmrss, vmmaxrss
			  );
}

static
off_t
format_process_status (_pinfo *p, char *destbuf, size_t maxsize)
{
  char cmd[MAX_PATH];
  int state = 'R';
  const char *state_str = "unknown";
  unsigned long vmsize = 0UL, vmrss = 0UL, vmdata = 0UL, vmlib = 0UL, vmtext = 0UL,
		vmshare = 0UL;
  if (p->process_state & (PID_ZOMBIE | PID_EXITED))
    strcpy (cmd, "<defunct>");
  else
    {
      strcpy (cmd, p->progname);
      char *last_slash = strrchr (cmd, '\\');
      if (last_slash != NULL)
	strcpy (cmd, last_slash + 1);
      int len = strlen (cmd);
      if (len > 4)
	{
	  char *s = cmd + len - 4;
	  if (strcasecmp (s, ".exe") == 0)
	    *s = 0;
	 }
    }
  /*
   * Note: under Windows, a _process_ is always running - it's only _threads_
   * that get suspended. Therefore the default state is R (runnable).
   */
  if (p->process_state & PID_ZOMBIE)
    state = 'Z';
  else if (p->process_state & PID_STOPPED)
    state = 'T';
  else if (wincap.is_winnt ())
    state = get_process_state (p->dwProcessId);
  switch (state)
    {
    case 'O':
      state_str = "running";
      break;
    case 'D':
    case 'S':
      state_str = "sleeping";
      break;
    case 'R':
      state_str = "runnable";
      break;
    case 'Z':
      state_str = "zombie";
      break;
    case 'T':
      state_str = "stopped";
      break;
    }
  if (wincap.is_winnt ())
    {
      if (!get_mem_values (p->dwProcessId, &vmsize, &vmrss, &vmtext, &vmdata, &vmlib, &vmshare))
	return 0;
      unsigned page_size = getpagesize ();
      vmsize *= page_size; vmrss *= page_size; vmdata *= page_size;
      vmtext *= page_size; vmlib *= page_size;
    }
  // The real uid value for *this* process is stored at cygheap->user.real_uid
  // but we can't get at the real uid value for any other process, so
  // just fake it as p->uid. Similar for p->gid.
  return __small_sprintf (destbuf, "Name:   %s\n"
				   "State:  %c (%s)\n"
				   "Tgid:   %d\n"
				   "Pid:    %d\n"
				   "PPid:   %d\n"
				   "Uid:    %d %d %d %d\n"
				   "Gid:    %d %d %d %d\n"
				   "VmSize: %8d kB\n"
				   "VmLck:  %8d kB\n"
				   "VmRSS:  %8d kB\n"
				   "VmData: %8d kB\n"
				   "VmStk:  %8d kB\n"
				   "VmExe:  %8d kB\n"
				   "VmLib:  %8d kB\n"
				   "SigPnd: %016x\n"
				   "SigBlk: %016x\n"
				   "SigIgn: %016x\n",
			  cmd,
			  state, state_str,
			  p->pgid,
			  p->pid,
			  p->ppid,
			  p->uid, p->uid, p->uid, p->uid,
			  p->gid, p->gid, p->gid, p->gid,
			  vmsize >> 10, 0, vmrss >> 10, vmdata >> 10, 0, vmtext >> 10, vmlib >> 10,
			  0, 0, p->getsigmask ()
			  );
}

static
off_t
format_process_statm (_pinfo *p, char *destbuf, size_t maxsize)
{
  unsigned long vmsize = 0UL, vmrss = 0UL, vmtext = 0UL, vmdata = 0UL,
		vmlib = 0UL, vmshare = 0UL;
  if (wincap.is_winnt ())
    {
      if (!get_mem_values (p->dwProcessId, &vmsize, &vmrss, &vmtext, &vmdata,
			   &vmlib, &vmshare))
	return 0;
    }
  return __small_sprintf (destbuf, "%ld %ld %ld %ld %ld %ld %ld",
			  vmsize, vmrss, vmshare, vmtext, vmlib, vmdata, 0
			  );
}

static
int
get_process_state (DWORD dwProcessId)
{
  /*
   * This isn't really heavy magic - just go through the processes'
   * threads one by one and return a value accordingly
   * Errors are silently ignored.
   */
  NTSTATUS ret;
  SYSTEM_PROCESSES *sp;
  ULONG n = 0x1000;
  PULONG p = new ULONG[n];
  int state =' ';
  while (STATUS_INFO_LENGTH_MISMATCH ==
	 (ret = NtQuerySystemInformation (SystemProcessesAndThreadsInformation,
					 (PVOID) p,
					 n * sizeof *p, NULL)))
    delete [] p, p = new ULONG[n *= 2];
  if (ret != STATUS_SUCCESS)
    {
      debug_printf ("NtQuerySystemInformation: ret = %d, "
		   "Dos(ret) = %d",
		   ret, RtlNtStatusToDosError (ret));
      goto out;
    }
  state = 'Z';
  sp = (SYSTEM_PROCESSES *) p;
  for (;;)
    {
      if (sp->ProcessId == dwProcessId)
	{
	  SYSTEM_THREADS *st;
	  if (wincap.has_process_io_counters ())
	    /*
	     * Windows 2000 and XP have an extra member in SYSTEM_PROCESSES
	     * which means the offset of the first SYSTEM_THREADS entry is
	     * different on these operating systems compared to NT 4.
	     */
	    st = &sp->Threads[0];
	  else
	    /*
	     * 136 is the offset of the first SYSTEM_THREADS entry on
	     * Windows NT 4.
	     */
	    st = (SYSTEM_THREADS *) ((char *) sp + 136);
	  state = 'S';
	  for (unsigned i = 0; i < sp->ThreadCount; i++)
	    {
	      if (st->State == StateRunning ||
		  st->State == StateReady)
		{
		  state = 'R';
		  goto out;
		}
	      st++;
	    }
	  break;
	}
      if (!sp->NextEntryDelta)
	 break;
      sp = (SYSTEM_PROCESSES *) ((char *) sp + sp->NextEntryDelta);
    }
out:
  delete [] p;
  return state;
}

static
bool
get_mem_values (DWORD dwProcessId, unsigned long *vmsize, unsigned long *vmrss,
		unsigned long *vmtext, unsigned long *vmdata,
		unsigned long *vmlib, unsigned long *vmshare)
{
  bool res = true;
  NTSTATUS ret;
  HANDLE hProcess;
  VM_COUNTERS vmc;
  MEMORY_WORKING_SET_LIST *mwsl;
  ULONG n = 0x1000, length;
  PULONG p = new ULONG[n];
  unsigned page_size = getpagesize ();
  hProcess = OpenProcess (PROCESS_QUERY_INFORMATION,
			  FALSE, dwProcessId);
  if (hProcess == NULL)
    {
      DWORD error = GetLastError ();
      __seterrno_from_win_error (error);
      debug_printf ("OpenProcess: ret = %d",
		    error);
      return false;
    }
  while ((ret = NtQueryVirtualMemory (hProcess, 0,
				      MemoryWorkingSetList,
				      (PVOID) p,
				      n * sizeof *p, &length)),
	 (ret == STATUS_SUCCESS || ret == STATUS_INFO_LENGTH_MISMATCH) &&
	 length >= n * sizeof *p)
    delete [] p, p = new ULONG[n *= 2];
  if (ret != STATUS_SUCCESS)
    {
      debug_printf ("NtQueryVirtualMemory: ret = %d, "
		   "Dos(ret) = %d",
		   ret, RtlNtStatusToDosError (ret));
      res = false;
      goto out;
    }
  mwsl = (MEMORY_WORKING_SET_LIST *) p;
  for (unsigned long i = 0; i < mwsl->NumberOfPages; i++)
    {
      ++*vmrss;
      unsigned flags = mwsl->WorkingSetList[i] & 0x0FFF;
      if (flags & (WSLE_PAGE_EXECUTE | WSLE_PAGE_SHAREABLE) == (WSLE_PAGE_EXECUTE | WSLE_PAGE_SHAREABLE))
	  ++*vmlib;
      else if (flags & WSLE_PAGE_SHAREABLE)
	  ++*vmshare;
      else if (flags & WSLE_PAGE_EXECUTE)
	  ++*vmtext;
      else
	  ++*vmdata;
    }
  ret = NtQueryInformationProcess (hProcess,
				   ProcessVmCounters,
				   (PVOID) &vmc,
				   sizeof vmc, NULL);
  if (ret != STATUS_SUCCESS)
    {
      debug_printf ("NtQueryInformationProcess: ret = %d, "
		    "Dos(ret) = %d",
		    ret, RtlNtStatusToDosError (ret));
      res = false;
      goto out;
    }
  *vmsize = vmc.VirtualSize / page_size;
out:
  delete [] p;
  CloseHandle (hProcess);
  return res;
}

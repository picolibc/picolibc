/* fhandler_proc.cc: fhandler for /proc virtual filesystem

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
#include "path.h"
#include "pinfo.h"
#include "dtable.h"
#include "cygheap.h"
#include <assert.h>
#include <sys/utsname.h>
#include <sys/param.h>
#include "ntdll.h"

#define _COMPILING_NEWLIB
#include <dirent.h>

/* offsets in proc_listing */
static const int PROC_LOADAVG  = 2;     // /proc/loadavg
static const int PROC_MEMINFO  = 3;     // /proc/meminfo
static const int PROC_REGISTRY = 4;     // /proc/registry
static const int PROC_STAT     = 5;     // /proc/stat
static const int PROC_VERSION  = 6;     // /proc/version
static const int PROC_UPTIME   = 7;     // /proc/uptime

/* names of objects in /proc */
static const char *proc_listing[] = {
  ".",
  "..",
  "loadavg",
  "meminfo",
  "registry",
  "stat",
  "version",
  "uptime",
  NULL
};

static const int PROC_LINK_COUNT = (sizeof (proc_listing) / sizeof (const char *)) - 1;

/* FH_PROC in the table below means the file/directory is handles by
 * fhandler_proc.
 */
static const DWORD proc_fhandlers[PROC_LINK_COUNT] = {
  FH_PROC,
  FH_PROC,
  FH_PROC,
  FH_PROC,
  FH_REGISTRY,
  FH_PROC,
  FH_PROC,
  FH_PROC
};

/* name of the /proc filesystem */
const char proc[] = "/proc";
const int proc_len = sizeof (proc) - 1;

static off_t format_proc_meminfo (char *destbuf, size_t maxsize);
static off_t format_proc_stat (char *destbuf, size_t maxsize);
static off_t format_proc_uptime (char *destbuf, size_t maxsize);

/* auxillary function that returns the fhandler associated with the given path
 * this is where it would be nice to have pattern matching in C - polymorphism
 * just doesn't cut it
 */
DWORD
fhandler_proc::get_proc_fhandler (const char *path)
{
  debug_printf ("get_proc_fhandler(%s)", path);
  path += proc_len;
  /* Since this method is called from path_conv::check we can't rely on
   * it being normalised and therefore the path may have runs of slashes
   * in it.
   */
  while (SLASH_P (*path))
    path++;

  /* Check if this is the root of the virtual filesystem (i.e. /proc).  */
  if (*path == 0)
    return FH_PROC;

  for (int i = 0; proc_listing[i]; i++)
    {
      if (path_prefix_p (proc_listing[i], path, strlen (proc_listing[i])))
	return proc_fhandlers[i];
    }

  if (pinfo (atoi (path)))
    return FH_PROCESS;

  bool has_subdir = false;
  while (*path)
    if (SLASH_P (*path++))
      {
	has_subdir = true;
	break;
      }

  if (has_subdir)
    /* The user is trying to access a non-existent subdirectory of /proc. */
    return FH_BAD;
  else
    /* Return FH_PROC so that we can return EROFS if the user is trying to create
       a file. */
    return FH_PROC;
}

/* Returns 0 if path doesn't exist, >0 if path is a directory,
 * <0 if path is a file.
 */
int
fhandler_proc::exists ()
{
  const char *path = get_name ();
  debug_printf ("exists (%s)", path);
  path += proc_len;
  if (*path == 0)
    return 2;
  for (int i = 0; proc_listing[i]; i++)
    if (pathmatch (path + 1, proc_listing[i]))
      return (proc_fhandlers[i] == FH_PROC) ? -1 : 1;
  return 0;
}

fhandler_proc::fhandler_proc ():
  fhandler_virtual (FH_PROC)
{
}

fhandler_proc::fhandler_proc (DWORD devtype):
  fhandler_virtual (devtype)
{
}

int
fhandler_proc::fstat (struct __stat64 *buf, path_conv *pc)
{
  const char *path = get_name ();
  debug_printf ("fstat (%s)", path);

  path += proc_len;
  (void) fhandler_base::fstat (buf, pc);

  buf->st_mode &= ~_IFMT & NO_W;

  if (!*path)
    {
      buf->st_nlink = PROC_LINK_COUNT;
      buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
      return 0;
    }
  else
    {
      path++;
      for (int i = 0; proc_listing[i]; i++)
	if (pathmatch (path, proc_listing[i]))
	  {
	    if (proc_fhandlers[i] != FH_PROC)
	      buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
	    else
	      {
		buf->st_mode &= NO_X;
		buf->st_mode |= S_IFREG;
	      }
	    return 0;
	  }
    }
  set_errno (ENOENT);
  return -1;
}

struct dirent *
fhandler_proc::readdir (DIR * dir)
{
  if (dir->__d_position >= PROC_LINK_COUNT)
    {
      winpids pids;
      int found = 0;
      for (unsigned i = 0; i < pids.npids; i++)
	if (found++ == dir->__d_position - PROC_LINK_COUNT)
	  {
	    __small_sprintf (dir->__d_dirent->d_name, "%d", pids[i]->pid);
	    dir->__d_position++;
	    return dir->__d_dirent;
	  }
      set_errno (ENMFILE);
      return NULL;
    }

  strcpy (dir->__d_dirent->d_name, proc_listing[dir->__d_position++]);
  syscall_printf ("%p = readdir (%p) (%s)", &dir->__d_dirent, dir,
		  dir->__d_dirent->d_name);
  return dir->__d_dirent;
}

int
fhandler_proc::open (path_conv *pc, int flags, mode_t mode)
{
  int proc_file_no = -1;

  int res = fhandler_virtual::open (pc, flags, mode);
  if (!res)
    goto out;

  set_nohandle (true);

  const char *path;

  path = get_name () + proc_len;

  if (!*path)
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

  proc_file_no = -1;
  for (int i = 0; proc_listing[i]; i++)
    if (path_prefix_p (proc_listing[i], path + 1, strlen (proc_listing[i])))
      {
	proc_file_no = i;
	if (proc_fhandlers[i] != FH_PROC)
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
      }

  if (proc_file_no == -1)
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

  fileid = proc_file_no;
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
fhandler_proc::fill_filebuf ()
{
  switch (fileid)
    {
    case PROC_VERSION:
      {
	if (!filebuf)
	  {
	    struct utsname uts_name;
	    uname (&uts_name);
		bufalloc = strlen (uts_name.sysname) + 1 + strlen (uts_name.release) +
			  1 + strlen (uts_name.version) + 2;
	    filebuf = (char *) realloc (filebuf, bufalloc);
		filesize = __small_sprintf (filebuf, "%s %s %s\n", uts_name.sysname,
			     uts_name.release, uts_name.version);
	  }
	break;
      }
    case PROC_UPTIME:
      {
	filebuf = (char *) realloc (filebuf, bufalloc = 80);
	filesize = format_proc_uptime (filebuf, bufalloc);
	break;
      }
    case PROC_STAT:
      {
	filebuf = (char *) realloc (filebuf, bufalloc = 2048);
	filesize = format_proc_stat (filebuf, bufalloc);
	break;
      }
    case PROC_LOADAVG:
      {
	/*
	 * not really supported - Windows doesn't keep track of these values
	 * Windows 95/98/me does have the KERNEL/CPUUsage performance counter
	 * which is similar.
	 */
	filebuf = (char *) realloc (filebuf, bufalloc = 16);
	filesize = __small_sprintf (filebuf, "%u.%02u %u.%02u %u.%02u\n",
				    0, 0, 0, 0, 0, 0);
	break;
      }
    case PROC_MEMINFO:
      {
	filebuf = (char *) realloc (filebuf, bufalloc = 2048);
	filesize = format_proc_meminfo (filebuf, bufalloc);
	break;
      }
    }
    return true;
}

static
off_t
format_proc_meminfo (char *destbuf, size_t maxsize)
{
  unsigned long mem_total = 0UL, mem_free = 0UL, swap_total = 0UL,
		swap_free = 0UL;
  MEMORYSTATUS memory_status;
  GlobalMemoryStatus (&memory_status);
  mem_total = memory_status.dwTotalPhys;
  mem_free = memory_status.dwAvailPhys;
  swap_total = memory_status.dwTotalPageFile;
  swap_free = memory_status.dwAvailPageFile;
  return __small_sprintf (destbuf, "         total:      used:      free:\n"
				   "Mem:  %10lu %10lu %10lu\n"
				   "Swap: %10lu %10lu %10lu\n"
				   "MemTotal:     %10lu kB\n"
				   "MemFree:      %10lu kB\n"
				   "MemShared:             0 kB\n"
				   "HighTotal:             0 kB\n"
				   "HighFree:              0 kB\n"
				   "LowTotal:     %10lu kB\n"
				   "LowFree:      %10lu kB\n"
				   "SwapTotal:    %10lu kB\n"
				   "SwapFree:     %10lu kB\n",
				   mem_total, mem_total - mem_free, mem_free,
				   swap_total, swap_total - swap_free, swap_free,
				   mem_total >> 10, mem_free >> 10,
				   mem_total >> 10, mem_free >> 10,
				   swap_total >> 10, swap_free >> 10);
}

static
off_t
format_proc_uptime (char *destbuf, size_t maxsize)
{
  unsigned long long uptime = 0ULL, idle_time = 0ULL;
  SYSTEM_PROCESSOR_TIMES spt;

  NTSTATUS ret = NtQuerySystemInformation (SystemProcessorTimes, (PVOID) &spt,
					   sizeof spt, NULL);
  if (!ret && GetLastError () == ERROR_PROC_NOT_FOUND)
    uptime = GetTickCount () / 10;
  else if (ret != STATUS_SUCCESS)
    {
      __seterrno_from_win_error (RtlNtStatusToDosError (ret));
      debug_printf("NtQuerySystemInformation: ret = %d, "
		   "Dos(ret) = %d",
		   ret, RtlNtStatusToDosError (ret));
      return 0;
    }
  else
    {
      idle_time = spt.IdleTime.QuadPart / 100000ULL;
      uptime = (spt.KernelTime.QuadPart +
			spt.UserTime.QuadPart) / 100000ULL;
    }

  return __small_sprintf (destbuf, "%U.%02u %U.%02u\n",
			  uptime / 100, long (uptime % 100),
			  idle_time / 100, long (idle_time % 100));
}

static
off_t
format_proc_stat (char *destbuf, size_t maxsize)
{
  unsigned long long user_time = 0ULL, kernel_time = 0ULL, idle_time = 0ULL;
  unsigned long pages_in = 0UL, pages_out = 0UL, interrupt_count = 0UL,
		context_switches = 0UL, swap_in = 0UL, swap_out = 0UL;
  time_t boot_time = 0;

  if (wincap.is_winnt ())
    {
      NTSTATUS ret;
      SYSTEM_PROCESSOR_TIMES spt;
      SYSTEM_PERFORMANCE_INFORMATION spi;
      SYSTEM_TIME_OF_DAY_INFORMATION stodi;
      ret = NtQuerySystemInformation (SystemProcessorTimes,
				      (PVOID) &spt,
				      sizeof spt, NULL);
      if (ret == STATUS_SUCCESS)
	ret = NtQuerySystemInformation (SystemPerformanceInformation,
					(PVOID) &spi,
					sizeof spi, NULL);
      if (ret == STATUS_SUCCESS)
	ret = NtQuerySystemInformation (SystemTimeOfDayInformation,
					(PVOID) &stodi,
					sizeof stodi, NULL);
      if (ret != STATUS_SUCCESS)
	{
	  __seterrno_from_win_error (RtlNtStatusToDosError (ret));
	  debug_printf("NtQuerySystemInformation: ret = %d, "
		       "Dos(ret) = %d",
		       ret, RtlNtStatusToDosError (ret));
	  return 0;
	}
      kernel_time = (spt.KernelTime.QuadPart - spt.IdleTime.QuadPart) * HZ / 10000000ULL;
      user_time = spt.UserTime.QuadPart * HZ / 10000000ULL;
      idle_time = spt.IdleTime.QuadPart * HZ / 10000000ULL;
      interrupt_count = spt.InterruptCount;
      pages_in = spi.PagesRead;
      pages_out = spi.PagefilePagesWritten + spi.MappedFilePagesWritten;
      /*
       * Note: there is no distinction made in this structure between pages
       * read from the page file and pages read from mapped files, but there
       * is such a distinction made when it comes to writing. Goodness knows
       * why. The value of swap_in, then, will obviously be wrong but its our
       * best guess.
       */
      swap_in = spi.PagesRead;
      swap_out = spi.PagefilePagesWritten;
      context_switches = spi.ContextSwitches;
      boot_time = to_time_t ((FILETIME *) &stodi.BootTime.QuadPart);
    }
  /*
   * else
   *   {
   * There are only two relevant performance counters on Windows 95/98/me,
   * VMM/cPageIns and VMM/cPageOuts. The extra effort needed to read these
   * counters is by no means worth it.
   *   }
   */
  return __small_sprintf (destbuf, "cpu %U %U %U %U\n"
				   "page %u %u\n"
				   "swap %u %u\n"
				   "intr %u\n"
				   "ctxt %u\n"
				   "btime %u\n",
			  user_time, 0ULL,
			  kernel_time, idle_time,
			  pages_in, pages_out,
			  swap_in, swap_out,
			  interrupt_count,
			  context_switches,
			  boot_time);
}

/* fhandler_proc.cc: fhandler for /proc virtual filesystem

   Copyright 2002, 2003 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define _WIN32_WINNT 0x0501

#include "winsup.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/cygwin.h>
#include <ntdef.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "pinfo.h"
#include "dtable.h"
#include "cygheap.h"
#include <assert.h>
#include <sys/utsname.h>
#include <sys/param.h>
#include "ntdll.h"
#include <winioctl.h>
#include "cpuid.h"

#define _COMPILING_NEWLIB
#include <dirent.h>

/* offsets in proc_listing */
static const int PROC_LOADAVG  = 2;     // /proc/loadavg
static const int PROC_MEMINFO  = 3;     // /proc/meminfo
static const int PROC_REGISTRY = 4;     // /proc/registry
static const int PROC_STAT     = 5;     // /proc/stat
static const int PROC_VERSION  = 6;     // /proc/version
static const int PROC_UPTIME   = 7;     // /proc/uptime
static const int PROC_CPUINFO  = 8;     // /proc/cpuinfo
static const int PROC_PARTITIONS = 9;   // /proc/partitions

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
  "cpuinfo",
  "partitions",
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
  FH_PROC,
  FH_PROC,
  FH_PROC,
};

/* name of the /proc filesystem */
const char proc[] = "/proc";
const int proc_len = sizeof (proc) - 1;

static _off64_t format_proc_meminfo (char *destbuf, size_t maxsize);
static _off64_t format_proc_stat (char *destbuf, size_t maxsize);
static _off64_t format_proc_uptime (char *destbuf, size_t maxsize);
static _off64_t format_proc_cpuinfo (char *destbuf, size_t maxsize);
static _off64_t format_proc_partitions (char *destbuf, size_t maxsize);

/* Auxillary function that returns the fhandler associated with the given path
   this is where it would be nice to have pattern matching in C - polymorphism
   just doesn't cut it. */
DWORD
fhandler_proc::get_proc_fhandler (const char *path)
{
  debug_printf ("get_proc_fhandler(%s)", path);
  path += proc_len;
  /* Since this method is called from path_conv::check we can't rely on
     it being normalised and therefore the path may have runs of slashes
     in it.  */
  while (isdirsep (*path))
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
    if (isdirsep (*path++))
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
   <0 if path is a file.  */
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
  fhandler_virtual ()
{
}

int
fhandler_proc::fstat (struct __stat64 *buf)
{
  const char *path = get_name ();
  debug_printf ("fstat (%s)", path);

  path += proc_len;
  (void) fhandler_base::fstat (buf);

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
      return NULL;
    }

  strcpy (dir->__d_dirent->d_name, proc_listing[dir->__d_position++]);
  syscall_printf ("%p = readdir (%p) (%s)", &dir->__d_dirent, dir,
		  dir->__d_dirent->d_name);
  return dir->__d_dirent;
}

int
fhandler_proc::open (int flags, mode_t mode)
{
  int proc_file_no = -1;

  int res = fhandler_virtual::open (flags, mode);
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
	filebuf = (char *) realloc (filebuf, bufalloc = 16384);
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
    case PROC_CPUINFO:
      {
	filebuf = (char *) realloc (filebuf, bufalloc = 16384);
	filesize = format_proc_cpuinfo (filebuf, bufalloc);
	break;
      }
    case PROC_PARTITIONS:
      {
	filebuf = (char *) realloc (filebuf, bufalloc = 4096);
	filesize = format_proc_partitions (filebuf, bufalloc);
	break;
      }
    }
    return true;
}

static _off64_t
format_proc_meminfo (char *destbuf, size_t maxsize)
{
  unsigned long mem_total = 0UL, mem_free = 0UL, swap_total = 0UL,
		swap_free = 0UL;
  MEMORYSTATUS memory_status;
  GlobalMemoryStatus (&memory_status);
  mem_total = memory_status.dwTotalPhys;
  mem_free = memory_status.dwAvailPhys;
  swap_total = memory_status.dwTotalPageFile - mem_total;
  swap_free = memory_status.dwAvailPageFile - mem_total;
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

static _off64_t
format_proc_uptime (char *destbuf, size_t maxsize)
{
  unsigned long long uptime = 0ULL, idle_time = 0ULL;
  SYSTEM_PROCESSOR_TIMES spt;

  if (!GetSystemTimes ((FILETIME *) &spt.IdleTime, (FILETIME *) &spt.KernelTime,
		       (FILETIME *) &spt.UserTime)
      && GetLastError () == ERROR_PROC_NOT_FOUND)
    {
      NTSTATUS ret = NtQuerySystemInformation (SystemProcessorTimes, (PVOID) &spt,
					       sizeof spt, NULL);
      if (!ret && GetLastError () == ERROR_PROC_NOT_FOUND)
	{
	  uptime = GetTickCount () / 10;
	  goto out;
	}
      else if (ret != STATUS_SUCCESS)
	{
	  __seterrno_from_win_error (RtlNtStatusToDosError (ret));
	  debug_printf("NtQuerySystemInformation: ret = %d, "
		       "Dos(ret) = %d",
		       ret, RtlNtStatusToDosError (ret));
	  return 0;
	}
    }
  idle_time = spt.IdleTime.QuadPart / 100000ULL;
  uptime = (spt.KernelTime.QuadPart +
	    spt.UserTime.QuadPart) / 100000ULL;
out:

  return __small_sprintf (destbuf, "%U.%02u %U.%02u\n",
			  uptime / 100, long (uptime % 100),
			  idle_time / 100, long (idle_time % 100));
}

static _off64_t
format_proc_stat (char *destbuf, size_t maxsize)
{
  unsigned long pages_in = 0UL, pages_out = 0UL, interrupt_count = 0UL,
		context_switches = 0UL, swap_in = 0UL, swap_out = 0UL;
  time_t boot_time = 0;

  char *eobuf = destbuf;
  if (!wincap.is_winnt ())
    eobuf += __small_sprintf (destbuf, "cpu %U %U %U %U\n", 0ULL, 0ULL, 0ULL, 0ULL);
  else
    {
      NTSTATUS ret;
      SYSTEM_PERFORMANCE_INFORMATION spi;
      SYSTEM_TIME_OF_DAY_INFORMATION stodi;

      SYSTEM_BASIC_INFORMATION sbi;
      if ((ret = NtQuerySystemInformation (SystemBasicInformation,
					   (PVOID) &sbi, sizeof sbi, NULL))
	  != STATUS_SUCCESS)
	{
	  __seterrno_from_win_error (RtlNtStatusToDosError (ret));
	  debug_printf ("NtQuerySystemInformation: ret = %d, "
			"Dos(ret) = %d",
			ret, RtlNtStatusToDosError (ret));
	  sbi.NumberProcessors = 1;
	}

      SYSTEM_PROCESSOR_TIMES spt[sbi.NumberProcessors];
      ret = NtQuerySystemInformation (SystemProcessorTimes, (PVOID) spt,
				      sizeof spt[0] * sbi.NumberProcessors, NULL);
      interrupt_count = 0;
      if (ret == STATUS_SUCCESS)
	{
	  unsigned long long user_time = 0ULL, kernel_time = 0ULL, idle_time = 0ULL;
	  for (int i = 0; i < sbi.NumberProcessors; i++)
	    {
	      kernel_time += (spt[i].KernelTime.QuadPart - spt[i].IdleTime.QuadPart) * HZ / 10000000ULL;
	      user_time += spt[i].UserTime.QuadPart * HZ / 10000000ULL;
	      idle_time += spt[i].IdleTime.QuadPart * HZ / 10000000ULL;
	    }

	  eobuf += __small_sprintf (eobuf, "cpu %U %U %U %U\n",
				    user_time, 0ULL, kernel_time, idle_time);
	  user_time = 0ULL, kernel_time = 0ULL, idle_time = 0ULL;
	  for (int i = 0; i < sbi.NumberProcessors; i++)
	    {
	      interrupt_count += spt[i].InterruptCount;
	      kernel_time = (spt[i].KernelTime.QuadPart - spt[i].IdleTime.QuadPart) * HZ / 10000000ULL;
	      user_time = spt[i].UserTime.QuadPart * HZ / 10000000ULL;
	      idle_time = spt[i].IdleTime.QuadPart * HZ / 10000000ULL;
	      eobuf += __small_sprintf (eobuf, "cpu%d %U %U %U %U\n", i,
					user_time, 0ULL, kernel_time, idle_time);
	    }

	  ret = NtQuerySystemInformation (SystemPerformanceInformation,
					  (PVOID) &spi, sizeof spi, NULL);
	}
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
  eobuf += __small_sprintf (eobuf, "page %u %u\n"
				   "swap %u %u\n"
				   "intr %u\n"
				   "ctxt %u\n"
				   "btime %u\n",
				   pages_in, pages_out,
				   swap_in, swap_out,
				   interrupt_count,
				   context_switches,
				   boot_time);
  return eobuf - destbuf;
}

#define read_value(x,y) \
      do {\
	dwCount = BUFSIZE; \
	if ((dwError = RegQueryValueEx (hKey, x, NULL, &dwType, (BYTE *) szBuffer, &dwCount)), \
	    (dwError != ERROR_SUCCESS && dwError != ERROR_MORE_DATA)) \
	  { \
	    __seterrno_from_win_error (dwError); \
	    debug_printf ("RegQueryValueEx failed retcode %d", dwError); \
	    return 0; \
	  } \
	if (dwType != y) \
	  { \
	    debug_printf ("Value %s had an unexpected type (expected %d, found %d)", y, dwType); \
	    return 0; \
	  }\
      } while (0)

#define print(x) \
	do { \
	  strcpy (bufptr, x), \
	  bufptr += sizeof (x) - 1; \
	} while (0)

static _off64_t
format_proc_cpuinfo (char *destbuf, size_t maxsize)
{
  SYSTEM_INFO siSystemInfo;
  HKEY hKey;
  DWORD dwError, dwCount, dwType;
  DWORD dwOldThreadAffinityMask;
  int cpu_number;
  const int BUFSIZE = 256;
  CHAR szBuffer[BUFSIZE];
  char *bufptr = destbuf;

  GetSystemInfo (&siSystemInfo);

  for (cpu_number = 0;;cpu_number++)
    {
      __small_sprintf (szBuffer, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\%d", cpu_number);

      if ((dwError = RegOpenKeyEx (HKEY_LOCAL_MACHINE, szBuffer, 0, KEY_QUERY_VALUE, &hKey)) != ERROR_SUCCESS)
	{
	  if (dwError == ERROR_FILE_NOT_FOUND)
	    break;
	  __seterrno_from_win_error (dwError);
	  debug_printf ("RegOpenKeyEx failed retcode %d", dwError);
	  return 0;
	}

      dwOldThreadAffinityMask = SetThreadAffinityMask (GetCurrentThread (), 1 << cpu_number);
      if (dwOldThreadAffinityMask == 0)
	debug_printf ("SetThreadAffinityMask failed %E");
      // I'm not sure whether the thread changes processor immediately
      // and I'm not sure whether this function will cause the thread to be rescheduled
      low_priority_sleep (0);

      bool has_cpuid = false;

      if (!can_set_flag (0x00040000))
	debug_printf ("386 processor - no cpuid");
      else
	{
	  debug_printf ("486 processor");
	  if (can_set_flag (0x00200000))
	    {
	      debug_printf ("processor supports CPUID instruction");
	      has_cpuid = true;
	    }
	  else
	    debug_printf ("processor does not support CPUID instruction");
	}


      if (!has_cpuid)
	{
	  bufptr += __small_sprintf (bufptr, "processor       : %d\n", cpu_number);
	  read_value ("VendorIdentifier", REG_SZ);
	  bufptr += __small_sprintf (bufptr, "vendor_id       : %s\n", szBuffer);
	  read_value ("Identifier", REG_SZ);
	  bufptr += __small_sprintf (bufptr, "identifier      : %s\n", szBuffer);
	  if (wincap.is_winnt ())
	    {
	      read_value ("~Mhz", REG_DWORD);
	      bufptr += __small_sprintf (bufptr, "cpu MHz         : %u\n", *(DWORD *) szBuffer);

	      print ("flags           :");
	      if (IsProcessorFeaturePresent (PF_3DNOW_INSTRUCTIONS_AVAILABLE))
		print (" 3dnow");
	      if (IsProcessorFeaturePresent (PF_COMPARE_EXCHANGE_DOUBLE))
		print (" cx8");
	      if (!IsProcessorFeaturePresent (PF_FLOATING_POINT_EMULATED))
		print (" fpu");
	      if (IsProcessorFeaturePresent (PF_MMX_INSTRUCTIONS_AVAILABLE))
		print (" mmx");
	      if (IsProcessorFeaturePresent (PF_PAE_ENABLED))
		print (" pae");
	      if (IsProcessorFeaturePresent (PF_RDTSC_INSTRUCTION_AVAILABLE))
		print (" tsc");
	      if (IsProcessorFeaturePresent (PF_XMMI_INSTRUCTIONS_AVAILABLE))
		print (" sse");
	      if (IsProcessorFeaturePresent (PF_XMMI64_INSTRUCTIONS_AVAILABLE))
		print (" sse2");
	    }
	}
      else
	{
	  bufptr += __small_sprintf (bufptr, "processor       : %d\n", cpu_number);
	  unsigned maxf, vendor_id[4], unused;
	  cpuid (&maxf, &vendor_id[0], &vendor_id[2], &vendor_id[1], 0);
	  maxf &= 0xffff;
	  vendor_id[3] = 0;
	  bufptr += __small_sprintf (bufptr, "vendor_id       : %s\n", (char *)vendor_id);
	  unsigned cpu_mhz  = 0;
	  if (wincap.is_winnt ())
	    {
	      read_value ("~Mhz", REG_DWORD);
	      cpu_mhz = *(DWORD *)szBuffer;
	    }
	  if (maxf >= 1)
	    {
	      unsigned features2, features1, extra_info, cpuid_sig;
	      cpuid (&cpuid_sig, &extra_info, &features2, &features1, 1);
	      /* unsigned extended_family = (cpuid_sig & 0x0ff00000) >> 20,
			  extended_model  = (cpuid_sig & 0x000f0000) >> 16; */
	      unsigned type            = (cpuid_sig & 0x00003000) >> 12,
		       family          = (cpuid_sig & 0x00000f00) >> 8,
		       model           = (cpuid_sig & 0x000000f0) >> 4,
		       stepping        = cpuid_sig & 0x0000000f;
	      unsigned brand_id        = extra_info & 0x0000000f,
		       cpu_count       = (extra_info & 0x00ff0000) >> 16,
		       apic_id         = (extra_info & 0xff000000) >> 24;
	      const char *type_str;
	      switch (type)
		{
		case 0:
		  type_str = "primary processor";
		  break;
		case 1:
		  type_str = "overdrive processor";
		  break;
		case 2:
		  type_str = "secondary processor";
		  break;
		case 3:
		default:
		  type_str = "reserved";
		  break;
		}
	      unsigned maxe = 0;
	      cpuid (&maxe, &unused, &unused, &unused, 0x80000000);
	      if (maxe >= 0x80000004)
		{
		  unsigned *model_name = (unsigned *) szBuffer;
		  cpuid (&model_name[0], &model_name[1], &model_name[2], &model_name[3], 0x80000002);
		  cpuid (&model_name[4], &model_name[5], &model_name[6], &model_name[7], 0x80000003);
		  cpuid (&model_name[8], &model_name[9], &model_name[10], &model_name[11], 0x80000004);
		  model_name[12] = 0;
		}
	      else
		{
		  // could implement a lookup table here if someone needs it
		  strcpy (szBuffer, "unknown");
		}
	      if (wincap.is_winnt ())
		{
		  bufptr += __small_sprintf (bufptr, "type            : %s\n"
						     "cpu family      : %d\n"
						     "model           : %d\n"
						     "model name      : %s\n"
						     "stepping        : %d\n"
						     "brand id        : %d\n"
						     "cpu count       : %d\n"
						     "apic id         : %d\n"
						     "cpu MHz         : %d\n"
						     "fpu             : %s\n",
					     type_str,
					     family,
					     model,
					     szBuffer,
					     stepping,
					     brand_id,
					     cpu_count,
					     apic_id,
					     cpu_mhz,
					     (features1 & (1 << 0)) ? "yes" : "no");
		}
	      else
		{
		  bufptr += __small_sprintf (bufptr, "type            : %s\n"
						     "cpu family      : %d\n"
						     "model           : %d\n"
						     "model name      : %s\n"
						     "stepping        : %d\n"
						     "brand id        : %d\n"
						     "cpu count       : %d\n"
						     "apic id         : %d\n"
						     "fpu             : %s\n",
					     type_str,
					     family,
					     model,
					     szBuffer,
					     stepping,
					     brand_id,
					     cpu_count,
					     apic_id,
					     (features1 & (1 << 0)) ? "yes" : "no");
		}
	      print ("flags           :");
	      if (features1 & (1 << 0))
		print (" fpu");
	      if (features1 & (1 << 1))
		print (" vme");
	      if (features1 & (1 << 2))
		print (" de");
	      if (features1 & (1 << 3))
		print (" pse");
	      if (features1 & (1 << 4))
		print (" tsc");
	      if (features1 & (1 << 5))
		print (" msr");
	      if (features1 & (1 << 6))
		print (" pae");
	      if (features1 & (1 << 7))
		print (" mce");
	      if (features1 & (1 << 8))
		print (" cx8");
	      if (features1 & (1 << 9))
		print (" apic");
	      if (features1 & (1 << 11))
		print (" sep");
	      if (features1 & (1 << 12))
		print (" mtrr");
	      if (features1 & (1 << 13))
		print (" pge");
	      if (features1 & (1 << 14))
		print (" mca");
	      if (features1 & (1 << 15))
		print (" cmov");
	      if (features1 & (1 << 16))
		print (" pat");
	      if (features1 & (1 << 17))
		print (" pse36");
	      if (features1 & (1 << 18))
		print (" psn");
	      if (features1 & (1 << 19))
		print (" clfl");
	      if (features1 & (1 << 21))
		print (" dtes");
	      if (features1 & (1 << 22))
		print (" acpi");
	      if (features1 & (1 << 23))
		print (" mmx");
	      if (features1 & (1 << 24))
		print (" fxsr");
	      if (features1 & (1 << 25))
		print (" sse");
	      if (features1 & (1 << 26))
		print (" sse2");
	      if (features1 & (1 << 27))
		print (" ss");
	      if (features1 & (1 << 28))
		print (" htt");
	      if (features1 & (1 << 29))
		print (" tmi");
	      if (features1 & (1 << 30))
		print (" ia-64");
	      if (features1 & (1 << 31))
		print (" pbe");
	      if (features2 & (1 << 0))
		print (" sse3");
	      if (features2 & (1 << 3))
		print (" mon");
	      if (features2 & (1 << 4))
		print (" dscpl");
	      if (features2 & (1 << 8))
		print (" tm2");
	      if (features2 & (1 << 10))
		print (" cid");
	    }
	  else if (wincap.is_winnt ())
	    {
	      bufptr += __small_sprintf (bufptr, "cpu MHz         : %d\n"
						 "fpu             : %s\n",
						 cpu_mhz,
						 IsProcessorFeaturePresent (PF_FLOATING_POINT_EMULATED) ? "no" : "yes");
	    }
	}
      if (dwOldThreadAffinityMask != 0)
	SetThreadAffinityMask (GetCurrentThread (), dwOldThreadAffinityMask);

      RegCloseKey (hKey);
      bufptr += __small_sprintf (bufptr, "\n");
  }

  return bufptr - destbuf;
}

#undef read_value

static _off64_t
format_proc_partitions (char *destbuf, size_t maxsize)
{
  char *bufptr = destbuf;
  print ("major minor  #blocks  name\n\n");

  if (wincap.is_winnt ())
    {
      for (int drive_number=0;;drive_number++)
	{
	  CHAR szDriveName[MAX_PATH];
	  __small_sprintf (szDriveName, "\\\\.\\PHYSICALDRIVE%d", drive_number);
	  HANDLE hDevice;
	  hDevice = CreateFile (szDriveName,
				GENERIC_READ,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				0,
				NULL);
	  if (hDevice == INVALID_HANDLE_VALUE)
	    {
	      if (GetLastError () == ERROR_PATH_NOT_FOUND)
		  break;
	      __seterrno ();
	      debug_printf ("CreateFile %d %E", GetLastError ());
	      break;
	    }
	  else
	    {
	      DWORD dwBytesReturned, dwRetCode;
	      DISK_GEOMETRY dg;
	      int buf_size = 4096;
	      char buf[buf_size];
	      dwRetCode = DeviceIoControl (hDevice,
					   IOCTL_DISK_GET_DRIVE_GEOMETRY,
					   NULL,
					   0,
					   &dg,
					   sizeof (dg),
					   &dwBytesReturned,
					   NULL);
	      if (!dwRetCode)
		debug_printf ("DeviceIoControl %E");
	      else
		{
		  bufptr += __small_sprintf (bufptr, "%5d %5d %9U sd%c\n",
					     FH_FLOPPY,
					     drive_number * 16 + 32,
					     (long long)((dg.Cylinders.QuadPart * dg.TracksPerCylinder *
					      dg.SectorsPerTrack * dg.BytesPerSector) >> 6),
					     drive_number + 'a');
		}
	      while (dwRetCode = DeviceIoControl (hDevice,
						  IOCTL_DISK_GET_DRIVE_LAYOUT,
						  NULL,
						  0,
						  (DRIVE_LAYOUT_INFORMATION *) buf,
						  buf_size,
						  &dwBytesReturned,
						  NULL),
		     !dwRetCode && GetLastError () == ERROR_INSUFFICIENT_BUFFER)
	      buf_size *= 2;
	      if (!dwRetCode)
		debug_printf ("DeviceIoControl %E");
	      else
		{
		  DRIVE_LAYOUT_INFORMATION *dli = (DRIVE_LAYOUT_INFORMATION *) buf;
		  for (unsigned partition = 0; partition < dli->PartitionCount; partition++)
		    {
		      if (dli->PartitionEntry[partition].PartitionLength.QuadPart == 0)
			continue;
		      bufptr += __small_sprintf (bufptr, "%5d %5d %9U sd%c%d\n",
						 FH_FLOPPY,
						 drive_number * 16 + partition + 33,
						 (long long)(dli->PartitionEntry[partition].PartitionLength.QuadPart >> 6),
						 drive_number + 'a',
						 partition + 1);
		    }
		}

	      CloseHandle (hDevice);
	    }
	}
    }
  else
    {
      // not worth the effort
      // you need a 16 bit thunk DLL to access the partition table on Win9x
      // and then you have to decode it yourself
    }
  return bufptr - destbuf;
}

#undef print

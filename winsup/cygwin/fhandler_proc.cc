/* fhandler_proc.cc: fhandler for /proc virtual filesystem

   Copyright 2002, 2003, 2004, 2005, 2006, 2007 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

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
#include <ctype.h>
#include <winioctl.h>
#include <wchar.h>
#include "cpuid.h"

#define _COMPILING_NEWLIB
#include <dirent.h>

/* offsets in proc_listing */
static const int PROC_LOADAVG    =  2;   // /proc/loadavg
static const int PROC_MEMINFO    =  3;   // /proc/meminfo
static const int PROC_REGISTRY   =  4;   // /proc/registry
static const int PROC_STAT       =  5;   // /proc/stat
static const int PROC_VERSION    =  6;   // /proc/version
static const int PROC_UPTIME     =  7;   // /proc/uptime
static const int PROC_CPUINFO    =  8;   // /proc/cpuinfo
static const int PROC_PARTITIONS =  9;   // /proc/partitions
static const int PROC_SELF       = 10;   // /proc/self
static const int PROC_REGISTRY32 = 11;   // /proc/registry32
static const int PROC_REGISTRY64 = 12;   // /proc/registry64
static const int PROC_NET        = 13;   // /proc/net

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
  "self",
  "registry32",
  "registry64",
  "net",
  NULL
};

#define PROC_DIR_COUNT 4

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
  FH_PROC,
  FH_REGISTRY,
  FH_REGISTRY,
  FH_PROCNET,
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
    /* Return FH_PROC so that we can return EROFS if the user is trying to
       create a file. */
    return FH_PROC;
}

/* Returns 0 if path doesn't exist, >0 if path is a directory,
   -1 if path is a file, -2 if it's a symlink.  */
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
      {
	fileid = i;
	return (proc_fhandlers[i] == FH_PROC) ? (i == PROC_SELF ? -2 : -1) : 1;
      }
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
  fhandler_base::fstat (buf);

  buf->st_mode &= ~_IFMT & NO_W;

  if (!*path)
    {
      winpids pids ((DWORD) 0);
      buf->st_ino = 2;
      buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
      buf->st_nlink = PROC_DIR_COUNT + 2 + pids.npids;
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
	    else if (i == PROC_SELF)
	      buf->st_mode = S_IFLNK | S_IRWXU | S_IRWXG | S_IRWXO;
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

int
fhandler_proc::readdir (DIR *dir, dirent *de)
{
  int res;
  if (dir->__d_position < PROC_LINK_COUNT)
    {
      strcpy (de->d_name, proc_listing[dir->__d_position++]);
      dir->__flags |= dirent_saw_dot | dirent_saw_dot_dot;
      res = 0;
    }
  else
    {
      winpids pids ((DWORD) 0);
      int found = 0;
      res = ENMFILE;
      for (unsigned i = 0; i < pids.npids; i++)
	if (found++ == dir->__d_position - PROC_LINK_COUNT)
	  {
	    __small_sprintf (de->d_name, "%d", pids[i]->pid);
	    dir->__d_position++;
	    res = 0;
	    break;
	  }
    }

  syscall_printf ("%d = readdir (%p, %p) (%s)", res, dir, de, de->d_name);
  return res;
}

int
fhandler_proc::open (int flags, mode_t mode)
{
  int proc_file_no = -1;

  int res = fhandler_virtual::open (flags, mode);
  if (!res)
    goto out;

  nohandle (true);

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
	    bufalloc = strlen (uts_name.sysname) + 1
		       + strlen (uts_name.release) + 1
		       + strlen (uts_name.version) + 2;
	    filebuf = (char *) crealloc_abort (filebuf, bufalloc);
	    filesize = __small_sprintf (filebuf, "%s %s %s\n",
					uts_name.sysname, uts_name.release,
					uts_name.version);
	  }
	break;
      }
    case PROC_UPTIME:
      {
	filebuf = (char *) crealloc_abort (filebuf, bufalloc = 80);
	filesize = format_proc_uptime (filebuf, bufalloc);
	break;
      }
    case PROC_STAT:
      {
	filebuf = (char *) crealloc_abort (filebuf, bufalloc = 16384);
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
	filebuf = (char *) crealloc_abort (filebuf, bufalloc = 16);
	filesize = __small_sprintf (filebuf, "%u.%02u %u.%02u %u.%02u\n",
				    0, 0, 0, 0, 0, 0);
	break;
      }
    case PROC_MEMINFO:
      {
	filebuf = (char *) crealloc_abort (filebuf, bufalloc = 2048);
	filesize = format_proc_meminfo (filebuf, bufalloc);
	break;
      }
    case PROC_CPUINFO:
      {
	filebuf = (char *) crealloc_abort (filebuf, bufalloc = 16384);
	filesize = format_proc_cpuinfo (filebuf, bufalloc);
	break;
      }
    case PROC_PARTITIONS:
      {
	filebuf = (char *) crealloc_abort (filebuf, bufalloc = 4096);
	filesize = format_proc_partitions (filebuf, bufalloc);
	break;
      }
    case PROC_SELF:
      {
	filebuf = (char *) crealloc_abort (filebuf, bufalloc = 32);
	filesize = __small_sprintf (filebuf, "%d", getpid ());
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
  PSYSTEM_PAGEFILE_INFORMATION spi = NULL;
  ULONG size = 512;
  NTSTATUS ret = STATUS_SUCCESS;
  spi = (PSYSTEM_PAGEFILE_INFORMATION) malloc (size);
  if (spi)
    {
      ret = NtQuerySystemInformation (SystemPagefileInformation, (PVOID) spi,
				      size, &size);
      if (ret == STATUS_INFO_LENGTH_MISMATCH)
	{
	  free (spi);
	  spi = (PSYSTEM_PAGEFILE_INFORMATION) malloc (size);
	  if (spi)
	    ret = NtQuerySystemInformation (SystemPagefileInformation,
					    (PVOID) spi, size, &size);
	}
    }
  if (!spi || ret || (!ret && GetLastError () == ERROR_PROC_NOT_FOUND))
    {
      swap_total = memory_status.dwTotalPageFile - mem_total;
      swap_free = memory_status.dwAvailPageFile - mem_total;
    }
  else
    {
      PSYSTEM_PAGEFILE_INFORMATION spp = spi;
      do
	{
	  swap_total += spp->CurrentSize * getsystempagesize ();
	  swap_free += (spp->CurrentSize - spp->TotalUsed)
		       * getsystempagesize ();
	}
      while (spp->NextEntryOffset
	     && (spp = (PSYSTEM_PAGEFILE_INFORMATION)
			   ((char *) spp + spp->NextEntryOffset)));
    }
  if (spi)
    free (spi);
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

  NTSTATUS ret;
  SYSTEM_BASIC_INFORMATION sbi;
  SYSTEM_TIME_OF_DAY_INFORMATION stodi;
  SYSTEM_PERFORMANCE_INFORMATION spi;

  ret = NtQuerySystemInformation (SystemBasicInformation, (PVOID) &sbi,
				  sizeof sbi, NULL);
  if (!NT_SUCCESS (ret))
    {
      debug_printf ("NtQuerySystemInformation: ret %d", ret);
      sbi.NumberProcessors = 1;
    }

  ret = NtQuerySystemInformation (SystemTimeOfDayInformation, &stodi,
				  sizeof stodi, NULL);
  if (NT_SUCCESS (ret))
    uptime = (stodi.CurrentTime.QuadPart - stodi.BootTime.QuadPart) / 100000ULL;

  ret = NtQuerySystemInformation (SystemPerformanceInformation, &spi,
				  sizeof spi, NULL);
  if (NT_SUCCESS (ret))
    idle_time = (spi.IdleTime.QuadPart / sbi.NumberProcessors) / 100000ULL;

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
  NTSTATUS ret;
  SYSTEM_PERFORMANCE_INFORMATION spi;
  SYSTEM_TIME_OF_DAY_INFORMATION stodi;

  SYSTEM_BASIC_INFORMATION sbi;
  if ((ret = NtQuerySystemInformation (SystemBasicInformation,
				       (PVOID) &sbi, sizeof sbi, NULL))
      != STATUS_SUCCESS)
    {
      debug_printf ("NtQuerySystemInformation: ret %d", ret);
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
	  kernel_time += (spt[i].KernelTime.QuadPart - spt[i].IdleTime.QuadPart)
			 * HZ / 10000000ULL;
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
      debug_printf ("NtQuerySystemInformation: ret %d", ret);
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

  for (cpu_number = 0; ; cpu_number++)
    {
      if (cpu_number)
	print ("\n");

      __small_sprintf (szBuffer, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\%d", cpu_number);

      if ((dwError = RegOpenKeyEx (HKEY_LOCAL_MACHINE, szBuffer, 0, KEY_QUERY_VALUE, &hKey)) != ERROR_SUCCESS)
	{
	  if (dwError == ERROR_FILE_NOT_FOUND)
	    break;
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
      else
	{
	  bufptr += __small_sprintf (bufptr, "processor\t: %d\n", cpu_number);
	  unsigned maxf, vendor_id[4], unused;
	  cpuid (&maxf, &vendor_id[0], &vendor_id[2], &vendor_id[1], 0);
	  maxf &= 0xffff;
	  vendor_id[3] = 0;

	  // vendor identification
	  bool is_amd = false, is_intel = false;
	  if (!strcmp ((char*)vendor_id, "AuthenticAMD"))
	    is_amd = true;
	  else if (!strcmp ((char*)vendor_id, "GenuineIntel"))
	    is_intel = true;

	  bufptr += __small_sprintf (bufptr, "vendor_id\t: %s\n",
				     (char *)vendor_id);
	  read_value ("~Mhz", REG_DWORD);
	  unsigned cpu_mhz = *(DWORD *)szBuffer;
	  if (maxf >= 1)
	    {
	      unsigned features2, features1, extra_info, cpuid_sig;
	      cpuid (&cpuid_sig, &extra_info, &features2, &features1, 1);
	      /* unsigned extended_family = (cpuid_sig & 0x0ff00000) >> 20,
			  extended_model  = (cpuid_sig & 0x000f0000) >> 16,
			  type		  = (cpuid_sig & 0x00003000) >> 12; */
	      unsigned family		= (cpuid_sig & 0x00000f00) >> 8,
		       model		= (cpuid_sig & 0x000000f0) >> 4,
		       stepping		= cpuid_sig & 0x0000000f;
	      /* Not printed on Linux */
	      //unsigned brand_id		= extra_info & 0x0000000f;
	      //unsigned cpu_count	= (extra_info & 0x00ff0000) >> 16;
	      unsigned apic_id		= (extra_info & 0xff000000) >> 24;
	      if (family == 15)
		family += (cpuid_sig >> 20) & 0xff;
	      if (family >= 6)
		model += ((cpuid_sig >> 16) & 0x0f) << 4;
	      unsigned maxe = 0;
	      cpuid (&maxe, &unused, &unused, &unused, 0x80000000);
	      if (maxe >= 0x80000004)
		{
		  unsigned *model_name = (unsigned *) szBuffer;
		  cpuid (&model_name[0], &model_name[1], &model_name[2],
			 &model_name[3], 0x80000002);
		  cpuid (&model_name[4], &model_name[5], &model_name[6],
			 &model_name[7], 0x80000003);
		  cpuid (&model_name[8], &model_name[9], &model_name[10],
			 &model_name[11], 0x80000004);
		  model_name[12] = 0;
		}
	      else
		{
		  // could implement a lookup table here if someone needs it
		  strcpy (szBuffer, "unknown");
		}
	      int cache_size = -1,
		  tlb_size = -1,
		  clflush = 64,
		  cache_alignment = 64;
	      if (features1 & (1 << 19)) // CLFSH
		clflush = ((extra_info >> 8) & 0xff) << 3;
	      if (is_intel && family == 15)
		cache_alignment = clflush * 2;
	      if (maxe >= 0x80000005) // L1 Cache and TLB Identifiers
		{
		  unsigned data_cache, inst_cache;
		  cpuid (&unused, &unused, &data_cache, &inst_cache,
			 0x80000005);

		  cache_size = (inst_cache >> 24) + (data_cache >> 24);
		  tlb_size = 0;
		}
	      if (maxe >= 0x80000006) // L2 Cache and L2 TLB Identifiers
		{
		  unsigned tlb, l2;
		  cpuid (&unused, &tlb, &l2, &unused, 0x80000006);

		  cache_size = l2 >> 16;
		  tlb_size = ((tlb >> 16) & 0xfff) + (tlb & 0xfff);
		}
	      bufptr += __small_sprintf (bufptr, "cpu family\t: %d\n"
						 "model\t\t: %d\n"
						 "model name\t: %s\n"
						 "stepping\t: %d\n"
						 "cpu MHz\t\t: %d\n",
					 family,
					 model,
					 szBuffer + strspn (szBuffer, " 	"),
					 stepping,
					 cpu_mhz);
	      if (cache_size >= 0)
		bufptr += __small_sprintf (bufptr, "cache size\t: %d KB\n",
					   cache_size);

	      // Recognize multi-core CPUs
	      if (is_amd && maxe >= 0x80000008)
		{
		  unsigned core_info;
		  cpuid (&unused, &unused, &core_info, &unused, 0x80000008);

		  int max_cores = 1 + (core_info & 0xff);
		  if (max_cores > 1)
		    {
		      int shift = (core_info >> 12) & 0x0f;
		      if (!shift)
			while ((1 << shift) < max_cores)
			  ++shift;
		      int core_id = apic_id & ((1 << shift) - 1);
		      apic_id >>= shift;

		      bufptr += __small_sprintf (bufptr, "physical id\t: %d\n"
							 "core id\t\t: %d\n"
							 "cpu cores\t: %d\n",
						 apic_id, core_id, max_cores);
		    }
		}
	      // Recognize Intel Hyper-Transport CPUs
	      else if (is_intel && (features1 & (1 << 28)) && maxf >= 4)
		{
		  /* TODO */
		}

	      bufptr += __small_sprintf (bufptr, "fpu\t\t: %s\n"
						 "fpu_exception\t: %s\n"
						 "cpuid level\t: %d\n"
						 "wp\t\t: yes\n",
					 (features1 & (1 << 0)) ? "yes" : "no",
					 (features1 & (1 << 0)) ? "yes" : "no",
					 maxf);
	      print ("flags\t\t:");
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
		print (" pn");
	      if (features1 & (1 << 19))
		print (" clflush");
	      if (is_intel && features1 & (1 << 21))
		print (" dts");
	      if (is_intel && features1 & (1 << 22))
		print (" acpi");
	      if (features1 & (1 << 23))
		print (" mmx");
	      if (features1 & (1 << 24))
		print (" fxsr");
	      if (features1 & (1 << 25))
		print (" sse");
	      if (features1 & (1 << 26))
		print (" sse2");
	      if (is_intel && (features1 & (1 << 27)))
		print (" ss");
	      if (features1 & (1 << 28))
		print (" ht");
	      if (is_intel)
		{
		  if (features1 & (1 << 29))
		    print (" tm");
		  if (features1 & (1 << 30))
		    print (" ia64");
		  if (features1 & (1 << 31))
		    print (" pbe");
		}

	      if (is_amd && maxe >= 0x80000001)
		{
		  unsigned features;
		  cpuid (&unused, &unused, &unused, &features, 0x80000001);

		  if (features & (1 << 11))
		    print (" syscall");
		  if (features & (1 << 19)) // Huh?  Not in AMD64 specs.
		    print (" mp");
		  if (features & (1 << 20))
		    print (" nx");
		  if (features & (1 << 22))
		    print (" mmxext");
		  if (features & (1 << 25))
		    print (" fxsr_opt");
		  if (features & (1 << 27))
		    print (" rdtscp");
		  if (features & (1 << 29))
		    print (" lm");
		  if (features & (1 << 30)) // 31th bit is on
		    print (" 3dnowext");
		  if (features & (1 << 31)) // 32th bit (highest) is on
		    print (" 3dnow");
		}

	      if (features2 & (1 << 0))
		print (" pni");
	      if (is_intel)
		{
		  if (features2 & (1 << 3))
		    print (" monitor");
		  if (features2 & (1 << 4))
		    print (" ds_cpl");
		  if (features2 & (1 << 7))
		    print (" tm2");
		  if (features2 & (1 << 8))
		    print (" est");
		  if (features2 & (1 << 10))
		    print (" cid");
		}
	      if (features2 & (1 << 13))
		print (" cx16");

	      if (is_amd && maxe >= 0x80000001)
		{
		  unsigned features;
		  cpuid (&unused, &unused, &features, &unused, 0x80000001);

		  if (features & (1 << 0))
		    print (" lahf_lm");
		  if (features & (1 << 1))
		    print (" cmp_legacy");
		  if (features & (1 << 2))
		    print (" svm");
		  if (features & (1 << 4))
		    print (" cr8_legacy");
		}

	      print ("\n");

	      /* TODO: bogomips */

	      if (tlb_size >= 0)
		bufptr += __small_sprintf (bufptr,
					   "TLB size\t: %d 4K pages\n",
					   tlb_size);
	      bufptr += __small_sprintf (bufptr, "clflush size\t: %d\n"
						 "cache_alignment\t: %d\n",
					 clflush,
					 cache_alignment);

	      if (maxe >= 0x80000008) // Address size
		{
		  unsigned addr_size, phys, virt;
		  cpuid (&addr_size, &unused, &unused, &unused, 0x80000008);

		  phys = addr_size & 0xff;
		  virt = (addr_size >> 8) & 0xff;
		  /* Fix an errata on Intel CPUs */
		  if (is_intel && family == 15 && model == 3 && stepping == 4)
		    phys = 36;
		  bufptr += __small_sprintf (bufptr, "address sizes\t: "
						     "%u bits physical, "
						     "%u bits virtual\n",
					     phys, virt);
		}

	      if (maxe >= 0x80000007) // advanced power management
		{
		  cpuid (&unused, &unused, &unused, &features2, 0x80000007);

		  print ("power management:");
		  if (features2 & (1 << 0))
		    print (" ts");
		  if (features2 & (1 << 1))
		    print (" fid");
		  if (features2 & (1 << 2))
		    print (" vid");
		  if (features2 & (1 << 3))
		    print (" ttp");
		  if (features2 & (1 << 4))
		    print (" tm");
		  if (features2 & (1 << 5))
		    print (" stc");
		}
	    }
	  else
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

  char devname[NAME_MAX + 1];
  OBJECT_ATTRIBUTES attr;
  HANDLE dirhdl, devhdl;
  IO_STATUS_BLOCK io;
  NTSTATUS status;

  /* Open \Device object directory. */
  wchar_t wpath[MAX_PATH] = L"\\Device";
  UNICODE_STRING upath = {14, 16, wpath};
  InitializeObjectAttributes (&attr, &upath, OBJ_CASE_INSENSITIVE, NULL, NULL);
  status = NtOpenDirectoryObject (&dirhdl, DIRECTORY_QUERY, &attr);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("NtOpenDirectoryObject %x", status);
      return bufptr - destbuf;
    }

  /* Traverse \Device directory ... */
  PDIRECTORY_BASIC_INFORMATION dbi = (PDIRECTORY_BASIC_INFORMATION)
				     alloca (640);
  BOOLEAN restart = TRUE;
  ULONG context = 0;
  while (NT_SUCCESS (NtQueryDirectoryObject (dirhdl, dbi, 640, TRUE, restart,
					     &context, NULL)))
    {
      restart = FALSE;
      sys_wcstombs (devname, NAME_MAX + 1, dbi->ObjectName.Buffer,
		    dbi->ObjectName.Length / 2);
      /* ... and check for a "Harddisk[0-9]*" entry. */
      if (!strncasematch (devname, "Harddisk", 8)
	  || dbi->ObjectName.Length < 18
	  || !isdigit (devname[8]))
	continue;
      /* Construct path name for partition 0, which is the whole disk,
	 and try to open. */
      wcscpy (wpath, dbi->ObjectName.Buffer);
      wcscpy (wpath + dbi->ObjectName.Length / 2, L"\\Partition0");
      upath.Length = 22 + dbi->ObjectName.Length;
      upath.MaximumLength = upath.Length + 2;
      InitializeObjectAttributes (&attr, &upath, OBJ_CASE_INSENSITIVE,
				  dirhdl, NULL);
      status = NtOpenFile (&devhdl, READ_CONTROL | FILE_READ_DATA, &attr, &io,
			   FILE_SHARE_VALID_FLAGS, 0);
      if (!NT_SUCCESS (status))
	{
	  /* Retry with READ_CONTROL only for non-privileged users.  This
	     at least prints the Partition0 info, but it doesn't allow access
	     to the drive's layout information.  It beats me, though, why
	     a non-privileged user shouldn't get read access to the drive
	     layout information. */
	  status = NtOpenFile (&devhdl, READ_CONTROL, &attr, &io,
			       FILE_SHARE_VALID_FLAGS, 0);
	  if (!NT_SUCCESS (status))
	    {
	      debug_printf ("NtOpenFile(%s) %x", devname, status);
	      continue;
	    }
	}

      /* Use a buffer since some ioctl buffers aren't fixed size. */
      char buf[256];
      PARTITION_INFORMATION *pi = NULL;
      PARTITION_INFORMATION_EX *pix = NULL;
      DISK_GEOMETRY *dg = NULL;
      DWORD bytes;
      unsigned long drive_number = strtoul (devname + 8, NULL, 10);
      unsigned long long size;

      if (wincap.has_disk_ex_ioctls ()
	  && DeviceIoControl (devhdl, IOCTL_DISK_GET_PARTITION_INFO_EX,
			      NULL, 0, buf, 256, &bytes, NULL))
	{
	  pix = (PARTITION_INFORMATION_EX *) buf;
	  size = pix->PartitionLength.QuadPart;
	}
      else if (DeviceIoControl (devhdl, IOCTL_DISK_GET_PARTITION_INFO,
				NULL, 0, buf, 256, &bytes, NULL))
	{
	  pi = (PARTITION_INFORMATION *) buf;
	  size = pi->PartitionLength.QuadPart;
	}
      else if (DeviceIoControl (devhdl, IOCTL_DISK_GET_DRIVE_GEOMETRY,
				NULL, 0, buf, 256, &bytes, NULL))
	{
	  dg = (DISK_GEOMETRY *) buf;
	  size = (unsigned long long) dg->Cylinders.QuadPart
		       * dg->TracksPerCylinder
		       * dg->SectorsPerTrack
		       * dg->BytesPerSector;
	}
      else
	size = 0;
      if (!pi && !pix && !dg)
	debug_printf ("DeviceIoControl %E");
      else
	{
	  device dev;
	  dev.parsedisk (drive_number, 0);
	  bufptr += __small_sprintf (bufptr, "%5d %5d %9U %s\n",
				     dev.major, dev.minor,
				     size >> 10, dev.name + 5);
	}
      size_t buf_size = 8192;
      while (true)
	{
	  char buf[buf_size];
	  if (DeviceIoControl (devhdl, IOCTL_DISK_GET_DRIVE_LAYOUT,
				NULL, 0, (DRIVE_LAYOUT_INFORMATION *) buf,
				buf_size, &bytes, NULL))
	    /* fall through */;
	  else if (GetLastError () == ERROR_INSUFFICIENT_BUFFER)
	    {
	      buf_size *= 2;
	      continue;
	    }
	  else
	    {
	      debug_printf ("DeviceIoControl %E");
	      break;
	    }
	  DRIVE_LAYOUT_INFORMATION *dli = (DRIVE_LAYOUT_INFORMATION *) buf;
	  for (unsigned part = 0; part < dli->PartitionCount; part++)
	    {
	      if (!dli->PartitionEntry[part].PartitionLength.QuadPart
		  || !dli->PartitionEntry[part].RecognizedPartition)
		continue;
	      device dev;
	      dev.parsedisk (drive_number,
			     dli->PartitionEntry[part].PartitionNumber);
	      size = dli->PartitionEntry[part].PartitionLength.QuadPart >> 10;
	      bufptr += __small_sprintf (bufptr, "%5d %5d %9U %s\n",
					 dev.major, dev.minor,
					 size, dev.name + 5);
	    }
	  break;
	}
      NtClose (devhdl);
    }
  NtClose (dirhdl);

  return bufptr - destbuf;
}

#undef print

/* fhandler_proc.cc: fhandler for /proc virtual filesystem

   Copyright 2002, 2003, 2004, 2005, 2006, 2007, 2009 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "miscfuncs.h"
#include <unistd.h>
#include <stdlib.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "fhandler_virtual.h"
#include "pinfo.h"
#include "dtable.h"
#include "cygheap.h"
#include "tls_pbuf.h"
#include <sys/utsname.h>
#include <sys/param.h>
#include "ntdll.h"
#include <ctype.h>
#include <winioctl.h>
#include <wchar.h>
#include "cpuid.h"

#define _COMPILING_NEWLIB
#include <dirent.h>

static _off64_t format_proc_loadavg (void *, char *&);
static _off64_t format_proc_meminfo (void *, char *&);
static _off64_t format_proc_stat (void *, char *&);
static _off64_t format_proc_version (void *, char *&);
static _off64_t format_proc_uptime (void *, char *&);
static _off64_t format_proc_cpuinfo (void *, char *&);
static _off64_t format_proc_partitions (void *, char *&);
static _off64_t format_proc_self (void *, char *&);
static _off64_t format_proc_mounts (void *, char *&);

/* names of objects in /proc */
static const virt_tab_t proc_tab[] = {
  { ".",	  FH_PROC,	virt_directory,	NULL },
  { "..",	  FH_PROC,	virt_directory,	NULL },
  { "loadavg",	  FH_PROC,	virt_file,	format_proc_loadavg },
  { "meminfo",	  FH_PROC,	virt_file,	format_proc_meminfo },
  { "registry",	  FH_REGISTRY,	virt_directory,	NULL  },
  { "stat",	  FH_PROC,	virt_file,	format_proc_stat },
  { "version",	  FH_PROC,	virt_file,	format_proc_version },
  { "uptime",	  FH_PROC,	virt_file,	format_proc_uptime },
  { "cpuinfo",	  FH_PROC,	virt_file,	format_proc_cpuinfo },
  { "partitions", FH_PROC,	virt_file,	format_proc_partitions },
  { "self",	  FH_PROC,	virt_symlink,	format_proc_self },
  { "mounts",	  FH_PROC,	virt_symlink,	format_proc_mounts },
  { "registry32", FH_REGISTRY,	virt_directory,	NULL },
  { "registry64", FH_REGISTRY,	virt_directory,	NULL },
  { "net",	  FH_PROCNET,	virt_directory,	NULL },
  { NULL,	  0,		virt_none,	NULL }
};

#define PROC_DIR_COUNT 4

static const int PROC_LINK_COUNT = (sizeof (proc_tab) / sizeof (virt_tab_t)) - 1;

/* name of the /proc filesystem */
const char proc[] = "/proc";
const int proc_len = sizeof (proc) - 1;

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

  for (int i = 0; proc_tab[i].name; i++)
    {
      if (path_prefix_p (proc_tab[i].name, path, strlen (proc_tab[i].name),
			 false))
	return proc_tab[i].fhandler;
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
    return virt_rootdir;
  for (int i = 0; proc_tab[i].name; i++)
    if (!strcmp (path + 1, proc_tab[i].name))
      {
	fileid = i;
	return proc_tab[i].type;
      }
  return virt_none;
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
      for (int i = 0; proc_tab[i].name; i++)
	if (!strcmp (path, proc_tab[i].name))
	  {
	    if (proc_tab[i].type == virt_directory)
	      buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
	    else if (proc_tab[i].type == virt_symlink)
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
      strcpy (de->d_name, proc_tab[dir->__d_position++].name);
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
  for (int i = 0; proc_tab[i].name; i++)
    if (path_prefix_p (proc_tab[i].name, path + 1, strlen (proc_tab[i].name),
		       false))
      {
	proc_file_no = i;
	if (proc_tab[i].fhandler != FH_PROC)
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
  if (fileid < PROC_LINK_COUNT && proc_tab[fileid].format_func)
    {
      filesize = proc_tab[fileid].format_func (NULL, filebuf);
      return true;
    }
  return false;
}

static _off64_t
format_proc_version (void *, char *&destbuf)
{
  struct utsname uts_name;

  uname (&uts_name);
  destbuf = (char *) crealloc_abort (destbuf, strlen (uts_name.sysname)
					      + strlen (uts_name.release)
					      + strlen (uts_name.version)
					      + 4);
  return __small_sprintf (destbuf, "%s %s %s\n",
			  uts_name.sysname, uts_name.release, uts_name.version);
}

static _off64_t
format_proc_loadavg (void *, char *&destbuf)
{
  destbuf = (char *) crealloc_abort (destbuf, 16);
  return __small_sprintf (destbuf, "%u.%02u %u.%02u %u.%02u\n",
				    0, 0, 0, 0, 0, 0);
}

static _off64_t
format_proc_meminfo (void *, char *&destbuf)
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
  destbuf = (char *) crealloc_abort (destbuf, 512);
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
format_proc_uptime (void *, char *&destbuf)
{
  unsigned long long uptime = 0ULL, idle_time = 0ULL;
  NTSTATUS ret;
  SYSTEM_TIME_OF_DAY_INFORMATION stodi;
  /* Sizeof SYSTEM_PERFORMANCE_INFORMATION on 64 bit systems.  It
     appears to contain some trailing additional information from
     what I can tell after examining the content.
     FIXME: It would be nice if this could be verified somehow. */
  const size_t sizeof_spi = sizeof (SYSTEM_PERFORMANCE_INFORMATION) + 16;
  PSYSTEM_PERFORMANCE_INFORMATION spi = (PSYSTEM_PERFORMANCE_INFORMATION)
					alloca (sizeof_spi);

  if (!system_info.dwNumberOfProcessors)
    GetSystemInfo (&system_info);

  ret = NtQuerySystemInformation (SystemTimeOfDayInformation, &stodi,
				  sizeof stodi, NULL);
  if (NT_SUCCESS (ret))
    uptime = (stodi.CurrentTime.QuadPart - stodi.BootTime.QuadPart) / 100000ULL;
  else
    debug_printf ("NtQuerySystemInformation(SystemTimeOfDayInformation), "
		  "status %p", ret);

  if (NT_SUCCESS (NtQuerySystemInformation (SystemPerformanceInformation,
						 spi, sizeof_spi, NULL)))
    idle_time = (spi->IdleTime.QuadPart / system_info.dwNumberOfProcessors)
		/ 100000ULL;

  destbuf = (char *) crealloc_abort (destbuf, 80);
  return __small_sprintf (destbuf, "%U.%02u %U.%02u\n",
			  uptime / 100, long (uptime % 100),
			  idle_time / 100, long (idle_time % 100));
}

static _off64_t
format_proc_stat (void *, char *&destbuf)
{
  unsigned long pages_in = 0UL, pages_out = 0UL, interrupt_count = 0UL,
		context_switches = 0UL, swap_in = 0UL, swap_out = 0UL;
  time_t boot_time = 0;
  NTSTATUS ret;
  /* Sizeof SYSTEM_PERFORMANCE_INFORMATION on 64 bit systems.  It
     appears to contain some trailing additional information from
     what I can tell after examining the content.
     FIXME: It would be nice if this could be verified somehow. */
  const size_t sizeof_spi = sizeof (SYSTEM_PERFORMANCE_INFORMATION) + 16;
  PSYSTEM_PERFORMANCE_INFORMATION spi = (PSYSTEM_PERFORMANCE_INFORMATION)
					alloca (sizeof_spi);
  SYSTEM_TIME_OF_DAY_INFORMATION stodi;
  tmp_pathbuf tp;

  char *buf = tp.c_get ();
  char *eobuf = buf;

  if (!system_info.dwNumberOfProcessors)
    GetSystemInfo (&system_info);

  SYSTEM_PROCESSOR_TIMES spt[system_info.dwNumberOfProcessors];
  ret = NtQuerySystemInformation (SystemProcessorTimes, (PVOID) spt,
				  sizeof spt[0] * system_info.dwNumberOfProcessors, NULL);
  if (!NT_SUCCESS (ret))
    debug_printf ("NtQuerySystemInformation(SystemProcessorTimes), "
		  "status %p", ret);
  else
    {
      unsigned long long user_time = 0ULL, kernel_time = 0ULL, idle_time = 0ULL;
      for (unsigned long i = 0; i < system_info.dwNumberOfProcessors; i++)
	{
	  kernel_time += (spt[i].KernelTime.QuadPart - spt[i].IdleTime.QuadPart)
			 * HZ / 10000000ULL;
	  user_time += spt[i].UserTime.QuadPart * HZ / 10000000ULL;
	  idle_time += spt[i].IdleTime.QuadPart * HZ / 10000000ULL;
	}

      eobuf += __small_sprintf (eobuf, "cpu %U %U %U %U\n",
				user_time, 0ULL, kernel_time, idle_time);
      user_time = 0ULL, kernel_time = 0ULL, idle_time = 0ULL;
      for (unsigned long i = 0; i < system_info.dwNumberOfProcessors; i++)
	{
	  interrupt_count += spt[i].InterruptCount;
	  kernel_time = (spt[i].KernelTime.QuadPart - spt[i].IdleTime.QuadPart) * HZ / 10000000ULL;
	  user_time = spt[i].UserTime.QuadPart * HZ / 10000000ULL;
	  idle_time = spt[i].IdleTime.QuadPart * HZ / 10000000ULL;
	  eobuf += __small_sprintf (eobuf, "cpu%d %U %U %U %U\n", i,
				    user_time, 0ULL, kernel_time, idle_time);
	}

      ret = NtQuerySystemInformation (SystemPerformanceInformation,
				      (PVOID) spi, sizeof_spi, NULL);
      if (!NT_SUCCESS (ret))
	{
	  debug_printf ("NtQuerySystemInformation(SystemPerformanceInformation)"
	  		", status %p", ret);
	  memset (spi, 0, sizeof_spi);
	}
      ret = NtQuerySystemInformation (SystemTimeOfDayInformation,
				      (PVOID) &stodi,
				      sizeof stodi, NULL);
      if (!NT_SUCCESS (ret))
	debug_printf ("NtQuerySystemInformation(SystemTimeOfDayInformation), "
		      "status %p", ret);
    }
  if (!NT_SUCCESS (ret))
    return 0;

  pages_in = spi->PagesRead;
  pages_out = spi->PagefilePagesWritten + spi->MappedFilePagesWritten;
  /*
   * Note: there is no distinction made in this structure between pages
   * read from the page file and pages read from mapped files, but there
   * is such a distinction made when it comes to writing. Goodness knows
   * why. The value of swap_in, then, will obviously be wrong but its our
   * best guess.
   */
  swap_in = spi->PagesRead;
  swap_out = spi->PagefilePagesWritten;
  context_switches = spi->ContextSwitches;
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
  destbuf = (char *) crealloc_abort (destbuf, eobuf - buf);
  memcpy (destbuf, buf, eobuf - buf);
  return eobuf - buf;
}

#define read_value(x,y) \
      do {\
	dwCount = BUFSIZE; \
	if ((dwError = RegQueryValueEx (hKey, x, NULL, &dwType, in_buf.b, &dwCount)), \
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
format_proc_cpuinfo (void *, char *&destbuf)
{
  SYSTEM_INFO siSystemInfo;
  HKEY hKey;
  DWORD dwError, dwCount, dwType;
  DWORD dwOldThreadAffinityMask;
  int cpu_number;
  const int BUFSIZE = 256;
  union
  {
    BYTE b[BUFSIZE];
    char s[BUFSIZE];
    DWORD d;
    unsigned m[13];
  } in_buf;
  tmp_pathbuf tp;

  char *buf = tp.c_get ();
  char *bufptr = buf;

  GetSystemInfo (&siSystemInfo);
  for (cpu_number = 0; ; cpu_number++)
    {
      if (cpu_number)
	print ("\n");

      __small_sprintf (in_buf.s, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\%d", cpu_number);

      if ((dwError = RegOpenKeyEx (HKEY_LOCAL_MACHINE, in_buf.s, 0, KEY_QUERY_VALUE, &hKey)) != ERROR_SUCCESS)
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
	  bufptr += __small_sprintf (bufptr, "vendor_id       : %s\n", in_buf.s);
	  read_value ("Identifier", REG_SZ);
	  bufptr += __small_sprintf (bufptr, "identifier      : %s\n", in_buf.s);
	  read_value ("~Mhz", REG_DWORD);
	  bufptr += __small_sprintf (bufptr, "cpu MHz         : %u\n", in_buf.d);

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
	  unsigned cpu_mhz = in_buf.d;
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
		  cpuid (&in_buf.m[0], &in_buf.m[1], &in_buf.m[2],
			 &in_buf.m[3], 0x80000002);
		  cpuid (&in_buf.m[4], &in_buf.m[5], &in_buf.m[6],
			 &in_buf.m[7], 0x80000003);
		  cpuid (&in_buf.m[8], &in_buf.m[9], &in_buf.m[10],
			 &in_buf.m[11], 0x80000004);
		  in_buf.m[12] = 0;
		}
	      else
		{
		  // could implement a lookup table here if someone needs it
		  strcpy (in_buf.s, "unknown");
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
					 in_buf.s + strspn (in_buf.s, " 	"),
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
		  if (features & (1 << 26))
		    print (" pdpe1gb");
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
		  if (features2 & (1 << 2))
		    print (" dtes64");
		  if (features2 & (1 << 3))
		    print (" monitor");
		  if (features2 & (1 << 4))
		    print (" ds_cpl");
		  if (features2 & (1 << 5))
		    print (" vmx");
		  if (features2 & (1 << 6))
		    print (" smx");
		  if (features2 & (1 << 7))
		    print (" est");
		  if (features2 & (1 << 8))
		    print (" tm2");
		  if (features2 & (1 << 9))
		    print (" ssse3");
		  if (features2 & (1 << 10))
		    print (" cid");
		  if (features2 & (1 << 12))
		    print (" fma");
		}
	      if (features2 & (1 << 13))
		print (" cx16");
	      if (is_intel)
		{
		  if (features2 & (1 << 14))
		    print (" xtpr");
		  if (features2 & (1 << 15))
		    print (" pdcm");
		  if (features2 & (1 << 18))
		    print (" dca");
		  if (features2 & (1 << 19))
		    print (" sse4_1");
		  if (features2 & (1 << 20))
		    print (" sse4_2");
		  if (features2 & (1 << 21))
		    print (" x2apic");
		  if (features2 & (1 << 22))
		    print (" movbe");
		  if (features2 & (1 << 23))
		    print (" popcnt");
		  if (features2 & (1 << 25))
		    print (" aes");
		  if (features2 & (1 << 26))
		    print (" xsave");
		  if (features2 & (1 << 27))
		    print (" osxsave");
		  if (features2 & (1 << 28))
		    print (" avx");
		}

	      if (maxe >= 0x80000001)
		{
		  unsigned features;
		  cpuid (&unused, &unused, &features, &unused, 0x80000001);

		  if (features & (1 << 0))
		    print (" lahf_lm");
		  if (features & (1 << 1))
		    print (" cmp_legacy");
		  if (is_amd)
		    {
		      if (features & (1 << 2))
			print (" svm");
		      if (features & (1 << 3))
			print (" extapic");
		      if (features & (1 << 4))
			print (" cr8_legacy");
		      if (features & (1 << 5))
			print (" abm");
		      if (features & (1 << 6))
			print (" sse4a");
		      if (features & (1 << 7))
			print (" misalignsse");
		      if (features & (1 << 8))
			print (" 3dnowprefetch");
		      if (features & (1 << 9))
			print (" osvw");
		    }
		  if (features & (1 << 10))
		    print (" ibs");
		  if (is_amd)
		    {
		      if (features & (1 << 11))
			print (" sse5");
		      if (features & (1 << 12))
			print (" skinit");
		      if (features & (1 << 13))
			print (" wdt");
		    }
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
		  if (features2 & (1 << 6))
		    print (" 100mhzsteps");
		  if (features2 & (1 << 7))
		    print (" hwpstate");
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

  destbuf = (char *) crealloc_abort (destbuf, bufptr - buf);
  memcpy (destbuf, buf, bufptr - buf);
  return bufptr - buf;
}

#undef read_value

static _off64_t
format_proc_partitions (void *, char *&destbuf)
{
  char devname[NAME_MAX + 1];
  OBJECT_ATTRIBUTES attr;
  HANDLE dirhdl, devhdl;
  IO_STATUS_BLOCK io;
  NTSTATUS status;
  tmp_pathbuf tp;

  char *buf = tp.c_get ();
  char *bufptr = buf;

  /* Open \Device object directory. */
  wchar_t wpath[MAX_PATH] = L"\\Device";
  UNICODE_STRING upath = {14, 16, wpath};
  InitializeObjectAttributes (&attr, &upath, OBJ_CASE_INSENSITIVE, NULL, NULL);
  status = NtOpenDirectoryObject (&dirhdl, DIRECTORY_QUERY, &attr);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("NtOpenDirectoryObject, status %p", status);
      return 0;
    }

  print ("major minor  #blocks  name\n\n");
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
	      debug_printf ("NtOpenFile(%s), status %p", devname, status);
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

  destbuf = (char *) crealloc_abort (destbuf, bufptr - buf);
  memcpy (destbuf, buf, bufptr - buf);
  return bufptr - buf;
}

static _off64_t
format_proc_self (void *, char *&destbuf)
{
  destbuf = (char *) crealloc_abort (destbuf, 16);
  return __small_sprintf (destbuf, "%d", getpid ());
}

static _off64_t
format_proc_mounts (void *, char *&destbuf)
{
  destbuf = (char *) crealloc_abort (destbuf, sizeof ("self/mounts"));
  return __small_sprintf (destbuf, "self/mounts");
}

#undef print

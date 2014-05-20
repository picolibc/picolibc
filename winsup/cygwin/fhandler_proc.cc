/* fhandler_proc.cc: fhandler for /proc virtual filesystem

   Copyright 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012,
   2013, 2014 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "miscfuncs.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
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
#include <sys/sysinfo.h>
#include "ntdll.h"
#include <winioctl.h>
#include <wchar.h>
#include <wctype.h>
#include "cpuid.h"
#include "mount.h"

#define _COMPILING_NEWLIB
#include <dirent.h>

static off_t format_proc_loadavg (void *, char *&);
static off_t format_proc_meminfo (void *, char *&);
static off_t format_proc_stat (void *, char *&);
static off_t format_proc_version (void *, char *&);
static off_t format_proc_uptime (void *, char *&);
static off_t format_proc_cpuinfo (void *, char *&);
static off_t format_proc_partitions (void *, char *&);
static off_t format_proc_self (void *, char *&);
static off_t format_proc_mounts (void *, char *&);
static off_t format_proc_filesystems (void *, char *&);
static off_t format_proc_swaps (void *, char *&);
static off_t format_proc_devices (void *, char *&);
static off_t format_proc_misc (void *, char *&);

/* names of objects in /proc */
static const virt_tab_t proc_tab[] = {
  { _VN ("."),		 FH_PROC,	virt_directory,	NULL },
  { _VN (".."),		 FH_PROC,	virt_directory,	NULL },
  { _VN ("cpuinfo"),	 FH_PROC,	virt_file,	format_proc_cpuinfo },
  { _VN ("devices"),	 FH_PROC,	virt_file,	format_proc_devices },
  { _VN ("filesystems"), FH_PROC,	virt_file,	format_proc_filesystems },
  { _VN ("loadavg"),	 FH_PROC,	virt_file,	format_proc_loadavg },
  { _VN ("meminfo"),	 FH_PROC,	virt_file,	format_proc_meminfo },
  { _VN ("misc"),	 FH_PROC,	virt_file,	format_proc_misc },
  { _VN ("mounts"),	 FH_PROC,	virt_symlink,	format_proc_mounts },
  { _VN ("net"),	 FH_PROCNET,	virt_directory,	NULL },
  { _VN ("partitions"),  FH_PROC,	virt_file,	format_proc_partitions },
  { _VN ("registry"),	 FH_REGISTRY,	virt_directory,	NULL  },
  { _VN ("registry32"),  FH_REGISTRY,	virt_directory,	NULL },
  { _VN ("registry64"),  FH_REGISTRY,	virt_directory,	NULL },
  { _VN ("self"),	 FH_PROC,	virt_symlink,	format_proc_self },
  { _VN ("stat"),	 FH_PROC,	virt_file,	format_proc_stat },
  { _VN ("swaps"),	 FH_PROC,	virt_file,	format_proc_swaps },
  { _VN ("sys"),	 FH_PROCSYS,	virt_directory,	NULL },
  { _VN ("sysvipc"),	 FH_PROCSYSVIPC,	virt_directory,	NULL },
  { _VN ("uptime"),	 FH_PROC,	virt_file,	format_proc_uptime },
  { _VN ("version"),	 FH_PROC,	virt_file,	format_proc_version },
  { NULL, 0,	   	 FH_NADA,	virt_none,	NULL }
};

#define PROC_DIR_COUNT 4

static const int PROC_LINK_COUNT = (sizeof (proc_tab) / sizeof (virt_tab_t)) - 1;

/* name of the /proc filesystem */
const char proc[] = "/proc";
const size_t proc_len = sizeof (proc) - 1;

/* bsearch compare function. */
static int
proc_tab_cmp (const void *key, const void *memb)
{
  int ret = strncmp (((virt_tab_t *) key)->name, ((virt_tab_t *) memb)->name,
		     ((virt_tab_t *) memb)->name_len);
  if (!ret && ((virt_tab_t *) key)->name[((virt_tab_t *) memb)->name_len] != '\0' && ((virt_tab_t *) key)->name[((virt_tab_t *) memb)->name_len] != '/')
    return 1;
  return ret;
}

/* Helper function to perform a binary search of the incoming pathname
   against the alpha-sorted virtual file table. */
virt_tab_t *
virt_tab_search (const char *path, bool prefix, const virt_tab_t *table,
		 size_t nelem)
{
  virt_tab_t key = { path, 0, FH_NADA, virt_none, NULL };
  virt_tab_t *entry = (virt_tab_t *) bsearch (&key, table, nelem,
					      sizeof (virt_tab_t),
					      proc_tab_cmp);
  if (entry && (path[entry->name_len] == '\0'
		|| (prefix && path[entry->name_len] == '/')))
    return entry;
  return NULL;
}

/* Auxillary function that returns the fhandler associated with the given
   path. */
fh_devices
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

  virt_tab_t *entry = virt_tab_search (path, true, proc_tab,
				       PROC_LINK_COUNT);
  if (entry)
    return entry->fhandler;

  int pid = atoi (path);
  pinfo p (pid);
  /* If p->pid != pid, then pid is actually the Windows PID for an execed
     Cygwin process, and the pinfo entry is the additional entry created
     at exec time.  We don't want to enable the user to access a process
     entry by using the Win32 PID, though. */
  if (p && p->pid == pid)
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
    return FH_NADA;
  else
    /* Return FH_PROC so that we can return EROFS if the user is trying to
       create a file. */
    return FH_PROC;
}

/* Returns 0 if path doesn't exist, >0 if path is a directory,
   -1 if path is a file, -2 if it's a symlink.  */
virtual_ftype_t
fhandler_proc::exists ()
{
  const char *path = get_name ();
  debug_printf ("exists (%s)", path);
  path += proc_len;
  if (*path == 0)
    return virt_rootdir;
  virt_tab_t *entry = virt_tab_search (path + 1, false, proc_tab,
				       PROC_LINK_COUNT);
  if (entry)
    {
      fileid = entry - proc_tab;
      return entry->type;
    }
  return virt_none;
}

fhandler_proc::fhandler_proc ():
  fhandler_virtual ()
{
}

int __reg2
fhandler_proc::fstat (struct stat *buf)
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
      virt_tab_t *entry = virt_tab_search (path + 1, false, proc_tab,
					   PROC_LINK_COUNT);
      if (entry)
	{
	  if (entry->type == virt_directory)
	    buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
	  else if (entry->type == virt_symlink)
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

DIR *
fhandler_proc::opendir (int fd)
{
  DIR *dir = fhandler_virtual::opendir (fd);
  if (dir && !(dir->__handle = (void *) new winpids ((DWORD) 0)))
    {
      free (dir);
      dir = NULL;
      set_errno (ENOMEM);
    }
  return dir;
}

int
fhandler_proc::closedir (DIR *dir)
{
  delete (winpids *) dir->__handle;
  return fhandler_virtual::closedir (dir);
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
      winpids &pids = *(winpids *) dir->__handle;
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

  syscall_printf ("%d = readdir(%p, %p) (%s)", res, dir, de, de->d_name);
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
  syscall_printf ("%d = fhandler_proc::open(%y, 0%o)", res, flags, mode);
  return res;
}

bool
fhandler_proc::fill_filebuf ()
{
  if (fileid < PROC_LINK_COUNT && proc_tab[fileid].format_func)
    {
      filesize = proc_tab[fileid].format_func (NULL, filebuf);
      if (filesize > 0)
	return true;
    }
  return false;
}

static off_t
format_proc_version (void *, char *&destbuf)
{
  tmp_pathbuf tp;
  char *buf = tp.c_get ();
  char *bufptr = buf;
  struct utsname uts_name;

  uname (&uts_name);
  bufptr += __small_sprintf (bufptr, "%s version %s (%s@%s) (%s) %s\n",
			  uts_name.sysname, uts_name.release, USERNAME, HOSTNAME,
			  GCC_VERSION, uts_name.version);

  destbuf = (char *) crealloc_abort (destbuf, bufptr - buf);
  memcpy (destbuf, buf, bufptr - buf);
  return bufptr - buf;
}

static off_t
format_proc_loadavg (void *, char *&destbuf)
{
  extern int get_process_state (DWORD dwProcessId);
  unsigned running = 0;
  winpids pids ((DWORD) 0);

  for (unsigned i = 0; i < pids.npids; i++)
    switch (get_process_state (i)) {
      case 'O':
      case 'R':
	running++;
	break;
    }

  destbuf = (char *) crealloc_abort (destbuf, 48);
  return __small_sprintf (destbuf, "%u.%02u %u.%02u %u.%02u %u/%u\n",
				    0, 0, 0, 0, 0, 0, running, pids.npids);
}

static off_t
format_proc_meminfo (void *, char *&destbuf)
{
  unsigned long long mem_total, mem_free, swap_total, swap_free;
  struct sysinfo info;

  sysinfo (&info);
  mem_total = (unsigned long long) info.totalram * info.mem_unit;
  mem_free = (unsigned long long) info.freeram * info.mem_unit;
  swap_total = (unsigned long long) info.totalswap * info.mem_unit;
  swap_free = (unsigned long long) info.freeswap * info.mem_unit;

  destbuf = (char *) crealloc_abort (destbuf, 512);
  return sprintf (destbuf, "MemTotal:     %10llu kB\n"
			   "MemFree:      %10llu kB\n"
			   "HighTotal:             0 kB\n"
			   "HighFree:              0 kB\n"
			   "LowTotal:     %10llu kB\n"
			   "LowFree:      %10llu kB\n"
			   "SwapTotal:    %10llu kB\n"
			   "SwapFree:     %10llu kB\n",
			   mem_total >> 10, mem_free >> 10,
			   mem_total >> 10, mem_free >> 10,
			   swap_total >> 10, swap_free >> 10);
}

static off_t
format_proc_uptime (void *, char *&destbuf)
{
  unsigned long long uptime = 0ULL, idle_time = 0ULL;
  NTSTATUS status;
  SYSTEM_TIMEOFDAY_INFORMATION stodi;
  /* Sizeof SYSTEM_PERFORMANCE_INFORMATION on 64 bit systems.  It
     appears to contain some trailing additional information from
     what I can tell after examining the content.
     FIXME: It would be nice if this could be verified somehow. */
  const size_t sizeof_spi = sizeof (SYSTEM_PERFORMANCE_INFORMATION) + 16;
  PSYSTEM_PERFORMANCE_INFORMATION spi = (PSYSTEM_PERFORMANCE_INFORMATION)
					alloca (sizeof_spi);

  status = NtQuerySystemInformation (SystemTimeOfDayInformation, &stodi,
				     sizeof stodi, NULL);
  if (NT_SUCCESS (status))
    uptime = (stodi.CurrentTime.QuadPart - stodi.BootTime.QuadPart) / 100000ULL;
  else
    debug_printf ("NtQuerySystemInformation(SystemTimeOfDayInformation), "
		  "status %y", status);

  if (NT_SUCCESS (NtQuerySystemInformation (SystemPerformanceInformation,
						 spi, sizeof_spi, NULL)))
    idle_time = (spi->IdleTime.QuadPart / wincap.cpu_count ())
		/ 100000ULL;

  destbuf = (char *) crealloc_abort (destbuf, 80);
  return __small_sprintf (destbuf, "%U.%02u %U.%02u\n",
			  uptime / 100, long (uptime % 100),
			  idle_time / 100, long (idle_time % 100));
}

static off_t
format_proc_stat (void *, char *&destbuf)
{
  unsigned long pages_in = 0UL, pages_out = 0UL, interrupt_count = 0UL,
		context_switches = 0UL, swap_in = 0UL, swap_out = 0UL;
  time_t boot_time = 0;
  NTSTATUS status;
  /* Sizeof SYSTEM_PERFORMANCE_INFORMATION on 64 bit systems.  It
     appears to contain some trailing additional information from
     what I can tell after examining the content.
     FIXME: It would be nice if this could be verified somehow. */
  const size_t sizeof_spi = sizeof (SYSTEM_PERFORMANCE_INFORMATION) + 16;
  PSYSTEM_PERFORMANCE_INFORMATION spi = (PSYSTEM_PERFORMANCE_INFORMATION)
					alloca (sizeof_spi);
  SYSTEM_TIMEOFDAY_INFORMATION stodi;
  tmp_pathbuf tp;

  char *buf = tp.c_get ();
  char *eobuf = buf;

  SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION spt[wincap.cpu_count ()];
  status = NtQuerySystemInformation (SystemProcessorPerformanceInformation,
				     (PVOID) spt,
				     sizeof spt[0] * wincap.cpu_count (), NULL);
  if (!NT_SUCCESS (status))
    debug_printf ("NtQuerySystemInformation(SystemProcessorPerformanceInformation), "
		  "status %y", status);
  else
    {
      unsigned long long user_time = 0ULL, kernel_time = 0ULL, idle_time = 0ULL;
      for (unsigned long i = 0; i < wincap.cpu_count (); i++)
	{
	  kernel_time += (spt[i].KernelTime.QuadPart - spt[i].IdleTime.QuadPart)
			 * HZ / 10000000ULL;
	  user_time += spt[i].UserTime.QuadPart * HZ / 10000000ULL;
	  idle_time += spt[i].IdleTime.QuadPart * HZ / 10000000ULL;
	}

      eobuf += __small_sprintf (eobuf, "cpu %U %U %U %U\n",
				user_time, 0ULL, kernel_time, idle_time);
      user_time = 0ULL, kernel_time = 0ULL, idle_time = 0ULL;
      for (unsigned long i = 0; i < wincap.cpu_count (); i++)
	{
	  interrupt_count += spt[i].InterruptCount;
	  kernel_time = (spt[i].KernelTime.QuadPart - spt[i].IdleTime.QuadPart) * HZ / 10000000ULL;
	  user_time = spt[i].UserTime.QuadPart * HZ / 10000000ULL;
	  idle_time = spt[i].IdleTime.QuadPart * HZ / 10000000ULL;
	  eobuf += __small_sprintf (eobuf, "cpu%d %U %U %U %U\n", i,
				    user_time, 0ULL, kernel_time, idle_time);
	}

      status = NtQuerySystemInformation (SystemPerformanceInformation,
					 (PVOID) spi, sizeof_spi, NULL);
      if (!NT_SUCCESS (status))
	{
	  debug_printf ("NtQuerySystemInformation(SystemPerformanceInformation)"
			", status %y", status);
	  memset (spi, 0, sizeof_spi);
	}
      status = NtQuerySystemInformation (SystemTimeOfDayInformation,
					 (PVOID) &stodi, sizeof stodi, NULL);
      if (!NT_SUCCESS (status))
	debug_printf ("NtQuerySystemInformation(SystemTimeOfDayInformation), "
		      "status %y", status);
    }
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return 0;
    }

  pages_in = spi->PagesRead;
  pages_out = spi->PagefilePagesWritten + spi->MappedFilePagesWritten;
  /* Note: there is no distinction made in this structure between pages read
     from the page file and pages read from mapped files, but there is such
     a distinction made when it comes to writing.  Goodness knows why.  The
     value of swap_in, then, will obviously be wrong but its our best guess. */
  swap_in = spi->PagesRead;
  swap_out = spi->PagefilePagesWritten;
  context_switches = spi->ContextSwitches;
  boot_time = to_time_t (&stodi.BootTime);

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

#define print(x) { bufptr = stpcpy (bufptr, (x)); }

static off_t
format_proc_cpuinfo (void *, char *&destbuf)
{
  DWORD orig_affinity_mask;
  int cpu_number;
  const int BUFSIZE = 256;
  union
  {
    BYTE b[BUFSIZE];
    char s[BUFSIZE];
    WCHAR w[BUFSIZE / sizeof (WCHAR)];
    DWORD d;
    unsigned m[13];
  } in_buf;
  tmp_pathbuf tp;

  char *buf = tp.c_get ();
  char *bufptr = buf;

  for (cpu_number = 0; ; cpu_number++)
    {
      WCHAR cpu_key[128];
      __small_swprintf (cpu_key, L"\\Registry\\Machine\\HARDWARE\\DESCRIPTION"
				  "\\System\\CentralProcessor\\%d", cpu_number);
      if (!NT_SUCCESS (RtlCheckRegistryKey (RTL_REGISTRY_ABSOLUTE, cpu_key)))
	break;
      if (cpu_number)
	print ("\n");

      orig_affinity_mask = SetThreadAffinityMask (GetCurrentThread (),
						  1 << cpu_number);
      if (orig_affinity_mask == 0)
	debug_printf ("SetThreadAffinityMask failed %E");
      /* I'm not sure whether the thread changes processor immediately
	 and I'm not sure whether this function will cause the thread
	 to be rescheduled */
      yield ();

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
	  WCHAR vendor[64], id[64];
	  UNICODE_STRING uvendor, uid;
	  RtlInitEmptyUnicodeString (&uvendor, vendor, sizeof (vendor));
	  RtlInitEmptyUnicodeString (&uid, id, sizeof (id));
	  DWORD cpu_mhz = 0;
	  RTL_QUERY_REGISTRY_TABLE tab[4] = {
	   { NULL, RTL_QUERY_REGISTRY_NOEXPAND | RTL_QUERY_REGISTRY_DIRECT,
	     L"VendorIdentifier", &uvendor, REG_NONE, NULL, 0 },
	   { NULL, RTL_QUERY_REGISTRY_NOEXPAND | RTL_QUERY_REGISTRY_DIRECT,
	     L"Identifier", &uid, REG_NONE, NULL, 0 },
	   { NULL, RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_NOSTRING,
	     L"~Mhz", &cpu_mhz, REG_NONE, NULL, 0 },
	   { NULL, 0, NULL, NULL, 0, NULL, 0 }
	  };

	  RtlQueryRegistryValues (RTL_REGISTRY_ABSOLUTE, cpu_key, tab,
				  NULL, NULL);
	  bufptr += __small_sprintf (bufptr,
				     "processor       : %d\n"
				     "vendor_id       : %S\n"
				     "identifier      : %S\n"
				     "cpu MHz         : %u\n",
				     cpu_number, &uvendor, &uid, cpu_mhz);
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
	  DWORD cpu_mhz = 0;
	  RTL_QUERY_REGISTRY_TABLE tab[2] = {
	    { NULL, RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_NOSTRING,
	      L"~Mhz", &cpu_mhz, REG_NONE, NULL, 0 },
	    { NULL, 0, NULL, NULL, 0, NULL, 0 }
	  };

	  RtlQueryRegistryValues (RTL_REGISTRY_ABSOLUTE, cpu_key, tab,
				  NULL, NULL);
	  bufptr += __small_sprintf (bufptr, "processor\t: %d\n", cpu_number);
	  unsigned maxf, vendor_id[4], unused;
	  cpuid (&maxf, &vendor_id[0], &vendor_id[2], &vendor_id[1], 0);
	  maxf &= 0xffff;
	  vendor_id[3] = 0;

	  /* Vendor identification. */
	  bool is_amd = false, is_intel = false;
	  if (!strcmp ((char*)vendor_id, "AuthenticAMD"))
	    is_amd = true;
	  else if (!strcmp ((char*)vendor_id, "GenuineIntel"))
	    is_intel = true;

	  bufptr += __small_sprintf (bufptr, "vendor_id\t: %s\n",
				     (char *)vendor_id);
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
		  /* Could implement a lookup table here if someone needs it. */
		  strcpy (in_buf.s, "unknown");
		}
	      int cache_size = -1,
		  tlb_size = -1,
		  clflush = 64,
		  cache_alignment = 64;
	      if (features1 & (1 << 19)) /* CLFSH */
		clflush = ((extra_info >> 8) & 0xff) << 3;
	      if (is_intel && family == 15)
		cache_alignment = clflush * 2;
	      if (maxe >= 0x80000005) /* L1 Cache and TLB Identifiers. */
		{
		  unsigned data_cache, inst_cache;
		  cpuid (&unused, &unused, &data_cache, &inst_cache,
			 0x80000005);

		  cache_size = (inst_cache >> 24) + (data_cache >> 24);
		  tlb_size = 0;
		}
	      if (maxe >= 0x80000006) /* L2 Cache and L2 TLB Identifiers. */
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

	      /* Recognize multi-core CPUs. */
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
	      /* Recognize Intel Hyper-Transport CPUs. */
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
		  if (features & (1 << 19)) /* Huh?  Not in AMD64 specs. */
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
		  if (features & (1 << 30)) /* 31th bit is on. */
		    print (" 3dnowext");
		  if (features & (1 << 31)) /* 32th bit (highest) is on. */
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

	      if (maxe >= 0x80000008) /* Address size. */
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

	      if (maxe >= 0x80000007) /* Advanced power management. */
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
      if (orig_affinity_mask != 0)
	SetThreadAffinityMask (GetCurrentThread (), orig_affinity_mask);
      print ("\n");
    }

  destbuf = (char *) crealloc_abort (destbuf, bufptr - buf);
  memcpy (destbuf, buf, bufptr - buf);
  return bufptr - buf;
}

static off_t
format_proc_partitions (void *, char *&destbuf)
{
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  NTSTATUS status;
  HANDLE dirhdl;
  tmp_pathbuf tp;

  char *buf = tp.c_get ();
  char *bufptr = buf;
  char *ioctl_buf = tp.c_get ();

  /* Open \Device object directory. */
  wchar_t wpath[MAX_PATH] = L"\\Device";
  UNICODE_STRING upath = {14, 16, wpath};
  InitializeObjectAttributes (&attr, &upath, OBJ_CASE_INSENSITIVE, NULL, NULL);
  status = NtOpenDirectoryObject (&dirhdl, DIRECTORY_QUERY, &attr);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("NtOpenDirectoryObject, status %y", status);
      __seterrno_from_nt_status (status);
      return 0;
    }

  /* Traverse \Device directory ... */
  PDIRECTORY_BASIC_INFORMATION dbi = (PDIRECTORY_BASIC_INFORMATION)
				     alloca (640);
  BOOLEAN restart = TRUE;
  bool got_one = false;
  ULONG context = 0;
  while (NT_SUCCESS (NtQueryDirectoryObject (dirhdl, dbi, 640, TRUE, restart,
					     &context, NULL)))
    {
      HANDLE devhdl;
      PARTITION_INFORMATION_EX *pix = NULL;
      PARTITION_INFORMATION *pi = NULL;
      DWORD bytes_read;
      DWORD part_cnt = 0;
      unsigned long long size;
      device dev;

      restart = FALSE;
      /* ... and check for a "Harddisk[0-9]*" entry. */
      if (dbi->ObjectName.Length < 9 * sizeof (WCHAR)
	  || wcsncasecmp (dbi->ObjectName.Buffer, L"Harddisk", 8) != 0
	  || !iswdigit (dbi->ObjectName.Buffer[8]))
	continue;
      /* Got it.  Now construct the path to the entire disk, which is
	 "\\Device\\HarddiskX\\Partition0", and open the disk with
	 minimum permssions. */
      unsigned long drive_num = wcstoul (dbi->ObjectName.Buffer + 8, NULL, 10);
      wcscpy (wpath, dbi->ObjectName.Buffer);
      PWCHAR wpart = wpath + dbi->ObjectName.Length / sizeof (WCHAR);
      __small_swprintf (wpart, L"\\Partition0");
      upath.Length = dbi->ObjectName.Length
		     + wcslen (wpart) * sizeof (WCHAR);
      upath.MaximumLength = upath.Length + sizeof (WCHAR);
      InitializeObjectAttributes (&attr, &upath, OBJ_CASE_INSENSITIVE,
				  dirhdl, NULL);
      status = NtOpenFile (&devhdl, READ_CONTROL, &attr, &io,
			   FILE_SHARE_VALID_FLAGS, 0);
      if (!NT_SUCCESS (status))
	{
	  debug_printf ("NtOpenFile(%S), status %y", &upath, status);
	  __seterrno_from_nt_status (status);
	  continue;
	}
      if (!got_one)
	{
	  print ("major minor  #blocks  name\n\n");
	  got_one = true;
	}
      /* Fetch partition info for the entire disk to get its size. */
      if (DeviceIoControl (devhdl, IOCTL_DISK_GET_PARTITION_INFO_EX, NULL, 0,
			   ioctl_buf, NT_MAX_PATH, &bytes_read, NULL))
	{
	  pix = (PARTITION_INFORMATION_EX *) ioctl_buf;
	  size = pix->PartitionLength.QuadPart;
	}
      else if (DeviceIoControl (devhdl, IOCTL_DISK_GET_PARTITION_INFO, NULL, 0,
				ioctl_buf, NT_MAX_PATH, &bytes_read, NULL))
	{
	  pi = (PARTITION_INFORMATION *) ioctl_buf;
	  size = pi->PartitionLength.QuadPart;
	}
      else
	{
	  debug_printf ("DeviceIoControl (%S, "
			 "IOCTL_DISK_GET_PARTITION_INFO{_EX}) %E", &upath);
	  size = 0;
	}
      dev.parsedisk (drive_num, 0);
      bufptr += __small_sprintf (bufptr, "%5d %5d %9U %s\n",
				 dev.get_major (), dev.get_minor (),
				 size >> 10, dev.name + 5);
      /* Fetch drive layout info to get size of all partitions on the disk. */
      if (DeviceIoControl (devhdl, IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
			   NULL, 0, ioctl_buf, NT_MAX_PATH, &bytes_read, NULL))
	{
	  PDRIVE_LAYOUT_INFORMATION_EX pdlix = (PDRIVE_LAYOUT_INFORMATION_EX)
					       ioctl_buf;
	  part_cnt = pdlix->PartitionCount;
	  pix = pdlix->PartitionEntry;
	}
      else if (DeviceIoControl (devhdl, IOCTL_DISK_GET_DRIVE_LAYOUT,
				NULL, 0, ioctl_buf, NT_MAX_PATH, &bytes_read, NULL))
	{
	  PDRIVE_LAYOUT_INFORMATION pdli = (PDRIVE_LAYOUT_INFORMATION) ioctl_buf;
	  part_cnt = pdli->PartitionCount;
	  pi = pdli->PartitionEntry;
	}
      else
	debug_printf ("DeviceIoControl(%S, "
		      "IOCTL_DISK_GET_DRIVE_LAYOUT{_EX}): %E", &upath);
      /* Loop over partitions. */
      if (pix || pi)
	for (DWORD i = 0; i < part_cnt; ++i)
	  {
	    DWORD part_num;

	    if (pix)
	      {
		size = pix->PartitionLength.QuadPart;
		part_num = pix->PartitionNumber;
		++pix;
	      }
	    else
	      {
		size = pi->PartitionLength.QuadPart;
		part_num = pi->PartitionNumber;
		++pi;
	      }
	    /* A partition number of 0 denotes an extended partition or a
	       filler entry as described in fhandler_dev_floppy::lock_partition.
	       Just skip. */
	    if (part_num == 0)
	      continue;
	    dev.parsedisk (drive_num, part_num);
	    bufptr += __small_sprintf (bufptr, "%5d %5d %9U %s\n",
				       dev.get_major (), dev.get_minor (),
				       size >> 10, dev.name + 5);
	  }
      NtClose (devhdl);
    }
  NtClose (dirhdl);

  if (!got_one)
    return 0;

  destbuf = (char *) crealloc_abort (destbuf, bufptr - buf);
  memcpy (destbuf, buf, bufptr - buf);
  return bufptr - buf;
}

static off_t
format_proc_self (void *, char *&destbuf)
{
  destbuf = (char *) crealloc_abort (destbuf, 16);
  return __small_sprintf (destbuf, "%d", getpid ());
}

static off_t
format_proc_mounts (void *, char *&destbuf)
{
  destbuf = (char *) crealloc_abort (destbuf, sizeof ("self/mounts"));
  return __small_sprintf (destbuf, "self/mounts");
}

static off_t
format_proc_filesystems (void *, char *&destbuf)
{
  tmp_pathbuf tp;
  char *buf = tp.c_get ();
  char *bufptr = buf;

  /* start at 1 to skip type "none" */
  for (int i = 1; fs_names[i].name; i++)
    bufptr += __small_sprintf(bufptr, "%s\t%s\n",
			      fs_names[i].block_device ? "" : "nodev",
			      fs_names[i].name);

  destbuf = (char *) crealloc_abort (destbuf, bufptr - buf);
  memcpy (destbuf, buf, bufptr - buf);
  return bufptr - buf;
}

static off_t
format_proc_swaps (void *, char *&destbuf)
{
  unsigned long long total = 0ULL, used = 0ULL;
  PSYSTEM_PAGEFILE_INFORMATION spi = NULL;
  ULONG size = 512;
  NTSTATUS status = STATUS_SUCCESS;

  tmp_pathbuf tp;
  char *buf = tp.c_get ();
  char *bufptr = buf;

  spi = (PSYSTEM_PAGEFILE_INFORMATION) malloc (size);
  if (spi)
    {
      status = NtQuerySystemInformation (SystemPagefileInformation, (PVOID) spi,
					 size, &size);
      if (status == STATUS_INFO_LENGTH_MISMATCH)
	{
	  free (spi);
	  spi = (PSYSTEM_PAGEFILE_INFORMATION) malloc (size);
	  if (spi)
	    status = NtQuerySystemInformation (SystemPagefileInformation,
					       (PVOID) spi, size, &size);
	}
    }

  bufptr += __small_sprintf (bufptr,
			     "Filename\t\t\t\tType\t\tSize\tUsed\tPriority\n");

  if (spi && NT_SUCCESS (status))
    {
      PSYSTEM_PAGEFILE_INFORMATION spp = spi;
      char *filename = tp.c_get ();
      do
	{
	  total = (unsigned long long) spp->CurrentSize * wincap.page_size ();
	  used = (unsigned long long) spp->TotalUsed * wincap.page_size ();
	  cygwin_conv_path (CCP_WIN_W_TO_POSIX, spp->FileName.Buffer,
			    filename, NT_MAX_PATH);
	  bufptr += sprintf (bufptr, "%-40s%-16s%-8llu%-8llu%-8d\n",
			     filename, "file", total >> 10, used >> 10, 0);
	}
      while (spp->NextEntryOffset
	     && (spp = (PSYSTEM_PAGEFILE_INFORMATION)
		       ((char *) spp + spp->NextEntryOffset)));
    }

  if (spi)
    free (spi);

  destbuf = (char *) crealloc_abort (destbuf, bufptr - buf);
  memcpy (destbuf, buf, bufptr - buf);
  return bufptr - buf;
}

static off_t
format_proc_devices (void *, char *&destbuf)
{
  tmp_pathbuf tp;
  char *buf = tp.c_get ();
  char *bufptr = buf;

  bufptr += __small_sprintf (bufptr,
			     "Character devices:\n"
			     "%3d mem\n"
			     "%3d cons\n"
			     "%3d /dev/tty\n"
			     "%3d /dev/console\n"
			     "%3d /dev/ptmx\n"
			     "%3d st\n"
			     "%3d misc\n"
			     "%3d sound\n"
			     "%3d ttyS\n"
			     "%3d tty\n"
			     "\n"
			     "Block devices:\n"
			     "%3d fd\n"
			     "%3d sd\n"
			     "%3d sr\n"
			     "%3d sd\n"
			     "%3d sd\n"
			     "%3d sd\n"
			     "%3d sd\n"
			     "%3d sd\n"
			     "%3d sd\n"
			     "%3d sd\n",
			     DEV_MEM_MAJOR, DEV_CONS_MAJOR, _major (FH_TTY),
			     _major (FH_CONSOLE), _major (FH_PTMX),
			     DEV_TAPE_MAJOR, DEV_MISC_MAJOR, DEV_SOUND_MAJOR,
			     DEV_SERIAL_MAJOR, DEV_PTYS_MAJOR, DEV_FLOPPY_MAJOR,
			     DEV_SD_MAJOR, DEV_CDROM_MAJOR, DEV_SD1_MAJOR,
			     DEV_SD2_MAJOR, DEV_SD3_MAJOR, DEV_SD4_MAJOR,
			     DEV_SD5_MAJOR, DEV_SD6_MAJOR, DEV_SD7_MAJOR);

  destbuf = (char *) crealloc_abort (destbuf, bufptr - buf);
  memcpy (destbuf, buf, bufptr - buf);
  return bufptr - buf;
}

static off_t
format_proc_misc (void *, char *&destbuf)
{
  tmp_pathbuf tp;
  char *buf = tp.c_get ();
  char *bufptr = buf;

  bufptr += __small_sprintf (bufptr,
			     "%3d clipboard\n"
			     "%3d windows\n",
			     _minor (FH_CLIPBOARD), _minor (FH_WINDOWS));

  destbuf = (char *) crealloc_abort (destbuf, bufptr - buf);
  memcpy (destbuf, buf, bufptr - buf);
  return bufptr - buf;
}

#undef print

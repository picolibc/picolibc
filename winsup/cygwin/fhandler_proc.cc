/* fhandler_proc.cc: fhandler for /proc virtual filesystem

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
#include "shared_info.h"
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
#include <math.h>

#define _LIBC
#include <dirent.h>

static off_t format_proc_loadavg (void *, char *&);
static off_t format_proc_meminfo (void *, char *&);
static off_t format_proc_stat (void *, char *&);
static off_t format_proc_version (void *, char *&);
static off_t format_proc_uptime (void *, char *&);
static off_t format_proc_cpuinfo (void *, char *&);
static off_t format_proc_partitions (void *, char *&);
static off_t format_proc_self (void *, char *&);
static off_t format_proc_cygdrive (void *, char *&);
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
  { _VN ("cygdrive"),	 FH_PROC,	virt_symlink,	format_proc_cygdrive },
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
  { NULL, 0,		 FH_NADA,	virt_none,	NULL }
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

  char *e;
  pid_t pid = strtoul (path, &e, 10);
  if (*e != '/' && *e != '\0')
    return FH_NADA;
  pinfo p (pid);
  /* If p->pid != pid, then pid is actually the Windows PID for an execed
     Cygwin process, and the pinfo entry is the additional entry created
     at exec time.  We don't want to enable the user to access a process
     entry by using the Win32 PID, though. */
  if (p && p->pid == pid)
    {
      /* Check for entry in fd subdir */
      if (!strncmp (++e, "fd/", 3) && e[3] != '\0')
	return FH_PROCESSFD;
      return FH_PROCESS;
    }

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
      strcpy (de->d_name, proc_tab[dir->__d_position].name);
      de->d_type = virt_ftype_to_dtype (proc_tab[dir->__d_position].type);
      dir->__d_position++;
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
	    de->d_type = DT_DIR;
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
	  diropen = true;
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
		diropen = true;
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

extern "C" int uname_x (struct utsname *);

static off_t
format_proc_version (void *, char *&destbuf)
{
  tmp_pathbuf tp;
  char *buf = tp.c_get ();
  char *bufptr = buf;
  struct utsname uts_name;

  uname_x (&uts_name);
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
  unsigned int running = 0;
  winpids pids ((DWORD) 0);

  for (unsigned i = 0; i < pids.npids; i++)
    switch (get_process_state (i)) {
      case 'O':
      case 'R':
	running++;
	break;
    }

  double loadavg[3] = { 0.0, 0.0, 0.0 };
  getloadavg (loadavg, 3);

#define HUNDRETHS(l) (int)((l - floor(l))*100)

  destbuf = (char *) crealloc_abort (destbuf, 48);
  return __small_sprintf (destbuf, "%u.%02u %u.%02u %u.%02u %u/%u\n",
			  (int)loadavg[0], HUNDRETHS(loadavg[0]),
			  (int)loadavg[1], HUNDRETHS(loadavg[1]),
			  (int)loadavg[2], HUNDRETHS(loadavg[2]),
			  running, (unsigned int)pids.npids);
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
			 * CLOCKS_PER_SEC / NS100PERSEC;
	  user_time += spt[i].UserTime.QuadPart * CLOCKS_PER_SEC / NS100PERSEC;
	  idle_time += spt[i].IdleTime.QuadPart * CLOCKS_PER_SEC / NS100PERSEC;
	}

      eobuf += __small_sprintf (eobuf, "cpu %U %U %U %U\n",
				user_time, 0ULL, kernel_time, idle_time);
      user_time = 0ULL, kernel_time = 0ULL, idle_time = 0ULL;
      for (unsigned long i = 0; i < wincap.cpu_count (); i++)
	{
	  interrupt_count += spt[i].InterruptCount;
	  kernel_time = (spt[i].KernelTime.QuadPart - spt[i].IdleTime.QuadPart)
			* CLOCKS_PER_SEC / NS100PERSEC;
	  user_time = spt[i].UserTime.QuadPart * CLOCKS_PER_SEC / NS100PERSEC;
	  idle_time = spt[i].IdleTime.QuadPart * CLOCKS_PER_SEC / NS100PERSEC;
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
/* feature test unconditional print */
#define ftuprint(msg) print (" " msg)
/* feature test bit position (0-32) and conditional print */
#define ftcprint(feat,bitno,msg) if ((feat) & (1 << (bitno))) { ftuprint (msg); }

static inline uint32_t
get_msb (uint32_t in)
{
  return 32 - __builtin_clz (in);
}

static inline uint32_t
mask_bits (uint32_t in)
{
  uint32_t bits = get_msb (in) - 1;
  if (in & (in - 1))
    ++bits;
  return bits;
}

static off_t
format_proc_cpuinfo (void *, char *&destbuf)
{
  WCHAR cpu_key[128], *cpu_num_p;
  DWORD orig_affinity_mask = 0;
  GROUP_AFFINITY orig_group_affinity;
  int cpu_number;
  const int BUFSIZE = 256;
  union
  {
    BYTE b[BUFSIZE];
    char s[BUFSIZE];
    WCHAR w[BUFSIZE / sizeof (WCHAR)];
    DWORD d;
    uint32_t m[13];
  } in_buf;
  tmp_pathbuf tp;

  char *buf = tp.c_get ();
  char *bufptr = buf;

  //WORD num_cpu_groups = 1;	/* Pre Windows 7, only one group... */
  WORD num_cpu_per_group = __get_cpus_per_group ();

  cpu_num_p = wcpcpy (cpu_key, L"\\Registry\\Machine\\HARDWARE\\DESCRIPTION"
				"\\System\\CentralProcessor\\");
  for (cpu_number = 0; ; cpu_number++)
    {
      __small_swprintf (cpu_num_p, L"%d", cpu_number);
      if (!NT_SUCCESS (RtlCheckRegistryKey (RTL_REGISTRY_ABSOLUTE, cpu_key)))
	break;
      if (cpu_number)
	print ("\n");

      WORD cpu_group = cpu_number / num_cpu_per_group;
      KAFFINITY cpu_mask = 1L << (cpu_number % num_cpu_per_group);
      GROUP_AFFINITY affinity = {
	.Mask	= cpu_mask,
	.Group	= cpu_group,
      };

      if (!SetThreadGroupAffinity (GetCurrentThread (), &affinity,
				   &orig_group_affinity))
	system_printf ("SetThreadGroupAffinity(%x,%d (%x/%d)) failed %E", cpu_mask, cpu_group, cpu_number, cpu_number);
      orig_affinity_mask = 1; /* Just mark success. */
      /* I'm not sure whether the thread changes processor immediately
	 and I'm not sure whether this function will cause the thread
	 to be rescheduled */
      yield ();

      DWORD cpu_mhz = 0;
      union
        {
	  LONG uc_len;		/* -max size of buffer before call */
	  char uc_microcode[16];	/* at least 8 bytes */
        } uc[4];		/* microcode values changed historically */

      RTL_QUERY_REGISTRY_TABLE tab[6] =
        {
	  { NULL, RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_NOSTRING,
	    L"~Mhz",		       &cpu_mhz, REG_NONE, NULL, 0 },
	  { NULL, RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_NOSTRING,
	    L"Update Revision",		 &uc[0], REG_NONE, NULL, 0 },
							/* latest MSR */
	  { NULL, RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_NOSTRING,
	    L"Update Signature",	 &uc[1], REG_NONE, NULL, 0 },
							/* previous MSR */
	  { NULL, RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_NOSTRING,
	    L"CurrentPatchLevel",	 &uc[2], REG_NONE, NULL, 0 },
							/* earlier MSR */
	  { NULL, RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_NOSTRING,
	    L"Platform Specific Field1", &uc[3], REG_NONE, NULL, 0 },
							/* alternative */
	  { NULL, 0, NULL, NULL, 0, NULL, 0 }
        };

      for (size_t uci = 0; uci < sizeof (uc)/sizeof (*uc); ++uci)
	{
	  memset (&uc[uci], 0, sizeof (uc[uci]));
	  uc[uci].uc_len = -(LONG)sizeof (uc[0].uc_microcode);
							/* neg buffer size */
	}

      RtlQueryRegistryValues (RTL_REGISTRY_ABSOLUTE, cpu_key, tab,
			      NULL, NULL);
      cpu_mhz = ((cpu_mhz - 1) / 10 + 1) * 10;	/* round up to multiple of 10 */
      DWORD bogomips = cpu_mhz * 2; /* bogomips is double cpu MHz since MMX */

      unsigned long long microcode = 0;	/* needs 8 bytes */
      for (size_t uci = 0; uci < sizeof (uc)/sizeof (*uc) && !microcode; ++uci)
	{
	  /* still neg buffer size => no data */
	  if (-(LONG)sizeof (uc[uci].uc_microcode) != uc[uci].uc_len)
	    {
	      memcpy (&microcode, uc[uci].uc_microcode, sizeof (microcode));

	      if (!(microcode & 0xFFFFFFFFLL))	/* some values in high bits */
		  microcode >>= 32;		/* shift them down */
	    }
	}

      bufptr += __small_sprintf (bufptr, "processor\t: %d\n", cpu_number);
      uint32_t maxf, vendor_id[4], unused;

      cpuid (&maxf, &vendor_id[0], &vendor_id[2], &vendor_id[1], 0x00000000);
      maxf &= 0xffff;
      vendor_id[3] = 0;

      /* Vendor identification. */
      bool is_amd = false, is_intel = false;
      if (!strcmp ((char*)vendor_id, "AuthenticAMD")
          || !strcmp((char*)vendor_id, "HygonGenuine"))
	is_amd = true;
      else if (!strcmp ((char*)vendor_id, "GenuineIntel"))
	is_intel = true;

      bufptr += __small_sprintf (bufptr, "vendor_id\t: %s\n",
				 (char *)vendor_id);

      uint32_t features1, features2, extra_info, cpuid_sig;
      cpuid (&cpuid_sig, &extra_info, &features2, &features1, 0x00000001);
      uint32_t family		= (cpuid_sig  & 0x00000f00) >> 8,
	       model		= (cpuid_sig  & 0x000000f0) >> 4,
	       stepping		= cpuid_sig   & 0x0000000f,
	       apic_id		= (extra_info & 0xff000000) >> 24;
      if (family == 15)
	family += (cpuid_sig >> 20) & 0xff;
      if (family >= 6)
	model |= ((cpuid_sig >> 16) & 0x0f) << 4; /* ext model << 4 | model */

      uint32_t maxe = 0;
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
	  clflush = 64,
	  cache_alignment = 64;
      long (*get_cpu_cache) (int, uint32_t) = NULL;
      uint32_t max;
      if (features1 & (1 << 19)) /* CLFSH */
	clflush = ((extra_info >> 8) & 0xff) << 3;
      if (is_intel && family == 15)
	cache_alignment = clflush * 2;
      if (is_intel)
	{
	  extern long get_cpu_cache_intel (int sysc, uint32_t maxf);
	  get_cpu_cache = get_cpu_cache_intel;
	  max = maxf;	/* Intel uses normal cpuid levels */
	}
      else if (is_amd)
	{
	  extern long get_cpu_cache_amd (int sysc, uint32_t maxe);
	  get_cpu_cache = get_cpu_cache_amd;
	  max = maxe;	/* AMD uses extended cpuid levels */
	}
      if (get_cpu_cache)
	{
	  long cs;

	  cs = get_cpu_cache (_SC_LEVEL3_CACHE_SIZE, max);
	  if (cs <= 0)
	    cs = get_cpu_cache (_SC_LEVEL2_CACHE_SIZE, max);
	  if (cs <= 0)
	    {
	      cs = get_cpu_cache (_SC_LEVEL1_ICACHE_SIZE, max);
	      if (cs > 0)
		cache_size = cs;
	      cs = get_cpu_cache (_SC_LEVEL1_DCACHE_SIZE, max);
	      if (cs > 0)
		cache_size += cs;
	    }
	  else
	    cache_size = cs;
	  if (cache_size > 0)
	    cache_size >>= 10;
	}
      bufptr += __small_sprintf (bufptr, "cpu family\t: %d\n"
					 "model\t\t: %d\n"
					 "model name\t: %s\n"
					 "stepping\t: %d\n"
					 "microcode\t: 0x%X\n"
					 "cpu MHz\t\t: %d.000\n",
				 family,
				 model,
				 in_buf.s + strspn (in_buf.s, " \t"),
				 stepping,
				 microcode,
				 cpu_mhz);

      if (cache_size >= 0)
	bufptr += __small_sprintf (bufptr, "cache size\t: %d KB\n",
				   cache_size);

      /* Recognize multi-core CPUs. */
      if (features1 & (1 << 28)) /* HTT */
	{
	  uint32_t siblings = 0;
	  uint32_t cpu_cores = 0;
	  uint32_t phys_id = 0;
	  uint32_t core_id = 0;
	  uint32_t initial_apic_id = apic_id;

	  uint32_t logical_bits = 0;	/* # of logical core bits in apicid. */
	  uint32_t ht_bits = 0;		/* # of thread bits in apic_id. */

	  if (is_intel)
	    {
	      bool valid = false;
	      if (maxf >= 0x0000000b)	/* topoext supported? */
		{
		  uint32_t bits, logical, level, unused;

		  /* Threads */
		  cpuid (&bits, &logical, &level, &unused,
			 0x0000000b, 0x00);
		  /* Even if topoext is supposedly supported, it can return
		     "invalid". */
		  if (bits != 0 && ((level >> 8) & 0xff) == 1)
		    {
		      valid = true;
		      ht_bits = (bits & 0x1f);
		      siblings = (logical & 0xffff);
		      cpu_cores = siblings;
		      for (uint32_t idx = 1; ; ++idx)
			{
			  cpuid (&bits, &logical, &level, &initial_apic_id,
				 0x0000000b, idx);

			  uint32_t level_type = ((level >> 8) & 0xff);
			  if (level_type == 0)	/* Invalid */
			    break;
			  if (level_type == 2)	/* Core */
			    {
			      logical_bits = (bits & 0x1f);
			      siblings = (logical & 0xffff);
			      cpu_cores = siblings >> ht_bits;
			      break;
			    }
			}
		    }
		}
	      if (!valid && maxf >= 0x00000004)
		{
		  uint32_t apic_reserved;

		  cpuid (&apic_reserved, &unused, &unused, &unused,
			 0x00000004, 0x00);
		  if (apic_reserved & 0x1f)
		    {
		      valid = true;
		      cpu_cores = ((apic_reserved >> 26) & 0x3f) + 1;
		      siblings = (extra_info >> 16) & 0xff;
		      if (siblings <= 1) /* HT could be fused out */
			{
			  logical_bits = mask_bits (cpu_cores);
			  ht_bits = 0;
			}
		      else
			{
			  logical_bits = mask_bits (siblings);
			  ht_bits = mask_bits (siblings / cpu_cores);
			}
		    }
		}
	      if (!valid)	/* single core, multi thread */
		{
		  cpu_cores = 1;
		  siblings = (extra_info >> 16) & 0xff;
		  logical_bits = mask_bits (siblings);
		  ht_bits = logical_bits;
		}
	    }
	  else if (is_amd)
	    {
	      if (maxe >= 0x8000001e)
		{
		  uint32_t cus, core_info;

		  cpuid (&unused, &unused, &core_info, &unused, 0x80000008);
		  cpuid (&unused, &cus, &unused, &unused, 0x8000001e);
		  siblings = cpu_cores = (core_info & 0xff) + 1;
		  logical_bits = (core_info >> 12) & 0xf;
		  cus = ((cus >> 8) & 0x3) + 1;
		  ht_bits = mask_bits (cus);
		}
	      else if (maxe >= 0x80000008)
		{
		  uint32_t core_info;

		  cpuid (&unused, &unused, &core_info, &unused, 0x80000008);
		  siblings = (core_info & 0xff) + 1;
		  cpu_cores = siblings;
		  logical_bits = (core_info >> 12) & 0xf;
		  if (!logical_bits)
		    logical_bits = mask_bits (siblings);
		  ht_bits = 0;
		}
	      else
		{
		  siblings = (extra_info >> 16) & 0xff;
		  cpu_cores = siblings;
		  logical_bits = mask_bits (siblings);
		  ht_bits = 0;
		}
	    }
	  phys_id = initial_apic_id >> logical_bits;
	  core_id = (initial_apic_id & ((1 << logical_bits) - 1)) >> ht_bits;

	  bufptr += __small_sprintf (bufptr, "physical id\t: %d\n", phys_id);
	  if (siblings > 0)
	    bufptr += __small_sprintf (bufptr, "siblings\t: %u\n", siblings);
	  bufptr += __small_sprintf (bufptr, "core id\t\t: %d\n"
					     "cpu cores\t: %d\n",
				     core_id, cpu_cores);
	  if (features1 & (1 << 9))	/* apic */
	    bufptr += __small_sprintf (bufptr, "apicid\t\t: %d\n"
					       "initial apicid\t: %d\n",
				       apic_id, initial_apic_id);

	}

      /* level is number of non-zero leafs exc. sub-leafs */
      int level = maxf + 1 + (maxe & 0x7fffffff) + 1;

      for (uint32_t l = maxe; 0x80000001 < l; --l)
        {
	  uint32_t a, b, c, d;
	  cpuid (&a, &b, &c, &d, l);
	  if (!(a | b | c | d))	--level;
        }

      for (uint32_t l = maxf; 1 < l; --l)
        {
	  uint32_t a, b, c, d;
	  cpuid (&a, &b, &c, &d, l);
	  if (!(a | b | c | d))	--level;
        }

      bufptr += __small_sprintf (bufptr, "fpu\t\t: %s\n"
					 "fpu_exception\t: %s\n"
					 "cpuid level\t: %d\n"
					 "wp\t\t: yes\n",
				 (features1 & (1 << 0)) ? "yes" : "no",
				 (features1 & (1 << 0)) ? "yes" : "no",
				 level);
      print ("flags\t\t:");
      /* cpuid 0x00000001 edx */
      ftcprint (features1,  0, "fpu");	/* x87 floating point */
      ftcprint (features1,  1, "vme");  /* VM enhancements */
      ftcprint (features1,  2, "de");   /* debugging extensions */
      ftcprint (features1,  3, "pse");  /* page size extensions */
      ftcprint (features1,  4, "tsc");  /* rdtsc/p */
      ftcprint (features1,  5, "msr");  /* rd/wrmsr */
      ftcprint (features1,  6, "pae");  /* phy addr extensions */
      ftcprint (features1,  7, "mce");  /* Machine check exception */
      ftcprint (features1,  8, "cx8");  /* cmpxchg8b */
      ftcprint (features1,  9, "apic"); /* APIC enabled */
      ftcprint (features1, 11, "sep");  /* sysenter/sysexit */
      ftcprint (features1, 12, "mtrr"); /* memory type range registers */
      ftcprint (features1, 13, "pge");  /* page global extension */
      ftcprint (features1, 14, "mca");  /* machine check architecture */
      ftcprint (features1, 15, "cmov"); /* conditional move */
      ftcprint (features1, 16, "pat");  /* page attribute table */
      ftcprint (features1, 17, "pse36");/* 36 bit page size extensions */
      ftcprint (features1, 18, "pn");   /* processor serial number */
      ftcprint (features1, 19, "clflush"); /* clflush instruction */
      ftcprint (features1, 21, "dts");  /* debug store */
      ftcprint (features1, 22, "acpi"); /* ACPI via MSR */
      ftcprint (features1, 23, "mmx");  /* multimedia extensions */
      ftcprint (features1, 24, "fxsr"); /* fxsave/fxrstor */
      ftcprint (features1, 25, "sse");  /* xmm sse */
      ftcprint (features1, 26, "sse2"); /* xmm2 sse2 */
      ftcprint (features1, 27, "ss");   /* CPU self snoop */
      ftcprint (features1, 28, "ht");   /* hyper threading */
      ftcprint (features1, 29, "tm");   /* acc automatic clock control */
      ftcprint (features1, 30, "ia64"); /* IA 64 processor */
      ftcprint (features1, 31, "pbe");  /* pending break enable */

      /* AMD cpuid 0x80000001 edx */
      if (is_amd && maxe >= 0x80000001)
	{
	  cpuid (&unused, &unused, &unused, &features1, 0x80000001);

	  ftcprint (features1, 11, "syscall");	/* syscall/sysret */
	  ftcprint (features1, 19, "mp");       /* MP capable */
	  ftcprint (features1, 20, "nx");       /* no-execute protection */
	  ftcprint (features1, 22, "mmxext");   /* MMX extensions */
	  ftcprint (features1, 25, "fxsr_opt"); /* fxsave/fxrstor optims */
	  ftcprint (features1, 26, "pdpe1gb");  /* GB large pages */
	  ftcprint (features1, 27, "rdtscp");   /* rdtscp */
	  ftcprint (features1, 29, "lm");       /* long mode (x86-64 amd64) */
	  ftcprint (features1, 30, "3dnowext"); /* 3DNow extensions */
	  ftcprint (features1, 31, "3dnow");    /* 3DNow */
	}

      /* cpuid 0x80000007 edx */
      if (maxe >= 0x80000007)
	{
	  cpuid (&unused, &unused, &unused, &features1, 0x80000007);

	  ftcprint (features1,  8, "constant_tsc"); /* TSC ticks at a constant rate */
	}

/*	  ftcprint (features2,  9, "up");	 *//* SMP kernel running on UP N/A */

      /* cpuid 0x00000007 ebx */
      if (maxf >= 0x00000007)
	  cpuid (&unused, &features1, &unused, &unused, 0x00000007, 0);
      else
	  features1 = 0;
      /* cpuid 0x80000007 edx */
      if (maxe >= 0x80000007)
	  cpuid (&unused, &unused, &unused, &features2, 0x80000007);
      else
	  features2 = 0;
      uint32_t cr_den, cr_num, Hz;
      /* cpuid 0x00000015 eax ebx ecx clock ratios optional Hz */
      if (is_intel && maxf >= 0x00000015)
	  cpuid (&cr_den, &cr_num, &Hz, &unused, 0x00000015);
      else
	  cr_den = 0;
      /* TSC requires adjustment, nonstop, and clock ratio divider min */
      if ((features1 & (1 << 1)) &&	/* TSC adjustment MSR 0x3B */
	  (features2 & (1 << 8)) &&	/* nonstop C states */
		(cr_den > 1))		/* clock ratio denominator > min */
	  ftuprint ("art");	/* Always running timer (ART) */

      /* Intel cpuid 0x0000000a eax Arch Perf Mon */
      if (is_intel && maxf >= 0x0000000a)
	{
	  cpuid (&features2, &unused, &unused, &unused, 0x0000000a);

	  /* rev > 0 and # counters/cpu > 1 */
	  if ((features2 & 0xff) > 0 && (((features2 >> 8) & 0xff) > 1))
	    ftuprint ("arch_perfmon"); /* Intel Arch Perf Mon */
	}

/*	  ftcprint (features2, 12, "pebs");*//* MSR_IA32_MISC_ENABLE 12 Precise-Event Based Sampling */
/*	  ftcprint (features2, 13, "bts"); *//* MSR_IA32_MISC_ENABLE 11 Branch Trace Store */

      /* AMD cpuid 0x00000001 eax */
      if (is_amd && maxf >= 0x00000001)
	  cpuid(&features2, &unused, &unused, &unused, 0x00000001);
      else
	  features2 = 0;

/* Intel family 6 or AMD family >= x10 or ... */
/* ... or AMD cpuid 0x00000001 eax in [0x0f58,) or [0x0f48,0x0f50) */
      if ((is_intel && family == 6) ||
	  (is_amd && (family >= 0x10 ||
			(features2 >= 0x0f58 ||
			(features2 >= 0x0f48 && features2 < 0x0f50)))))
	  ftuprint ("rep_good");	/* REP microcode works well */

      /* cpuid 0x80000007 edx Advanced power management */
      if (maxe >= 0x80000007)
	{
	  cpuid (&unused, &unused, &unused, &features2, 0x80000007);

	  ftcprint (features2, 12, "acc_power"); /* accum power */
	}

#if __x86_64__
      ftuprint ("nopl");	/* NOPL (0F 1F) instructions */
#endif

/* cpuid 0x0000000b ecx[8:15] type */
#define BAD_TYPE    0
#define SMT_TYPE    1
#define CORE_TYPE   2
      /* cpuid 0x0000000b ebx ecx */
      if (maxf >= 0x0000000b)
        {
	  cpuid(&unused, &features1, &features2, &unused, 0x0000000b);

	/* check if SMT implemented */
	  if (features1 != 0 && (((features2 >> 8) & 0xff) == SMT_TYPE))
	      ftuprint ("xtopology");	/* CPU topology enum extensions */
	}

      /* AMD cpuid 0x80000007 edx */
      if (is_amd && maxe >= 0x80000007)
	{
	  cpuid (&unused, &unused, &unused, &features1, 0x80000007);

	  ftcprint (features1,  8, "tsc_reliable");	/* TSC constant rate */
	  ftcprint (features1,  8, "nonstop_tsc");	/* nonstop C states */
	}

      if (is_amd || is_intel)
	  ftuprint ("cpuid");		/* CPU has CPUID instruction */

      if (is_amd && family > 0x16)
	  ftuprint ("extd_apicid");	/* Extended APICID (8 bits) */

      /* AMD cpuid 0x8000001e ecx */
      if (is_amd && maxe >= 0x8000001e)
	{
	  cpuid (&unused, &unused, &features1, &unused, 0x8000001e);

	  if (((features1 >> 8) & 7) + 1 > 1)	/* nodes/socket */
	      ftuprint ("amd_dcm"); /* AMD multi-node processor */
	}

      /* cpuid 0x00000006 ecx */
      if (maxf >= 0x00000006)
	{
	  cpuid (&unused, &unused, &features1, &unused, 0x00000006);

	  ftcprint (features1,  0, "aperfmperf");   /* P state hw coord fb */
	}

      /* cpuid 0x80000007 edx Advanced power management */
      if (maxe >= 0x80000007)
	{
	  cpuid (&unused, &unused, &unused, &features2, 0x80000007);

	  ftcprint (features2, 14, "rapl"); /* runtime avg power limit */
	}

      /* Penwell, Cloverview, ... TSC doesn't sleep on S3 */
      if (is_intel && family == 6)
	switch (model)
	  {
	    case 0x27: /* Atom Saltwell Mid Penwell */
	    case 0x35: /* Atom Saltwell Tablet Cloverview */
	    case 0x4a: /* Atom Silvermont Mid Merriefield */
	    case 0x75: /* Atom Airmont NP Lightning Mountain */
	      ftuprint ("nonstop_tsc_s3"); /* TSC doesn't stop in S3 state */
	      break;
	  }

      /* cpuid 0x00000015 eax ebx ecx clock ratios optional Hz */
      if (is_intel && maxf >= 0x00000015)
	{
	  uint32_t cr_den, cr_num, Hz, kHz;
	  cpuid (&cr_den, &cr_num, &Hz, &unused, 0x00000015);
	  if (cr_num != 0 && cr_den != 0)
	    {
	      kHz = Hz / 1000;
	      /* Denverton don't report hz nor support cpuid 0x16 so set 25MHz */
	      if (kHz == 0 && model == 0x5F) /* Atom Goldmont D Denverton */
		  kHz = 25000;

	      /* cpuid TSC frequency is known */
	      if (kHz != 0)
		  ftuprint ("tsc_known_freq"); /* TSC has known frequency */
#if 0 /* keep for future and doc */
	      else /* kHz == 0 */
	      /* Skylake and Kabylake don't report clock so use CPU speed and ratio */
	      if (maxf >= 0x00000016)
		{
		  uint32_t mHz;
		  cpuid(&mHz, &unused, &unused, &unused, 0x00000016);
		  kHz = mHz * 1000 * cr_num / cr_den;
		}
#endif
	    }
	}

      /* cpuid 0x00000001 ecx */
      cpuid (&unused, &unused, &features2, &unused, 0x00000001);

      ftcprint (features2,  0, "pni");	    /* xmm3 sse3 */
      ftcprint (features2,  1, "pclmuldq"); /* pclmulqdq instruction */
      ftcprint (features2,  2, "dtes64");   /* 64-bit debug store */
      ftcprint (features2,  3, "monitor");  /* monitor/mwait support */
      ftcprint (features2,  4, "ds_cpl");   /* CPL-qual debug store */
      ftcprint (features2,  5, "vmx");      /* hardware virtualization */
      ftcprint (features2,  6, "smx");      /* safer mode extensions */
      ftcprint (features2,  7, "est");      /* enhanced speedstep */
      ftcprint (features2,  8, "tm2");      /* thermal monitor 2 */
      ftcprint (features2,  9, "ssse3");    /* supplemental sse3 */
      ftcprint (features2, 10, "cid");      /* context id */
      ftcprint (features2, 11, "sdbg");     /* silicon debug */
      ftcprint (features2, 12, "fma");      /* fused multiply add */
      ftcprint (features2, 13, "cx16");     /* cmpxchg16b instruction */
      ftcprint (features2, 14, "xtpr");     /* send task priority messages */
      ftcprint (features2, 15, "pdcm");     /* perf/debug capabilities MSR */
      ftcprint (features2, 17, "pcid");     /* process context identifiers */
      ftcprint (features2, 18, "dca");      /* direct cache access */
      ftcprint (features2, 19, "sse4_1");   /* xmm 4_1 sse 4.1 */
      ftcprint (features2, 20, "sse4_2");   /* xmm 4_2 sse 4.2 */
      ftcprint (features2, 21, "x2apic");   /* x2 APIC */
      ftcprint (features2, 22, "movbe");    /* movbe instruction */
      ftcprint (features2, 23, "popcnt");   /* popcnt instruction */
      ftcprint (features2, 24, "tsc_deadline_timer"); /* TSC deadline timer */
      ftcprint (features2, 25, "aes");	    /* AES instructions */
      ftcprint (features2, 26, "xsave");    /* xsave/xrstor/xsetbv/xgetbv */
/*    ftcprint (features2, 27, "osxsave");*//* "" XSAVE supported in OS */
      ftcprint (features2, 28, "avx");	    /* advanced vector extensions */
      ftcprint (features2, 29, "f16c");     /* 16 bit FP conversions */
      ftcprint (features2, 30, "rdrand");   /* RNG rdrand instruction */
      ftcprint (features2, 31, "hypervisor"); /* hypervisor guest */

      /* cpuid 0x80000001 ecx */
      if (maxe >= 0x80000001)
	{
	  cpuid (&unused, &unused, &features1, &unused, 0x80000001);

	  ftcprint (features1,  0, "lahf_lm");		/* l/sahf long mode */
	  ftcprint (features1,  1, "cmp_legacy");       /* HT not valid */
	  if (is_amd)
	    {
	      ftcprint (features1,  2, "svm");          /* secure VM */
	      ftcprint (features1,  3, "extapic");      /* ext APIC space */
	      ftcprint (features1,  4, "cr8_legacy");   /* CR8 32 bit mode */
	      ftcprint (features1,  5, "abm");          /* adv bit manip lzcnt */
	      ftcprint (features1,  6, "sse4a");        /* sse 4a */
	      ftcprint (features1,  7, "misalignsse");  /* misaligned SSE ok */
	      ftcprint (features1,  8, "3dnowprefetch"); /* 3DNow prefetch */
	      ftcprint (features1,  9, "osvw");         /* OS vis workaround */
	    }
	  ftcprint (features1, 10, "ibs");	/* instr based sampling */
	  if (is_amd)
	    {
	      ftcprint (features1, 11, "xop");		/* sse 5 extended AVX */
	      ftcprint (features1, 12, "skinit");       /* skinit/stgi */
	      ftcprint (features1, 13, "wdt");          /* watchdog timer */
	      ftcprint (features1, 15, "lwp");          /* light weight prof */
	      ftcprint (features1, 16, "fma4");         /* 4 operand MAC */
	      ftcprint (features1, 17, "tce");          /* translat cache ext */
	      ftcprint (features1, 19, "nodeid_msr");   /* nodeid MSR */
	      ftcprint (features1, 21, "tbm");          /* trailing bit manip */
	      ftcprint (features1, 22, "topoext");      /* topology ext */
	      ftcprint (features1, 23, "perfctr_core"); /* core perf ctr ext */
	      ftcprint (features1, 24, "perfctr_nb");   /* NB perf ctr ext */
	      ftcprint (features1, 26, "bpext");        /* data brkpt ext */
	      ftcprint (features1, 27, "ptsc");         /* perf timestamp ctr */
	      ftcprint (features1, 28, "perfctr_llc");  /* ll cache perf ctr */
	      ftcprint (features1, 29, "mwaitx");       /* monitor/mwaitx ext */
	    }
	}

	/* ring 3 monitor/mwait */
	if (is_intel && family == 6)
	    switch (model)
	      {
		case 0x57: /* Xeon Phi Knights Landing */
		case 0x85: /* Xeon Phi Knights Mill */
		  ftuprint ("ring3mwait"); /* Ring 3 MONITOR/MWAIT instructions */
		  break;
	      }

/*	  ftcprint (features1,  1, "cpuid_fault");*//* MSR_PLATFORM_INFO Intel CPUID faulting */

/* features scattered in various CPUID levels. */
      /* cpuid 0x80000007 edx */
      if (maxe >= 0x80000007)
	{
	  cpuid (&unused, &unused, &unused, &features1, 0x80000007);

	  ftcprint (features1,  9, "cpb");	/* core performance boost */
	}
      /* cpuid 0x00000006 ecx */
      if (maxf >= 0x00000006)
	{
	  cpuid (&unused, &unused, &features1, &unused, 0x00000006);

	  ftcprint (features1,  3, "epb");	/* energy perf bias */
	}
      /* cpuid 0x00000007:1 ebx */
      if (maxf >= 0x00000007)
	{
	  cpuid (&unused, &features1, &unused, &unused, 0x00000007, 1);

	  ftcprint (features1,  0, "intel_ppin"); /* Prot Proc Id No */
	}
      /* cpuid 0x00000010 ebx */
      if (maxf >= 0x00000010)
	{
	  cpuid (&unused, &features1, &unused, &unused, 0x00000010);

	  ftcprint (features1,  1, "cat_l3");	/* cache alloc tech l3 */
	  ftcprint (features1,  2, "cat_l2");	/* cache alloc tech l2 */

	  /* cpuid 0x00000010:1 ecx */
	  cpuid (&unused, &unused, &features1, &unused, 0x00000010, 1);

	  ftcprint (features1,  2, "cdp_l3");	/* code data prior l3 */
	}

/*	  ftcprint (features1,  7, "invpcid_single");*//* INVPCID && CR4.PCIDE=1 */

      /* cpuid 0x80000007 edx */
      if (maxe >= 0x80000007)
	{
	  cpuid (&unused, &unused, &unused, &features1, 0x80000007);

	  ftcprint (features1,  7, "hw_pstate");	/* hw P state */
	  ftcprint (features1, 11, "proc_feedback"); /* proc feedback interf */
	}

/*	  ftcprint (features1, 11, "pti");*//* Page Table Isolation reqd with Meltdown */

      /* cpuid 0x00000010:2 ecx */
      if (maxf >= 0x00000010)
	{
	  cpuid (&unused, &unused, &features2, &unused, 0x00000010, 2);

	  ftcprint (features2,  2, "cdp_l2");	/* code data prior l2 */
	}
      /* cpuid 0x80000008 ebx */
      if (maxe >= 0x80000008)
        {
	  cpuid (&unused, &features1, &unused, &unused, 0x80000008, 0);

	  ftcprint (features1, 24, "ssbd");	/* spec store byp dis */
        }
      /* cpuid 0x00000010 ebx */
      if (maxf >= 0x00000010)
	{
	  cpuid (&unused, &features2, &unused, &unused, 0x00000010);

	  ftcprint (features2,  3, "mba");	/* memory bandwidth alloc */
	}
      /* cpuid 0x80000008 ebx */
      if (maxe >= 0x80000008)
	{
/*	  cpuid (&unused, &features1, &unused, &unused, 0x80000008, 0); */
/*	  from above */
	  ftcprint (features1,  6, "mba");	/* memory bandwidth alloc */
	}
      /* cpuid 0x80000008 ebx */
      if (maxe >= 0x80000008)
        {
/*	  cpuid (&unused, &features1, &unused, &unused, 0x80000008, 0); */
/*	  from above */
/*	  ftcprint (features1,  0, "clzero");	*//* clzero instruction */
/*	  ftcprint (features1,  1, "irperf");   *//* instr retired count */
/*	  ftcprint (features1,  2, "xsaveerptr");*//* save/rest FP err ptrs */
/*	  ftcprint (features1,  4, "rdpru");	*//* user level rd proc reg */
/*	  ftcprint (features1,  6, "mba");	*//* memory BW alloc */
/*	  ftcprint (features1,  9, "wbnoinvd"); *//* wbnoinvd instruction */
	  ftcprint (features1, 14, "ibrs");	/* ind br restricted spec */
	  ftcprint (features1, 12, "ibpb");	/* ind br pred barrier */
	  ftcprint (features1, 15, "stibp");	/* 1 thread ind br pred */
	  ftcprint (features1, 16, "ibrs_enhanced"); /* IBRS_ALL enhanced IBRS always on */
/*	  ftcprint (features1, 17, "stibp_always_on"); */ /* stibp always on */
/*	  ftcprint (features1, 18, "ibrs_pref");*//* IBRS_PREF IBRS preferred */
/*	  ftcprint (features1, 23, "amd_ppin"); *//* protected proc id no */
/*	  ftcprint (features1, 24, "ssbd");	*//* spec store byp dis */
/*	  ftcprint (features1, 25, "virt_ssbd");*//* vir spec store byp dis */
/*	  ftcprint (features1, 26, "ssb_no");	*//* ssb fixed in hardware */
        }

      /* cpuid 0x00000007 ebx */
      if (maxf >= 0x00000007)
	{
	  cpuid (&unused, &features1, &unused, &unused, 0x00000007, 0);

	  ftcprint (features1,  0, "fsgsbase");	    /* rd/wr fs/gs base */
	  ftcprint (features1,  1, "tsc_adjust");   /* TSC adjustment MSR 0x3B */
	  ftcprint (features1,  2, "sgx");	    /* software guard extensions */
	  ftcprint (features1,  3, "bmi1");         /* bit manip ext group 1 */
	  ftcprint (features1,  4, "hle");          /* hardware lock elision */
	  ftcprint (features1,  5, "avx2");         /* AVX ext instructions */
/*	  ftcprint (features1,  6, "fpdx"); */	    /* "" FP data ptr upd on exc */
	  ftcprint (features1,  7, "smep");         /* super mode exec prot */
	  ftcprint (features1,  8, "bmi2");         /* bit manip ext group 2 */
	  ftcprint (features1,  9, "erms");         /* enh rep movsb/stosb */
	  ftcprint (features1, 10, "invpcid");      /* inv proc context id */
	  ftcprint (features1, 11, "rtm");          /* restricted txnal mem */
	  ftcprint (features1, 12, "cqm");          /* cache QoS monitoring */
/*	  ftcprint (features1, 13, "fpcsdsz"); */   /* "" zero FP cs/ds */
	  ftcprint (features1, 14, "mpx");          /* mem prot ext */
	  ftcprint (features1, 15, "rdt_a");        /* rsrc dir tech alloc */
	  ftcprint (features1, 16, "avx512f");      /* vec foundation */
	  ftcprint (features1, 17, "avx512dq");     /* vec dq granular */
	  ftcprint (features1, 18, "rdseed");       /* RNG rdseed instruction */
	  ftcprint (features1, 19, "adx");          /* adcx/adox */
	  ftcprint (features1, 20, "smap");         /* sec mode access prev */
	  ftcprint (features1, 21, "avx512ifma");   /* vec int FMA */
	  ftcprint (features1, 23, "clflushopt");   /* cache line flush opt */
	  ftcprint (features1, 24, "clwb");         /* cache line write back */
	  ftcprint (features1, 25, "intel_pt");     /* intel processor trace */
	  ftcprint (features1, 26, "avx512pf");     /* vec prefetch */
	  ftcprint (features1, 27, "avx512er");     /* vec exp/recip aprx */
	  ftcprint (features1, 28, "avx512cd");     /* vec conflict detect */
	  ftcprint (features1, 29, "sha_ni");       /* SHA extensions */
	  ftcprint (features1, 30, "avx512bw");     /* vec byte/word gran */
	  ftcprint (features1, 31, "avx512vl");     /* vec vec len ext */
	}

      /* more random feature flags */
      /* cpuid 0x0000000d:1 eax */
      if (maxf >= 0x0000000d)
	{
	  cpuid (&features1, &unused, &unused, &unused, 0x0000000d, 1);

	  ftcprint (features1,  0, "xsaveopt");	/* xsaveopt instruction */
	  ftcprint (features1,  1, "xsavec");   /* xsavec instruction */
	  ftcprint (features1,  2, "xgetbv1");  /* xgetbv ecx 1 */
	  ftcprint (features1,  3, "xsaves");   /* xsaves/xrstors */
	}
      /* cpuid 0x0000000f edx */
      if (maxf >= 0x0000000f)
	{
	  cpuid (&unused, &unused, &unused, &features1, 0x0000000f);

	  ftcprint (features1,  1, "cqm_llc");		/* llc QoS */

	  /* cpuid 0x0000000f:1 edx */
	  cpuid (&unused, &unused, &unused, &features1, 0x0000000f, 1);

	  ftcprint (features1,  0, "cqm_occup_llc");	/* llc occup monitor */
	  ftcprint (features1,  1, "cqm_mbm_total");	/* llc total MBM mon */
	  ftcprint (features1,  2, "cqm_mbm_local");	/* llc local MBM mon */
	}

/*	  ftcprint (features1,  6, "split_lock_detect");*//* MSR_TEST_CTRL split lock */

      /* cpuid 0x00000007:1 eax */
      if (maxf >= 0x00000007)
	{
	  cpuid (&features1, &unused, &unused, &unused, 0x00000007, 1);

	  ftcprint (features1,  4, "avx_vnni");	    /* vex enc NN vec */
	  ftcprint (features1,  5, "avx512_bf16");  /* vec bfloat16 short */
	}

      /* AMD cpuid 0x80000008 ebx */
      if (is_amd && maxe >= 0x80000008)
        {
	  cpuid (&unused, &features1, &unused, &unused, 0x80000008, 0);

	  ftcprint (features1,  0, "clzero");	    /* clzero instruction */
	  ftcprint (features1,  1, "irperf");       /* instr retired count */
	  ftcprint (features1,  2, "xsaveerptr");   /* save/rest FP err ptrs */
	  ftcprint (features1,  4, "rdpru");	    /* user level rd proc reg */
/*	  ftcprint (features1,  6, "mba"); */	    /* memory BW alloc */
	  ftcprint (features1,  9, "wbnoinvd");     /* wbnoinvd instruction */
/*	  ftcprint (features1, 12, "ibpb" ); */	    /* ind br pred barrier */
/*	  ftcprint (features1, 14, "ibrs" ); */	    /* ind br restricted spec */
/*	  ftcprint (features1, 15, "stibp"); */	    /* 1 thread ind br pred */
/*	  ftcprint (features1, 16, "ibrs_enhanced");*//* IBRS_ALL enhanced IBRS always on */
/*	  ftcprint (features1, 17, "stibp_always_on"); */ /* stibp always on */
/*	  ftcprint (features1, 18, "ibrs_pref");*//* IBRS_PREF IBRS preferred */
	  ftcprint (features1, 23, "amd_ppin");     /* protected proc id no */
/*	  ftcprint (features1, 24, "ssbd"); */	    /* spec store byp dis */
	  ftcprint (features1, 25, "virt_ssbd");    /* vir spec store byp dis */
/*	  ftcprint (features1, 26, "ssb_no"); */    /* ssb fixed in hardware */
	  ftcprint (features1, 27, "cppc");	    /* collab proc perf ctl */
        }

      /* thermal & power cpuid 0x00000006 eax */
      if (maxf >= 0x00000006)
	{
	  cpuid (&features1, &unused, &features2, &unused, 0x00000006);

	  ftcprint (features1,  0, "dtherm");	/* digital thermal sensor */
	  ftcprint (features1,  1, "ida");      /* Intel dynamic acceleration */
	  ftcprint (features1,  2, "arat");     /* always running APIC timer */
	  ftcprint (features1,  4, "pln");      /* power limit notification */
	  ftcprint (features1,  6, "pts");      /* package thermal status */
	  ftcprint (features1,  7, "hwp");      /* hardware P states */
	  ftcprint (features1,  8, "hwp_notify"); /* HWP notification */
	  ftcprint (features1,  9, "hwp_act_window"); /* HWP activity window */
	  ftcprint (features1, 10, "hwp_epp");  /* HWP energy perf pref */
	  ftcprint (features1, 11, "hwp_pkg_req"); /* HWP package level req */
	  ftcprint (features1, 19, "hfi");	/* Hardware Feedback Interface */
	}

      /* AMD SVM cpuid 0x8000000a edx */
      if (is_amd && maxe >= 0x8000000a)
        {
	  cpuid (&unused, &unused, &unused, &features1, 0x8000000a, 0);

	  ftcprint (features1,  0, "npt");		/* nested paging */
	  ftcprint (features1,  1, "lbrv");             /* lbr virtualization */
	  ftcprint (features1,  2, "svm_lock");         /* SVM locking MSR */
	  ftcprint (features1,  3, "nrip_save");        /* SVM next rip save */
	  ftcprint (features1,  4, "tsc_scale");        /* TSC rate control */
	  ftcprint (features1,  5, "vmcb_clean");       /* VMCB clean bits */
	  ftcprint (features1,  6, "flushbyasid");      /* flush by ASID */
	  ftcprint (features1,  7, "decode_assists");   /* decode assists */
	  ftcprint (features1, 10, "pausefilter");      /* filt pause intrcpt */
	  ftcprint (features1, 12, "pfthreshold");      /* pause filt thresh */
	  ftcprint (features1, 13, "avic");             /* virt int control */
	  ftcprint (features1, 15, "v_vmsave_vmload");  /* virt vmsave vmload */
	  ftcprint (features1, 16, "vgif");             /* virt glb int flag */
	  ftcprint (features1, 20, "v_spec_ctrl");	/* virt spec ctrl support */
/*	  ftcprint (features1, 28, "svme_addr_chk");  *//* secure vmexit addr check */
        }

      /* Intel cpuid 0x00000007 ecx */
      if (is_intel && maxf >= 0x00000007)
        {
	  cpuid (&unused, &unused, &features1, &unused, 0x00000007, 0);

	  ftcprint (features1,  1, "avx512vbmi");	/* vec bit manip */
	  ftcprint (features1,  2, "umip");             /* user mode ins prot */
	  ftcprint (features1,  3, "pku");              /* prot key userspace */
	  ftcprint (features1,  4, "ospke");            /* OS prot keys en */
	  ftcprint (features1,  5, "waitpkg");          /* umon/umwait/tpause */
	  ftcprint (features1,  6, "avx512_vbmi2");     /* vec bit manip 2 */
	  ftcprint (features1,  8, "gfni");             /* Galois field instr */
	  ftcprint (features1,  9, "vaes");             /* vector AES */
	  ftcprint (features1, 10, "vpclmulqdq");       /* nc mul dbl quad */
	  ftcprint (features1, 11, "avx512_vnni");      /* vec neural net */
	  ftcprint (features1, 12, "avx512_bitalg");    /* vpopcnt/b/w vpshuf */
	  ftcprint (features1, 13, "tme");              /* total mem encrypt */
	  ftcprint (features1, 14, "avx512_vpopcntdq"); /* vec popcnt dw/qw */
	  ftcprint (features1, 16, "la57");             /* 5 level paging */
	  ftcprint (features1, 22, "rdpid");            /* rdpid instruction */
	  ftcprint (features1, 24, "bus_lock_detect");	/* bus lock detect dbg excptn */
	  ftcprint (features1, 25, "cldemote");         /* cldemote instr */
	  ftcprint (features1, 27, "movdiri");          /* movdiri instr */
	  ftcprint (features1, 28, "movdir64b");        /* movdir64b instr */
	  ftcprint (features1, 29, "enqcmd");		/* enqcmd/s instructions*/
	  ftcprint (features1, 30, "sgx_lc");		/* sgx launch control */
        }

      /* AMD MCA cpuid 0x80000007 ebx */
      if (is_amd && maxe >= 0x80000007)
        {
          cpuid (&unused, &features1, &unused, &unused, 0x80000007, 0);

          ftcprint (features1,  0, "overflow_recov");	/* MCA oflow recovery */
          ftcprint (features1,  1, "succor");           /* uncor err recovery */
          ftcprint (features1,  3, "smca");             /* scalable MCA */
        }

      /* Intel cpuid 0x00000007 edx */
      if (is_intel && maxf >= 0x00000007)
        {
          cpuid (&unused, &unused, &unused, &features1, 0x00000007, 0);

          ftcprint (features1,  2, "avx512_4vnniw");	   /* vec dot prod dw */
          ftcprint (features1,  3, "avx512_4fmaps");       /* vec 4 FMA single */
          ftcprint (features1,  4, "fsrm");		   /* fast short REP MOVSB */
          ftcprint (features1,  8, "avx512_vp2intersect"); /* vec intcpt d/q */
          ftcprint (features1, 10, "md_clear");            /* verw clear buf */
          ftcprint (features1, 14, "serialize");           /* SERIALIZE instruction */
          ftcprint (features1, 16, "tsxldtrk");		   /* TSX Susp Ld Addr Track */
          ftcprint (features1, 18, "pconfig");		   /* platform config */
          ftcprint (features1, 19, "arch_lbr");		   /* last branch records */
	  ftcprint (features1, 20, "ibt");		   /* Indirect Branch Tracking */
	  ftcprint (features1, 22, "amx_bf16");	    /* Advanced Matrix eXtensions Brain Float 16 dot product */
          ftcprint (features1, 23, "avx512_fp16");	   /* avx512 fp16 */
	  ftcprint (features1, 24, "amx_tile");	    /* Advanced Matrix eXtensions Tile matrix multiply */
	  ftcprint (features1, 25, "amx_int8");	    /* Advanced Matrix eXtensions Int 8 byte dot product */
          ftcprint (features1, 28, "flush_l1d");	   /* flush l1d cache */
          ftcprint (features1, 29, "arch_capabilities");   /* arch cap MSR */
        }

      /* cpuid x8000001f eax */
      if (is_amd && maxe >= 0x8000001f)
	{
	  cpuid (&features2, &unused, &unused, &unused, 0x8000001f);

	  ftcprint (features2,  0, "sme");	/* secure memory encryption */
	  ftcprint (features2,  1, "sev");	/* AMD secure encrypted virt */
/*	  ftcprint (features2,  2, "vm_page_flush");*/	/* VM page flush MSR */
	  ftcprint (features2,  3, "sev_es");	/* AMD SEV encrypted state */
/*	  ftcprint (features2,  4, "sev_snp");*//* AMD SEV secure nested paging */
/*	  ftcprint (features2,  5, "vmpl");   *//* VM permission levels support */
/*	  ftcprint (features2, 10, "sme_coherent");   *//* SME h/w cache coherent */
/*	  ftcprint (features2, 11, "sev_64b");*//* SEV 64 bit host guest only */
/*	  ftcprint (features2, 12, "sev_rest_inj");   *//* SEV restricted injection */
/*	  ftcprint (features2, 13, "sev_alt_inj");    *//* SEV alternate injection */
/*	  ftcprint (features2, 14, "sev_es_dbg_swap");*//* SEV-ES debug state swap */
/*	  ftcprint (features2, 15, "no_host_ibs");    *//* host IBS unsupported */
/*	  ftcprint (features2, 16, "vte");    *//* virtual transparent encryption */
	}

      print ("\n");

      bufptr += __small_sprintf (bufptr, "bogomips\t: %d.00\n",
						bogomips);

      /* cpuid 0x80000006 ebx TLB size */
      if (maxe >= 0x80000006)
	{
	  cpuid( &unused, &features1, &unused, &unused, 0x80000006, 0);
	  uint32_t tlbsize = ((features1 >> 16) & 0xfff) + (features1 & 0xfff);
	  if (tlbsize > 0)
	    bufptr += __small_sprintf (bufptr, "TLB size\t: %d 4K pages\n",
						tlbsize);
	}

      bufptr += __small_sprintf (bufptr, "clflush size\t: %d\n"
					 "cache_alignment\t: %d\n",
				 clflush,
				 cache_alignment);

      if (maxe >= 0x80000008) /* Address size. */
	{
	  uint32_t addr_size, phys, virt;
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

      /* cpuid 0x80000007 edx */
      if (maxe >= 0x80000007) /* Advanced power management. */
	{
	  cpuid (&unused, &unused, &unused, &features1, 0x80000007);

	  print ("power management:");
	  ftcprint (features1,  0, "ts");	    /* temperature sensor */
	  ftcprint (features1,  1, "fid");          /* frequency id control */
	  ftcprint (features1,  2, "vid");          /* voltage id control */
	  ftcprint (features1,  3, "ttp");          /* thermal trip */
	  ftcprint (features1,  4, "tm");           /* hw thermal control */
	  ftcprint (features1,  5, "stc");          /* sw thermal control */
	  ftcprint (features1,  6, "100mhzsteps");  /* 100 MHz mult control */
	  ftcprint (features1,  7, "hwpstate");     /* hw P state control */
/*	  ftcprint (features1,  8, "invariant_tsc"); */ /* TSC invariant */
	  ftcprint (features1,  9, "cpb");          /* core performance boost */
	  ftcprint (features1, 10, "eff_freq_ro");  /* ro eff freq interface */
	  ftcprint (features1, 11, "proc_feedback");/* proc feedback if */
	  ftcprint (features1, 12, "acc_power");    /* core power reporting */
/*	  ftcprint (features1, 13, "connstby"); */  /* connected standby */
/*	  ftcprint (features1, 14, "rapl"); */	    /* running average power limit */
	}

      if (orig_affinity_mask != 0)
	SetThreadGroupAffinity (GetCurrentThread (), &orig_group_affinity,
				NULL);
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

  tmp_pathbuf tp;
  char *buf = tp.c_get ();
  char *bufptr = buf;
  char *ioctl_buf = tp.c_get ();
  PWCHAR mp_buf = tp.w_get ();
  PDIRECTORY_BASIC_INFORMATION dbi_buf = (PDIRECTORY_BASIC_INFORMATION)
					 tp.w_get ();
  WCHAR fpath[MAX_PATH];
  WCHAR gpath[MAX_PATH];
  DWORD len;

  /* Traverse \Device directory ... */
  BOOLEAN restart = TRUE;
  bool got_one = false;
  bool last_run = false;
  ULONG context = 0;
  while (!last_run)
    {
      status = NtQueryDirectoryObject (dirhdl, dbi_buf, 65536, FALSE, restart,
				       &context, NULL);
      if (!NT_SUCCESS (status))
	{
	  debug_printf ("NtQueryDirectoryObject, status %y", status);
	  __seterrno_from_nt_status (status);
	  break;
	}
      if (status != STATUS_MORE_ENTRIES)
	last_run = true;
      restart = FALSE;
      for (PDIRECTORY_BASIC_INFORMATION dbi = dbi_buf;
	   dbi->ObjectName.Length > 0;
	   dbi++)
	{
	  HANDLE devhdl;
	  PARTITION_INFORMATION_EX *pix = NULL;
	  PARTITION_INFORMATION *pi = NULL;
	  DWORD bytes_read;
	  DWORD part_cnt = 0;
	  unsigned long drive_num;
	  unsigned long long size;

	  /* ... and check for a "Harddisk[0-9]*" entry. */
	  if (dbi->ObjectName.Length < 9 * sizeof (WCHAR)
	      || wcsncasecmp (dbi->ObjectName.Buffer, L"Harddisk", 8) != 0
	      || !iswdigit (dbi->ObjectName.Buffer[8]))
	    continue;
	  /* Got it.  Now construct the path to the entire disk, which is
	     "\\Device\\HarddiskX\\Partition0", and open the disk with
	     minimum permissions. */
	  drive_num = wcstoul (dbi->ObjectName.Buffer + 8, NULL, 10);
	  wcscpy (wpath, dbi->ObjectName.Buffer);
	  PWCHAR wpart = wpath + dbi->ObjectName.Length / sizeof (WCHAR);
	  wcpcpy (wpart, L"\\Partition0");
	  upath.Length = dbi->ObjectName.Length + 22;
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
	      print ("major minor  #blocks  name   win-mounts\n\n");
	      got_one = true;
	    }
	  /* Fetch partition info for the entire disk to get its size. */
	  if (DeviceIoControl (devhdl, IOCTL_DISK_GET_PARTITION_INFO_EX, NULL,
			       0, ioctl_buf, NT_MAX_PATH, &bytes_read, NULL))
	    {
	      pix = (PARTITION_INFORMATION_EX *) ioctl_buf;
	      size = pix->PartitionLength.QuadPart;
	    }
	  else if (DeviceIoControl (devhdl, IOCTL_DISK_GET_PARTITION_INFO, NULL,
				    0, ioctl_buf, NT_MAX_PATH, &bytes_read,
				    NULL))
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
	  device dev (drive_num, 0);
	  bufptr += __small_sprintf (bufptr, "%5d %5d %9U %s\n",
				     dev.get_major (), dev.get_minor (),
				     size >> 10, dev.name () + 5);
	  /* Fetch drive layout info to get size of all partitions on disk. */
	  if (DeviceIoControl (devhdl, IOCTL_DISK_GET_DRIVE_LAYOUT_EX, NULL, 0,
			       ioctl_buf, NT_MAX_PATH, &bytes_read, NULL))
	    {
	      PDRIVE_LAYOUT_INFORMATION_EX pdlix =
		  (PDRIVE_LAYOUT_INFORMATION_EX) ioctl_buf;
	      part_cnt = pdlix->PartitionCount;
	      pix = pdlix->PartitionEntry;
	    }
	  else if (DeviceIoControl (devhdl, IOCTL_DISK_GET_DRIVE_LAYOUT, NULL,
				    0, ioctl_buf, NT_MAX_PATH, &bytes_read,
				    NULL))
	    {
	      PDRIVE_LAYOUT_INFORMATION pdli =
		  (PDRIVE_LAYOUT_INFORMATION) ioctl_buf;
	      part_cnt = pdli->PartitionCount;
	      pi = pdli->PartitionEntry;
	    }
	  else
	    debug_printf ("DeviceIoControl(%S, "
			  "IOCTL_DISK_GET_DRIVE_LAYOUT{_EX}): %E", &upath);
	  /* Loop over partitions. */
	  if (pix || pi)
	    for (DWORD i = 0; i < part_cnt && i < 64; ++i)
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
		   filler entry as described in
		   fhandler_dev_floppy::lock_partition.  Just skip. */
		if (part_num == 0)
		  continue;
		device dev (drive_num, part_num);

		bufptr += __small_sprintf (bufptr, "%5d %5d %9U %s",
					   dev.get_major (), dev.get_minor (),
					   size >> 10, dev.name () + 5);
		/* Check if the partition is mounted in Windows and, if so,
		   print the mount point list. */
		__small_swprintf (fpath,
				L"\\\\?\\GLOBALROOT\\Device\\%S\\Partition%u\\",
				&dbi->ObjectName, part_num);
		if (GetVolumeNameForVolumeMountPointW (fpath, gpath, MAX_PATH)
		    && GetVolumePathNamesForVolumeNameW (gpath, mp_buf,
							 NT_MAX_PATH, &len))
		  {
		    len = strlen (dev.name () + 5);
		    while (len++ < 6)
		      *bufptr++ = ' ';
		    for (PWCHAR p = mp_buf; *p; p = wcschr (p, L'\0') + 1)
		      bufptr += __small_sprintf (bufptr, " %W", p);
		  }

		*bufptr++ = '\n';
	      }
	  NtClose (devhdl);
	}
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
format_proc_cygdrive (void *, char *&destbuf)
{
  destbuf = (char *) crealloc_abort (destbuf, mount_table->cygdrive_len + 1);
  char *dend = stpcpy (destbuf, mount_table->cygdrive);
  if (dend > destbuf + 1)	/* cygdrive != "/"? */
    *--dend = '\0';
  return dend - destbuf;
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
			"Filename\t\t\t\tType\t\tSize\t\tUsed\t\tPriority\n");

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
	  /* ensure space between fields for clarity */
	  size_t tabo = strlen (filename) / 8;	/* offset tabs to space name */
	  bufptr += sprintf (bufptr, "%s%s%s\t\t%llu%s\t%llu%s\t%d\n",
				    filename,
				    tabo < 5 ? "\t\t\t\t\t" + tabo : " ",
					"file",
					    total >> 10,
					    total < 10000000000 ? "\t" : "",
						used  >> 10,
						used  < 10000000000 ? "\t" : "",
								0);
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

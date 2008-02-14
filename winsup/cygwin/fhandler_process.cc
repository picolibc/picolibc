/* fhandler_process.cc: fhandler for /proc/<pid> virtual filesystem

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
#include "shared_info.h"
#include "dtable.h"
#include "cygheap.h"
#include "ntdll.h"
#include <sys/param.h>
#include <assert.h>
#include <sys/sysmacros.h>
#include <ctype.h>
#include <psapi.h>

#define _COMPILING_NEWLIB
#include <dirent.h>

static const int PROCESS_PPID = 2;
static const int PROCESS_WINPID = 3;
static const int PROCESS_WINEXENAME = 4;
static const int PROCESS_STATUS = 5;
static const int PROCESS_UID = 6;
static const int PROCESS_GID = 7;
static const int PROCESS_PGID = 8;
static const int PROCESS_SID = 9;
static const int PROCESS_CTTY = 10;
static const int PROCESS_STAT = 11;
static const int PROCESS_STATM = 12;
static const int PROCESS_CMDLINE = 13;
static const int PROCESS_MAPS = 14;
static const int PROCESS_FD = 15;
static const int PROCESS_EXENAME = 16;
/* Keep symlinks always the last entries. */
static const int PROCESS_ROOT = 17;
static const int PROCESS_EXE = 18;
static const int PROCESS_CWD = 19;

/* The position of "root" defines the beginning of symlik entries. */
#define is_symlink(nr) ((nr) >= PROCESS_ROOT)

static const char * const process_listing[] =
{
  ".",
  "..",
  "ppid",
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
  "maps",
  "fd",
  "exename",
  /* Keep symlinks always the last entries. */
  "root",
  "exe",
  "cwd",
  NULL
};

static const int PROCESS_LINK_COUNT =
  (sizeof (process_listing) / sizeof (const char *)) - 1;

static _off64_t format_process_maps (_pinfo *p, char *&destbuf, size_t maxsize);
static _off64_t format_process_stat (_pinfo *p, char *destbuf, size_t maxsize);
static _off64_t format_process_status (_pinfo *p, char *destbuf, size_t maxsize);
static _off64_t format_process_statm (_pinfo *p, char *destbuf, size_t maxsize);
static int get_process_state (DWORD dwProcessId);
static bool get_mem_values (DWORD dwProcessId, unsigned long *vmsize,
			    unsigned long *vmrss, unsigned long *vmtext,
			    unsigned long *vmdata, unsigned long *vmlib,
			    unsigned long *vmshare);

/* Returns 0 if path doesn't exist, >0 if path is a directory,
 * -1 if path is a file, -2 if path is a symlink, -3 if path is a pipe,
 * -4 if path is a socket.
 */
int
fhandler_process::exists ()
{
  const char *path = get_name ();
  debug_printf ("exists (%s)", path);
  path += proc_len + 1;
  while (*path != 0 && !isdirsep (*path))
    path++;
  if (*path == 0)
    return 2;

  for (int i = 0; process_listing[i]; i++)
    if (pathmatch (path + 1, process_listing[i]))
      {
	fileid = i;
	return is_symlink (i) ? -2 : (i == PROCESS_FD) ? 1 : -1;
      }
  if (pathnmatch (strchr (path, '/') + 1, "fd/", 3))
    {
      fileid = PROCESS_FD;
      if (fill_filebuf ())
	return -2;
      /* Check for nameless device entries. */
      path = strrchr (path, '/');
      if (path && *++path)
	{
	  if (!strncmp (path, "pipe:[", 6))
	    return -3;
	  else if (!strncmp (path, "socket:[", 8))
	    return -4;
	}
    }
  return 0;
}

fhandler_process::fhandler_process ():
  fhandler_proc ()
{
}

int
fhandler_process::fstat (struct __stat64 *buf)
{
  const char *path = get_name ();
  int file_type = exists ();
  fhandler_base::fstat (buf);
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
    case 2:
      buf->st_ctime = buf->st_mtime = buf->st_birthtime = p->start_time;
      buf->st_ctim.tv_nsec = buf->st_mtim.tv_nsec
	= buf->st_birthtim.tv_nsec = 0;
      time_as_timestruc_t (&buf->st_atim);
      buf->st_uid = p->uid;
      buf->st_gid = p->gid;
      buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
      if (file_type == 1)
	buf->st_nlink = 2;
      else
	buf->st_nlink = 3;
      return 0;
    case -2:
      buf->st_uid = p->uid;
      buf->st_gid = p->gid;
      buf->st_mode = S_IFLNK | S_IRWXU | S_IRWXG | S_IRWXO;
      return 0;
    case -3:
      buf->st_uid = p->uid;
      buf->st_gid = p->gid;
      buf->st_mode = S_IFIFO | S_IRUSR | S_IWUSR;
      return 0;
    case -4:
      buf->st_uid = p->uid;
      buf->st_gid = p->gid;
      buf->st_mode = S_IFSOCK | S_IRUSR | S_IWUSR;
      return 0;
    case -1:
    default:
      buf->st_uid = p->uid;
      buf->st_gid = p->gid;
      buf->st_mode |= S_IFREG | S_IRUSR | S_IRGRP | S_IROTH;
      return 0;
    }
}

DIR *
fhandler_process::opendir (int fd)
{
  DIR *dir = fhandler_virtual::opendir (fd);
  if (dir && fileid == PROCESS_FD)
    fill_filebuf ();
  return dir;
}

int
fhandler_process::readdir (DIR *dir, dirent *de)
{
  int res = ENMFILE;
  if (fileid == PROCESS_FD)
    {
      if (dir->__d_position >= 2 + filesize / sizeof (int))
	goto out;
    }
  else if (dir->__d_position >= PROCESS_LINK_COUNT)
    goto out;
  if (fileid == PROCESS_FD && dir->__d_position > 1)
    {
      int *p = (int *) filebuf;
      __small_sprintf (de->d_name, "%d", p[dir->__d_position++ - 2]);
    }
  else
    strcpy (de->d_name, process_listing[dir->__d_position++]);
  dir->__flags |= dirent_saw_dot | dirent_saw_dot_dot;
  res = 0;
out:
  syscall_printf ("%d = readdir (%p, %p) (%s)", res, dir, de, de->d_name);
  return res;
}

int
fhandler_process::open (int flags, mode_t mode)
{
  int process_file_no = -1;

  int res = fhandler_virtual::open (flags, mode);
  if (!res)
    goto out;

  nohandle (true);

  const char *path;
  path = get_name () + proc_len + 1;
  pid = atoi (path);
  while (*path != 0 && !isdirsep (*path))
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
  if (process_file_no == PROCESS_FD)
    {
      flags |= O_DIROPEN;
      goto success;
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
  const char *path;
  path = get_name () + proc_len + 1;
  if (!pid)
    pid = atoi (path);

  pinfo p (pid);

  if (!p)
    {
      set_errno (ENOENT);
      return false;
    }

  switch (fileid)
    {
    case PROCESS_FD:
      {
	size_t fs;
	char *fdp = strrchr (path, '/');
	if (!fdp || *++fdp == 'f') /* The "fd" directory itself. */
	  {
	    if (filebuf)
	      cfree (filebuf);
	    filebuf = p->fds (fs);
	  }
	else
	  {
	    if (filebuf)
	      cfree (filebuf);
	    int fd = atoi (fdp);
	    if (fd < 0 || (fd == 0 && !isdigit (*fdp)))
	      {
		set_errno (ENOENT);
		return false;
	      }
	    filebuf = p->fd (fd, fs);
	    if (!filebuf || !*filebuf)
	      {
		set_errno (ENOENT);
		return false;
	      }
	  }
	filesize = fs;
	break;
      }
    case PROCESS_UID:
    case PROCESS_GID:
    case PROCESS_PGID:
    case PROCESS_SID:
    case PROCESS_CTTY:
    case PROCESS_PPID:
      {
	filebuf = (char *) crealloc_abort (filebuf, bufalloc = 40);
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
    case PROCESS_ROOT:
    case PROCESS_CWD:
    case PROCESS_CMDLINE:
      {
	if (filebuf)
	  {
	    cfree (filebuf);
	    filebuf = NULL;
	  }
	size_t fs;
	switch (fileid)
	  {
	  case PROCESS_ROOT:
	    filebuf = p->root (fs);
	    break;
	  case PROCESS_CWD:
	    filebuf = p->cwd (fs);
	    break;
	  case PROCESS_CMDLINE:
	    filebuf = p->cmdline (fs);
	    break;
	  }
	filesize = fs;
	if (!filebuf || !*filebuf)
	  {
	    filebuf = cstrdup ("<defunct>");
	    filesize = strlen (filebuf) + 1;
	  }
	break;
      }
    case PROCESS_EXENAME:
    case PROCESS_EXE:
      {
	filebuf = (char *) crealloc_abort (filebuf, bufalloc = NT_MAX_PATH);
	if (p->process_state & PID_EXITED)
	  strcpy (filebuf, "<defunct>");
	else
	  {
	    mount_table->conv_to_posix_path (p->progname, filebuf, 1);
	    /* If transparent_exe isn't set, the link keeps its suffix so that
	       an open(2) call will succeed on /proc/$PID/exe. */
	    if (transparent_exe)
	      {
		int len = strlen (filebuf);
		if (len > 4)
		  {
		    char *s = filebuf + len - 4;
		    if (ascii_strcasematch (s, ".exe"))
		      *s = 0;
		  }
	      }
	  }
	filesize = strlen (filebuf);
	break;
      }
    case PROCESS_WINPID:
      {
	filebuf = (char *) crealloc_abort (filebuf, bufalloc = 40);
	__small_sprintf (filebuf, "%d\n", p->dwProcessId);
	filesize = strlen (filebuf);
	break;
      }
    case PROCESS_WINEXENAME:
      {
	int len = strlen (p->progname);
	filebuf = (char *) crealloc_abort (filebuf, bufalloc = (len + 2));
	strcpy (filebuf, p->progname);
	filebuf[len] = '\n';
	filesize = len + 1;
	break;
      }
    case PROCESS_STATUS:
      {
	filebuf = (char *) crealloc_abort (filebuf, bufalloc = 2048);
	filesize = format_process_status (*p, filebuf, bufalloc);
	break;
      }
    case PROCESS_STAT:
      {
	filebuf = (char *) crealloc_abort (filebuf, bufalloc = 2048);
	filesize = format_process_stat (*p, filebuf, bufalloc);
	break;
      }
    case PROCESS_STATM:
      {
	filebuf = (char *) crealloc_abort (filebuf, bufalloc = 2048);
	filesize = format_process_statm (*p, filebuf, bufalloc);
	break;
      }
    case PROCESS_MAPS:
      {
	filebuf = (char *) crealloc_abort (filebuf, bufalloc = 2048);
	filesize = format_process_maps (*p, filebuf, bufalloc);
	break;
      }
    }

  return true;
}

static _off64_t
format_process_maps (_pinfo *p, char *&destbuf, size_t maxsize)
{
  HANDLE proc = OpenProcess (PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
			     FALSE,
			     p->dwProcessId);
  if (!proc)
    return 0;

  _off64_t len = 0;
  HMODULE *modules;
  DWORD needed, i;
  DWORD_PTR wset_size;
  DWORD_PTR *workingset = NULL;
  MODULEINFO info;
  WCHAR modname[NT_MAX_PATH];
  char posix_modname[NT_MAX_PATH];

  if (!EnumProcessModules (proc, NULL, 0, &needed))
    {
      __seterrno ();
      len = -1;
      goto out;
    }
  modules = (HMODULE*) alloca (needed);
  if (!EnumProcessModules (proc, modules, needed, &needed))
    {
      __seterrno ();
      len = -1;
      goto out;
    }

  QueryWorkingSet (proc, (void *) &wset_size, sizeof wset_size);
  if (GetLastError () == ERROR_BAD_LENGTH)
    {
      workingset = (DWORD_PTR *) alloca (sizeof (DWORD_PTR) * ++wset_size);
      if (!QueryWorkingSet (proc, (void *) workingset,
			    sizeof (DWORD_PTR) * wset_size))
	workingset = NULL;
    }
  for (i = 0; i < needed / sizeof (HMODULE); i++)
    if (GetModuleInformation (proc, modules[i], &info, sizeof info)
	&& GetModuleFileNameExW (proc, modules[i], modname, sizeof modname))
      {
	char access[5];
	strcpy (access, "r--p");
	struct __stat64 st;
	if (mount_table->conv_to_posix_path (modname, posix_modname, 0))
	  sys_wcstombs (posix_modname, NT_MAX_PATH, modname);
	if (stat64 (posix_modname, &st))
	  {
	    st.st_dev = 0;
	    st.st_ino = 0;
	  }
	if (len + strlen (posix_modname) + 62 > maxsize - 1)
	  destbuf = (char *) crealloc_abort (destbuf, maxsize += 2048);
	if (workingset)
	  for (unsigned i = 1; i <= wset_size; ++i)
	    {
	      DWORD_PTR addr = workingset[i] & 0xfffff000UL;
	      if ((char *)addr >= info.lpBaseOfDll
		  && (char *)addr < (char *)info.lpBaseOfDll + info.SizeOfImage)
		{
		  access[0] = (workingset[i] & 0x5) ? 'r' : '-';
		  access[1] = (workingset[i] & 0x4) ? 'w' : '-';
		  access[2] = (workingset[i] & 0x2) ? 'x' : '-';
		  access[3] = (workingset[i] & 0x100) ? 's' : 'p';
		}
	    }
	int written = __small_sprintf (destbuf + len,
				"%08lx-%08lx %s %08lx %04x:%04x %U   ",
				info.lpBaseOfDll,
				(unsigned long)info.lpBaseOfDll
				+ info.SizeOfImage,
				access,
				info.EntryPoint,
				st.st_dev >> 16,
				st.st_dev & 0xffff,
				st.st_ino);
	while (written < 62)
	  destbuf[len + written++] = ' ';
	len += written;
	len += __small_sprintf (destbuf + len, "%s\n", posix_modname);
      }
out:
  CloseHandle (proc);
  return len;
}

static _off64_t
format_process_stat (_pinfo *p, char *destbuf, size_t maxsize)
{
  char cmd[NAME_MAX + 1];
  int state = 'R';
  unsigned long fault_count = 0UL,
		utime = 0UL, stime = 0UL,
		start_time = 0UL,
		vmsize = 0UL, vmrss = 0UL, vmmaxrss = 0UL;
  int priority = 0;
  if (p->process_state & PID_EXITED)
    strcpy (cmd, "<defunct>");
  else
    {
      char *last_slash = strrchr (p->progname, '\\');
      strcpy (cmd, last_slash ? last_slash + 1 : p->progname);
      int len = strlen (cmd);
      if (len > 4)
	{
	  char *s = cmd + len - 4;
	  if (ascii_strcasematch (s, ".exe"))
	    *s = 0;
	 }
    }
  /*
   * Note: under Windows, a _process_ is always running - it's only _threads_
   * that get suspended. Therefore the default state is R (runnable).
   */
  if (p->process_state & PID_EXITED)
    state = 'Z';
  else if (p->process_state & PID_STOPPED)
    state = 'T';
  else
    state = get_process_state (p->dwProcessId);
  start_time = (GetTickCount () / 1000 - time (NULL) + p->start_time) * HZ;

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
      debug_printf ("OpenProcess: ret %d", error);
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
      __seterrno_from_nt_status (ret);
      debug_printf ("NtQueryInformationProcess: ret %d, Dos(ret) %E", ret);
      return 0;
    }
  fault_count = vmc.PageFaultCount;
  utime = put.UserTime.QuadPart * HZ / 10000000ULL;
  stime = put.KernelTime.QuadPart * HZ / 10000000ULL;
#if 0
   if (stodi.CurrentTime.QuadPart > put.CreateTime.QuadPart)
     start_time = (spt.KernelTime.QuadPart + spt.UserTime.QuadPart -
		   stodi.CurrentTime.QuadPart + put.CreateTime.QuadPart) * HZ / 10000000ULL;
   else
     /*
      * sometimes stodi.CurrentTime is a bit behind
      * Note: some older versions of procps are broken and can't cope
      * with process start times > time(NULL).
      */
     start_time = (spt.KernelTme.QuadPart + spt.UserTime.QuadPart) * HZ / 10000000ULL;
#endif
  priority = pbi.BasePriority;
  unsigned page_size = getsystempagesize ();
  vmsize = vmc.PagefileUsage;
  vmrss = vmc.WorkingSetSize / page_size;
  vmmaxrss = ql.MaximumWorkingSetSize / page_size;

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

static _off64_t
format_process_status (_pinfo *p, char *destbuf, size_t maxsize)
{
  char cmd[NAME_MAX + 1];
  int state = 'R';
  const char *state_str = "unknown";
  unsigned long vmsize = 0UL, vmrss = 0UL, vmdata = 0UL, vmlib = 0UL, vmtext = 0UL,
		vmshare = 0UL;
  if (p->process_state & PID_EXITED)
    strcpy (cmd, "<defunct>");
  else
    {
      char *last_slash = strrchr (p->progname, '\\');
      strcpy (cmd, last_slash ? last_slash + 1 : p->progname);
      int len = strlen (cmd);
      if (len > 4)
	{
	  char *s = cmd + len - 4;
	  if (ascii_strcasematch (s, ".exe"))
	    *s = 0;
	 }
    }
  /*
   * Note: under Windows, a _process_ is always running - it's only _threads_
   * that get suspended. Therefore the default state is R (runnable).
   */
  if (p->process_state & PID_EXITED)
    state = 'Z';
  else if (p->process_state & PID_STOPPED)
    state = 'T';
  else
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
  if (!get_mem_values (p->dwProcessId, &vmsize, &vmrss, &vmtext, &vmdata,
		       &vmlib, &vmshare))
    return 0;
  unsigned page_size = getsystempagesize ();
  vmsize *= page_size; vmrss *= page_size; vmdata *= page_size;
  vmtext *= page_size; vmlib *= page_size;
  // The real uid value for *this* process is stored at cygheap->user.real_uid
  // but we can't get at the real uid value for any other process, so
  // just fake it as p->uid. Similar for p->gid.
  return __small_sprintf (destbuf, "Name:\t%s\n"
				   "State:\t%c (%s)\n"
				   "Tgid:\t%d\n"
				   "Pid:\t%d\n"
				   "PPid:\t%d\n"
				   "Uid:\t%d %d %d %d\n"
				   "Gid:\t%d %d %d %d\n"
				   "VmSize:\t%8d kB\n"
				   "VmLck:\t%8d kB\n"
				   "VmRSS:\t%8d kB\n"
				   "VmData:\t%8d kB\n"
				   "VmStk:\t%8d kB\n"
				   "VmExe:\t%8d kB\n"
				   "VmLib:\t%8d kB\n"
				   "SigPnd:\t%016x\n"
				   "SigBlk:\t%016x\n"
				   "SigIgn:\t%016x\n",
			  cmd,
			  state, state_str,
			  p->pgid,
			  p->pid,
			  p->ppid,
			  p->uid, p->uid, p->uid, p->uid,
			  p->gid, p->gid, p->gid, p->gid,
			  vmsize >> 10, 0, vmrss >> 10, vmdata >> 10, 0,
			  vmtext >> 10, vmlib >> 10,
			  0, 0, p->getsigmask ()
			  );
}

static _off64_t
format_process_statm (_pinfo *p, char *destbuf, size_t maxsize)
{
  unsigned long vmsize = 0UL, vmrss = 0UL, vmtext = 0UL, vmdata = 0UL,
		vmlib = 0UL, vmshare = 0UL;
  if (!get_mem_values (p->dwProcessId, &vmsize, &vmrss, &vmtext, &vmdata,
		       &vmlib, &vmshare))
    return 0;
  return __small_sprintf (destbuf, "%ld %ld %ld %ld %ld %ld %ld",
			  vmsize, vmrss, vmshare, vmtext, vmlib, vmdata, 0);
}

static int
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
      debug_printf ("NtQuerySystemInformation: ret %d, Dos(ret) %d",
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

static bool
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
  PULONG p = (PULONG) malloc (sizeof (ULONG) * n);
  unsigned page_size = getsystempagesize ();
  hProcess = OpenProcess (PROCESS_QUERY_INFORMATION,
			  FALSE, dwProcessId);
  if (hProcess == NULL)
    {
      DWORD error = GetLastError ();
      __seterrno_from_win_error (error);
      debug_printf ("OpenProcess: ret %d", error);
      return false;
    }
  while ((ret = NtQueryVirtualMemory (hProcess, 0,
				      MemoryWorkingSetList,
				      (PVOID) p,
				      n * sizeof *p, &length)),
	 (ret == STATUS_SUCCESS || ret == STATUS_INFO_LENGTH_MISMATCH) &&
	 length >= (n * sizeof (*p)))
      p = (PULONG) realloc (p, n *= (2 * sizeof (ULONG)));

  if (ret != STATUS_SUCCESS)
    {
      debug_printf ("NtQueryVirtualMemory: ret %d, Dos(ret) %d",
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
  ret = NtQueryInformationProcess (hProcess, ProcessVmCounters, (PVOID) &vmc,
				   sizeof vmc, NULL);
  if (ret != STATUS_SUCCESS)
    {
      debug_printf ("NtQueryInformationProcess: ret %d, Dos(ret) %d",
		    ret, RtlNtStatusToDosError (ret));
      res = false;
      goto out;
    }
  *vmsize = vmc.PagefileUsage / page_size;
out:
  free (p);
  CloseHandle (hProcess);
  return res;
}

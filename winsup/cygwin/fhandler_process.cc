/* fhandler_process.cc: fhandler for /proc/<pid> virtual filesystem

   Copyright 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009,
   2010, 2011 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/cygwin.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "fhandler_virtual.h"
#include "pinfo.h"
#include "shared_info.h"
#include "dtable.h"
#include "cygheap.h"
#include "ntdll.h"
#include "cygtls.h"
#include "pwdgrp.h"
#include "tls_pbuf.h"
#include <sys/param.h>
#include <ctype.h>
#include <psapi.h>
#include <tlhelp32.h>

#define _COMPILING_NEWLIB
#include <dirent.h>

static _off64_t format_process_maps (void *, char *&);
static _off64_t format_process_stat (void *, char *&);
static _off64_t format_process_status (void *, char *&);
static _off64_t format_process_statm (void *, char *&);
static _off64_t format_process_winexename (void *, char *&);
static _off64_t format_process_winpid (void *, char *&);
static _off64_t format_process_exename (void *, char *&);
static _off64_t format_process_root (void *, char *&);
static _off64_t format_process_cwd (void *, char *&);
static _off64_t format_process_cmdline (void *, char *&);
static _off64_t format_process_ppid (void *, char *&);
static _off64_t format_process_uid (void *, char *&);
static _off64_t format_process_pgid (void *, char *&);
static _off64_t format_process_sid (void *, char *&);
static _off64_t format_process_gid (void *, char *&);
static _off64_t format_process_ctty (void *, char *&);
static _off64_t format_process_fd (void *, char *&);
static _off64_t format_process_mounts (void *, char *&);

static const virt_tab_t process_tab[] =
{
  { _VN ("."),          FH_PROCESS,   virt_directory, NULL },
  { _VN (".."),         FH_PROCESS,   virt_directory, NULL },
  { _VN ("cmdline"),    FH_PROCESS,   virt_file,      format_process_cmdline },
  { _VN ("ctty"),       FH_PROCESS,   virt_file,      format_process_ctty },
  { _VN ("cwd"),        FH_PROCESS,   virt_symlink,   format_process_cwd },
  { _VN ("exe"),        FH_PROCESS,   virt_symlink,   format_process_exename },
  { _VN ("exename"),    FH_PROCESS,   virt_file,      format_process_exename },
  { _VN ("fd"),         FH_PROCESSFD, virt_directory, format_process_fd },
  { _VN ("gid"),        FH_PROCESS,   virt_file,      format_process_gid },
  { _VN ("maps"),       FH_PROCESS,   virt_file,      format_process_maps },
  { _VN ("mounts"),     FH_PROCESS,   virt_file,      format_process_mounts },
  { _VN ("pgid"),       FH_PROCESS,   virt_file,      format_process_pgid },
  { _VN ("ppid"),       FH_PROCESS,   virt_file,      format_process_ppid },
  { _VN ("root"),       FH_PROCESS,   virt_symlink,   format_process_root },
  { _VN ("sid"),        FH_PROCESS,   virt_file,      format_process_sid },
  { _VN ("stat"),       FH_PROCESS,   virt_file,      format_process_stat },
  { _VN ("statm"),      FH_PROCESS,   virt_file,      format_process_statm },
  { _VN ("status"),     FH_PROCESS,   virt_file,      format_process_status },
  { _VN ("uid"),        FH_PROCESS,   virt_file,      format_process_uid },
  { _VN ("winexename"), FH_PROCESS,   virt_file,      format_process_winexename },
  { _VN ("winpid"),     FH_PROCESS,   virt_file,      format_process_winpid },
  { NULL, 0,	        FH_NADA,       virt_none,      NULL }
};

static const int PROCESS_LINK_COUNT =
  (sizeof (process_tab) / sizeof (virt_tab_t)) - 1;
int get_process_state (DWORD dwProcessId);
static bool get_mem_values (DWORD dwProcessId, unsigned long *vmsize,
			    unsigned long *vmrss, unsigned long *vmtext,
			    unsigned long *vmdata, unsigned long *vmlib,
			    unsigned long *vmshare);

/* Returns 0 if path doesn't exist, >0 if path is a directory,
   -1 if path is a file, -2 if path is a symlink, -3 if path is a pipe,
   -4 if path is a socket. */
virtual_ftype_t
fhandler_process::exists ()
{
  const char *path = get_name ();
  debug_printf ("exists (%s)", path);
  path += proc_len + 1;
  while (*path != 0 && !isdirsep (*path))
    path++;
  if (*path == 0)
    return virt_rootdir;

  virt_tab_t *entry = virt_tab_search (path + 1, true, process_tab,
				       PROCESS_LINK_COUNT);
  if (entry)
    {
      if (!path[entry->name_len + 1])
	{
	  fileid = entry - process_tab;
	  return entry->type;
	}
      if (entry->type == virt_directory)
	{
	  fileid = entry - process_tab;
	  if (fill_filebuf ())
	    return virt_symlink;
	  /* Check for nameless device entries. */
	  path = strrchr (path, '/');
	  if (path && *++path)
	    {
	      if (!strncmp (path, "pipe:[", 6))
		return virt_pipe;
	      else if (!strncmp (path, "socket:[", 8))
		return virt_socket;
	    }
	}
    }
  return virt_none;
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
  /* If p->pid != pid, then pid is actually the Windows PID for an execed
     Cygwin process, and the pinfo entry is the additional entry created
     at exec time.  We don't want to enable the user to access a process
     entry by using the Win32 PID, though. */
  if (!p || p->pid != pid)
    {
      set_errno (ENOENT);
      return -1;
    }

  buf->st_mode &= ~_IFMT & NO_W;

  switch (file_type)
    {
    case virt_none:
      set_errno (ENOENT);
      return -1;
    case virt_directory:
    case virt_rootdir:
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
    case virt_symlink:
      buf->st_uid = p->uid;
      buf->st_gid = p->gid;
      buf->st_mode = S_IFLNK | S_IRWXU | S_IRWXG | S_IRWXO;
      return 0;
    case virt_pipe:
      buf->st_uid = p->uid;
      buf->st_gid = p->gid;
      buf->st_mode = S_IFIFO | S_IRUSR | S_IWUSR;
      return 0;
    case virt_socket:
      buf->st_uid = p->uid;
      buf->st_gid = p->gid;
      buf->st_mode = S_IFSOCK | S_IRUSR | S_IWUSR;
      return 0;
    case virt_file:
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
  if (dir && process_tab[fileid].fhandler == FH_PROCESSFD)
    fill_filebuf ();
  return dir;
}

int
fhandler_process::closedir (DIR *dir)
{
  return fhandler_virtual::closedir (dir);
}

int
fhandler_process::readdir (DIR *dir, dirent *de)
{
  int res = ENMFILE;
  if (process_tab[fileid].fhandler == FH_PROCESSFD)
    {
      if (dir->__d_position >= 2 + filesize / sizeof (int))
	goto out;
    }
  else if (dir->__d_position >= PROCESS_LINK_COUNT)
    goto out;
  if (process_tab[fileid].fhandler == FH_PROCESSFD && dir->__d_position > 1)
    {
      int *p = (int *) filebuf;
      __small_sprintf (de->d_name, "%d", p[dir->__d_position++ - 2]);
    }
  else
    strcpy (de->d_name, process_tab[dir->__d_position++].name);
  dir->__flags |= dirent_saw_dot | dirent_saw_dot_dot;
  res = 0;
out:
  syscall_printf ("%d = readdir(%p, %p) (%s)", res, dir, de, de->d_name);
  return res;
}

int
fhandler_process::open (int flags, mode_t mode)
{
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

  virt_tab_t *entry;
  entry = virt_tab_search (path + 1, true, process_tab, PROCESS_LINK_COUNT);
  if (!entry)
    {
      set_errno ((flags & O_CREAT) ? EROFS : ENOENT);
      res = 0;
      goto out;
    }
  if (entry->fhandler == FH_PROCESSFD)
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

  fileid = entry - process_tab;
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
  syscall_printf ("%d = fhandler_proc::open(%p, %d)", res, flags, mode);
  return res;
}

struct process_fd_t {
  const char *path;
  _pinfo *p;
};

bool
fhandler_process::fill_filebuf ()
{
  const char *path;
  path = get_name () + proc_len + 1;
  if (!pid)
    pid = atoi (path);

  pinfo p (pid);
  /* If p->pid != pid, then pid is actually the Windows PID for an execed
     Cygwin process, and the pinfo entry is the additional entry created
     at exec time.  We don't want to enable the user to access a process
     entry by using the Win32 PID, though. */
  if (!p || p->pid != pid)
    {
      set_errno (ENOENT);
      return false;
    }

  if (process_tab[fileid].format_func)
    {
      if (process_tab[fileid].fhandler == FH_PROCESSFD)
	{
	  process_fd_t fd = { path, p };
	  filesize = process_tab[fileid].format_func (&fd, filebuf);
	}
      else
	filesize = process_tab[fileid].format_func (p, filebuf);
      return !filesize ? false : true;
    }
  return false;
}

static _off64_t
format_process_fd (void *data, char *&destbuf)
{
  _pinfo *p = ((process_fd_t *) data)->p;
  const char *path = ((process_fd_t *) data)->path;
  size_t fs = 0;
  char *fdp = strrchr (path, '/');

  if (!fdp || *++fdp == 'f') /* The "fd" directory itself. */
    {
      if (destbuf)
	cfree (destbuf);
      destbuf = p->fds (fs);
    }
  else
    {
      if (destbuf)
	cfree (destbuf);
      int fd = atoi (fdp);
      if (fd < 0 || (fd == 0 && !isdigit (*fdp)))
	{
	  set_errno (ENOENT);
	  return 0;
	}
      destbuf = p->fd (fd, fs);
      if (!destbuf || !*destbuf)
	{
	  set_errno (ENOENT);
	  return 0;
	}
    }
  return fs;
}

static _off64_t
format_process_ppid (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  destbuf = (char *) crealloc_abort (destbuf, 40);
  return __small_sprintf (destbuf, "%d\n", p->ppid);
}

static _off64_t
format_process_uid (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  destbuf = (char *) crealloc_abort (destbuf, 40);
  return __small_sprintf (destbuf, "%d\n", p->uid);
}

static _off64_t
format_process_pgid (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  destbuf = (char *) crealloc_abort (destbuf, 40);
  return __small_sprintf (destbuf, "%d\n", p->pgid);
}

static _off64_t
format_process_sid (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  destbuf = (char *) crealloc_abort (destbuf, 40);
  return __small_sprintf (destbuf, "%d\n", p->sid);
}

static _off64_t
format_process_gid (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  destbuf = (char *) crealloc_abort (destbuf, 40);
  return __small_sprintf (destbuf, "%d\n", p->gid);
}

static _off64_t
format_process_ctty (void *data, char *&destbuf)
{
  device d;
  _pinfo *p = (_pinfo *) data;
  d.parse (p->ctty);
  destbuf = (char *) crealloc_abort (destbuf, strlen (d.name) + 2);
  return __small_sprintf (destbuf, "%s\n", d.name);
}

static _off64_t
format_process_root (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  size_t fs;

  if (destbuf)
    {
      cfree (destbuf);
      destbuf = NULL;
    }
  destbuf = p->root (fs);
  if (!destbuf || !*destbuf)
    {
      destbuf = cstrdup ("<defunct>");
      fs = strlen (destbuf) + 1;
    }
  return fs;
}

static _off64_t
format_process_cwd (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  size_t fs;

  if (destbuf)
    {
      cfree (destbuf);
      destbuf = NULL;
    }
  destbuf = p->cwd (fs);
  if (!destbuf || !*destbuf)
    {
      destbuf = cstrdup ("<defunct>");
      fs = strlen (destbuf) + 1;
    }
  return fs;
}

static _off64_t
format_process_cmdline (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  size_t fs;

  if (destbuf)
    {
      cfree (destbuf);
      destbuf = NULL;
    }
  destbuf = p->cmdline (fs);
  if (!destbuf || !*destbuf)
    {
      destbuf = cstrdup ("<defunct>");
      fs = strlen (destbuf) + 1;
    }
  return fs;
}

static _off64_t
format_process_exename (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  int len;
  tmp_pathbuf tp;

  char *buf = tp.c_get ();
  if (p->process_state & PID_EXITED)
    stpcpy (buf, "<defunct>");
  else
    {
      mount_table->conv_to_posix_path (p->progname, buf, 1);
      len = strlen (buf);
      if (len > 4)
	{
	  char *s = buf + len - 4;
	  if (ascii_strcasematch (s, ".exe"))
	    *s = 0;
	}
    }
  destbuf = (char *) crealloc_abort (destbuf, (len = strlen (buf)) + 1);
  stpcpy (destbuf, buf);
  return len;
}

static _off64_t
format_process_winpid (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  destbuf = (char *) crealloc_abort (destbuf, 20);
  return __small_sprintf (destbuf, "%d\n", p->dwProcessId);
}

static _off64_t
format_process_winexename (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  size_t len = sys_wcstombs (NULL, 0, p->progname);
  destbuf = (char *) crealloc_abort (destbuf, len + 1);
  sys_wcstombs (destbuf, len, p->progname);
  destbuf[len] = '\n';
  return len + 1;
}

static bool
get_volume_path_names_for_volume_name (LPCWSTR vol, LPWSTR mounts)
{
  DWORD len;
  if (GetVolumePathNamesForVolumeNameW (vol, mounts, NT_MAX_PATH, &len))
    return true;

  /* Windows 2000 doesn't have GetVolumePathNamesForVolumeNameW.
     Just assume that mount points are not longer than MAX_PATH. */
  WCHAR drives[MAX_PATH], dvol[MAX_PATH], mp[MAX_PATH + 3];
  if (!GetLogicalDriveStringsW (MAX_PATH, drives))
    return false;
  for (PWCHAR drive = drives; *drive; drive = wcschr (drive, '\0') + 1)
    {
      if (!GetVolumeNameForVolumeMountPointW (drive, dvol, MAX_PATH))
	continue;
      if (!wcscasecmp (vol, dvol))
	mounts = wcpcpy (mounts, drive) + 1;
      wcscpy (mp, drive);
      HANDLE h = FindFirstVolumeMountPointW (dvol, mp + 3, MAX_PATH);
      if (h == INVALID_HANDLE_VALUE)
	continue;
      do
	{
	  if (GetVolumeNameForVolumeMountPointW (mp, dvol, MAX_PATH))
	    if (!wcscasecmp (vol, dvol))
	      mounts = wcpcpy (mounts, drive) + 1;
	}
      while (FindNextVolumeMountPointW (h, mp, MAX_PATH));
      FindVolumeMountPointClose (h);
    }
  *mounts = L'\0';
  return true;
}

struct dos_drive_mappings
{
  struct mapping
  {
    mapping *next;
    size_t doslen;
    size_t ntlen;
    wchar_t *dospath;
    wchar_t *ntdevpath;
  };
  mapping *mappings;

  dos_drive_mappings ()
    : mappings(0)
  {
    tmp_pathbuf tp;
    wchar_t vol[64]; /* Long enough for Volume GUID string */
    wchar_t *devpath = tp.w_get ();
    wchar_t *mounts = tp.w_get ();

    /* Iterate over all volumes, fetch the first path from the list of
       DOS paths the volume is mounted to, or use the GUID volume path
       otherwise. */
    HANDLE sh = FindFirstVolumeW (vol, 64);
    if (sh == INVALID_HANDLE_VALUE)
      debug_printf ("FindFirstVolumeW, %E");
    else
      do
	{
	  /* Skip drives which are not mounted. */
	  if (!get_volume_path_names_for_volume_name (vol, mounts)
	      || mounts[0] == L'\0')
	    continue;
	  *wcsrchr (vol, L'\\') = L'\0';
	  if (QueryDosDeviceW (vol + 4, devpath, NT_MAX_PATH))
	    {
	      /* The DOS drive mapping can be another symbolic link.  If so,
		 the mapping won't work since the section name is the name
		 after resolving all symlinks.  Resolve symlinks here, too. */
	      for (int syml_cnt = 0; syml_cnt < SYMLOOP_MAX; ++syml_cnt)
		{
		  UNICODE_STRING upath;
		  OBJECT_ATTRIBUTES attr;
		  NTSTATUS status;
		  HANDLE h;

		  RtlInitUnicodeString (&upath, devpath);
		  InitializeObjectAttributes (&attr, &upath,
					      OBJ_CASE_INSENSITIVE, NULL, NULL);
		  status = NtOpenSymbolicLinkObject (&h, SYMBOLIC_LINK_QUERY,
						     &attr);
		  if (!NT_SUCCESS (status))
		    break;
		  RtlInitEmptyUnicodeString (&upath, devpath, (NT_MAX_PATH - 1)
							      * sizeof (WCHAR));
		  status = NtQuerySymbolicLinkObject (h, &upath, NULL);
		  NtClose (h);
		  if (!NT_SUCCESS (status))
		    break;
		  devpath[upath.Length / sizeof (WCHAR)] = L'\0';
		}
	      mapping *m = new mapping ();
	      if (m)
		{
		  m->dospath = wcsdup (mounts);
		  m->ntdevpath = wcsdup (devpath);
		  if (!m->dospath || !m->ntdevpath)
		    {
		      free (m->dospath);
		      free (m->ntdevpath);
		      delete m;
		      continue;
		    }
		  m->doslen = wcslen (m->dospath);
		  m->dospath[--m->doslen] = L'\0'; /* Drop trailing backslash */
		  m->ntlen = wcslen (m->ntdevpath);
		  m->next = mappings;
		  mappings = m;
		}
	    }
	  else
	    debug_printf ("Unable to determine the native mapping for %ls "
			  "(error %lu)", vol, GetLastError ());
	}
      while (FindNextVolumeW (sh, vol, 64));
      FindVolumeClose (sh);
  }

  wchar_t *fixup_if_match (wchar_t *path)
  {
    /* Check for network drive first. */
    if (!wcsncmp (path, L"\\Device\\Mup\\", 12))
      {
	path += 10;
	path[0] = L'\\';
	return path;
      }
    /* Then test local drives. */
    for (mapping *m = mappings; m; m = m->next)
      if (!wcsncmp (m->ntdevpath, path, m->ntlen))
	{
	  wchar_t *tmppath;

	  if (m->ntlen > m->doslen)
	    wcsncpy (path += m->ntlen - m->doslen, m->dospath, m->doslen);
	  else if ((tmppath = wcsdup (path + m->ntlen)) != NULL)
	    {
	      wcpcpy (wcpcpy (path, m->dospath), tmppath);
	      free (tmppath);
	    }
	  break;
	}
    return path;
  }

  ~dos_drive_mappings ()
  {
    mapping *n = 0;
    for (mapping *m = mappings; m; m = n)
      {
	n = m->next;
	free (m->dospath);
	free (m->ntdevpath);
	delete m;
      }
  }
};

struct heap_info
{
  struct heap
  {
    heap *next;
    unsigned heap_id;
    char *base;
    char *end;
    unsigned long flags;
  };
  heap *heap_vm_chunks;

  heap_info (DWORD pid)
    : heap_vm_chunks (NULL)
  {
    PDEBUG_BUFFER buf;
    NTSTATUS status;
    PDEBUG_HEAP_ARRAY harray;

    buf = RtlCreateQueryDebugBuffer (0, FALSE);
    if (!buf)
      return;
    status = RtlQueryProcessDebugInformation (pid, PDI_HEAPS | PDI_HEAP_BLOCKS,
					      buf);
    if (NT_SUCCESS (status)
	&& (harray = (PDEBUG_HEAP_ARRAY) buf->HeapInformation) != NULL)
      for (ULONG hcnt = 0; hcnt < harray->Count; ++hcnt)
	{
	  PDEBUG_HEAP_BLOCK barray = (PDEBUG_HEAP_BLOCK)
				     harray->Heaps[hcnt].Blocks;
	  if (!barray)
	    continue;
	  for (ULONG bcnt = 0; bcnt < harray->Heaps[hcnt].BlockCount; ++bcnt)
	    if (barray[bcnt].Flags & 2)
	      {
		heap *h = (heap *) malloc (sizeof (heap));
		if (h)
		  {
		    *h = (heap) { heap_vm_chunks,
				  hcnt, (char *) barray[bcnt].Address,
				  (char *) barray[bcnt].Address
					   + barray[bcnt].Size,
				  harray->Heaps[hcnt].Flags };
		    heap_vm_chunks = h;
		  }
	      }
	}
    RtlDestroyQueryDebugBuffer (buf);
  }

  char *fill_if_match (char *base, ULONG type, char *dest)
  {
    for (heap *h = heap_vm_chunks; h; h = h->next)
      if (base >= h->base && base < h->end)
	{
	  char *p = dest + __small_sprintf (dest, "[win heap %ld", h->heap_id);
	  if (!(h->flags & HEAP_FLAG_NONDEFAULT))
	    p = stpcpy (p, " default");
	  if ((h->flags & HEAP_FLAG_SHAREABLE) && (type & MEM_MAPPED))
	    p = stpcpy (p, " shared");
	  if (h->flags & HEAP_FLAG_EXECUTABLE)
	    p = stpcpy (p, " exec");
	  if (h->flags & HEAP_FLAG_GROWABLE)
	    p = stpcpy (p, " grow");
	  if (h->flags & HEAP_FLAG_NOSERIALIZE)
	    p = stpcpy (p, " noserial");
	  if (h->flags == HEAP_FLAG_DEBUGGED)
	    p = stpcpy (p, " debug");
	  stpcpy (p, "]");
	  return dest;
	}
    return 0;
  }

  ~heap_info ()
  {
    heap *n = 0;
    for (heap *m = heap_vm_chunks; m; m = n)
      {
	n = m->next;
	free (m);
      }
  }
};

struct thread_info
{
  struct region
  {
    region *next;
    ULONG thread_id;
    char *start;
    char *end;
    bool teb;
  };
  region *regions;

  thread_info (DWORD pid, HANDLE process)
    : regions (NULL)
  {
    NTSTATUS status;
    PVOID buf = NULL;
    size_t size = 50 * (sizeof (SYSTEM_PROCESSES)
			+ 16 * sizeof (SYSTEM_THREADS));
    PSYSTEM_PROCESSES proc;
    PSYSTEM_THREADS thread;

    do
      {
	buf = realloc (buf, size);
	status = NtQuerySystemInformation (SystemProcessesAndThreadsInformation,
					   buf, size, NULL);
	size <<= 1;
      }
    while (status == STATUS_INFO_LENGTH_MISMATCH);
    if (!NT_SUCCESS (status))
      {
	if (buf)
	  free (buf);
	debug_printf ("NtQuerySystemInformation, %p", status);
	return;
      }
    proc = (PSYSTEM_PROCESSES) buf;
    while (true)
      {
	if (proc->ProcessId == pid)
	  break;
	if (!proc->NextEntryDelta)
	  {
	    free (buf);
	    return;
	  }
	proc = (PSYSTEM_PROCESSES) ((PBYTE) proc + proc->NextEntryDelta);
      }
    thread = proc->Threads;
    for (ULONG i = 0; i < proc->ThreadCount; ++i)
      {
	THREAD_BASIC_INFORMATION tbi;
	TEB teb;
	HANDLE thread_h;

	if (!(thread_h = OpenThread (THREAD_QUERY_INFORMATION, FALSE,
				     (ULONG) thread[i].ClientId.UniqueThread)))
	  continue;
	status = NtQueryInformationThread (thread_h, ThreadBasicInformation,
					   &tbi, sizeof tbi, NULL);
	CloseHandle (thread_h);
	if (!NT_SUCCESS (status))
	  continue;
	region *r = (region *) malloc (sizeof (region));
	if (r)
	  {
	    *r = (region) { regions, (ULONG) thread[i].ClientId.UniqueThread,
			    (char *) tbi.TebBaseAddress,
			    (char *) tbi.TebBaseAddress + wincap.page_size (),
			    true };
	    regions = r;
	  }
	if (!ReadProcessMemory (process, (PVOID) tbi.TebBaseAddress,
				&teb, sizeof teb, NULL))
	  continue;
	r = (region *) malloc (sizeof (region));
	if (r)
	  {
	    *r = (region) { regions, (ULONG) thread[i].ClientId.UniqueThread,
			    (char *) (teb.DeallocationStack
				      ?: teb.Tib.StackLimit),
			    (char *) teb.Tib.StackBase,
			    false };
	    regions = r;
	  }
      }
    free (buf);
  }

  char *fill_if_match (char *base, ULONG type, char *dest)
  {
    for (region *r = regions; r; r = r->next)
      if ((base >= r->start && base < r->end)
	  /* Special case WOW64.  The TEB is 8K within the region reserved
	     for it.  No idea what the lower 8K are used for. */
	  || (r->teb && wincap.is_wow64 ()
	      && r->start == base + 2 * wincap.page_size ()))
	{
	  char *p = dest + __small_sprintf (dest, "[%s (tid %ld)",
					    r->teb ? "teb" : "stack",
					    r->thread_id);
	  if (type & MEM_MAPPED)
	    p = stpcpy (p, " shared");
	  stpcpy (p, "]");
	  return dest;
	}
    return 0;
  }

  ~thread_info ()
  {
    region *n = 0;
    for (region *m = regions; m; m = n)
      {
	n = m->next;
	free (m);
      }
  }
};

static _off64_t
format_process_maps (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  HANDLE proc = OpenProcess (PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
			     FALSE, p->dwProcessId);
  if (!proc)
    return 0;

  NTSTATUS status;
  PROCESS_BASIC_INFORMATION pbi;
  PPEB peb = NULL;

  memset (&pbi, 0, sizeof (pbi));
  status = NtQueryInformationProcess (proc, ProcessBasicInformation,
				      &pbi, sizeof pbi, NULL);
  if (NT_SUCCESS (status))
    peb = pbi.PebBaseAddress;
  /* myself is in the same spot in every process, so is the pointer to the
     procinfo.  But make sure the destructor doesn't try to release procinfo! */
  pinfo proc_pinfo;
  if (ReadProcessMemory (proc, &myself, &proc_pinfo, sizeof proc_pinfo, NULL))
    proc_pinfo.preserve ();
  /* The heap info on the cygheap is also in the same spot in each process
     because the cygheap is located at the same address. */
  user_heap_info user_heap;
  ReadProcessMemory (proc, &cygheap->user_heap, &user_heap,
		     sizeof user_heap, NULL);

  _off64_t len = 0;

  union access
  {
    char flags[8];
    _off64_t word;
  } a;

  struct region {
    access a;
    char *abase;
    char *rbase;
    char *rend;
  } cur = {{{'\0'}}, (char *)1, 0, 0};

  MEMORY_BASIC_INFORMATION mb;
  dos_drive_mappings drive_maps;
  heap_info heaps (p->dwProcessId);
  thread_info threads (p->dwProcessId, proc);
  struct __stat64 st;
  long last_pass = 0;

  tmp_pathbuf tp;
  PMEMORY_SECTION_NAME msi = (PMEMORY_SECTION_NAME) tp.w_get ();
  char *posix_modname = tp.c_get ();
  size_t maxsize = 0;

  if (destbuf)
    {
      cfree (destbuf);
      destbuf = NULL;
    }

  /* Iterate over each VM region in the address space, coalescing
     memory regions with the same permissions. Once we run out, do one
     last_pass to trigger output of the last accumulated region. */
  for (char *i = 0;
       VirtualQueryEx (proc, i, &mb, sizeof(mb)) || (1 == ++last_pass);
       i = cur.rend)
    {
      if (last_pass)
	posix_modname[0] = '\0';
      if (mb.State == MEM_FREE)
	a.word = 0;
      else if (mb.State == MEM_RESERVE)
	{
	  char *p = stpcpy (a.flags, "===");
	  stpcpy (p, (mb.Type & MEM_MAPPED) ? "s" : "p");
	}
      else
	{
	  static DWORD const RO = (PAGE_EXECUTE_READ | PAGE_READONLY);
	  static DWORD const RW = (PAGE_EXECUTE_READWRITE | PAGE_READWRITE
				   | PAGE_EXECUTE_WRITECOPY | PAGE_WRITECOPY);
	  static DWORD const X = (PAGE_EXECUTE | PAGE_EXECUTE_READ
				  | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);
	  static DWORD const WC = (PAGE_EXECUTE_WRITECOPY | PAGE_WRITECOPY);
	  DWORD p = mb.Protect;
	  a = (access) {{
	      (p & (RO | RW))				? 'r' : '-',
	      (p & (RW))				? 'w' : '-',
	      (p & (X))					? 'x' : '-',
	      (mb.Type & MEM_MAPPED) && !(p & (WC))	? 's'
	      : (p & PAGE_GUARD)			? 'g' : 'p',
	      '\0', // zero-fill the remaining bytes
	    }};
	}

      region next = { a,
		      (char *) mb.AllocationBase,
		      (char *) mb.BaseAddress,
		      (char *) mb.BaseAddress+mb.RegionSize
      };

      /* Windows permissions are more fine-grained than the unix rwxp,
	 so we reduce clutter by manually coalescing regions sharing
	 the same allocation base and effective permissions. */
      bool newbase = (next.abase != cur.abase);
      if (!last_pass && !newbase && next.a.word == cur.a.word)
	  cur.rend = next.rend; /* merge with previous */
      else
	{
	  /* output the current region if it's "interesting". */
	  if (cur.a.word)
	    {
	      size_t newlen = strlen (posix_modname) + 62;
	      if (len + newlen >= maxsize)
		destbuf = (char *) crealloc_abort (destbuf,
						   maxsize += roundup2 (newlen,
									2048));
	      int written = __small_sprintf (destbuf + len,
					     "%08lx-%08lx %s %08lx %04x:%04x %U   ",
					     cur.rbase, cur.rend, cur.a.flags,
					     cur.rbase - cur.abase,
					     st.st_dev >> 16,
					     st.st_dev & 0xffff,
					     st.st_ino);
	      while (written < 62)
		destbuf[len + written++] = ' ';
	      len += written;
	      len += __small_sprintf (destbuf + len, "%s\n", posix_modname);
	    }
	  /* start of a new region (but possibly still the same allocation). */
	  cur = next;
	  /* if a new allocation, figure out what kind it is. */
	  if (newbase && !last_pass && mb.State != MEM_FREE)
	    {
	      /* If the return length pointer is missing, NtQueryVirtualMemory
		 returns with STATUS_ACCESS_VIOLATION on Windows 2000. */
	      ULONG ret_len = 0;

	      st.st_dev = 0;
	      st.st_ino = 0;
	      if ((mb.Type & (MEM_MAPPED | MEM_IMAGE))
		  && NT_SUCCESS (status = NtQueryVirtualMemory (proc, cur.abase,
						       MemorySectionName,
						       msi, 65536, &ret_len)))
		{
		  PWCHAR dosname =
		      drive_maps.fixup_if_match (msi->SectionFileName.Buffer);
		  if (mount_table->conv_to_posix_path (dosname,
						       posix_modname, 0))
		    sys_wcstombs (posix_modname, NT_MAX_PATH, dosname);
		  stat64 (posix_modname, &st);
		}
	      else if (!threads.fill_if_match (cur.abase, mb.Type,
					       posix_modname)
		       && !heaps.fill_if_match (cur.abase, mb.Type,
						posix_modname))
		{
		  if (cur.abase == (char *) peb)
		    strcpy (posix_modname, "[peb]");
		  else if (cur.abase == (char *) &SharedUserData)
		    strcpy (posix_modname, "[shared-user-data]");
		  else if (cur.abase == (char *) cygwin_shared)
		    strcpy (posix_modname, "[cygwin-shared]");
		  else if (cur.abase == (char *) user_shared)
		    strcpy (posix_modname, "[cygwin-user-shared]");
		  else if (cur.abase == (char *) *proc_pinfo)
		    strcpy (posix_modname, "[procinfo]");
		  else if (cur.abase == user_heap.base)
		    strcpy (posix_modname, "[heap]");
		  else
		    posix_modname[0] = 0;
		}
	    }
	}
    }
  CloseHandle (proc);
  return len;
}

static _off64_t
format_process_stat (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  char cmd[NAME_MAX + 1];
  WCHAR wcmd[NAME_MAX + 1];
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
      PWCHAR last_slash = wcsrchr (p->progname, L'\\');
      wcscpy (wcmd, last_slash ? last_slash + 1 : p->progname);
      sys_wcstombs (cmd, NAME_MAX + 1, wcmd);
      int len = strlen (cmd);
      if (len > 4)
	{
	  char *s = cmd + len - 4;
	  if (ascii_strcasematch (s, ".exe"))
	    *s = 0;
	 }
    }
  /* Note: under Windows, a process is always running - it's only threads
     that get suspended.  Therefore the default state is R (runnable). */
  if (p->process_state & PID_EXITED)
    state = 'Z';
  else if (p->process_state & PID_STOPPED)
    state = 'T';
  else
    state = get_process_state (p->dwProcessId);
  start_time = (GetTickCount () / 1000 - time (NULL) + p->start_time) * HZ;

  NTSTATUS status;
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
      status = NtQueryInformationProcess (hProcess, ProcessVmCounters,
					  (PVOID) &vmc, sizeof vmc, NULL);
      if (NT_SUCCESS (status))
	status = NtQueryInformationProcess (hProcess, ProcessTimes,
					    (PVOID) &put, sizeof put, NULL);
      if (NT_SUCCESS (status))
	status = NtQueryInformationProcess (hProcess, ProcessBasicInformation,
					    (PVOID) &pbi, sizeof pbi, NULL);
      if (NT_SUCCESS (status))
	status = NtQueryInformationProcess (hProcess, ProcessQuotaLimits,
					    (PVOID) &ql, sizeof ql, NULL);
      CloseHandle (hProcess);
    }
  else
    {
      DWORD error = GetLastError ();
      __seterrno_from_win_error (error);
      debug_printf ("OpenProcess: ret %d", error);
      return 0;
    }
  if (NT_SUCCESS (status))
    status = NtQuerySystemInformation (SystemTimeOfDayInformation,
				       (PVOID) &stodi, sizeof stodi, NULL);
  if (NT_SUCCESS (status))
    status = NtQuerySystemInformation (SystemProcessorTimes, (PVOID) &spt,
				       sizeof spt, NULL);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      debug_printf ("NtQueryInformationProcess: status %p, %E", status);
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
  unsigned page_size = wincap.page_size ();
  vmsize = vmc.PagefileUsage;
  vmrss = vmc.WorkingSetSize / page_size;
  vmmaxrss = ql.MaximumWorkingSetSize / page_size;

  destbuf = (char *) crealloc_abort (destbuf, strlen (cmd) + 320);
  return __small_sprintf (destbuf, "%d (%s) %c "
				   "%d %d %d %d %d "
				   "%lu %lu %lu %lu %lu %lu %lu "
				   "%ld %ld %ld %ld %ld %ld "
				   "%lu %lu "
				   "%ld "
				   "%lu",
			  p->pid, cmd,
			  state,
			  p->ppid, p->pgid, p->sid, p->ctty,
			  -1, 0, fault_count, fault_count, 0, 0, utime, stime,
			  utime, stime, priority, 0, 0, 0,
			  start_time, vmsize,
			  vmrss, vmmaxrss
			  );
}

static _off64_t
format_process_status (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  char cmd[NAME_MAX + 1];
  WCHAR wcmd[NAME_MAX + 1];
  int state = 'R';
  const char *state_str = "unknown";
  unsigned long vmsize = 0UL, vmrss = 0UL, vmdata = 0UL, vmlib = 0UL, vmtext = 0UL,
		vmshare = 0UL;
  PWCHAR last_slash = wcsrchr (p->progname, L'\\');
  wcscpy (wcmd, last_slash ? last_slash + 1 : p->progname);
  sys_wcstombs (cmd, NAME_MAX + 1, wcmd);
  int len = strlen (cmd);
  if (len > 4)
    {
      char *s = cmd + len - 4;
      if (ascii_strcasematch (s, ".exe"))
	*s = 0;
     }
  /* Note: under Windows, a process is always running - it's only threads
     that get suspended.  Therefore the default state is R (runnable). */
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
  unsigned page_size = wincap.page_size ();
  vmsize *= page_size; vmrss *= page_size; vmdata *= page_size;
  vmtext *= page_size; vmlib *= page_size;
  /* The real uid value for *this* process is stored at cygheap->user.real_uid
     but we can't get at the real uid value for any other process, so
     just fake it as p->uid.  Similar for p->gid. */
  destbuf = (char *) crealloc_abort (destbuf, strlen (cmd) + 320);
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
			  0, 0, _my_tls.sigmask
			  );
}

static _off64_t
format_process_statm (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  unsigned long vmsize = 0UL, vmrss = 0UL, vmtext = 0UL, vmdata = 0UL,
		vmlib = 0UL, vmshare = 0UL;
  if (!get_mem_values (p->dwProcessId, &vmsize, &vmrss, &vmtext, &vmdata,
		       &vmlib, &vmshare))
    return 0;
  destbuf = (char *) crealloc_abort (destbuf, 96);
  return __small_sprintf (destbuf, "%ld %ld %ld %ld %ld %ld %ld",
			  vmsize, vmrss, vmshare, vmtext, vmlib, vmdata, 0);
}

extern "C" {
  FILE *setmntent (const char *, const char *);
  struct mntent *getmntent (FILE *);
};

static _off64_t
format_process_mounts (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  user_info *u_shared = NULL;
  HANDLE u_hdl = NULL;
  _off64_t len = 0;
  struct mntent *mnt;

  if (p->pid != myself->pid)
    {
      WCHAR sid_string[UNLEN + 1] = L""; /* Large enough for SID */

      cygsid p_sid;

      if (!p_sid.getfrompw (internal_getpwuid (p->uid)))
	return 0;
      p_sid.string (sid_string);
      u_shared = (user_info *) open_shared (sid_string, USER_VERSION, u_hdl,
					    sizeof (user_info), SH_JUSTOPEN,
					    &sec_none_nih);
      if (!u_shared)
	return 0;
    }
  else
    u_shared = user_shared;

  /* Store old value of _my_tls.locals here. */
  int iteration = _my_tls.locals.iteration;
  unsigned available_drives = _my_tls.locals.available_drives;
  /* This reinitializes the above values in _my_tls. */
  setmntent (NULL, NULL);
  while ((mnt = getmntent (NULL)))
    {
      destbuf = (char *) crealloc_abort (destbuf, len
						  + strlen (mnt->mnt_fsname)
						  + strlen (mnt->mnt_dir)
						  + strlen (mnt->mnt_type)
						  + strlen (mnt->mnt_opts)
						  + 28);
      len += __small_sprintf (destbuf + len, "%s %s %s %s %d %d\n",
			      mnt->mnt_fsname, mnt->mnt_dir, mnt->mnt_type,
			      mnt->mnt_opts, mnt->mnt_freq, mnt->mnt_passno);
    }
  /* Restore old value of _my_tls.locals here. */
  _my_tls.locals.iteration = iteration;
  _my_tls.locals.available_drives = available_drives;

  if (u_hdl) /* Only not-NULL if open_shared has been called. */
    {
      UnmapViewOfFile (u_shared);
      CloseHandle (u_hdl);
    }
  return len;
}

int
get_process_state (DWORD dwProcessId)
{
  /* This isn't really heavy magic - just go through the processes' threads
     one by one and return a value accordingly.  Errors are silently ignored. */
  NTSTATUS status;
  PSYSTEM_PROCESSES p, sp;
  ULONG n = 0x4000;
  int state =' ';

  p = (PSYSTEM_PROCESSES) malloc (n);
  if (!p)
    return state;
  while (true)
    {
      status = NtQuerySystemInformation (SystemProcessesAndThreadsInformation,
					 (PVOID) p, n, NULL);
      if (status != STATUS_INFO_LENGTH_MISMATCH)
	break;
      n <<= 1;
      PSYSTEM_PROCESSES new_p = (PSYSTEM_PROCESSES) realloc (p, n);
      if (!new_p)
      	goto out;
      p = new_p;
    }
  if (!NT_SUCCESS (status))
    {
      debug_printf ("NtQuerySystemInformation: status %p, %lu",
		    status, RtlNtStatusToDosError (status));
      goto out;
    }
  state = 'Z';
  sp = p;
  for (;;)
    {
      if (sp->ProcessId == dwProcessId)
	{
	  SYSTEM_THREADS *st;
	  st = &sp->Threads[0];
	  state = 'S';
	  for (unsigned i = 0; i < sp->ThreadCount; i++)
	    {
	      /* FIXME: at some point we should consider generating 'O' */
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
      sp = (PSYSTEM_PROCESSES) ((char *) sp + sp->NextEntryDelta);
    }
out:
  free (p);
  return state;
}

static bool
get_mem_values (DWORD dwProcessId, unsigned long *vmsize, unsigned long *vmrss,
		unsigned long *vmtext, unsigned long *vmdata,
		unsigned long *vmlib, unsigned long *vmshare)
{
  bool res = false;
  NTSTATUS status;
  HANDLE hProcess;
  VM_COUNTERS vmc;
  PMEMORY_WORKING_SET_LIST p;
  ULONG n = 0x4000, length;

  p = (PMEMORY_WORKING_SET_LIST) malloc (n);
  if (!p)
    return false;
  hProcess = OpenProcess (PROCESS_QUERY_INFORMATION, FALSE, dwProcessId);
  if (hProcess == NULL)
    {
      __seterrno ();
      debug_printf ("OpenProcess, %E");
      return false;
    }
  while (true)
    {
      status = NtQueryVirtualMemory (hProcess, 0, MemoryWorkingSetList,
				     (PVOID) p, n,
				     (length = ULONG_MAX, &length));
      if (status != STATUS_INFO_LENGTH_MISMATCH)
	break;
      n <<= 1;
      PMEMORY_WORKING_SET_LIST new_p = (PMEMORY_WORKING_SET_LIST)
				       realloc (p, n);
      if (!new_p)
	goto out;
      p = new_p;
    }
  if (!NT_SUCCESS (status))
    {
      debug_printf ("NtQueryVirtualMemory: status %p", status);
      if (status == STATUS_PROCESS_IS_TERMINATING)
	{
	  *vmsize = *vmrss = *vmtext = *vmdata = *vmlib = *vmshare = 0;
	  res = true;
	}
      else
	__seterrno_from_nt_status (status);
      goto out;
    }
  for (unsigned long i = 0; i < p->NumberOfPages; i++)
    {
      ++*vmrss;
      unsigned flags = p->WorkingSetList[i] & 0x0FFF;
      if ((flags & (WSLE_PAGE_EXECUTE | WSLE_PAGE_SHAREABLE))
	  == (WSLE_PAGE_EXECUTE | WSLE_PAGE_SHAREABLE))
	++*vmlib;
      else if (flags & WSLE_PAGE_SHAREABLE)
	++*vmshare;
      else if (flags & WSLE_PAGE_EXECUTE)
	++*vmtext;
      else
	++*vmdata;
    }
  status = NtQueryInformationProcess (hProcess, ProcessVmCounters, (PVOID) &vmc,
				      sizeof vmc, NULL);
  if (!NT_SUCCESS (status))
    {
      debug_printf ("NtQueryInformationProcess: status %p", status);
      __seterrno_from_nt_status (status);
      goto out;
    }
  *vmsize = vmc.PagefileUsage / wincap.page_size ();
  res = true;
out:
  free (p);
  CloseHandle (hProcess);
  return res;
}

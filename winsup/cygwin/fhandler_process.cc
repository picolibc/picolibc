/* fhandler_process.cc: fhandler for /proc/<pid> virtual filesystem

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
#include "mount.h"
#include "tls_pbuf.h"
#include <sys/sysmacros.h>
#include <sys/param.h>
#include <ctype.h>

#define _COMPILING_NEWLIB
#include <dirent.h>

static off_t format_process_maps (void *, char *&);
static off_t format_process_stat (void *, char *&);
static off_t format_process_status (void *, char *&);
static off_t format_process_statm (void *, char *&);
static off_t format_process_winexename (void *, char *&);
static off_t format_process_winpid (void *, char *&);
static off_t format_process_exename (void *, char *&);
static off_t format_process_root (void *, char *&);
static off_t format_process_cwd (void *, char *&);
static off_t format_process_cmdline (void *, char *&);
static off_t format_process_ppid (void *, char *&);
static off_t format_process_uid (void *, char *&);
static off_t format_process_pgid (void *, char *&);
static off_t format_process_sid (void *, char *&);
static off_t format_process_gid (void *, char *&);
static off_t format_process_ctty (void *, char *&);
static off_t format_process_fd (void *, char *&);
static off_t format_process_mounts (void *, char *&);
static off_t format_process_mountinfo (void *, char *&);
static off_t format_process_environ (void *, char *&);

static const virt_tab_t process_tab[] =
{
  { _VN ("."),          FH_PROCESS,   virt_directory, NULL },
  { _VN (".."),         FH_PROCESS,   virt_directory, NULL },
  { _VN ("cmdline"),    FH_PROCESS,   virt_file,      format_process_cmdline },
  { _VN ("ctty"),       FH_PROCESS,   virt_file,      format_process_ctty },
  { _VN ("cwd"),        FH_PROCESS,   virt_symlink,   format_process_cwd },
  { _VN ("environ"),    FH_PROCESS,   virt_file,      format_process_environ },
  { _VN ("exe"),        FH_PROCESS,   virt_symlink,   format_process_exename },
  { _VN ("exename"),    FH_PROCESS,   virt_file,      format_process_exename },
  { _VN ("fd"),         FH_PROCESSFD, virt_directory, format_process_fd },
  { _VN ("gid"),        FH_PROCESS,   virt_file,      format_process_gid },
  { _VN ("maps"),       FH_PROCESS,   virt_file,      format_process_maps },
  { _VN ("mountinfo"),  FH_PROCESS,   virt_file,      format_process_mountinfo },
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
  { NULL, 0,	        FH_NADA,      virt_none,      NULL }
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
      if (entry->type == virt_directory)	/* fd subdir only */
	{
	  fileid = entry - process_tab;
	  if (fill_filebuf ())
	    return fd_type;
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

int __reg2
fhandler_process::fstat (struct stat *buf)
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
      if ((size_t) dir->__d_position >= 2 + filesize / sizeof (int))
	goto out;
    }
  else if (dir->__d_position >= PROCESS_LINK_COUNT)
    goto out;
  if (process_tab[fileid].fhandler == FH_PROCESSFD && dir->__d_position > 1)
    {
      int *p = (int *) filebuf;
      __small_sprintf (de->d_name, "%d", p[dir->__d_position++ - 2]);
      de->d_type = DT_LNK;
    }
  else
    {
      strcpy (de->d_name, process_tab[dir->__d_position].name);
      de->d_type = virt_ftype_to_dtype (process_tab[dir->__d_position].type);
      dir->__d_position++;
    }
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
  syscall_printf ("%d = fhandler_proc::open(%y, 0%o)", res, flags, mode);
  return res;
}

struct process_fd_t {
  const char *path;
  _pinfo *p;
  virtual_ftype_t *fd_type;
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
	  process_fd_t fd = { path, p , &fd_type };
	  filesize = process_tab[fileid].format_func (&fd, filebuf);
	}
      else
	filesize = process_tab[fileid].format_func (p, filebuf);
      return !filesize ? false : true;
    }
  return false;
}

static off_t
format_process_fd (void *data, char *&destbuf)
{
  _pinfo *p = ((process_fd_t *) data)->p;
  const char *path = ((process_fd_t *) data)->path;
  size_t fs = 0;
  /* path looks like "$PID/fd", "$PID/fd/", "$PID/fd/[0-9]*".  In the latter
     case a trailing slash and more followup chars are allowed, provided the
     descriptor symlink points to a directory. */
  char *fdp = strchr (path, '/') + 3;
  /* The "fd" directory itself? */
  if (fdp[0] =='\0' || (fdp[0] == '/' && fdp[1] == '\0'))
    {
      if (destbuf)
	cfree (destbuf);
      destbuf = p ? p->fds (fs) : NULL;
      *((process_fd_t *) data)->fd_type = virt_symlink;
    }
  else
    {
      char *e;
      int fd;

      if (destbuf)
	cfree (destbuf);
      fd = strtol (++fdp, &e, 10);
      if (fd < 0 || e == fdp || (*e != '/' && *e != '\0'))
	{
	  set_errno (ENOENT);
	  return 0;
	}
      destbuf = p ? p->fd (fd, fs) : NULL;
      if (!destbuf || !*destbuf)
	{
	  set_errno (ENOENT);
	  return 0;
	}
      if (*e == '\0')
	*((process_fd_t *) data)->fd_type = virt_symlink;
      else /* trailing path */
	{
	  char *newbuf = (char *) cmalloc_abort (HEAP_STR, strlen (destbuf)
							   + strlen (e) + 1);
	  stpcpy (stpcpy (newbuf, destbuf), e);
	  cfree (destbuf);
	  destbuf = newbuf;
	  *((process_fd_t *) data)->fd_type = virt_fsdir;
	}
    }
  return fs;
}

static off_t
format_process_ppid (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  destbuf = (char *) crealloc_abort (destbuf, 40);
  return __small_sprintf (destbuf, "%d\n", p->ppid);
}

static off_t
format_process_uid (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  destbuf = (char *) crealloc_abort (destbuf, 40);
  return __small_sprintf (destbuf, "%d\n", p->uid);
}

static off_t
format_process_pgid (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  destbuf = (char *) crealloc_abort (destbuf, 40);
  return __small_sprintf (destbuf, "%d\n", p->pgid);
}

static off_t
format_process_sid (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  destbuf = (char *) crealloc_abort (destbuf, 40);
  return __small_sprintf (destbuf, "%d\n", p->sid);
}

static off_t
format_process_gid (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  destbuf = (char *) crealloc_abort (destbuf, 40);
  return __small_sprintf (destbuf, "%d\n", p->gid);
}

static off_t
format_process_ctty (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  if (p->ctty < 0)
    {
      destbuf = (char *) crealloc_abort (destbuf, 2);
      return __small_sprintf (destbuf, "\n");
    }
  device d;
  d.parse (p->ctty);
  destbuf = (char *) crealloc_abort (destbuf, strlen (d.name ()) + 2);
  return __small_sprintf (destbuf, "%s\n", d.name ());
}

static off_t
format_process_root (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  size_t fs;

  if (destbuf)
    {
      cfree (destbuf);
      destbuf = NULL;
    }
  destbuf = p ? p->root (fs) : NULL;
  if (!destbuf || !*destbuf)
    {
      destbuf = cstrdup ("<defunct>");
      fs = strlen (destbuf) + 1;
    }
  return fs;
}

static off_t
format_process_cwd (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  size_t fs;

  if (destbuf)
    {
      cfree (destbuf);
      destbuf = NULL;
    }
  destbuf = p ? p->cwd (fs) : NULL;
  if (!destbuf || !*destbuf)
    {
      destbuf = cstrdup ("<defunct>");
      fs = strlen (destbuf) + 1;
    }
  return fs;
}

static off_t
format_process_cmdline (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  size_t fs;

  if (destbuf)
    {
      cfree (destbuf);
      destbuf = NULL;
    }
  destbuf = p ? p->cmdline (fs) : NULL;
  if (!destbuf || !*destbuf)
    {
      destbuf = cstrdup ("<defunct>");
      fs = strlen (destbuf) + 1;
    }
  return fs;
}

static off_t
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
      mount_table->conv_to_posix_path (p->progname, buf, CCP_RELATIVE);
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

static off_t
format_process_winpid (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  destbuf = (char *) crealloc_abort (destbuf, 20);
  return __small_sprintf (destbuf, "%d\n", p->dwProcessId);
}

static off_t
format_process_winexename (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  size_t len = sys_wcstombs (NULL, 0, p->progname);
  destbuf = (char *) crealloc_abort (destbuf, len + 1);
  /* With trailing \0 for backward compat reasons. */
  sys_wcstombs (destbuf, len + 1, p->progname);
  return len;
}

static off_t
format_process_environ (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  size_t fs;

  if (destbuf)
    {
      cfree (destbuf);
      destbuf = NULL;
    }
  destbuf = p ? p->environ (fs) : NULL;
  if (!destbuf || !*destbuf)
    {
      destbuf = cstrdup ("<defunct>");
      fs = strlen (destbuf) + 1;
    }
  return fs;
}

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

    buf = RtlCreateQueryDebugBuffer (16 * 65536, FALSE);
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
    return NULL;
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
    ULONG size = 50 * (sizeof (SYSTEM_PROCESS_INFORMATION)
		       + 16 * sizeof (SYSTEM_THREADS));
    PSYSTEM_PROCESS_INFORMATION proc;
    PSYSTEM_THREADS thread;

    do
      {
	buf = realloc (buf, size);
	status = NtQuerySystemInformation (SystemProcessInformation,
					   buf, size, NULL);
	size <<= 1;
      }
    while (status == STATUS_INFO_LENGTH_MISMATCH);
    if (!NT_SUCCESS (status))
      {
	if (buf)
	  free (buf);
	debug_printf ("NtQuerySystemInformation, %y", status);
	return;
      }
    proc = (PSYSTEM_PROCESS_INFORMATION) buf;
    while (true)
      {
	if ((DWORD) (uintptr_t) proc->UniqueProcessId == pid)
	  break;
	if (!proc->NextEntryOffset)
	  {
	    free (buf);
	    return;
	  }
	proc = (PSYSTEM_PROCESS_INFORMATION) ((PBYTE) proc
					      + proc->NextEntryOffset);
      }
    thread = proc->Threads;
    for (ULONG i = 0; i < proc->NumberOfThreads; ++i)
      {
	THREAD_BASIC_INFORMATION tbi;
	TEB teb;
	HANDLE thread_h;

	thread_h = OpenThread (THREAD_QUERY_LIMITED_INFORMATION, FALSE,
			 (ULONG) (ULONG_PTR) thread[i].ClientId.UniqueThread);
	if (!thread_h)
	  continue;
	status = NtQueryInformationThread (thread_h, ThreadBasicInformation,
					   &tbi, sizeof tbi, NULL);
	CloseHandle (thread_h);
	if (!NT_SUCCESS (status))
	  continue;
	region *r = (region *) malloc (sizeof (region));
	if (r)
	  {
	    *r = (region) { regions,
			    (ULONG) (ULONG_PTR) thread[i].ClientId.UniqueThread,
			    (char *) tbi.TebBaseAddress,
			    (char *) tbi.TebBaseAddress
				     + 2 * wincap.page_size (),
			    true };
	    regions = r;
	  }
	if (!ReadProcessMemory (process, (PVOID) tbi.TebBaseAddress,
				&teb, sizeof teb, NULL))
	  continue;
	r = (region *) malloc (sizeof (region));
	if (r)
	  {
	    *r = (region) { regions,
			    (ULONG) (ULONG_PTR) thread[i].ClientId.UniqueThread,
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
    return NULL;
  }
  /* Helper to look for TEBs inside single allocated region since W10 1511. */
  char *fill_if_match (char *start, char *dest)
  {
    for (region *r = regions; r; r = r->next)
      if (r->teb && start == r->start)
	{
	  __small_sprintf (dest, "[teb (tid %ld)]", r->thread_id);
	  return r->end;
	}
    return NULL;
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

static off_t
format_process_maps (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  HANDLE proc = OpenProcess (PROCESS_QUERY_INFORMATION
			     | PROCESS_VM_READ, FALSE, p->dwProcessId);
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

  off_t len = 0;

  union access
  {
    char flags[8];
    off_t word;
  } a;

  struct region {
    access a;
    char *abase;
    char *rbase;
    char *rend;
    DWORD state;
  } cur = {{{'\0'}}, (char *)1, 0, 0};

  MEMORY_BASIC_INFORMATION mb;
  dos_drive_mappings drive_maps;
  heap_info heaps (p->dwProcessId);
  thread_info threads (p->dwProcessId, proc);
  struct stat st;
  long last_pass = 0;

  tmp_pathbuf tp;
  PMEMORY_SECTION_NAME msi = (PMEMORY_SECTION_NAME) tp.w_get ();
  char *posix_modname = tp.c_get ();
  size_t maxsize = 0;
  char *peb_teb_abase = NULL;

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
				  | PAGE_EXECUTE_READWRITE
				  | PAGE_EXECUTE_WRITECOPY);
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
		      (char *) mb.BaseAddress+mb.RegionSize,
		      mb.State
      };

      /* Windows permissions are more fine-grained than the unix rwxp,
	 so we reduce clutter by manually coalescing regions sharing
	 the same allocation base and effective permissions. */
      bool newbase = (next.abase != cur.abase);
      if (!last_pass && !newbase && next.a.word == cur.a.word)
	  cur.rend = next.rend; /* merge with previous */
      else
	{
	  char *peb_teb_end = NULL;
peb_teb_rinse_repeat:
	  /* Starting with W10 1511, PEB and TEBs don't get allocated
	     separately.  Rather they are created in a single region.  Examine
	     the region starting at the PEB address page-wise. */
	  if (wincap.has_new_pebteb_region ())
	    {
	      if (peb_teb_abase && !peb_teb_end && cur.abase == peb_teb_abase)
		{
		  posix_modname[0] = '\0';
		  peb_teb_end = cur.rend;
		  if (cur.state == MEM_COMMIT)
		    cur.rend = cur.rbase + wincap.page_size ();
		}
	      if (cur.state == MEM_COMMIT)
		{
		  if (!peb_teb_abase && cur.rbase == (char *) peb)
		    {
		      peb_teb_abase = cur.abase;
		      peb_teb_end = cur.rend;
		      cur.rend = cur.rbase + wincap.page_size ();
		      strcpy (posix_modname, "[peb]");
		    }
		  else if (peb_teb_end)
		    {
		      char *end;
		      posix_modname[0] = '\0';
		      end = threads.fill_if_match (cur.rbase, posix_modname);

		      if (end)
			cur.rend = end;
		      else
			{
			  char *base = cur.rbase;
			  do
			    {
			      base += wincap.page_size ();
			    }
			  while (!threads.fill_if_match (base, posix_modname)
				 && base < peb_teb_end);
			  if (posix_modname[0])
			    {
			      posix_modname[0] = '\0';
			      cur.rend = base;
			    }
			  else
			      cur.rend = peb_teb_end;
			}
		    }
		}
	    }
	  /* output the current region if it's "interesting". */
	  if (cur.a.word)
	    {
	      size_t newlen = strlen (posix_modname) + 62;
	      if (len + newlen >= maxsize)
		destbuf = (char *)
		  crealloc_abort (destbuf,
				  maxsize += roundup2 (newlen, 2048UL));
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

	  if (peb_teb_end && cur.state == MEM_COMMIT)
	    {
	      cur.rbase = cur.rend;
	      cur.rend += wincap.page_size ();
	      if (cur.rbase < peb_teb_end)
		goto peb_teb_rinse_repeat;
	    }
	  /* start of a new region (but possibly still the same allocation). */
	  cur = next;
	  /* if a new allocation, figure out what kind it is. */
	  if (newbase && !last_pass && cur.state != MEM_FREE)
	    {
	      /* If the return length pointer is missing, NtQueryVirtualMemory
		 returns with STATUS_ACCESS_VIOLATION on Windows 2000. */
	      SIZE_T ret_len = 0;

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

static off_t
format_process_stat (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  char cmd[NAME_MAX + 1];
  int state = 'R';
  unsigned long fault_count = 0UL,
		vmsize = 0UL, vmrss = 0UL, vmmaxrss = 0UL;
  uint64_t utime = 0ULL, stime = 0ULL, start_time = 0ULL;
  int priority = 0;
  if (p->process_state & PID_EXITED)
    strcpy (cmd, "<defunct>");
  else
    {
      PWCHAR last_slash = wcsrchr (p->progname, L'\\');
      sys_wcstombs (cmd, NAME_MAX + 1,
		    last_slash ? last_slash + 1 : p->progname);
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

  NTSTATUS status;
  HANDLE hProcess;
  VM_COUNTERS vmc;
  KERNEL_USER_TIMES put;
  PROCESS_BASIC_INFORMATION pbi;
  QUOTA_LIMITS ql;
  SYSTEM_TIMEOFDAY_INFORMATION stodi;
  SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION spt;
  hProcess = OpenProcess (PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ,
			  FALSE, p->dwProcessId);
  if (hProcess == NULL)
    {
      DWORD error = GetLastError ();
      __seterrno_from_win_error (error);
      debug_printf ("OpenProcess: ret %u", error);
      return 0;
    }

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
  if (NT_SUCCESS (status))
    status = NtQuerySystemInformation (SystemTimeOfDayInformation,
				       (PVOID) &stodi, sizeof stodi, NULL);
  if (NT_SUCCESS (status))
    status = NtQuerySystemInformation (SystemProcessorPerformanceInformation,
				       (PVOID) &spt, sizeof spt, NULL);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      debug_printf ("NtQueryInformationProcess: status %y, %E", status);
      return 0;
    }
  fault_count = vmc.PageFaultCount;
  utime = put.UserTime.QuadPart * CLOCKS_PER_SEC / NS100PERSEC;
  stime = put.KernelTime.QuadPart * CLOCKS_PER_SEC / NS100PERSEC;
  start_time = (put.CreateTime.QuadPart - stodi.BootTime.QuadPart)
	       * CLOCKS_PER_SEC / NS100PERSEC;
  /* The BasePriority returned to a 32 bit process under WOW64 is
     apparently broken, for 32 and 64 bit target processes.  64 bit
     processes get the correct base priority, even for 32 bit processes. */
  if (wincap.is_wow64 ())
    priority = 8; /* Default value. */
  else
    priority = pbi.BasePriority;
  unsigned page_size = wincap.page_size ();
  vmsize = vmc.PagefileUsage;
  vmrss = vmc.WorkingSetSize / page_size;
  vmmaxrss = ql.MaximumWorkingSetSize / page_size;

  destbuf = (char *) crealloc_abort (destbuf, strlen (cmd) + 320);
  return __small_sprintf (destbuf, "%d (%s) %c "
				   "%d %d %d %d %d "
				   "%u %lu %lu %u %u %lu %lu "
				   "%U %U %d %d %d %d "
				   "%U %lu "
				   "%ld %lu",
			  p->pid, cmd, state,
			  p->ppid, p->pgid, p->sid, p->ctty, -1,
			  0, fault_count, fault_count, 0, 0, utime, stime,
			  utime, stime, priority, 0, 0, 0,
			  start_time, vmsize,
			  vmrss, vmmaxrss
			  );
}

static off_t
format_process_status (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  char cmd[NAME_MAX + 1];
  int state = 'R';
  const char *state_str = "unknown";
  unsigned long vmsize = 0UL, vmrss = 0UL, vmdata = 0UL, vmlib = 0UL,
		vmtext = 0UL, vmshare = 0UL;

  PWCHAR last_slash = wcsrchr (p->progname, L'\\');
  sys_wcstombs (cmd, NAME_MAX + 1, last_slash ? last_slash + 1 : p->progname);
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
  unsigned page_size = wincap.allocation_granularity ();
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

static off_t
format_process_statm (void *data, char *&destbuf)
{
  _pinfo *p = (_pinfo *) data;
  unsigned long vmsize = 0UL, vmrss = 0UL, vmtext = 0UL, vmdata = 0UL,
		vmlib = 0UL, vmshare = 0UL;
  size_t page_scale;
  if (!get_mem_values (p->dwProcessId, &vmsize, &vmrss, &vmtext, &vmdata,
		       &vmlib, &vmshare))
    return 0;

  page_scale = wincap.allocation_granularity() / wincap.page_size();
  destbuf = (char *) crealloc_abort (destbuf, 96);
  return __small_sprintf (destbuf, "%ld %ld %ld %ld %ld %ld 0\n",
              vmsize / page_scale, vmrss / page_scale, vmshare / page_scale,
              vmtext / page_scale, vmlib / page_scale, vmdata / page_scale);
}

extern "C" {
  FILE *setmntent (const char *, const char *);
  struct mntent *getmntent (FILE *);
};

static off_t
format_process_mountstuff (void *data, char *&destbuf, bool mountinfo)
{
  _pinfo *p = (_pinfo *) data;
  user_info *u_shared = NULL;
  HANDLE u_hdl = NULL;
  off_t len = 0;
  struct mntent *mnt;

  if (p->uid != myself->uid)
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
  mount_info *mtab = &u_shared->mountinfo;

  /* Store old value of _my_tls.locals here. */
  int iteration = _my_tls.locals.iteration;
  unsigned available_drives = _my_tls.locals.available_drives;
  /* This reinitializes the above values in _my_tls. */
  setmntent (NULL, NULL);
  /* Restore iteration immediately since it's not used below.  We use the
     local iteration variable instead*/
  _my_tls.locals.iteration = iteration;

  for (iteration = 0; (mnt = mtab->getmntent (iteration)); ++iteration)
    {
      /* We have no access to the drives mapped into another user session and
	 _my_tls.locals.available_drives contains the mappings of the current
	 user.  So, when printing the mount table of another user, we check
	 each cygdrive entry if it's a remote drive.  If so, ignore it. */
      if (iteration >= mtab->nmounts && u_hdl)
	{
	  WCHAR drive[3] = { (WCHAR) mnt->mnt_fsname[0], L':', L'\0' };
	  disk_type dt = get_disk_type (drive);

	  if (dt == DT_SHARE_SMB || dt == DT_SHARE_NFS)
	    continue;
	}
      destbuf = (char *) crealloc_abort (destbuf, len
						  + strlen (mnt->mnt_fsname)
						  + strlen (mnt->mnt_dir)
						  + strlen (mnt->mnt_type)
						  + strlen (mnt->mnt_opts)
						  + 30);
      if (mountinfo)
	{
	  path_conv pc (mnt->mnt_dir, PC_SYM_NOFOLLOW | PC_POSIX);
	  dev_t dev = pc.exists () ? pc.fs_serial_number () : -1;

	  len += __small_sprintf (destbuf + len,
				  "%d %d %d:%d / %s %s - %s %s %s\n",
				  iteration, iteration,
				  major (dev), minor (dev),
				  mnt->mnt_dir, mnt->mnt_opts,
				  mnt->mnt_type, mnt->mnt_fsname,
				  (pc.fs_flags () & FILE_READ_ONLY_VOLUME)
				  ? "ro" : "rw");
	}
      else
	len += __small_sprintf (destbuf + len, "%s %s %s %s %d %d\n",
				mnt->mnt_fsname, mnt->mnt_dir, mnt->mnt_type,
				mnt->mnt_opts, mnt->mnt_freq, mnt->mnt_passno);
    }

  /* Restore available_drives */
  _my_tls.locals.available_drives = available_drives;

  if (u_hdl) /* Only not-NULL if open_shared has been called. */
    {
      UnmapViewOfFile (u_shared);
      CloseHandle (u_hdl);
    }
  return len;
}

static off_t
format_process_mounts (void *data, char *&destbuf)
{
  return format_process_mountstuff (data, destbuf, false);
}

static off_t
format_process_mountinfo (void *data, char *&destbuf)
{
  return format_process_mountstuff (data, destbuf, true);
}

int
get_process_state (DWORD dwProcessId)
{
  /* This isn't really heavy magic - just go through the processes' threads
     one by one and return a value accordingly.  Errors are silently ignored. */
  NTSTATUS status;
  PSYSTEM_PROCESS_INFORMATION p, sp;
  ULONG n = 0x4000;
  int state =' ';

  p = (PSYSTEM_PROCESS_INFORMATION) malloc (n);
  if (!p)
    return state;
  while (true)
    {
      status = NtQuerySystemInformation (SystemProcessInformation,
					 (PVOID) p, n, NULL);
      if (status != STATUS_INFO_LENGTH_MISMATCH)
	break;
      n <<= 1;
      PSYSTEM_PROCESS_INFORMATION new_p = (PSYSTEM_PROCESS_INFORMATION) realloc (p, n);
      if (!new_p)
      	goto out;
      p = new_p;
    }
  if (!NT_SUCCESS (status))
    {
      debug_printf ("NtQuerySystemInformation: status %y, %u",
		    status, RtlNtStatusToDosError (status));
      goto out;
    }
  state = 'Z';
  sp = p;
  for (;;)
    {
      if ((DWORD) (uintptr_t) sp->UniqueProcessId == dwProcessId)
	{
	  SYSTEM_THREADS *st;
	  st = &sp->Threads[0];
	  state = 'S';
	  for (unsigned i = 0; i < sp->NumberOfThreads; i++)
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
      if (!sp->NextEntryOffset)
	 break;
      sp = (PSYSTEM_PROCESS_INFORMATION) ((char *) sp + sp->NextEntryOffset);
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
  SIZE_T n = 0x4000, length;

  /* This appears to work despite MSDN claiming that QueryWorkingSet requires
     PROCESS_QUERY_INFORMATION *and* PROCESS_VM_READ.  Since we're trying to do
     everything with least perms, we stick to PROCESS_QUERY_INFORMATION only
     unless this changes in Windows for some reason. */
  hProcess = OpenProcess (PROCESS_QUERY_INFORMATION, FALSE, dwProcessId);
  if (hProcess == NULL)
    {
      __seterrno ();
      debug_printf ("OpenProcess, %E");
      return false;
    }
  p = (PMEMORY_WORKING_SET_LIST) malloc (n);
  if (!p)
    goto out;
  while (true)
    {
      status = NtQueryVirtualMemory (hProcess, 0, MemoryWorkingSetList,
				     (PVOID) p, n,
				     (length = (SIZE_T) -1, &length));
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
      debug_printf ("NtQueryVirtualMemory: status %y", status);
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
      debug_printf ("NtQueryInformationProcess: status %y", status);
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

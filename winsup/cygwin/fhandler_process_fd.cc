/* fhandler_process_fd.cc: fhandler for /proc/<pid>/fd/<desc> operations

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "path.h"
#include "fhandler.h"
#include "fhandler_virtual.h"
#include "pinfo.h"
#include "dtable.h"
#include "cygheap.h"
#include "tls_pbuf.h"

fhandler_base *
fhandler_process_fd::fetch_fh (HANDLE &out_hdl, uint32_t flags)
{
  const char *path;
  char *e;
  int fd;
  HANDLE proc;
  HANDLE hdl = NULL;
  path_conv pc;

  path = get_name () + proc_len + 1;
  pid = strtoul (path, &e, 10);
  path = e + 4;
  fd = strtoul (path, &e, 10);

  out_hdl = NULL;
  if (pid == myself->pid)
    {
      cygheap_fdget cfd (fd, true);
      if (cfd < 0)
	return NULL;
      if ((flags & FFH_LINKAT)
	 && (cfd->get_flags () & (O_TMPFILE | O_EXCL)) == (O_TMPFILE | O_EXCL))
	{
	  set_errno (ENOENT);
	  return NULL;
	}
      proc = GetCurrentProcess ();
      pc << cfd->pc;
      hdl = cfd->get_handle ();
    }
  else
    {
      pinfo p (pid);
      if (!p)
	{
	  set_errno (ENOENT);
	  return NULL;
	}
      proc = OpenProcess (PROCESS_DUP_HANDLE, false, p->dwProcessId);
      if (!proc)
	{
	  __seterrno ();
	  return NULL;
	}
      size_t size;
      void *buf = p->file_pathconv (fd, flags, size);
      if (size == 0)
	{
	  set_errno (ENOENT);
	  CloseHandle (proc);
	  return NULL;
	}
      hdl = pc.deserialize (buf);
    }
  if (hdl == NULL)
    {
      if (proc != GetCurrentProcess ())
	CloseHandle (proc);
      set_errno (EACCES);
      return NULL;
    }
  BOOL ret = DuplicateHandle (proc, hdl, GetCurrentProcess (), &hdl,
			      0, FALSE, DUPLICATE_SAME_ACCESS);
  if (proc != GetCurrentProcess ())
    CloseHandle (proc);
  if (!ret)
    {
      __seterrno ();
      CloseHandle (hdl);
      return NULL;
    }
  /* relative path?  This happens for special types like pipes and sockets. */
  if (*pc.get_posix () != '/')
    {
      tmp_pathbuf tp;
      char *fullpath = tp.c_get ();

      stpcpy (stpncpy (fullpath, get_name (), path - get_name ()),
	      pc.get_posix ());
      pc.set_posix (fullpath);
    }
  fhandler_base *fh = build_fh_pc (pc);
  if (!fh)
    {
      CloseHandle (hdl);
      return NULL;
    }
  out_hdl = hdl;
  return fh;
}

fhandler_base *
fhandler_process_fd::fd_reopen (int flags, mode_t mode)
{
  fhandler_base *fh;
  HANDLE hdl;

  fh = fetch_fh (hdl, 0);
  if (!fh)
    return NULL;
  fh->set_handle (hdl);
  int ret = fh->open_with_arch (flags, mode);
  CloseHandle (hdl);
  if (!ret)
    {
      delete fh;
      fh = NULL;
    }
  return fh;
}

int __reg2
fhandler_process_fd::fstat (struct stat *statbuf)
{
  if (!pc.follow_fd_symlink ())
    return fhandler_process::fstat (statbuf);

  fhandler_base *fh;
  HANDLE hdl;

  fh = fetch_fh (hdl, 0);
  if (!fh)
    return -1;
  fh->set_handle (hdl);
  int ret = fh->fstat (statbuf);
  CloseHandle (hdl);
  delete fh;
  return ret;
}

int
fhandler_process_fd::link (const char *newpath)
{
  fhandler_base *fh;
  HANDLE hdl;

  fh = fetch_fh (hdl, FFH_LINKAT);
  if (!fh)
    return -1;
  fh->set_handle (hdl);
  int ret = fh->link (newpath);
  CloseHandle (hdl);
  delete fh;
  return ret;
}

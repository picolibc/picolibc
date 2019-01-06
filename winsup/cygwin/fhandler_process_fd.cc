/* fhandler_process_fd.cc: fhandler for /proc/<pid>/fd/<desc> operations

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


fhandler_base *
fhandler_process_fd::fetch_fh (HANDLE &out_hdl)
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
      void *buf = p->file_pathconv (fd, size);
      if (size == 0)
	{
	  set_errno (EPERM);
	  CloseHandle (proc);
	  return NULL;
	}
      hdl = pc.deserialize (buf);
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
fhandler_process_fd::fd_reopen (int flags)
{
  fhandler_base *fh;
  HANDLE hdl;

  fh = fetch_fh (hdl);
  if (!fh)
    return NULL;
  fh->set_io_handle (hdl);
  int ret = fh->open_with_arch (flags, 0);
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

  fh = fetch_fh (hdl);
  if (!fh)
    return -1;
  fh->set_io_handle (hdl);
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

  fh = fetch_fh (hdl);
  if (!fh)
    return -1;
  fh->set_io_handle (hdl);
  int ret = fh->link (newpath);
  CloseHandle (hdl);
  delete fh;
  return ret;
}

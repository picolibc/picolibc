/* fhandler_netdrive.cc: fhandler for // and //MACHINE handling

   Copyright 2005 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/cygwin.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "sigproc.h"
#include "cygthread.h"
#include <assert.h>
#include <winnetwk.h>

#include <dirent.h>

enum
  {
    GET_RESOURCE_INFO = 0,
    GET_RESOURCE_OPENENUM = 1,
    GET_RESOURCE_OPENENUMTOP = 2,
    GET_RESOURCE_ENUM = 3
  };

struct netdriveinf
  {
    int what;
    int ret;
    PVOID in;
    PVOID out;
    DWORD outsize;
    HANDLE sem;
  };

static DWORD WINAPI
thread_netdrive (void *arg)
{
  netdriveinf *ndi = (netdriveinf *) arg;
  LPTSTR dummy = NULL;
  LPNETRESOURCE nro, nro2;
  DWORD size;
  HANDLE enumhdl;

  ReleaseSemaphore (ndi->sem, 1, NULL);
  switch (ndi->what)
    {
    case GET_RESOURCE_INFO:
      nro = (LPNETRESOURCE) alloca (size = 4096);
      ndi->ret = WNetGetResourceInformation ((LPNETRESOURCE) ndi->in,
					     nro, &size, &dummy);
      break;
    case GET_RESOURCE_OPENENUM:
    case GET_RESOURCE_OPENENUMTOP:
      nro = (LPNETRESOURCE) alloca (size = 4096);
      ndi->ret = WNetGetResourceInformation ((LPNETRESOURCE) ndi->in,
					     nro, &size, &dummy);
      if (ndi->ret != NO_ERROR)
	break;
      if (ndi->what == GET_RESOURCE_OPENENUMTOP)
	{
	  nro2 = nro;
	  nro = (LPNETRESOURCE) alloca (size = 4096);
	  ndi->ret = WNetGetResourceParent (nro2, nro, &size);
	  if (ndi->ret != NO_ERROR)
	    break;
	}
      ndi->ret = WNetOpenEnum (RESOURCE_GLOBALNET, RESOURCETYPE_DISK, 0, nro,
			       &enumhdl);
      if (ndi->ret == NO_ERROR)
	*(HANDLE *) ndi->out = enumhdl;
      break;
    case GET_RESOURCE_ENUM:
      ndi->ret = WNetEnumResource ((HANDLE) ndi->in, (size = 1, &size),
				   (LPNETRESOURCE) ndi->out, &ndi->outsize);
      break;
    }
  ReleaseSemaphore (ndi->sem, 1, NULL);
  return 0;
}

static DWORD
create_thread_and_wait (int what, PVOID in, PVOID out, DWORD outsize,
			const char *name)
{
  netdriveinf ndi = { what, 0, in, out, outsize,
		      CreateSemaphore (&sec_none_nih, 0, 2, NULL) };
  cygthread *thr = new cygthread (thread_netdrive, 0, &ndi, name);
  if (thr->detach (ndi.sem))
    ndi.ret = ERROR_OPERATION_ABORTED;
  CloseHandle (ndi.sem);
  return ndi.ret;
}

/* Returns 0 if path doesn't exist, >0 if path is a directory,
   -1 if path is a file, -2 if it's a symlink.  */
int
fhandler_netdrive::exists ()
{
  char *to;
  const char *from;
  size_t len = strlen (get_name ());
  if (len == 2)
    return 1;
  char namebuf[len + 1];
  for (to = namebuf, from = get_name (); *from; to++, from++)
    *to = (*from == '/') ? '\\' : *from;
  *to = '\0';

  NETRESOURCE nr = {0};
  nr.dwScope = RESOURCE_GLOBALNET;
  nr.dwType = RESOURCETYPE_DISK;
  nr.lpLocalName = NULL;
  nr.lpRemoteName = namebuf;
  DWORD ret = create_thread_and_wait (GET_RESOURCE_INFO, &nr, NULL, 0,
				      "WNetGetResourceInformation");
  if (ret != ERROR_MORE_DATA && ret != NO_ERROR)
    return 0;
  return 1;
}

fhandler_netdrive::fhandler_netdrive ():
  fhandler_virtual ()
{
}

int
fhandler_netdrive::fstat (struct __stat64 *buf)
{
  const char *path = get_name ();
  debug_printf ("fstat (%s)", path);

  fhandler_base::fstat (buf);

  buf->st_mode = S_IFDIR | STD_RBITS | STD_XBITS;
  buf->st_ino = get_ino ();

  return 0;
}

int
fhandler_netdrive::readdir (DIR *dir, dirent *de)
{
  DWORD size;
  NETRESOURCE *nro;
  DWORD ret;
  int res;

  if (!dir->__d_position)
    {
      size_t len = strlen (get_name ());
      char *namebuf;
      NETRESOURCE nr = { 0 };

      if (len == 2)	/* // */
	{
	  namebuf = (char *) alloca (MAX_COMPUTERNAME_LENGTH + 3);
	  strcpy (namebuf, "\\\\");
	  size = MAX_COMPUTERNAME_LENGTH + 1;
	  if (!GetComputerName (namebuf + 2, &size))
	    {
	      res = geterrno_from_win_error ();
	      goto out;
	    }
	}
      else
	{
	  const char *from;
	  char *to;
	  namebuf = (char *) alloca (len + 1);
	  for (to = namebuf, from = get_name (); *from; to++, from++)
	    *to = (*from == '/') ? '\\' : *from;
	  *to = '\0';
	}

      nr.lpRemoteName = namebuf;
      nr.dwType = RESOURCETYPE_DISK;
      nro = (NETRESOURCE *) alloca (4096);
      ret = create_thread_and_wait (len == 2 ? GET_RESOURCE_OPENENUMTOP
					     : GET_RESOURCE_OPENENUM,
				    &nr, &dir->__handle, 0, "WNetOpenEnum");
      if (ret != NO_ERROR)
	{
	  dir->__handle = INVALID_HANDLE_VALUE;
	  res = geterrno_from_win_error (ret);
	  goto out;
	}
    }
  ret = create_thread_and_wait (GET_RESOURCE_ENUM, dir->__handle,
				nro = (LPNETRESOURCE) alloca (16384),
				16384, "WnetEnumResource");
  if (ret != NO_ERROR)
    res = geterrno_from_win_error (ret);
  else
    {
      dir->__d_position++;
      char *bs = strrchr (nro->lpRemoteName, '\\');
      strcpy (de->d_name, bs ? bs + 1 : nro->lpRemoteName);
      if (strlen (get_name ()) == 2)
	de->d_ino = hash_path_name (get_ino (), de->d_name);
      else
	{
	  de->d_ino = readdir_get_ino (nro->lpRemoteName, false);
	  /* We can't trust remote inode numbers of only 32 bit.  That means,
	     all remote inode numbers when running under NT4, as well as
	     remote NT4 NTFS, as well as shares of Samba version < 3.0. */
	  if (de->d_ino <= UINT_MAX)
	    de->d_ino = hash_path_name (0, nro->lpRemoteName);
	}

      res = 0;
    }
out:
  syscall_printf ("%d = readdir (%p, %p)", res, dir, de);
  return res;
}

void
fhandler_netdrive::seekdir (DIR *dir, _off64_t pos)
{
  rewinddir (dir);
  if (pos < 0)
    return;
  while (dir->__d_position < pos)
    if (!readdir (dir, dir->__d_dirent))
      break;
}

void
fhandler_netdrive::rewinddir (DIR *dir)
{
  if (dir->__handle != INVALID_HANDLE_VALUE)
    WNetCloseEnum (dir->__handle);
  dir->__handle = INVALID_HANDLE_VALUE;
  return fhandler_virtual::rewinddir (dir);
}

int
fhandler_netdrive::closedir (DIR *dir)
{
  rewinddir (dir);
  return fhandler_virtual::closedir (dir);
}

int
fhandler_netdrive::open (int flags, mode_t mode)
{
  int res = fhandler_virtual::open (flags, mode);
  if (!res)
    goto out;

  nohandle (true);

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

  res = 1;
  set_flags ((flags & ~O_TEXT) | O_BINARY | O_DIROPEN);
  set_open_status ();
out:
  syscall_printf ("%d = fhandler_netdrive::open (%p, %d)", res, flags, mode);
  return res;
}


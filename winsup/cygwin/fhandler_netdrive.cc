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
#include <assert.h>
#include <winnetwk.h>

#include <dirent.h>

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
  LPTSTR sys = NULL;
  char buf[8192];
  DWORD n = sizeof (buf);
  DWORD rc = WNetGetResourceInformation (&nr, &buf, &n, &sys);
  if (rc != ERROR_MORE_DATA && rc != NO_ERROR)
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

  (void) fhandler_base::fstat (buf);

  buf->st_mode = S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;

  return 0;
}

struct dirent *
fhandler_netdrive::readdir (DIR *dir)
{
  DWORD size;
  NETRESOURCE *nro;
  DWORD ret;

  if (!dir->__d_position)
    {
      size_t len = strlen (get_name ());
      char *namebuf, *dummy;
      NETRESOURCE nr = { 0 }, *nro2;

      if (len == 2)	/* // */
        {
	  namebuf = (char *) alloca (MAX_COMPUTERNAME_LENGTH + 3);
	  strcpy (namebuf, "\\\\");
	  size = MAX_COMPUTERNAME_LENGTH + 1;
	  if (!GetComputerName (namebuf + 2, &size))
	    {
	      __seterrno ();
	      return NULL;
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
      size = 4096;
      nro = (NETRESOURCE *) alloca (size);
      ret = WNetGetResourceInformation (&nr, nro, &size, &dummy);
      if (ret != NO_ERROR)
	{
	  __seterrno ();
	  return NULL;
	}

      if (len == 2)
        {
	  nro2 = nro;
	  size = 4096;
	  nro = (NETRESOURCE *) alloca (size);
	  ret = WNetGetResourceParent (nro2, nro, &size);
	  if (ret != NO_ERROR)
	    {
	      __seterrno ();
	      return NULL;
	    }
	}
      ret = WNetOpenEnum(RESOURCE_GLOBALNET, RESOURCETYPE_DISK, 0,
      			 nro, &dir->__handle);
      if (ret != NO_ERROR)
	{
	  __seterrno ();
	  dir->__handle = INVALID_HANDLE_VALUE;
	  return NULL;
	}
    }
  /* FIXME: dot and dot_dot handling */
  DWORD cnt = 1;
  size = 16384;	/* As documented in MSDN. */
  nro = (NETRESOURCE *) alloca (size);
  ret = WNetEnumResource (dir->__handle, &cnt, nro, &size);
  if (ret != NO_ERROR)
    {
      if (ret != ERROR_NO_MORE_ITEMS)
	__seterrno ();
      return NULL;
    }
  dir->__d_position++;
  char *bs = strrchr (nro->lpRemoteName, '\\');
  strcpy (dir->__d_dirent->d_name, bs ? bs + 1 : nro->lpRemoteName);
  return dir->__d_dirent;
}

_off64_t
fhandler_netdrive::telldir (DIR *dir)
{
  return -1;
}

void
fhandler_netdrive::seekdir (DIR *, _off64_t)
{
}

int
fhandler_netdrive::closedir (DIR *dir)
{
  if (dir->__handle != INVALID_HANDLE_VALUE)
    WNetCloseEnum (dir->__handle);
  dir->__handle = INVALID_HANDLE_VALUE;
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


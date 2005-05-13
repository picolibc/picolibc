/* fhandler_netdrive.cc: fhandler for // and //MACHINE handling

   Copyright 2005 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <windows.h>
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
#include <shlwapi.h>

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
fhandler_netdrive::readdir (DIR * dir)
{
  return NULL;
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


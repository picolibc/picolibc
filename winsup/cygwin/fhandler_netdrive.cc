/* fhandler_netdrive.cc: fhandler for // and //MACHINE handling

   Copyright 2005 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <stdlib.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "cygthread.h"
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

struct net_hdls
  {
    HANDLE net;
    HANDLE dom;
  };

static DWORD WINAPI
thread_netdrive (void *arg)
{
  netdriveinf *ndi = (netdriveinf *) arg;
  char provider[256], *dummy = NULL;
  LPNETRESOURCE nro;
  DWORD cnt, size;
  struct net_hdls *nh;

  ReleaseSemaphore (ndi->sem, 1, NULL);
  switch (ndi->what)
    {
    case GET_RESOURCE_INFO:
      nro = (LPNETRESOURCE) alloca (size = 4096);
      ndi->ret = WNetGetResourceInformation ((LPNETRESOURCE) ndi->in,
					     nro, &size, &dummy);
      break;
    case GET_RESOURCE_OPENENUMTOP:
      nro = (LPNETRESOURCE) alloca (size = 4096);
      nh = (struct net_hdls *) ndi->out;
      ndi->ret = WNetGetProviderName (WNNC_NET_LANMAN, provider,
				      (size = 256, &size));
      if (ndi->ret != NO_ERROR)
	break;
      memset (nro, 0, sizeof *nro);
      nro->dwScope = RESOURCE_GLOBALNET;
      nro->dwType = RESOURCETYPE_ANY;
      nro->dwDisplayType = RESOURCEDISPLAYTYPE_GROUP;
      nro->dwUsage = RESOURCEUSAGE_RESERVED | RESOURCEUSAGE_CONTAINER;
      nro->lpRemoteName = provider;
      nro->lpProvider = provider;
      ndi->ret = WNetOpenEnum (RESOURCE_GLOBALNET, RESOURCETYPE_DISK,
			       RESOURCEUSAGE_ALL, nro, &nh->net);
      if (ndi->ret != NO_ERROR)
	break;
      while ((ndi->ret = WNetEnumResource (nh->net, (cnt = 1, &cnt), nro,
				(size = 4096, &size))) == NO_ERROR)
	{
	  ndi->ret = WNetOpenEnum (RESOURCE_GLOBALNET, RESOURCETYPE_DISK,
				   RESOURCEUSAGE_ALL, nro, &nh->dom);
	  if (ndi->ret == NO_ERROR)
	    break;
	}
      break;
    case GET_RESOURCE_OPENENUM:
      nro = (LPNETRESOURCE) alloca (size = 4096);
      nh = (struct net_hdls *) ndi->out;
      ndi->ret = WNetGetProviderName (WNNC_NET_LANMAN, provider,
				      (size = 256, &size));
      if (ndi->ret != NO_ERROR)
	break;
      ((LPNETRESOURCE) ndi->in)->lpProvider = provider;
      ndi->ret = WNetGetResourceInformation ((LPNETRESOURCE) ndi->in,
					     nro, &size, &dummy);
      if (ndi->ret != NO_ERROR)
	break;
      ndi->ret = WNetOpenEnum (RESOURCE_GLOBALNET, RESOURCETYPE_DISK,
			       RESOURCEUSAGE_ALL, nro, &nh->dom);
      break;
    case GET_RESOURCE_ENUM:
      nh = (struct net_hdls *) ndi->in;
      if (!nh->dom)
        {
	  ndi->ret = ERROR_NO_MORE_ITEMS;
	  break;
	}
      while ((ndi->ret = WNetEnumResource (nh->dom, (cnt = 1, &cnt),
					   (LPNETRESOURCE) ndi->out,
					   &ndi->outsize)) != NO_ERROR
	     && nh->net)
	{
	  WNetCloseEnum (nh->dom);
	  nh->dom = NULL;
	  nro = (LPNETRESOURCE) alloca (size = 4096);
	  while ((ndi->ret = WNetEnumResource (nh->net, (cnt = 1, &cnt), nro,
					     (size = 4096, &size))) == NO_ERROR)
	    {
	      ndi->ret = WNetOpenEnum (RESOURCE_GLOBALNET, RESOURCETYPE_DISK,
				       RESOURCEUSAGE_ALL, nro, &nh->dom);
	      if (ndi->ret == NO_ERROR)
		break;
	    }
	  if (ndi->ret != NO_ERROR)
	    break;
	}
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
  NETRESOURCE *nro;
  DWORD ret;
  int res;

  if (!dir->__d_position)
    {
      size_t len = strlen (get_name ());
      char *namebuf = NULL;
      NETRESOURCE nr = { 0 };
      struct net_hdls *nh;

      if (len != 2)	/* // */
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
      nh = (struct net_hdls *) ccalloc (HEAP_FHANDLER, 1, sizeof *nh);
      ret = create_thread_and_wait (len == 2 ? GET_RESOURCE_OPENENUMTOP
					     : GET_RESOURCE_OPENENUM,
				    &nr, nh, 0, "WNetOpenEnum");
      if (ret != NO_ERROR)
	{
	  dir->__handle = INVALID_HANDLE_VALUE;
	  res = geterrno_from_win_error (ret);
	  goto out;
	}
      dir->__handle = (HANDLE) nh;
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
	{
	  strlwr (de->d_name);
	  de->d_ino = hash_path_name (get_ino (), de->d_name);
	}
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
    {
      struct net_hdls *nh = (struct net_hdls *) dir->__handle;
      if (nh->dom)
	WNetCloseEnum (nh->dom);
      if (nh->net)
	WNetCloseEnum (nh->net);
      cfree (nh);
    }
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


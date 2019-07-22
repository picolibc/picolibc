/* fhandler_netdrive.cc: fhandler for // and //MACHINE handling

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
#include "tls_pbuf.h"

#include <dirent.h>

enum
  {
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
  WCHAR provider[256], *dummy = NULL;
  LPNETRESOURCEW nro;
  DWORD cnt, size;
  struct net_hdls *nh;
  tmp_pathbuf tp;

  ReleaseSemaphore (ndi->sem, 1, NULL);
  switch (ndi->what)
    {
    case GET_RESOURCE_OPENENUMTOP:
      nro = (LPNETRESOURCEW) tp.c_get ();
      nh = (struct net_hdls *) ndi->out;
      ndi->ret = WNetGetProviderNameW (WNNC_NET_LANMAN, provider,
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
      ndi->ret = WNetOpenEnumW (RESOURCE_GLOBALNET, RESOURCETYPE_DISK,
				RESOURCEUSAGE_ALL, nro, &nh->net);
      if (ndi->ret != NO_ERROR)
	break;
      while ((ndi->ret = WNetEnumResourceW (nh->net, (cnt = 1, &cnt), nro,
					    (size = NT_MAX_PATH, &size)))
	     == NO_ERROR)
	{
	  ndi->ret = WNetOpenEnumW (RESOURCE_GLOBALNET, RESOURCETYPE_DISK,
				    RESOURCEUSAGE_ALL, nro, &nh->dom);
	  if (ndi->ret == NO_ERROR)
	    break;
	}
      break;
    case GET_RESOURCE_OPENENUM:
      nro = (LPNETRESOURCEW) tp.c_get ();
      nh = (struct net_hdls *) ndi->out;
      ndi->ret = WNetGetProviderNameW (WNNC_NET_LANMAN, provider,
				      (size = 256, &size));
      if (ndi->ret != NO_ERROR)
	break;
      ((LPNETRESOURCEW) ndi->in)->lpProvider = provider;
      ndi->ret = WNetGetResourceInformationW ((LPNETRESOURCEW) ndi->in, nro,
					      (size = NT_MAX_PATH, &size),
					      &dummy);
      if (ndi->ret != NO_ERROR)
	break;
      ndi->ret = WNetOpenEnumW (RESOURCE_GLOBALNET, RESOURCETYPE_DISK,
				RESOURCEUSAGE_ALL, nro, &nh->dom);
      break;
    case GET_RESOURCE_ENUM:
      nh = (struct net_hdls *) ndi->in;
      if (!nh->dom)
	{
	  ndi->ret = ERROR_NO_MORE_ITEMS;
	  break;
	}
      nro = (LPNETRESOURCEW) tp.c_get ();
      while ((ndi->ret = WNetEnumResourceW (nh->dom, (cnt = 1, &cnt),
					    (LPNETRESOURCEW) ndi->out,
					    &ndi->outsize)) != NO_ERROR
	     && nh->net)
	{
	  WNetCloseEnum (nh->dom);
	  nh->dom = NULL;
	  while ((ndi->ret = WNetEnumResourceW (nh->net, (cnt = 1, &cnt), nro,
						(size = NT_MAX_PATH, &size)))
		 == NO_ERROR)
	    {
	      ndi->ret = WNetOpenEnumW (RESOURCE_GLOBALNET, RESOURCETYPE_DISK,
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
  cygthread *thr = new cygthread (thread_netdrive, &ndi, name);
  if (thr->detach (ndi.sem))
    ndi.ret = ERROR_OPERATION_ABORTED;
  CloseHandle (ndi.sem);
  return ndi.ret;
}

virtual_ftype_t
fhandler_netdrive::exists ()
{
  char *to;
  const char *from;
  size_t len = strlen (get_name ());
  if (len == 2)
    return virt_rootdir;

  char namebuf[len + 1];
  tmp_pathbuf tp;
  PWCHAR name = tp.w_get ();

  for (to = namebuf, from = get_name (); *from; to++, from++)
    *to = (*from == '/') ? '\\' : *from;
  *to = '\0';

  struct net_hdls nh =  { NULL, NULL };
  NETRESOURCEW nr = {0};
  nr.dwType = RESOURCETYPE_DISK;
  sys_mbstowcs (name, NT_MAX_PATH, namebuf);
  nr.lpRemoteName = name;
  DWORD ret = create_thread_and_wait (GET_RESOURCE_OPENENUM,
				      &nr, &nh, 0, "WNetOpenEnum");
  if (nh.dom)
    WNetCloseEnum (nh.dom);
  return ret != NO_ERROR ? virt_none : virt_directory;
}

fhandler_netdrive::fhandler_netdrive ():
  fhandler_virtual ()
{
}

int __reg2
fhandler_netdrive::fstat (struct stat *buf)
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
  NETRESOURCEW *nro;
  DWORD ret;
  int res;
  tmp_pathbuf tp;

  if (!dir->__d_position)
    {
      size_t len = strlen (get_name ());
      PWCHAR name = NULL;
      NETRESOURCEW nr = { 0 };
      struct net_hdls *nh;

      if (len != 2)	/* // */
	{
	  const char *from;
	  char *to;
	  char *namebuf = (char *) alloca (len + 1);
	  for (to = namebuf, from = get_name (); *from; to++, from++)
	    *to = (*from == '/') ? '\\' : *from;
	  *to = '\0';
	  name = tp.w_get ();
	  sys_mbstowcs (name, NT_MAX_PATH, namebuf);
	}
      nr.lpRemoteName = name;
      nr.dwType = RESOURCETYPE_DISK;
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
  nro = (LPNETRESOURCEW) tp.c_get ();
  ret = create_thread_and_wait (GET_RESOURCE_ENUM, dir->__handle, nro,
				NT_MAX_PATH, "WnetEnumResource");
  if (ret != NO_ERROR)
    res = geterrno_from_win_error (ret);
  else
    {
      dir->__d_position++;
      PWCHAR bs = wcsrchr (nro->lpRemoteName, L'\\');
      bs = bs ? bs + 1 : nro->lpRemoteName;
      if (strlen (get_name ()) == 2)
	{
	  UNICODE_STRING ss, ds;
	  
	  tp.u_get (&ds);
	  RtlInitUnicodeString (&ss, bs);
	  RtlDowncaseUnicodeString (&ds, &ss, FALSE);
	  sys_wcstombs (de->d_name, sizeof de->d_name,
			ds.Buffer, ds.Length / sizeof (WCHAR));
	  de->d_ino = hash_path_name (get_ino (), de->d_name);
	}
      else
	{
	  sys_wcstombs (de->d_name, sizeof de->d_name, bs);
	  char *rpath = tp.c_get ();
	  sys_wcstombs (rpath, NT_MAX_PATH, nro->lpRemoteName);
	  de->d_ino = readdir_get_ino (rpath, false);
	  /* We can't trust remote inode numbers of only 32 bit.  That means,
	     remote NT4 NTFS, as well as shares of Samba version < 3.0. */
	  if (de->d_ino <= UINT32_MAX)
	    de->d_ino = hash_path_name (0, rpath);
	}
      de->d_type = DT_DIR;

      res = 0;
    }
out:
  syscall_printf ("%d = readdir(%p, %p)", res, dir, de);
  return res;
}

void
fhandler_netdrive::seekdir (DIR *dir, long pos)
{
  rewinddir (dir);
  if (pos < 0)
    return;
  while (dir->__d_position < pos)
    if (readdir (dir, dir->__d_dirent))
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
  if ((flags & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL))
    {
      set_errno (EEXIST);
      return 0;
    }
  if (flags & O_WRONLY)
    {
      set_errno (EISDIR);
      return 0;
    }
  /* Open a fake handle to \\Device\\Null */
  return open_null (flags);
}

int
fhandler_netdrive::close ()
{
  /* Skip fhandler_virtual::close, which is a no-op. */
  return fhandler_base::close ();
}

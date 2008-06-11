/* fhandler_procnet.cc: fhandler for /proc/net virtual filesystem

   Copyright 2007 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#define  __INSIDE_CYGWIN_NET__

#include "winsup.h"
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"

#include <netdb.h>
#define USE_SYS_TYPES_FD_SET
#include <winsock2.h>
#include <iphlpapi.h>
#include <asm/byteorder.h>
#include <cygwin/in6.h>

#define _COMPILING_NEWLIB
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

extern "C" int ip_addr_prefix (PIP_ADAPTER_UNICAST_ADDRESS pua,
			       PIP_ADAPTER_PREFIX pap);
bool get_adapters_addresses (PIP_ADAPTER_ADDRESSES *pa0, ULONG family);

static const int PROCNET_IFINET6 = 2;

static const char * const process_listing[] =
{
  ".",
  "..",
  "if_inet6",
  NULL
};

static const int PROCESS_LINK_COUNT =
  (sizeof (process_listing) / sizeof (const char *)) - 1;

static _off64_t format_procnet_ifinet6 (char *&filebuf);

/* Returns 0 if path doesn't exist, >0 if path is a directory,
 * -1 if path is a file, -2 if path is a symlink, -3 if path is a pipe,
 * -4 if path is a socket.
 */
int
fhandler_procnet::exists ()
{
  const char *path = get_name ();
  debug_printf ("exists (%s)", path);
  path += proc_len + 1;
  while (*path != 0 && !isdirsep (*path))
    path++;
  if (*path == 0)
    return 1;

  for (int i = 0; process_listing[i]; i++)
    if (pathmatch (path + 1, process_listing[i]))
      {
	if (i == PROCNET_IFINET6)
	  {
	    if (!wincap.has_gaa_prefixes ()
	    	|| !get_adapters_addresses (NULL, AF_INET6))
	      return 0;
	  }
	fileid = i;
	return -1;
      }
  return 0;
}

fhandler_procnet::fhandler_procnet ():
  fhandler_proc ()
{
}

int
fhandler_procnet::fstat (struct __stat64 *buf)
{
  fhandler_base::fstat (buf);
  buf->st_mode &= ~_IFMT & NO_W;
  int file_type = exists ();
  switch (file_type)
    {
    case 0:
      set_errno (ENOENT);
      return -1;
    case 1:
    case 2:
      buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
      buf->st_nlink = 2;
      return 0;
    case -1:
    default:
      buf->st_mode |= S_IFREG | S_IRUSR | S_IRGRP | S_IROTH;
      return 0;
    }
}

int
fhandler_procnet::readdir (DIR *dir, dirent *de)
{
  int res = ENMFILE;
  if (dir->__d_position >= PROCESS_LINK_COUNT)
    goto out;
  if (dir->__d_position == PROCNET_IFINET6)
    {
      if (!wincap.has_gaa_prefixes ()
	  || !get_adapters_addresses (NULL, AF_INET6))
	goto out;
    }
  strcpy (de->d_name, process_listing[dir->__d_position++]);
  dir->__flags |= dirent_saw_dot | dirent_saw_dot_dot;
  res = 0;
out:
  syscall_printf ("%d = readdir (%p, %p) (%s)", res, dir, de, de->d_name);
  return res;
}

int
fhandler_procnet::open (int flags, mode_t mode)
{
  int process_file_no = -1;

  int res = fhandler_virtual::open (flags, mode);
  if (!res)
    goto out;

  nohandle (true);

  const char *path;
  path = get_name () + proc_len + 1;
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
fhandler_procnet::fill_filebuf ()
{
  switch (fileid)
    {
    case PROCNET_IFINET6:
      {
	bufalloc = filesize = format_procnet_ifinet6 (filebuf);
	break;
      }
    }

  return true;
}

/* Return the same scope values as Linux. */
static unsigned int
get_scope (struct in6_addr *addr)
{
  if (IN6_IS_ADDR_LOOPBACK (addr))
    return 0x10;
  if (IN6_IS_ADDR_LINKLOCAL (addr))
    return 0x20;
  if (IN6_IS_ADDR_SITELOCAL (addr))
    return 0x40;
  if (IN6_IS_ADDR_V4COMPAT (addr))
    return 0x80;
  return 0x0;
}

/* Convert DAD state into Linux compatible values. */
static unsigned int dad_to_flags[] =
{
  0x02,		/* Invalid -> NODAD */
  0x40,		/* Tentative -> TENTATIVE */
  0xc0,		/* Duplicate -> PERMANENT | TENTATIVE */
  0x20,		/* Deprecated -> DEPRECATED */
  0x80		/* Preferred -> PERMANENT */
};

static _off64_t
format_procnet_ifinet6 (char *&filebuf)
{
  PIP_ADAPTER_ADDRESSES pa0 = NULL, pap;
  PIP_ADAPTER_UNICAST_ADDRESS pua;
  ULONG alloclen;

  if (!wincap.has_gaa_prefixes ())
    return 0;
  _off64_t filesize = 0;
  if (!get_adapters_addresses (&pa0, AF_INET6))
    goto out;
  alloclen = 0;
  for (pap = pa0; pap; pap = pap->Next)
    for (pua = pap->FirstUnicastAddress; pua; pua = pua->Next)
      alloclen += 100;
  if (!alloclen)
    goto out;
  filebuf = (char *) crealloc (filebuf, alloclen);
  if (!filebuf)
    goto out;
  for (pap = pa0; pap; pap = pap->Next)
    for (pua = pap->FirstUnicastAddress; pua; pua = pua->Next)
      {
	struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)
				    pua->Address.lpSockaddr;
	for (int i = 0; i < 8; ++i)
	  /* __small_sprintf generates upper-case hex. */
	  filesize += sprintf (filebuf + filesize, "%04x",
			       ntohs (sin6->sin6_addr.s6_addr16[i]));
	filebuf[filesize++] = ' ';
	filesize += sprintf (filebuf + filesize,
			     "%02lx %02x %02x %02x %s\n",
			     pap->Ipv6IfIndex,
			     ip_addr_prefix (pua, pap->FirstPrefix),
			     get_scope (&((struct sockaddr_in6 *)
					pua->Address.lpSockaddr)->sin6_addr),
			     dad_to_flags [pua->DadState],
			     pap->AdapterName);
      }

out:
  if (pa0)
    free (pa0);
  return filesize;
}

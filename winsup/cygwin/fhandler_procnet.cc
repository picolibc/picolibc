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
#include "cygwin/in6.h"

#define _COMPILING_NEWLIB
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#ifndef GAA_FLAG_INCLUDE_ALL_INTERFACES
#define GAA_FLAG_INCLUDE_ALL_INTERFACES 0x0100
#endif

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
	    ULONG size;
	    if (GetAdaptersAddresses (AF_INET6,
				      GAA_FLAG_INCLUDE_PREFIX
				      | GAA_FLAG_INCLUDE_ALL_INTERFACES,
				      NULL, NULL, &size)
		!= ERROR_BUFFER_OVERFLOW)
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

DIR *
fhandler_procnet::opendir ()
{
  DIR *dir = fhandler_virtual::opendir ();
  if (dir)
    dir->__flags = 0;
  return dir;
}

int
fhandler_procnet::readdir (DIR *dir, dirent *de)
{
  int res = ENMFILE;
  if (dir->__d_position >= PROCESS_LINK_COUNT)
    goto out;
  if (dir->__d_position == PROCNET_IFINET6)
    {
      ULONG size;
      if (GetAdaptersAddresses (AF_INET6,
				GAA_FLAG_INCLUDE_PREFIX
				| GAA_FLAG_INCLUDE_ALL_INTERFACES,
				NULL, NULL, &size)
	  != ERROR_BUFFER_OVERFLOW)
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

static int in6_are_prefix_equal(struct in6_addr *, struct in6_addr *, int);

/* Vista: unicast address has additional OnLinkPrefixLength member. */
typedef struct _IP_ADAPTER_UNICAST_ADDRESS_VISTA {
    _ANONYMOUS_UNION union {
        ULONGLONG Alignment;
        _ANONYMOUS_UNION struct {
            ULONG Length;
            DWORD Flags;
        } DUMMYSTRUCTNAME;
    } DUMMYUNIONNAME;
    struct _IP_ADAPTER_UNICAST_ADDRESS_VISTA *Next;
    SOCKET_ADDRESS Address;

    IP_PREFIX_ORIGIN PrefixOrigin;
    IP_SUFFIX_ORIGIN SuffixOrigin;
    IP_DAD_STATE DadState;

    ULONG ValidLifetime;
    ULONG PreferredLifetime;
    ULONG LeaseLifetime;
    unsigned char OnLinkPrefixLength;
} IP_ADAPTER_UNICAST_ADDRESS_VISTA, *PIP_ADAPTER_UNICAST_ADDRESS_VISTA;

int
prefix (PIP_ADAPTER_UNICAST_ADDRESS pua, PIP_ADAPTER_PREFIX pap)
{
  if (wincap.has_gaa_on_link_prefix ())
    return (int) ((PIP_ADAPTER_UNICAST_ADDRESS_VISTA) pua)->OnLinkPrefixLength;
  /* Prior to Vista, the loopback prefix is erroneously set to 0 instead
     of to 128.  So just fake it here... */
  if (IN6_IS_ADDR_LOOPBACK (&((struct sockaddr_in6 *)
			      pua->Address.lpSockaddr)->sin6_addr))
    return 128;
  /* XP prior to service pack 1 has no prefixes linked list.  Let's fake. */
  if (!wincap.has_gaa_prefixes ())
    return 64;
  for ( ; pap; pap = pap->Next)
    if (in6_are_prefix_equal (
          &((struct sockaddr_in6 *) pua->Address.lpSockaddr)->sin6_addr,
          &((struct sockaddr_in6 *) pap->Address.lpSockaddr)->sin6_addr,
          pap->PrefixLength))
    return pap->PrefixLength;
  return 0;
}

static _off64_t
format_procnet_ifinet6 (char *&filebuf)
{
  PIP_ADAPTER_ADDRESSES pa0 = NULL, pap;
  PIP_ADAPTER_UNICAST_ADDRESS pua;
  ULONG ret, size = 0, alloclen;

  _off64_t filesize = 0;
  do
    {
      ret = GetAdaptersAddresses (AF_INET6, GAA_FLAG_INCLUDE_PREFIX
					    | GAA_FLAG_INCLUDE_ALL_INTERFACES,
				  NULL, pa0, &size);
      if (ret == ERROR_BUFFER_OVERFLOW)
	{
	  if (pa0)
	    free (pa0);
	  pa0 = (PIP_ADAPTER_ADDRESSES) malloc (size);
	}
    }
  while (ret == ERROR_BUFFER_OVERFLOW);
  if (ret != ERROR_SUCCESS)
    {
      if (pa0)
	free (pa0);
      return 0;
    }
  alloclen = 0;
  for (pap = pa0; pap; pap = pap->Next)
    {
      ULONG namelen = wcslen (pap->FriendlyName);
      for (pua = pap->FirstUnicastAddress; pua; pua = pua->Next)
	alloclen += 60 + namelen;
    }
  filebuf = (char *) crealloc (filebuf, alloclen);
  filesize = 0;
  for (pap = pa0; pap; pap = pap->Next)
    for (pua = pap->FirstUnicastAddress; pua; pua = pua->Next)
      {
	ULONG namelen = wcslen (pap->FriendlyName);
	struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)
				    pua->Address.lpSockaddr;
	for (int i = 0; i < 8; ++i)
	  /* __small_sprintf generates upper-case hex. */
	  filesize += sprintf (filebuf + filesize, "%04x",
			       ntohs (sin6->sin6_addr.s6_addr16[i]));
	filebuf[filesize++] = ' ';
	filesize += __small_sprintf (filebuf + filesize,
				"%02x %02x %02x %02x ",
				pap->Ipv6IfIndex,
				prefix (pua, pap->FirstPrefix),
				((struct sockaddr_in6 *)
				 pua->Address.lpSockaddr)->sin6_scope_id,
				pua->DadState);
	filesize += sys_wcstombs (filebuf + filesize, alloclen - filesize,
				  pap->FriendlyName, namelen);
	filebuf[filesize++] = '\n';
      }
  if (!filesize)
    filebuf[filesize++] = '\n';
  return filesize;
}

/* The below function has been taken from OpenBSD's src/sys/netinet6/in6.c. */

/*
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Copyright (c) 1982, 1986, 1991, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *      @(#)in.c        8.2 (Berkeley) 11/15/93
 */

static int
in6_are_prefix_equal(struct in6_addr *p1, struct in6_addr *p2, int len)
{
  int bytelen, bitlen;

  /* sanity check */
  if (0 > len || len > 128)
    return 0;
  
  bytelen = len / 8;
  bitlen = len % 8;

  if (memcmp (&p1->s6_addr, &p2->s6_addr, bytelen))
    return 0;
  /* len == 128 is ok because bitlen == 0 then */
  if (bitlen != 0 &&
      p1->s6_addr[bytelen] >> (8 - bitlen) !=
      p2->s6_addr[bytelen] >> (8 - bitlen))
    return 0;

  return 1;
}


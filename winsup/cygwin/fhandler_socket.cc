/* fhandler_socket.cc. See fhandler.h for a description of the fhandler classes.

   Copyright 2000, 2001, 2002 Red Hat, Inc.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

/* #define DEBUG_NEST_ON 1 */

#define  __INSIDE_CYGWIN_NET__

#include "winsup.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <asm/byteorder.h>

#include <stdlib.h>
#define USE_SYS_TYPES_FD_SET
#include <winsock2.h>
#include "cygerrno.h"
#include "security.h"
#include "cygwin/version.h"
#include "perprocess.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "sigproc.h"
#include "wsock_event.h"
#include "cygthread.h"
#include "select.h"
#include <unistd.h>

extern bool fdsock (cygheap_fdmanip& fd, const device *, SOCKET soc);
extern "C" {
int sscanf (const char *, const char *, ...);
} /* End of "C" section */

fhandler_dev_random* entropy_source;

static void
secret_event_name (char *buf, short port, int *secret_ptr)
{
  __small_sprintf (buf, "%scygwin.local_socket.secret.%d.%08x-%08x-%08x-%08x",
		   wincap.has_terminal_services () ? "Global\\" : "",
		   port,
		   secret_ptr [0], secret_ptr [1],
		   secret_ptr [2], secret_ptr [3]);
}

/* cygwin internal: map sockaddr into internet domain address */
static int
get_inet_addr (const struct sockaddr *in, int inlen,
	       struct sockaddr_in *out, int *outlen, int* secret = 0)
{
  int secret_buf [4];
  int* secret_ptr = (secret ? : secret_buf);

  if (in->sa_family == AF_INET)
    {
      *out = * (sockaddr_in *)in;
      *outlen = inlen;
      return 1;
    }
  else if (in->sa_family == AF_LOCAL)
    {
      path_conv pc (in->sa_data, PC_SYM_FOLLOW);
      if (pc.error)
	{
	  set_errno (pc.error);
	  return 0;
	}
      if (!pc.exists ())
	{
	  set_errno (ENOENT);
	  return 0;
	}
      if (!pc.issocket ())
	{
	  set_errno (EBADF);
	  return 0;
	}
      HANDLE fh = CreateFile (pc, GENERIC_READ, wincap.shared (), &sec_none,
			      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
      if (fh == INVALID_HANDLE_VALUE)
	{
	  __seterrno ();
	  return 0;
	}
      int ret = 0;
      DWORD len = 0;
      char buf[128];
      memset (buf, 0, sizeof buf);
      if (ReadFile (fh, buf, 128, &len, 0))
	{
	  sockaddr_in sin;
	  sin.sin_family = AF_INET;
	  sscanf (buf + strlen (SOCKET_COOKIE), "%hu %08x-%08x-%08x-%08x",
		  &sin.sin_port,
		  secret_ptr, secret_ptr + 1, secret_ptr + 2, secret_ptr + 3);
	  sin.sin_port = htons (sin.sin_port);
	  sin.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
	  *out = sin;
	  *outlen = sizeof sin;
	  ret = 1;
	}
      CloseHandle (fh);
      return ret;
    }
  else
    {
      set_errno (EAFNOSUPPORT);
      return 0;
    }
}

/**********************************************************************/
/* fhandler_socket */

fhandler_socket::fhandler_socket ()
  : fhandler_base (), sun_path (NULL)
{
  set_need_fork_fixup ();
  prot_info_ptr = (LPWSAPROTOCOL_INFOA) cmalloc (HEAP_BUF,
						 sizeof (WSAPROTOCOL_INFOA));
#if 0
  if (pc.is_fs_special ())
    {
      fhandler_socket * fhs = (fhandler_socket *) fh;
      fhs->set_addr_family (AF_LOCAL);
      fhs->set_sun_path (posix_path);
    }
#endif
}

fhandler_socket::~fhandler_socket ()
{
  if (prot_info_ptr)
    cfree (prot_info_ptr);
  if (sun_path)
    cfree (sun_path);
}

void
fhandler_socket::set_connect_secret ()
{
  if (!entropy_source)
    {
      void *buf = malloc (sizeof (fhandler_dev_random));
      entropy_source = new (buf) fhandler_dev_random ();
      entropy_source->dev () = *urandom_dev;
    }
  if (entropy_source &&
      !entropy_source->open (O_RDONLY))
    {
      delete entropy_source;
      entropy_source = NULL;
    }
  if (entropy_source)
    {
      size_t len = sizeof (connect_secret);
      entropy_source->read (connect_secret, len);
      if (len != sizeof (connect_secret))
	bzero ((char*) connect_secret, sizeof (connect_secret));
    }
}

void
fhandler_socket::get_connect_secret (char* buf)
{
  __small_sprintf (buf, "%08x-%08x-%08x-%08x",
		   connect_secret [0], connect_secret [1],
		   connect_secret [2], connect_secret [3]);
}

HANDLE
fhandler_socket::create_secret_event (int* secret)
{
  struct sockaddr_in sin;
  int sin_len = sizeof (sin);

  if (::getsockname (get_socket (), (struct sockaddr*) &sin, &sin_len))
    {
      debug_printf ("error getting local socket name (%d)", WSAGetLastError ());
      return NULL;
    }

  char event_name[MAX_PATH];
  secret_event_name (event_name, sin.sin_port, secret ?: connect_secret);
  LPSECURITY_ATTRIBUTES sec = get_inheritance (true);
  secret_event = CreateEvent (sec, FALSE, FALSE, event_name);
  if (!secret_event && GetLastError () == ERROR_ALREADY_EXISTS)
    secret_event = OpenEvent (EVENT_ALL_ACCESS, FALSE, event_name);

  if (!secret_event)
    /* nothing to do */;
  else if (sec == &sec_all_nih || sec == &sec_none_nih)
    ProtectHandle (secret_event);
  else
    ProtectHandleINH (secret_event);

  return secret_event;
}

void
fhandler_socket::signal_secret_event ()
{
  if (!secret_event)
    debug_printf ("no secret event?");
  else
    {
      SetEvent (secret_event);
      debug_printf ("signaled secret_event");
    }
}

void
fhandler_socket::close_secret_event ()
{
  if (secret_event)
    ForceCloseHandle (secret_event);
  secret_event = NULL;
}

int
fhandler_socket::check_peer_secret_event (struct sockaddr_in* peer, int* secret)
{

  char event_name[MAX_PATH];

  secret_event_name (event_name, peer->sin_port, secret ?: connect_secret);
  HANDLE ev = CreateEvent (&sec_all_nih, FALSE, FALSE, event_name);
  if (!ev && GetLastError () == ERROR_ALREADY_EXISTS)
    {
      debug_printf ("event \"%s\" already exists", event_name);
      ev = OpenEvent (EVENT_ALL_ACCESS, FALSE, event_name);
    }

  signal_secret_event ();

  if (ev)
    {
      DWORD rc = WaitForSingleObject (ev, 10000);
      debug_printf ("WFSO rc=%d", rc);
      CloseHandle (ev);
      return (rc == WAIT_OBJECT_0 ? 1 : 0 );
    }
  else
    return 0;
}

void
fhandler_socket::fixup_before_fork_exec (DWORD win_proc_id)
{
  if (!winsock2_active)
    {
      fhandler_base::fixup_before_fork_exec (win_proc_id);
      debug_printf ("Without Winsock 2.0");
    }
  else if (!WSADuplicateSocketA (get_socket (), win_proc_id, prot_info_ptr))
    debug_printf ("WSADuplicateSocket went fine, sock %p, win_proc_id %d, prot_info_ptr %p",
		  get_socket (), win_proc_id, prot_info_ptr);
  else
    {
      debug_printf ("WSADuplicateSocket error, sock %p, win_proc_id %d, prot_info_ptr %p",
		    get_socket (), win_proc_id, prot_info_ptr);
      set_winsock_errno ();
    }
}

extern "C" void __stdcall load_wsock32 ();
void
fhandler_socket::fixup_after_fork (HANDLE parent)
{
  SOCKET new_sock;

  debug_printf ("WSASocket begin, dwServiceFlags1=%d",
		prot_info_ptr->dwServiceFlags1);

  if ((new_sock = WSASocketA (FROM_PROTOCOL_INFO,
				   FROM_PROTOCOL_INFO,
				   FROM_PROTOCOL_INFO,
				   prot_info_ptr, 0, 0)) == INVALID_SOCKET)
    {
      debug_printf ("WSASocket error");
      set_io_handle ((HANDLE)INVALID_SOCKET);
      set_winsock_errno ();
    }
  else if (!new_sock && !winsock2_active)
    {
      load_wsock32 ();
      fhandler_base::fixup_after_fork (parent);
      debug_printf ("Without Winsock 2.0");
    }
  else
    {
      debug_printf ("WSASocket went fine new_sock %p, old_sock %p", new_sock, get_io_handle ());
      set_io_handle ((HANDLE) new_sock);
    }

  if (secret_event)
    fork_fixup (parent, secret_event, "secret_event");
}

void
fhandler_socket::fixup_after_exec (HANDLE parent)
{
  debug_printf ("here");
  if (!get_close_on_exec ())
    fixup_after_fork (parent);
#if 0
  else if (!winsock2_active)
    closesocket (get_socket ());
#endif
}

int
fhandler_socket::dup (fhandler_base *child)
{
  debug_printf ("here");
  fhandler_socket *fhs = (fhandler_socket *) child;
  fhs->addr_family = addr_family;
  if (get_addr_family () == AF_LOCAL)
    fhs->set_sun_path (get_sun_path ());
  fhs->set_socket_type (get_socket_type ());
  fhs->set_connect_state (get_connect_state ());

  if (winsock2_active)
    {
      /* Since WSADuplicateSocket() fails on NT systems when the process
	 is currently impersonating a non-privileged account, we revert
	 to the original account before calling WSADuplicateSocket() and
	 switch back afterwards as it's also in fork().
	 If WSADuplicateSocket() still fails for some reason, we fall back
	 to DuplicateHandle(). */
      WSASetLastError (0);
      cygheap->user.deimpersonate ();
      fhs->set_io_handle (get_io_handle ());
      fhs->fixup_before_fork_exec (GetCurrentProcessId ());
      cygheap->user.reimpersonate ();
      if (!WSAGetLastError ())
	{
	  fhs->fixup_after_fork (hMainProc);
	  if (fhs->get_io_handle() != (HANDLE) INVALID_SOCKET)
	    return 0;
	}
      debug_printf ("WSADuplicateSocket failed, trying DuplicateHandle");
    }

  /* We don't call fhandler_base::dup here since that requires to
     have winsock called from fhandler_base and it creates only
     inheritable sockets which is wrong for winsock2. */

  HANDLE nh;
  if (!DuplicateHandle (hMainProc, get_io_handle (), hMainProc, &nh, 0,
			!winsock2_active, DUPLICATE_SAME_ACCESS))
    {
      system_printf ("!DuplicateHandle(%x) failed, %E", get_io_handle ());
      __seterrno ();
      return -1;
    }
  fhs->set_io_handle (nh);
  return 0;
}

int __stdcall
fhandler_socket::fstat (struct __stat64 *buf)
{
  int res = fhandler_base::fstat (buf);
  if (!res)
    {
      if (get_socket_type ()) /* fstat */
	{
	  buf->st_dev = 0;
	  buf->st_ino = (__ino64_t) ((DWORD) get_handle ());
	  buf->st_mode = S_IFSOCK | S_IRWXU | S_IRWXG | S_IRWXO;
	}
      else
	{
	  path_conv spc ("/dev", PC_SYM_NOFOLLOW | PC_NULLEMPTY, NULL);
	  buf->st_dev = spc.volser ();
	  buf->st_ino = get_namehash ();
	  buf->st_mode &= ~S_IRWXO;
	  buf->st_rdev = (get_device () << 16) | get_unit ();
	}
    }
  return res;
}

int
fhandler_socket::bind (const struct sockaddr *name, int namelen)
{
  int res = -1;

  if (name->sa_family == AF_LOCAL)
    {
#define un_addr ((struct sockaddr_un *) name)
      struct sockaddr_in sin;
      int len = sizeof sin;

      if (strlen (un_addr->sun_path) >= UNIX_PATH_LEN)
	{
	  set_errno (ENAMETOOLONG);
	  goto out;
	}
      sin.sin_family = AF_INET;
      sin.sin_port = 0;
      sin.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
      if (::bind (get_socket (), (sockaddr *) &sin, len))
	{
	  syscall_printf ("AF_LOCAL: bind failed %d", get_errno ());
	  set_winsock_errno ();
	  goto out;
	}
      if (::getsockname (get_socket (), (sockaddr *) &sin, &len))
	{
	  syscall_printf ("AF_LOCAL: getsockname failed %d", get_errno ());
	  set_winsock_errno ();
	  goto out;
	}

      sin.sin_port = ntohs (sin.sin_port);
      debug_printf ("AF_LOCAL: socket bound to port %u", sin.sin_port);

      path_conv pc (un_addr->sun_path, PC_SYM_FOLLOW);
      if (pc.error)
	{
	  set_errno (pc.error);
	  goto out;
	}
      if (pc.exists ())
	{
	  set_errno (EADDRINUSE);
	  goto out;
	}
      mode_t mode = (S_IRWXU | S_IRWXG | S_IRWXO) & ~cygheap->umask;
      DWORD attr = FILE_ATTRIBUTE_SYSTEM;
      if (!(mode & (S_IWUSR | S_IWGRP | S_IWOTH)))
	attr |= FILE_ATTRIBUTE_READONLY;
      SECURITY_ATTRIBUTES sa = sec_none;
      if (allow_ntsec && pc.has_acls ())
	set_security_attribute (mode, &sa, alloca (4096), 4096);
      HANDLE fh = CreateFile (pc, GENERIC_WRITE, 0, &sa, CREATE_NEW, attr, 0);
      if (fh == INVALID_HANDLE_VALUE)
	{
	  if (GetLastError () == ERROR_ALREADY_EXISTS)
	    set_errno (EADDRINUSE);
	  else
	    __seterrno ();
	}

      set_connect_secret ();

      char buf[sizeof (SOCKET_COOKIE) + 80];
      __small_sprintf (buf, "%s%u ", SOCKET_COOKIE, sin.sin_port);
      get_connect_secret (strchr (buf, '\0'));
      DWORD blen = strlen (buf) + 1;
      if (!WriteFile (fh, buf, blen, &blen, 0))
	{
	  __seterrno ();
	  CloseHandle (fh);
	  DeleteFile (pc);
	}
      else
	{
	  CloseHandle (fh);
	  set_sun_path (un_addr->sun_path);
	  res = 0;
	}
#undef un_addr
    }
  else if (::bind (get_socket (), name, namelen))
    set_winsock_errno ();
  else
    res = 0;

out:
  return res;
}

int
fhandler_socket::connect (const struct sockaddr *name, int namelen)
{
  int res = -1;
  BOOL secret_check_failed = FALSE;
  BOOL in_progress = FALSE;
  sockaddr_in sin;
  int secret [4];
  DWORD err;

  if (!get_inet_addr (name, namelen, &sin, &namelen, secret))
    return -1;

  res = ::connect (get_socket (), (sockaddr *) &sin, namelen);

  if (res)
    {
      /* Special handling for connect to return the correct error code
	 when called on a non-blocking socket. */
      if (is_nonblocking () || is_connect_pending ())
	{
	  err = WSAGetLastError ();
	  if (err == WSAEWOULDBLOCK || err == WSAEALREADY)
	    in_progress = TRUE;

	  if (err == WSAEWOULDBLOCK)
	    WSASetLastError (WSAEINPROGRESS);
	  else if (err == WSAEINVAL)
	    WSASetLastError (WSAEISCONN);
	}
      set_winsock_errno ();
    }
  if (get_addr_family () == AF_LOCAL && get_socket_type () == SOCK_STREAM)
    {
      if (!res || in_progress)
	{
	  if (!create_secret_event (secret))
	    {
	      secret_check_failed = TRUE;
	    }
	  else if (in_progress)
	    signal_secret_event ();
	}

      if (!secret_check_failed && !res)
	{
	  if (!check_peer_secret_event (&sin, secret))
	    {
	      debug_printf ( "accept from unauthorized server" );
	      secret_check_failed = TRUE;
	    }
       }

      if (secret_check_failed)
	{
	  close_secret_event ();
	  if (res)
	    closesocket (res);
	  set_errno (ECONNREFUSED);
	  res = -1;
	}
    }

  err = WSAGetLastError ();
  if (err == WSAEINPROGRESS || err == WSAEALREADY)
    set_connect_state (CONNECT_PENDING);
  else
    set_connect_state (CONNECTED);

  return res;
}

int
fhandler_socket::listen (int backlog)
{
  int res = ::listen (get_socket (), backlog);
  if (res)
    set_winsock_errno ();
  else
    set_connect_state (CONNECTED);
  return res;
}

int
fhandler_socket::accept (struct sockaddr *peer, int *len)
{
  int res = -1;
  BOOL secret_check_failed = FALSE;
  BOOL in_progress = FALSE;

  /* Allows NULL peer and len parameters. */
  struct sockaddr_in peer_dummy;
  int len_dummy;
  if (!peer)
    peer = (struct sockaddr *) &peer_dummy;
  if (!len)
    {
      len_dummy = sizeof (struct sockaddr_in);
      len = &len_dummy;
    }

  /* accept on NT fails if len < sizeof (sockaddr_in)
   * some programs set len to
   * sizeof (name.sun_family) + strlen (name.sun_path) for UNIX domain
   */
  if (len && ((unsigned) *len < sizeof (struct sockaddr_in)))
    *len = sizeof (struct sockaddr_in);

  res = ::accept (get_socket (), peer, len);

  if ((SOCKET) res == INVALID_SOCKET && WSAGetLastError () == WSAEWOULDBLOCK)
    in_progress = TRUE;

  if (get_addr_family () == AF_LOCAL && get_socket_type () == SOCK_STREAM)
    {
      if ((SOCKET) res != INVALID_SOCKET || in_progress)
	{
	  if (!create_secret_event ())
	    secret_check_failed = TRUE;
	  else if (in_progress)
	    signal_secret_event ();
	}

      if (!secret_check_failed &&
	  (SOCKET) res != INVALID_SOCKET)
	{
	  if (!check_peer_secret_event ((struct sockaddr_in*) peer))
	    {
	      debug_printf ("connect from unauthorized client");
	      secret_check_failed = TRUE;
	    }
	}

      if (secret_check_failed)
	{
	  close_secret_event ();
	  if ((SOCKET) res != INVALID_SOCKET)
	    closesocket (res);
	  set_errno (ECONNABORTED);
	  return -1;
	}
    }

  if ((SOCKET) res == INVALID_SOCKET)
    set_winsock_errno ();
  else
    {
      cygheap_fdnew res_fd;
      if (res_fd >= 0 && fdsock (res_fd, &dev (), res))
        {
	  if (get_addr_family () == AF_LOCAL)
	    ((fhandler_socket *) res_fd)->set_sun_path (get_sun_path ());
	  ((fhandler_socket *) res_fd)->set_addr_family (get_addr_family ());
	  ((fhandler_socket *) res_fd)->set_socket_type (get_socket_type ());
	  ((fhandler_socket *) res_fd)->set_connect_state (CONNECTED);
	  res = res_fd;
	}
      else
	{
	  closesocket (res);
	  res = -1;
	}
    }

  debug_printf ("res %d", res);
  return res;
}

int
fhandler_socket::getsockname (struct sockaddr *name, int *namelen)
{
  int res = -1;

  if (get_addr_family () == AF_LOCAL)
    {
      struct sockaddr_un *sun = (struct sockaddr_un *) name;
      memset (sun, 0, *namelen);
      sun->sun_family = AF_LOCAL;

      if (!get_sun_path ())
	sun->sun_path[0] = '\0';
      else
	/* According to SUSv2 "If the actual length of the address is
	   greater than the length of the supplied sockaddr structure, the
	   stored address will be truncated."  We play it save here so
	   that the path always has a trailing 0 even if it's truncated. */
	strncpy (sun->sun_path, get_sun_path (),
		 *namelen - sizeof *sun + sizeof sun->sun_path - 1);

      *namelen = sizeof *sun - sizeof sun->sun_path
		 + strlen (sun->sun_path) + 1;
      res = 0;
    }
  else
    {
      res = ::getsockname (get_socket (), name, namelen);
      if (res)
	set_winsock_errno ();
    }

  return res;
}

int
fhandler_socket::getpeername (struct sockaddr *name, int *namelen)
{
  int res = ::getpeername (get_socket (), name, namelen);
  if (res)
    set_winsock_errno ();

  return res;
}

int
fhandler_socket::readv (const struct iovec *const iov, const int iovcnt,
			ssize_t tot)
{
  struct msghdr msg =
    {
      msg_name:		NULL,
      msg_namelen:	0,
      msg_iov:		(struct iovec *) iov, // const_cast
      msg_iovlen:	iovcnt,
      msg_accrights:	NULL,
      msg_accrightslen:	0
    };

  return recvmsg (&msg, 0, tot);
}

int
fhandler_socket::recvfrom (void *ptr, size_t len, int flags,
			   struct sockaddr *from, int *fromlen)
{
  int res;
  DWORD ret;

  flags &= MSG_WINMASK;
  if (!winsock2_active)
    ret = res = ::recvfrom (get_socket (),
			    (char *) ptr, len, flags,
			    from, fromlen);
  else
    {
      WSABUF wsabuf = { len, (char *) ptr };

      if (is_nonblocking ())
	res = WSARecvFrom (get_socket (), &wsabuf, 1, &ret, (DWORD *) &flags,
			   from, fromlen,
			   NULL, NULL);
      else
	{
	  wsock_event wsock_evt;
	  res = WSARecvFrom (get_socket (), &wsabuf, 1, &ret, (DWORD *) &flags,
			     from, fromlen,
			     wsock_evt.prepare (), NULL);

	  if (res == SOCKET_ERROR && WSAGetLastError () == WSA_IO_PENDING)
	    ret = res = wsock_evt.wait (get_socket (), (DWORD *) &flags);
	}
    }

  if (res == SOCKET_ERROR)
    {
      /* According to SUSv3, errno isn't set in that case and no error
	 condition is returned. */
      if (WSAGetLastError () == WSAEMSGSIZE)
	return len;

      res = -1;
      set_winsock_errno ();
    }
  else
    res = ret;

  return res;
}

int
fhandler_socket::recvmsg (struct msghdr *msg, int flags, ssize_t tot)
{
  if (get_addr_family () == AF_LOCAL)
    {
      /* On AF_LOCAL sockets the (fixed-size) name of the shared memory
	 area used for descriptor passing is transmitted first.
	 If this string is empty, no descriptors are passed and we can
	 go ahead recv'ing the normal data blocks.  Otherwise start
	 special handling for descriptor passing. */
      /*TODO*/
    }

  struct iovec *const iov = msg->msg_iov;
  const int iovcnt = msg->msg_iovlen;

  struct sockaddr *from = (struct sockaddr *) msg->msg_name;
  int *fromlen = from ? &msg->msg_namelen : NULL;

  int res;

  if (!winsock2_active)
    {
      if (iovcnt == 1)
	res = recvfrom (iov->iov_base, iov->iov_len, flags,
			from, fromlen);
      else
	{
	  if (tot == -1)	// i.e. if not pre-calculated by the caller.
	    {
	      tot = 0;
	      const struct iovec *iovptr = iov + iovcnt;
	      do
		{
		  iovptr -= 1;
		  tot += iovptr->iov_len;
		}
	      while (iovptr != iov);
	    }

	  char *buf = (char *) alloca (tot);

	  if (!buf)
	    {
	      set_errno (ENOMEM);
	      res = -1;
	    }
	  else
	    {
	      res = recvfrom (buf, tot, flags,
			      from, fromlen);

	      const struct iovec *iovptr = iov;
	      int nbytes = res;

	      while (nbytes > 0)
		{
		  const int frag = min (nbytes, (ssize_t) iovptr->iov_len);
		  memcpy (iovptr->iov_base, buf, frag);
		  buf += frag;
		  iovptr += 1;
		  nbytes -= frag;
		}
	    }
	}
    }
  else
    {
      WSABUF wsabuf[iovcnt];
      unsigned long len = 0L;

      {
	const struct iovec *iovptr = iov + iovcnt;
	WSABUF *wsaptr = wsabuf + iovcnt;
	do
	  {
	    iovptr -= 1;
	    wsaptr -= 1;
	    len += wsaptr->len = iovptr->iov_len;
	    wsaptr->buf = (char *) iovptr->iov_base;
	  }
	while (wsaptr != wsabuf);
      }

      DWORD ret;

      if (is_nonblocking ())
	res = WSARecvFrom (get_socket (),
			   wsabuf, iovcnt, &ret, (DWORD *) &flags,
			   from, fromlen,
			   NULL, NULL);
      else
	{
	  wsock_event wsock_evt;
	  res = WSARecvFrom (get_socket (),
			     wsabuf, iovcnt, &ret, (DWORD *) &flags,
			     from, fromlen,
			     wsock_evt.prepare (), NULL);

	  if (res == SOCKET_ERROR && WSAGetLastError () == WSA_IO_PENDING)
	    ret = res = wsock_evt.wait (get_socket (), (DWORD *) &flags);
	}

      if (res == SOCKET_ERROR)
	{
	  /* According to SUSv3, errno isn't set in that case and no error
	     condition is returned. */
	  if (WSAGetLastError () == WSAEMSGSIZE)
	    return len;

	  res = -1;
	  set_winsock_errno ();
	}
      else
	res = ret;
    }

  return res;
}

int
fhandler_socket::writev (const struct iovec *const iov, const int iovcnt,
			 ssize_t tot)
{
  struct msghdr msg =
    {
      msg_name:		NULL,
      msg_namelen:	0,
      msg_iov:		(struct iovec *) iov, // const_cast
      msg_iovlen:	iovcnt,
      msg_accrights:	NULL,
      msg_accrightslen:	0
    };

  return sendmsg (&msg, 0, tot);
}

int
fhandler_socket::sendto (const void *ptr, size_t len, int flags,
			 const struct sockaddr *to, int tolen)
{
  sockaddr_in sin;

  if (to && !get_inet_addr (to, tolen, &sin, &tolen))
    return -1;

  int res;
  DWORD ret;

  if (!winsock2_active)
    ret = res = ::sendto (get_socket (), (const char *) ptr, len,
			  flags & MSG_WINMASK,
			  (to ? (const struct sockaddr *) &sin : NULL), tolen);
  else
    {
      WSABUF wsabuf = { len, (char *) ptr };

      if (is_nonblocking ())
	res = WSASendTo (get_socket (), &wsabuf, 1, &ret,
			 flags & MSG_WINMASK,
			 (to ? (const struct sockaddr *) &sin : NULL), tolen,
			 NULL, NULL);
      else
	{
	  wsock_event wsock_evt;
	  res = WSASendTo (get_socket (), &wsabuf, 1, &ret,
			   flags & MSG_WINMASK,
			   (to ? (const struct sockaddr *) &sin : NULL), tolen,
			   wsock_evt.prepare (), NULL);

	  if (res == SOCKET_ERROR && WSAGetLastError () == WSA_IO_PENDING)
	    ret = res = wsock_evt.wait (get_socket (), (DWORD *) &flags);
	}
    }

  if (res == SOCKET_ERROR)
    {
      res = -1;
      set_winsock_errno ();
    }
  else
    res = ret;

  /* Special handling for EPIPE and SIGPIPE.

     EPIPE is generated if the local end has been shut down on a connection
     oriented socket.  In this case the process will also receive a SIGPIPE
     unless MSG_NOSIGNAL is set.  */
  if (res == -1 && get_errno () == ESHUTDOWN
      && get_socket_type () == SOCK_STREAM)
    {
      set_errno (EPIPE);
      if (! (flags & MSG_NOSIGNAL))
	raise (SIGPIPE);
    }

  return res;
}

int
fhandler_socket::sendmsg (const struct msghdr *msg, int flags, ssize_t tot)
{
  if (get_addr_family () == AF_LOCAL)
    {
      /* For AF_LOCAL/AF_UNIX sockets, if descriptors are given, start
	 the special handling for descriptor passing.  Otherwise just
	 transmit an empty string to tell the receiver that no
	 descriptor passing is done. */
      /*TODO*/
    }

  struct iovec *const iov = msg->msg_iov;
  const int iovcnt = msg->msg_iovlen;

  int res;

  if (!winsock2_active)
    {
      if (iovcnt == 1)
	res = sendto (iov->iov_base, iov->iov_len, flags,
		      (struct sockaddr *) msg->msg_name,
		      msg->msg_namelen);
      else
	{
	  if (tot == -1)	// i.e. if not pre-calculated by the caller.
	    {
	      tot = 0;
	      const struct iovec *iovptr = iov + iovcnt;
	      do
		{
		  iovptr -= 1;
		  tot += iovptr->iov_len;
		}
	      while (iovptr != iov);
	    }

	  char *const buf = (char *) alloca (tot);

	  if (!buf)
	    {
	      set_errno (ENOMEM);
	      res = -1;
	    }
	  else
	    {
	      char *bufptr = buf;
	      const struct iovec *iovptr = iov;
	      int nbytes = tot;

	      while (nbytes != 0)
		{
		  const int frag = min (nbytes, (ssize_t) iovptr->iov_len);
		  memcpy (bufptr, iovptr->iov_base, frag);
		  bufptr += frag;
		  iovptr += 1;
		  nbytes -= frag;
		}

	      res = sendto (buf, tot, flags,
			    (struct sockaddr *) msg->msg_name,
			    msg->msg_namelen);
	    }
	}
    }
  else
    {
      WSABUF wsabuf[iovcnt];

      {
	const struct iovec *iovptr = iov + iovcnt;
	WSABUF *wsaptr = wsabuf + iovcnt;
	do
	  {
	    iovptr -= 1;
	    wsaptr -= 1;
	    wsaptr->len = iovptr->iov_len;
	    wsaptr->buf = (char *) iovptr->iov_base;
	  }
	while (wsaptr != wsabuf);
      }

      DWORD ret;

      if (is_nonblocking ())
	res = WSASendTo (get_socket (), wsabuf, iovcnt, &ret, flags,
			 (struct sockaddr *) msg->msg_name,
			 msg->msg_namelen,
			 NULL, NULL);
      else
	{
	  wsock_event wsock_evt;
	  res = WSASendTo (get_socket (), wsabuf, iovcnt, &ret, flags,
			   (struct sockaddr *) msg->msg_name,
			   msg->msg_namelen,
			   wsock_evt.prepare (), NULL);

	  if (res == SOCKET_ERROR && WSAGetLastError () == WSA_IO_PENDING)
	    ret = res = wsock_evt.wait (get_socket (), (DWORD *) &flags);
	}

      if (res == SOCKET_ERROR)
	{
	  res = -1;
	  set_winsock_errno ();
	}
      else
	res = ret;
    }

  return res;
}

int
fhandler_socket::shutdown (int how)
{
  int res = ::shutdown (get_socket (), how);

  if (res)
    set_winsock_errno ();
  else
    switch (how)
      {
      case SHUT_RD:
	set_shutdown_read ();
	break;
      case SHUT_WR:
	set_shutdown_write ();
	break;
      case SHUT_RDWR:
	set_shutdown_read ();
	set_shutdown_write ();
	break;
      }
  return res;
}

int
fhandler_socket::close ()
{
  int res = 0;

  /* HACK to allow a graceful shutdown even if shutdown() hasn't been
     called by the application. Note that this isn't the ultimate
     solution but it helps in many cases. */
  struct linger linger;
  linger.l_onoff = 1;
  linger.l_linger = 240; /* seconds. default 2MSL value according to MSDN. */
  setsockopt (get_socket (), SOL_SOCKET, SO_LINGER,
	      (const char *)&linger, sizeof linger);

  while ((res = closesocket (get_socket ())) != 0)
    {
      if (WSAGetLastError () != WSAEWOULDBLOCK)
	{
	  set_winsock_errno ();
	  res = -1;
	  break;
	}
      if (WaitForSingleObject (signal_arrived, 10) == WAIT_OBJECT_0)
	{
	  set_errno (EINTR);
	  res = -1;
	  break;
	}
      WSASetLastError (0);
    }

  close_secret_event ();

  debug_printf ("%d = fhandler_socket::close()", res);
  return res;
}

#define ASYNC_MASK (FD_READ|FD_WRITE|FD_OOB|FD_ACCEPT|FD_CONNECT)

int
fhandler_socket::ioctl (unsigned int cmd, void *p)
{
  extern int get_ifconf (struct ifconf *ifc, int what); /* net.cc */
  int res;
  struct ifconf ifc, *ifcp;
  struct ifreq *ifr, *ifrp;

  switch (cmd)
    {
    case SIOCGIFCONF:
      ifcp = (struct ifconf *) p;
      if (!ifcp)
	{
	  set_errno (EINVAL);
	  return -1;
	}
      res = get_ifconf (ifcp, cmd);
      if (res)
	debug_printf ("error in get_ifconf");
      break;
    case SIOCGIFFLAGS:
      ifr = (struct ifreq *) p;
      if (ifr == 0)
	{
	  set_errno (EINVAL);
	  return -1;
	}
      ifr->ifr_flags = IFF_NOTRAILERS | IFF_UP | IFF_RUNNING;
      if (!strncmp(ifr->ifr_name, "lo", 2)
	  || ntohl (((struct sockaddr_in *) &ifr->ifr_addr)->sin_addr.s_addr)
	  == INADDR_LOOPBACK)
	ifr->ifr_flags |= IFF_LOOPBACK;
      else
	ifr->ifr_flags |= IFF_BROADCAST;
      res = 0;
      break;
    case SIOCGIFBRDADDR:
    case SIOCGIFNETMASK:
    case SIOCGIFADDR:
    case SIOCGIFHWADDR:
    case SIOCGIFMETRIC:
    case SIOCGIFMTU:
      {
	ifc.ifc_len = 2048;
	ifc.ifc_buf = (char *) alloca (2048);

	ifr = (struct ifreq *) p;
	if (ifr == 0)
	  {
	    debug_printf ("ifr == NULL");
	    set_errno (EINVAL);
	    return -1;
	  }

	res = get_ifconf (&ifc, cmd);
	if (res)
	  {
	    debug_printf ("error in get_ifconf");
	    break;
	  }

	debug_printf ("    name: %s", ifr->ifr_name);
	for (ifrp = ifc.ifc_req;
	     (caddr_t) ifrp < ifc.ifc_buf + ifc.ifc_len;
	     ++ifrp)
	  {
	    debug_printf ("testname: %s", ifrp->ifr_name);
	    if (! strcmp (ifrp->ifr_name, ifr->ifr_name))
	      {
		switch (cmd)
		  {
		  case SIOCGIFADDR:
		    ifr->ifr_addr = ifrp->ifr_addr;
		    break;
		  case SIOCGIFBRDADDR:
		    ifr->ifr_broadaddr = ifrp->ifr_broadaddr;
		    break;
		  case SIOCGIFNETMASK:
		    ifr->ifr_netmask = ifrp->ifr_netmask;
		    break;
		  case SIOCGIFHWADDR:
		    ifr->ifr_hwaddr = ifrp->ifr_hwaddr;
		    break;
		  case SIOCGIFMETRIC:
		    ifr->ifr_metric = ifrp->ifr_metric;
		    break;
		  case SIOCGIFMTU:
		    ifr->ifr_mtu = ifrp->ifr_mtu;
		    break;
		  }
		break;
	      }
	  }
	if ((caddr_t) ifrp >= ifc.ifc_buf + ifc.ifc_len)
	  {
	    set_errno (EINVAL);
	    return -1;
	  }
	break;
      }
    case FIOASYNC:
      res = WSAAsyncSelect (get_socket (), gethwnd (), WM_ASYNCIO,
	      *(int *) p ? ASYNC_MASK : 0);
      syscall_printf ("Async I/O on socket %s",
	      *(int *) p ? "started" : "cancelled");
      set_async (*(int *) p);
      break;
    default:
      /* We must cancel WSAAsyncSelect (if any) before setting socket to
       * blocking mode
       */
      if (cmd == FIONBIO && *(int *) p == 0)
	WSAAsyncSelect (get_socket (), gethwnd (), 0, 0);
      res = ioctlsocket (get_socket (), cmd, (unsigned long *) p);
      if (res == SOCKET_ERROR)
	  set_winsock_errno ();
      if (cmd == FIONBIO)
	{
	  syscall_printf ("socket is now %sblocking",
			    *(int *) p ? "non" : "");
	  /* Start AsyncSelect if async socket unblocked */
	  if (*(int *) p && get_async ())
	    WSAAsyncSelect (get_socket (), gethwnd (), WM_ASYNCIO, ASYNC_MASK);

	  set_nonblocking (*(int *) p);
	}
      break;
    }
  syscall_printf ("%d = ioctl_socket (%x, %x)", res, cmd, p);
  return res;
}

int
fhandler_socket::fcntl (int cmd, void *arg)
{
  int res = 0;
  int request, current;

  switch (cmd)
    {
    case F_SETFL:
      {
	/* Carefully test for the O_NONBLOCK or deprecated OLD_O_NDELAY flag.
	   Set only the flag that has been passed in.  If both are set, just
	   record O_NONBLOCK.   */
	int new_flags = (int) arg & O_NONBLOCK_MASK;
	if ((new_flags & OLD_O_NDELAY) && (new_flags & O_NONBLOCK))
	  new_flags = O_NONBLOCK;
	current = get_flags () & O_NONBLOCK_MASK;
	request = new_flags ? 1 : 0;
	if (!!current != !!new_flags && (res = ioctl (FIONBIO, &request)))
	  break;
	set_flags ((get_flags () & ~O_NONBLOCK_MASK) | new_flags);
	break;
      }
    default:
      res = fhandler_base::fcntl (cmd, arg);
      break;
    }
  return res;
}

void
fhandler_socket::set_close_on_exec (int val)
{
  if (!winsock2_active) /* < Winsock 2.0 */
    set_inheritance (get_handle (), val);
  set_close_on_exec_flag (val);
  debug_printf ("set close_on_exec for %s to %d", get_name (), val);
}

void
fhandler_socket::set_sun_path (const char *path)
{
  sun_path = path ? cstrdup (path) : NULL;
}

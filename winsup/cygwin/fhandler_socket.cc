/* fhandler_socket.cc. See fhandler.h for a description of the fhandler classes.

   Copyright 2000, 2001, 2002 Red Hat, Inc.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

/* #define DEBUG_NEST_ON 1 */

#define  __INSIDE_CYGWIN_NET__

#include "winsup.h"
#include <errno.h>
#include <sys/socket.h>
#include <asm/byteorder.h>

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#define USE_SYS_TYPES_FD_SET
#include <winsock2.h>
#include "cygerrno.h"
#include "security.h"
#include "cygwin/version.h"
#include "perprocess.h"
#include "fhandler.h"
#include "path.h"
#include "dtable.h"
#include "cygheap.h"
#include "sigproc.h"
#include "wsock_event.h"

#define SECRET_EVENT_NAME "cygwin.local_socket.secret.%d.%08x-%08x-%08x-%08x"
#define ENTROPY_SOURCE_NAME "/dev/urandom"
#define ENTROPY_SOURCE_DEV_UNIT 9

fhandler_dev_random* entropy_source;

/**********************************************************************/
/* fhandler_socket */

fhandler_socket::fhandler_socket ()
  : fhandler_base (FH_SOCKET), sun_path (NULL)
{
  set_need_fork_fixup ();
  prot_info_ptr = (LPWSAPROTOCOL_INFOA) cmalloc (HEAP_BUF,
						 sizeof (WSAPROTOCOL_INFOA));
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
      entropy_source = new (buf) fhandler_dev_random (ENTROPY_SOURCE_DEV_UNIT);
    }
  if (entropy_source &&
      !entropy_source->open (NULL, O_RDONLY))
    {
      delete entropy_source;
      entropy_source = NULL;
    }
  if (!entropy_source ||
      (entropy_source->read (connect_secret, sizeof (connect_secret)) !=
					     sizeof (connect_secret)))
    bzero ((char*) connect_secret, sizeof (connect_secret));
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
  char buf [128];
  int* secret_ptr = (secret ? : connect_secret);
  struct sockaddr_in sin;
  int sin_len = sizeof (sin);

  if (getsockname (get_socket (), (struct sockaddr*) &sin, &sin_len))
    {
      debug_printf ("error getting local socket name (%d)", WSAGetLastError ());
      return NULL;
    }

  __small_sprintf (buf, SECRET_EVENT_NAME, sin.sin_port,
		   secret_ptr [0], secret_ptr [1],
		   secret_ptr [2], secret_ptr [3]);
  secret_event = CreateEvent (get_inheritance(true), FALSE, FALSE, buf);
  if (!secret_event && GetLastError () == ERROR_ALREADY_EXISTS)
    secret_event = OpenEvent (EVENT_ALL_ACCESS, FALSE, buf);

  if (secret_event)
    ProtectHandle (secret_event);

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
  char buf [128];
  HANDLE ev;
  int* secret_ptr = (secret ? : connect_secret);

  __small_sprintf (buf, SECRET_EVENT_NAME, peer->sin_port,
		  secret_ptr [0], secret_ptr [1],
		  secret_ptr [2], secret_ptr [3]);
  ev = CreateEvent (&sec_all_nih, FALSE, FALSE, buf);
  if (!ev && GetLastError () == ERROR_ALREADY_EXISTS)
    {
      debug_printf ("%s event already exist");
      ev = OpenEvent (EVENT_ALL_ACCESS, FALSE, buf);
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
  fhs->set_io_handle (get_io_handle ());

  fhs->fixup_before_fork_exec (GetCurrentProcessId ());
  if (winsock2_active)
    {
      fhs->fixup_after_fork (hMainProc);
      return 0;
    }
  return fhandler_base::dup (child);
}

int __stdcall
fhandler_socket::fstat (struct __stat64 *buf, path_conv *pc)
{
  fhandler_disk_file fh;
  fh.set_name (get_name (), get_win32_name ());
  return fh.fstat (buf, pc);
}

int
fhandler_socket::recv (void *ptr, size_t len, unsigned int flags)
{
  int res = -1;
  wsock_event wsock_evt;
  LPWSAOVERLAPPED ovr;

  sigframe thisframe (mainthread);
  if (is_nonblocking () || !(ovr = wsock_evt.prepare ()))
    {
      debug_printf ("Fallback to winsock 1 recv call");
      if ((res = ::recv (get_socket (), (char *) ptr, len, flags))
	  == SOCKET_ERROR)
	{
	  set_winsock_errno ();
	  res = -1;
	}
    }
  else
    {
      WSABUF wsabuf = { len, (char *) ptr };
      DWORD ret = 0;
      if (WSARecv (get_socket (), &wsabuf, 1, &ret, (DWORD *)&flags,
		   ovr, NULL) != SOCKET_ERROR)
	res = ret;
      else if ((res = WSAGetLastError ()) != WSA_IO_PENDING)
	{
	  set_winsock_errno ();
	  res = -1;
	}
      else if ((res = wsock_evt.wait (get_socket (), (DWORD *)&flags)) == -1)
	set_winsock_errno ();
    }
  return res;
}

int __stdcall
fhandler_socket::read (void *ptr, size_t len)
{
  return recv (ptr, len, 0);
}

int
fhandler_socket::send (const void *ptr, size_t len, unsigned int flags)
{
  int res = -1;
  wsock_event wsock_evt;
  LPWSAOVERLAPPED ovr;

  sigframe thisframe (mainthread);
  if (is_nonblocking () || !(ovr = wsock_evt.prepare ()))
    {
      debug_printf ("Fallback to winsock 1 send call");
      if ((res = ::send (get_socket (), (const char *) ptr, len, flags))
	  == SOCKET_ERROR)
	{
	  set_winsock_errno ();
	  res = -1;
	}
    }
  else
    {
      WSABUF wsabuf = { len, (char *) ptr };
      DWORD ret = 0;
      if (WSASend (get_socket (), &wsabuf, 1, &ret, (DWORD)flags,
		   ovr, NULL) != SOCKET_ERROR)
	res = ret;
      else if ((res = WSAGetLastError ()) != WSA_IO_PENDING)
	{
	  set_winsock_errno ();
	  res = -1;
	}
      else if ((res = wsock_evt.wait (get_socket (), (DWORD *)&flags)) == -1)
	set_winsock_errno ();
    }
  return res;
}

int
fhandler_socket::write (const void *ptr, size_t len)
{
  return send (ptr, len, 0);
}

/* Cygwin internal */
int
fhandler_socket::close ()
{
  int res = 0;
  sigframe thisframe (mainthread);

  /* HACK to allow a graceful shutdown even if shutdown() hasn't been
     called by the application. Note that this isn't the ultimate
     solution but it helps in many cases. */
  struct linger linger;
  linger.l_onoff = 1;
  linger.l_linger = 240; /* seconds. default 2MSL value according to MSDN. */
  setsockopt (get_socket (), SOL_SOCKET, SO_LINGER,
	      (const char *)&linger, sizeof linger);

  while ((res = closesocket (get_socket ()))
	 && WSAGetLastError () == WSAEWOULDBLOCK)
    continue;
  if (res)
    {
      set_winsock_errno ();
      res = -1;
    }

  close_secret_event ();

  debug_printf ("%d = fhandler_socket::close()", res);
  return res;
}

#define ASYNC_MASK (FD_READ|FD_WRITE|FD_OOB|FD_ACCEPT|FD_CONNECT)

/* Cygwin internal */
int
fhandler_socket::ioctl (unsigned int cmd, void *p)
{
  extern int get_ifconf (struct ifconf *ifc, int what); /* net.cc */
  int res;
  struct ifconf ifc, *ifcp;
  struct ifreq *ifr, *ifrp;
  sigframe thisframe (mainthread);

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
	debug_printf ("error in get_ifconf\n");
      break;
    case SIOCGIFFLAGS:
      ifr = (struct ifreq *) p;
      if (ifr == 0)
	{
	  set_errno (EINVAL);
	  return -1;
	}
      ifr->ifr_flags = IFF_NOTRAILERS | IFF_UP | IFF_RUNNING;
      if (ntohl (((struct sockaddr_in *) &ifr->ifr_addr)->sin_addr.s_addr)
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
	    debug_printf ("ifr == NULL\n");
	    set_errno (EINVAL);
	    return -1;
	  }

	res = get_ifconf (&ifc, cmd);
	if (res)
	  {
	    debug_printf ("error in get_ifconf\n");
	    break;
	  }

	debug_printf ("    name: %s\n", ifr->ifr_name);
	for (ifrp = ifc.ifc_req;
	     (caddr_t) ifrp < ifc.ifc_buf + ifc.ifc_len;
	     ++ifrp)
	  {
	    debug_printf ("testname: %s\n", ifrp->ifr_name);
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
  if (sun_path)
    cfree (sun_path);
  sun_path = cstrdup (path);
}

/* fhandler_socket.cc. See fhandler.h for a description of the fhandler classes.

   Copyright 2000 Cygnus Solutions.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

/* #define DEBUG_NEST_ON 1 */

#define  __INSIDE_CYGWIN_NET__

#define Win32_Winsock
#include "winsup.h"
#include <errno.h>
#include <sys/socket.h>

#include <unistd.h>
#include <fcntl.h>
#include <winsock2.h>
#include "cygheap.h"
#include "cygerrno.h"
#include "fhandler.h"
#include "dtable.h"
#include "sigproc.h"

/**********************************************************************/
/* fhandler_socket */

fhandler_socket::fhandler_socket (const char *name) :
	fhandler_base (FH_SOCKET, name)
{
  set_cb (sizeof *this);
  set_need_fork_fixup ();
  prot_info_ptr = (LPWSAPROTOCOL_INFOA) cmalloc (HEAP_BUF,
  					         sizeof (WSAPROTOCOL_INFOA));
}

fhandler_socket::~fhandler_socket ()
{
  if (prot_info_ptr)
    cfree (prot_info_ptr);
}

void
fhandler_socket::fixup_before_fork_exec (DWORD win_proc_id)
{
  int ret = 1;

  if (prot_info_ptr &&
      (ret = WSADuplicateSocketA (get_socket (), win_proc_id, prot_info_ptr)))
    {
      debug_printf ("WSADuplicateSocket error");
      set_winsock_errno ();
    }
  if (!ret && ws2_32_handle)
    {
      debug_printf ("WSADuplicateSocket went fine, dwServiceFlags1=%d",
		    prot_info_ptr->dwServiceFlags1);
    }
  else
    {
      fhandler_base::fixup_before_fork_exec (win_proc_id);
      debug_printf ("Without Winsock 2.0");
    }
}

void
fhandler_socket::fixup_after_fork (HANDLE parent)
{
  SOCKET new_sock = INVALID_SOCKET;

  debug_printf ("WSASocket begin, dwServiceFlags1=%d",
		prot_info_ptr->dwServiceFlags1);
  if (prot_info_ptr &&
      (new_sock = WSASocketA (FROM_PROTOCOL_INFO,
                              FROM_PROTOCOL_INFO,
                              FROM_PROTOCOL_INFO,
                              prot_info_ptr, 0, 0)) == INVALID_SOCKET)
    {
      debug_printf ("WSASocket error");
      set_winsock_errno ();
    }
  if (new_sock != INVALID_SOCKET && ws2_32_handle)
    {
      debug_printf ("WSASocket went fine");
      set_io_handle ((HANDLE) new_sock);
    }
  else
    {
      fhandler_base::fixup_after_fork (parent);
      debug_printf ("Without Winsock 2.0");
    }
}

int
fhandler_socket::dup (fhandler_base *child)
{
  fhandler_socket *fhs = (fhandler_socket *) child;
  fhs->set_io_handle (get_io_handle ());
  fhs->fixup_before_fork_exec (GetCurrentProcessId ());
  if (ws2_32_handle)
    {
      fhs->fixup_after_fork (hMainProc);
      return 0;
    }
  return fhandler_base::dup (child);
}

int
fhandler_socket::read (void *ptr, size_t len)
{
  sigframe thisframe (mainthread);
  int res = recv (get_socket (), (char *) ptr, len, 0);
  if (res == SOCKET_ERROR)
    {
      set_winsock_errno ();
    }
  return res;
}

int
fhandler_socket::write (const void *ptr, size_t len)
{
  sigframe thisframe (mainthread);
  int res = send (get_socket (), (const char *) ptr, len, 0);
  if (res == SOCKET_ERROR)
    {
      set_winsock_errno ();
      if (get_errno () == ECONNABORTED || get_errno () == ECONNRESET)
	_raise (SIGPIPE);
    }
  return res;
}

/* Cygwin internal */
int
fhandler_socket::close ()
{
  int res = 0;
  sigframe thisframe (mainthread);

  if (closesocket (get_socket ()))
    {
      set_winsock_errno ();
      res = -1;
    }

  return res;
}

#define ASYNC_MASK (FD_READ|FD_WRITE|FD_OOB|FD_ACCEPT|FD_CONNECT)

/* Cygwin internal */
int
fhandler_socket::ioctl (unsigned int cmd, void *p)
{
  extern int get_ifconf (struct ifconf *ifc, int what); /* net.cc */
  int res;
  struct ifconf *ifc;
  struct ifreq *ifr;
  sigframe thisframe (mainthread);

  switch (cmd)
    {
    case SIOCGIFCONF:
      ifc = (struct ifconf *) p;
      if (ifc == 0)
	{
	  set_errno (EINVAL);
	  return -1;
	}
      res = get_ifconf (ifc, cmd);
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
      if (((struct sockaddr_in *) &ifr->ifr_addr)->sin_addr.s_addr
          == INADDR_LOOPBACK)
        ifr->ifr_flags |= IFF_LOOPBACK;
      else
        ifr->ifr_flags |= IFF_BROADCAST;
      res = 0;
      break;
    case SIOCGIFBRDADDR:
    case SIOCGIFNETMASK:
    case SIOCGIFADDR:
      {
	char buf[2048];
	struct ifconf ifc;
	ifc.ifc_len = sizeof (buf);
	ifc.ifc_buf = buf;
	struct ifreq *ifrp;

	struct ifreq *ifr = (struct ifreq *) p;
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
			    *(int *) p ? "un" : "");
	  /* Start AsyncSelect if async socket unblocked */
	  if (*(int *) p && get_async ())
	    WSAAsyncSelect (get_socket (), gethwnd (), WM_ASYNCIO, ASYNC_MASK);
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
        /* Care for the old O_NDELAY flag. If one of the flags is set,
           both flags are set. */
        int new_flags = (int) arg;
        if (new_flags & (O_NONBLOCK | OLD_O_NDELAY))
          new_flags |= O_NONBLOCK | OLD_O_NDELAY;
        request = (new_flags & O_NONBLOCK) ? 1 : 0;
        current = (get_flags () & O_NONBLOCK) ? 1 : 0;
        if (request != current && (res = ioctl (FIONBIO, &request)))
          break;
        if (request)
          set_flags (get_flags () | O_NONBLOCK | OLD_O_NDELAY);
        else
          set_flags (get_flags () & ~(O_NONBLOCK | OLD_O_NDELAY));
        break;
      }
    default:
      res = fhandler_base::fcntl (cmd, arg);
      break;
    }
  return res;
}


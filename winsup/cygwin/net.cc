/* net.cc: network-related routines.

   Copyright 1996, 1997, 1998, 1999, 2000, 2001 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* #define DEBUG_NEST_ON 1 */

#define  __INSIDE_CYGWIN_NET__

#include "winsup.h"
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <iphlpapi.h>

#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#define USE_SYS_TYPES_FD_SET
#include <winsock2.h>
#include "cygerrno.h"
#include "perprocess.h"
#include "fhandler.h"
#include "path.h"
#include "dtable.h"
#include "cygheap.h"
#include "sync.h"
#include "sigproc.h"
#include "pinfo.h"
#include "registry.h"

extern "C" {
int h_errno;

int __stdcall rcmd (char **ahost, unsigned short inport, char *locuser,
		    char *remuser, char *cmd, SOCKET *fd2p);
int __stdcall rexec (char **ahost, unsigned short inport, char *locuser,
		     char *password, char *cmd, SOCKET *fd2p);
int __stdcall rresvport (int *);
int sscanf (const char *, const char *, ...);
} /* End of "C" section */

static WSADATA wsadata;

/* Cygwin internal */
static SOCKET __stdcall
set_socket_inheritance (SOCKET sock)
{
  if (os_being_run == winNT)
    (void) SetHandleInformation ((HANDLE) sock, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
  else
    {
      SOCKET newsock;
      if (!DuplicateHandle (hMainProc, (HANDLE) sock, hMainProc, (HANDLE *) &newsock,
			    0, TRUE, DUPLICATE_SAME_ACCESS))
	small_printf ("DuplicateHandle failed %E");
      else
	{
	  closesocket (sock);
	  sock = newsock;
	}
    }
  return sock;
}

/* htonl: standards? */
extern "C" unsigned long int
htonl (unsigned long int x)
{
  return ((((x & 0x000000ffU) << 24) |
	   ((x & 0x0000ff00U) <<  8) |
	   ((x & 0x00ff0000U) >>  8) |
	   ((x & 0xff000000U) >> 24)));
}

/* ntohl: standards? */
extern "C" unsigned long int
ntohl (unsigned long int x)
{
  return htonl (x);
}

/* htons: standards? */
extern "C" unsigned short
htons (unsigned short x)
{
  return ((((x & 0x000000ffU) << 8) |
	   ((x & 0x0000ff00U) >> 8)));
}

/* ntohs: standards? */
extern "C" unsigned short
ntohs (unsigned short x)
{
  return htons (x);
}

/* Cygwin internal */
static void
dump_protoent (struct protoent *p)
{
  if (p)
    debug_printf ("protoent %s %x %x", p->p_name, p->p_aliases, p->p_proto);
}

/* exported as inet_ntoa: BSD 4.3 */
extern "C" char *
cygwin_inet_ntoa (struct in_addr in)
{
  char *res = inet_ntoa (in);
  return res;
}

/* exported as inet_addr: BSD 4.3 */
extern "C" unsigned long
cygwin_inet_addr (const char *cp)
{
  unsigned long res = inet_addr (cp);
  return res;
}

/* exported as inet_aton: BSD 4.3
   inet_aton is not exported by wsock32 and ws2_32,
   so it has to be implemented here. */
extern "C" int
cygwin_inet_aton (const char *cp, struct in_addr *inp)
{
  unsigned long res = inet_addr (cp);
  if (res == INADDR_NONE && strcmp (cp, "255.255.255.255"))
    return 0;
  if (inp)
    inp->s_addr = res;
  return 1;
}

/* undocumented in wsock32.dll */
extern "C" unsigned int	WINAPI inet_network (const char *);

extern "C" unsigned int
cygwin_inet_network (const char *cp)
{
  unsigned int res = inet_network (cp);
  return res;
}

/* inet_netof is in the standard BSD sockets library.  It is useless
   for modern networks, since it assumes network values which are no
   longer meaningful, but some existing code calls it.  */

extern "C" unsigned long
inet_netof (struct in_addr in)
{
  unsigned long i, res;


  i = ntohl (in.s_addr);
  if (IN_CLASSA (i))
    res = (i & IN_CLASSA_NET) >> IN_CLASSA_NSHIFT;
  else if (IN_CLASSB (i))
    res = (i & IN_CLASSB_NET) >> IN_CLASSB_NSHIFT;
  else
    res = (i & IN_CLASSC_NET) >> IN_CLASSC_NSHIFT;


  return res;
}

/* inet_makeaddr is in the standard BSD sockets library.  It is
   useless for modern networks, since it assumes network values which
   are no longer meaningful, but some existing code calls it.  */

extern "C" struct in_addr
inet_makeaddr (int net, int lna)
{
  unsigned long i;
  struct in_addr in;


  if (net < IN_CLASSA_MAX)
    i = (net << IN_CLASSA_NSHIFT) | (lna & IN_CLASSA_HOST);
  else if (net < IN_CLASSB_MAX)
    i = (net << IN_CLASSB_NSHIFT) | (lna & IN_CLASSB_HOST);
  else if (net < 0x1000000)
    i = (net << IN_CLASSC_NSHIFT) | (lna & IN_CLASSC_HOST);
  else
    i = net | lna;

  in.s_addr = htonl (i);


  return in;
}

struct tl
{
  int w;
  const char *s;
  int e;
};

static struct tl errmap[] =
{
 {WSAEWOULDBLOCK, "WSAEWOULDBLOCK", EWOULDBLOCK},
 {WSAEINPROGRESS, "WSAEINPROGRESS", EINPROGRESS},
 {WSAEALREADY, "WSAEALREADY", EALREADY},
 {WSAENOTSOCK, "WSAENOTSOCK", ENOTSOCK},
 {WSAEDESTADDRREQ, "WSAEDESTADDRREQ", EDESTADDRREQ},
 {WSAEMSGSIZE, "WSAEMSGSIZE", EMSGSIZE},
 {WSAEPROTOTYPE, "WSAEPROTOTYPE", EPROTOTYPE},
 {WSAENOPROTOOPT, "WSAENOPROTOOPT", ENOPROTOOPT},
 {WSAEPROTONOSUPPORT, "WSAEPROTONOSUPPORT", EPROTONOSUPPORT},
 {WSAESOCKTNOSUPPORT, "WSAESOCKTNOSUPPORT", ESOCKTNOSUPPORT},
 {WSAEOPNOTSUPP, "WSAEOPNOTSUPP", EOPNOTSUPP},
 {WSAEPFNOSUPPORT, "WSAEPFNOSUPPORT", EPFNOSUPPORT},
 {WSAEAFNOSUPPORT, "WSAEAFNOSUPPORT", EAFNOSUPPORT},
 {WSAEADDRINUSE, "WSAEADDRINUSE", EADDRINUSE},
 {WSAEADDRNOTAVAIL, "WSAEADDRNOTAVAIL", EADDRNOTAVAIL},
 {WSAENETDOWN, "WSAENETDOWN", ENETDOWN},
 {WSAENETUNREACH, "WSAENETUNREACH", ENETUNREACH},
 {WSAENETRESET, "WSAENETRESET", ENETRESET},
 {WSAECONNABORTED, "WSAECONNABORTED", ECONNABORTED},
 {WSAECONNRESET, "WSAECONNRESET", ECONNRESET},
 {WSAENOBUFS, "WSAENOBUFS", ENOBUFS},
 {WSAEISCONN, "WSAEISCONN", EISCONN},
 {WSAENOTCONN, "WSAENOTCONN", ENOTCONN},
 {WSAESHUTDOWN, "WSAESHUTDOWN", ESHUTDOWN},
 {WSAETOOMANYREFS, "WSAETOOMANYREFS", ETOOMANYREFS},
 {WSAETIMEDOUT, "WSAETIMEDOUT", ETIMEDOUT},
 {WSAECONNREFUSED, "WSAECONNREFUSED", ECONNREFUSED},
 {WSAELOOP, "WSAELOOP", ELOOP},
 {WSAENAMETOOLONG, "WSAENAMETOOLONG", ENAMETOOLONG},
 {WSAEHOSTDOWN, "WSAEHOSTDOWN", EHOSTDOWN},
 {WSAEHOSTUNREACH, "WSAEHOSTUNREACH", EHOSTUNREACH},
 {WSAENOTEMPTY, "WSAENOTEMPTY", ENOTEMPTY},
 {WSAEPROCLIM, "WSAEPROCLIM", EPROCLIM},
 {WSAEUSERS, "WSAEUSERS", EUSERS},
 {WSAEDQUOT, "WSAEDQUOT", EDQUOT},
 {WSAESTALE, "WSAESTALE", ESTALE},
 {WSAEREMOTE, "WSAEREMOTE", EREMOTE},
 {WSAEINVAL, "WSAEINVAL", EINVAL},
 {WSAEFAULT, "WSAEFAULT", EFAULT},
 {0, "NOERROR", 0},
 {0, NULL, 0}
};

static int
find_winsock_errno (int why)
{
  for (int i = 0; errmap[i].s != NULL; ++i)
    if (why == errmap[i].w)
      return errmap[i].e;

  return EPERM;
}

/* Cygwin internal */
void
__set_winsock_errno (const char *fn, int ln)
{
  DWORD werr = WSAGetLastError ();
  int err = find_winsock_errno (werr);
  set_errno (err);
  syscall_printf ("%s:%d - winsock error %d -> errno %d", fn, ln, werr, err);
}

/*
 * Since the member `s' isn't used for debug output we can use it
 * for the error text returned by herror and hstrerror.
 */
static struct tl host_errmap[] =
{
  {WSAHOST_NOT_FOUND, "Unknown host", HOST_NOT_FOUND},
  {WSATRY_AGAIN, "Host name lookup failure", TRY_AGAIN},
  {WSANO_RECOVERY, "Unknown server error", NO_RECOVERY},
  {WSANO_DATA, "No address associated with name", NO_DATA},
  {0, NULL, 0}
};

/* Cygwin internal */
static void
set_host_errno ()
{
  int i;

  int why = WSAGetLastError ();
  for (i = 0; host_errmap[i].w != 0; ++i)
    if (why == host_errmap[i].w)
      break;

  if (host_errmap[i].w != 0)
    h_errno = host_errmap[i].e;
  else
    h_errno = NETDB_INTERNAL;
}

/* exported as getprotobyname: standards? */
extern "C" struct protoent *
cygwin_getprotobyname (const char *p)
{
  struct protoent *res = getprotobyname (p);
  if (!res)
    set_winsock_errno ();

  dump_protoent (res);
  return res;
}

/* exported as getprotobynumber: standards? */
extern "C" struct protoent *
cygwin_getprotobynumber (int number)
{
  struct protoent *res = getprotobynumber (number);
  if (!res)
    set_winsock_errno ();

  dump_protoent (res);
  return res;
}

fhandler_socket *
fdsock (int fd, const char *name, SOCKET soc)
{
  if (wsadata.wVersion < 512) /* < Winsock 2.0 */
    soc = set_socket_inheritance (soc);
  fhandler_socket *fh = (fhandler_socket *) cygheap->fdtab.build_fhandler (fd, FH_SOCKET, name);
  fh->set_io_handle ((HANDLE) soc);
  fh->set_flags (O_RDWR);
  cygheap->fdtab.inc_need_fixup_before ();
  return fh;
}

/* exported as socket: standards? */
extern "C" int
cygwin_socket (int af, int type, int protocol)
{
  int res = -1;
  SetResourceLock (LOCK_FD_LIST, WRITE_LOCK|READ_LOCK, "socket");

  SOCKET soc;

  int fd = cygheap->fdtab.find_unused_handle ();

  if (fd < 0)
    set_errno (EMFILE);
  else
    {
      debug_printf ("socket (%d, %d, %d)", af, type, protocol);

      soc = socket (AF_INET, type, af == AF_UNIX ? 0 : protocol);

      if (soc == INVALID_SOCKET)
	{
	  set_winsock_errno ();
	  goto done;
	}

      const char *name;
      if (af == AF_INET)
	name = (type == SOCK_STREAM ? "/dev/tcp" : "/dev/udp");
      else
	name = (type == SOCK_STREAM ? "/dev/streamsocket" : "/dev/dgsocket");

      fdsock (fd, name, soc)->set_addr_family (af);
      res = fd;
    }

done:
  syscall_printf ("%d = socket (%d, %d, %d)", res, af, type, protocol);
  ReleaseResourceLock (LOCK_FD_LIST, WRITE_LOCK|READ_LOCK, "socket");
  return res;
}

/* cygwin internal: map sockaddr into internet domain address */

static int get_inet_addr (const struct sockaddr *in, int inlen,
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
  else if (in->sa_family == AF_UNIX)
    {
      int fd = _open (in->sa_data, O_RDONLY);
      if (fd == -1)
	return 0;

      int ret = 0;
      char buf[128];
      memset (buf, 0, sizeof buf);
      if (read (fd, buf, sizeof buf) != -1)
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
      _close (fd);
      return ret;
    }
  else
    {
      set_errno (EAFNOSUPPORT);
      return 0;
    }
}

/* exported as sendto: standards? */
extern "C" int
cygwin_sendto (int fd,
		 const void *buf,
		 int len,
		 unsigned int flags,
		 const struct sockaddr *to,
		 int tolen)
{
  fhandler_socket *h = (fhandler_socket *) cygheap->fdtab[fd];
  sockaddr_in sin;
  sigframe thisframe (mainthread);

  if (get_inet_addr (to, tolen, &sin, &tolen) == 0)
    return -1;

  int res = sendto (h->get_socket (), (const char *) buf, len,
		    flags, to, tolen);
  if (res == SOCKET_ERROR)
    {
      set_winsock_errno ();
      res = -1;
    }
  return res;
}

/* exported as recvfrom: standards? */
extern "C" int
cygwin_recvfrom (int fd,
		   char *buf,
		   int len,
		   int flags,
		   struct sockaddr *from,
		   int *fromlen)
{
  fhandler_socket *h = (fhandler_socket *) cygheap->fdtab[fd];
  sigframe thisframe (mainthread);

  debug_printf ("recvfrom %d", h->get_socket ());

  int res = recvfrom (h->get_socket (), buf, len, flags, from, fromlen);
  if (res == SOCKET_ERROR)
    {
      set_winsock_errno ();
      res = -1;
    }

  return res;
}

/* Cygwin internal */
fhandler_socket *
get (int fd)
{
  if (cygheap->fdtab.not_open (fd))
    {
      set_errno (EINVAL);
      return 0;
    }

  return cygheap->fdtab[fd]->is_socket ();
}

/* exported as setsockopt: standards? */
extern "C" int
cygwin_setsockopt (int fd,
		     int level,
		     int optname,
		     const void *optval,
		     int optlen)
{
  fhandler_socket *h = get (fd);
  int res = -1;
  const char *name = "error";

  if (h)
    {
      /* For the following debug_printf */
      switch (optname)
	{
	case SO_DEBUG:
	  name="SO_DEBUG";
	  break;
	case SO_ACCEPTCONN:
	  name="SO_ACCEPTCONN";
	  break;
	case SO_REUSEADDR:
	  name="SO_REUSEADDR";
	  break;
	case SO_KEEPALIVE:
	  name="SO_KEEPALIVE";
	  break;
	case SO_DONTROUTE:
	  name="SO_DONTROUTE";
	  break;
	case SO_BROADCAST:
	  name="SO_BROADCAST";
	  break;
	case SO_USELOOPBACK:
	  name="SO_USELOOPBACK";
	  break;
	case SO_LINGER:
	  name="SO_LINGER";
	  break;
	case SO_OOBINLINE:
	  name="SO_OOBINLINE";
	  break;
	case SO_ERROR:
	  name="SO_ERROR";
	  break;
	}

      res = setsockopt (h->get_socket (), level, optname,
				     (const char *) optval, optlen);

      if (optlen == 4)
	syscall_printf ("setsockopt optval=%x", *(long *) optval);

      if (res)
	set_winsock_errno ();
    }

  syscall_printf ("%d = setsockopt (%d, %d, %x (%s), %x, %d)",
		  res, fd, level, optname, name, optval, optlen);
  return res;
}

/* exported as getsockopt: standards? */
extern "C" int
cygwin_getsockopt (int fd,
		     int level,
		     int optname,
		     void *optval,
		     int *optlen)
{
  fhandler_socket *h = get (fd);
  int res = -1;
  const char *name = "error";
  if (h)
    {
      /* For the following debug_printf */
      switch (optname)
	{
	case SO_DEBUG:
	  name="SO_DEBUG";
	  break;
	case SO_ACCEPTCONN:
	  name="SO_ACCEPTCONN";
	  break;
	case SO_REUSEADDR:
	  name="SO_REUSEADDR";
	  break;
	case SO_KEEPALIVE:
	  name="SO_KEEPALIVE";
	  break;
	case SO_DONTROUTE:
	  name="SO_DONTROUTE";
	  break;
	case SO_BROADCAST:
	  name="SO_BROADCAST";
	  break;
	case SO_USELOOPBACK:
	  name="SO_USELOOPBACK";
	  break;
	case SO_LINGER:
	  name="SO_LINGER";
	  break;
	case SO_OOBINLINE:
	  name="SO_OOBINLINE";
	  break;
	case SO_ERROR:
	  name="SO_ERROR";
	  break;
	}

      res = getsockopt (h->get_socket (), level, optname,
				       (char *) optval, (int *) optlen);

      if (optname == SO_ERROR)
	{
	  int *e = (int *) optval;
	  *e = find_winsock_errno (*e);
	}

      if (res)
	set_winsock_errno ();
    }

  syscall_printf ("%d = getsockopt (%d, %d, %x (%s), %x, %d)",
		  res, fd, level, optname, name, optval, optlen);
  return res;
}

/* exported as connect: standards? */
extern "C" int
cygwin_connect (int fd,
		  const struct sockaddr *name,
		  int namelen)
{
  int res;
  BOOL secret_check_failed = FALSE;
  fhandler_socket *sock = get (fd);
  sockaddr_in sin;
  int secret [4];
  sigframe thisframe (mainthread);

  if (get_inet_addr (name, namelen, &sin, &namelen, secret) == 0)
    return -1;

  if (!sock)
    {
      res = -1;
    }
  else
    {
      res = connect (sock->get_socket (), (sockaddr *) &sin, namelen);
      if (res)
        {
	  /* Special handling for connect to return the correct error code
	     when called to early on a non-blocking socket. */
	  if (WSAGetLastError () == WSAEWOULDBLOCK)
 	    WSASetLastError (WSAEINPROGRESS);

	  set_winsock_errno ();
        }
      if (sock->get_addr_family () == AF_UNIX)
        {
          if (!res || errno == EINPROGRESS)
            {
              if (!sock->create_secret_event (secret))
                {   
		  secret_check_failed = TRUE;
		}
            }

          if (!secret_check_failed && !res)
            {
	      if (!sock->check_peer_secret_event (&sin, secret))
		{
		  debug_printf ( "accept from unauthorized server" );
		  secret_check_failed = TRUE;
		}
           }

          if (secret_check_failed)
            {
	      sock->close_secret_event ();
              if (res)
                closesocket (res);
	      set_errno (ECONNREFUSED);
	      res = -1;
            }
        }
    }
  return res;
}

/* exported as getservbyname: standards? */
extern "C" struct servent *
cygwin_getservbyname (const char *name, const char *proto)
{
  struct servent *p = getservbyname (name, proto);
  if (!p)
    set_winsock_errno ();

  syscall_printf ("%x = getservbyname (%s, %s)", p, name, proto);
  return p;
}

/* exported as getservbyport: standards? */
extern "C" struct servent *
cygwin_getservbyport (int port, const char *proto)
{
  struct servent *p = getservbyport (port, proto);
  if (!p)
    set_winsock_errno ();

  syscall_printf ("%x = getservbyport (%d, %s)", p, port, proto);
  return p;
}

extern "C" int
cygwin_gethostname (char *name, size_t len)
{
  int PASCAL win32_gethostname (char*, int);

  if (wsock32_handle == NULL ||
      win32_gethostname (name, len) == SOCKET_ERROR)
    {
      DWORD local_len = len;

      if (!GetComputerNameA (name, &local_len))
	{
	  set_winsock_errno ();
	  return -1;
	}
    }
  debug_printf ("name %s\n", name);
  h_errno = 0;
  return 0;
}

/* exported as gethostbyname: standards? */
extern "C" struct hostent *
cygwin_gethostbyname (const char *name)
{
  static unsigned char tmp_addr[4];
  static struct hostent tmp;
  static char *tmp_aliases[1] = {0};
  static char *tmp_addr_list[2] = {0, 0};
  static int a, b, c, d;

  if (sscanf (name, "%d.%d.%d.%d", &a, &b, &c, &d) == 4)
    {
      /* In case you don't have DNS, at least x.x.x.x still works */
      memset (&tmp, 0, sizeof (tmp));
      tmp_addr[0] = a;
      tmp_addr[1] = b;
      tmp_addr[2] = c;
      tmp_addr[3] = d;
      tmp_addr_list[0] = (char *)tmp_addr;
      tmp.h_name = name;
      tmp.h_aliases = tmp_aliases;
      tmp.h_addrtype = 2;
      tmp.h_length = 4;
      tmp.h_addr_list = tmp_addr_list;
      return &tmp;
    }

  struct hostent *ptr = gethostbyname (name);
  if (!ptr)
    {
      set_winsock_errno ();
      set_host_errno ();
    }
  else
    {
      debug_printf ("h_name %s", ptr->h_name);
      h_errno = 0;
    }
  return ptr;
}

/* exported as accept: standards? */
extern "C" int
cygwin_accept (int fd, struct sockaddr *peer, int *len)
{
  int res = -1;
  BOOL secret_check_failed = FALSE;
  sigframe thisframe (mainthread);

  fhandler_socket *sock = get (fd);
  if (sock)
    {
      /* accept on NT fails if len < sizeof (sockaddr_in)
       * some programs set len to
       * sizeof (name.sun_family) + strlen (name.sun_path) for UNIX domain
       */
      if (len && ((unsigned) *len < sizeof (struct sockaddr_in)))
	*len = sizeof (struct sockaddr_in);

      res = accept (sock->get_socket (), peer, len);  // can't use a blocking call inside a lock

      if (sock->get_addr_family () == AF_UNIX)
        {
          if (((SOCKET) res != (SOCKET) INVALID_SOCKET ||
               WSAGetLastError () == WSAEWOULDBLOCK))
            {
              if (!sock->create_secret_event ())
		{
		  secret_check_failed = TRUE;
                }
            }

          if (!secret_check_failed &&
              (SOCKET) res != (SOCKET) INVALID_SOCKET)
            {
	      if (!sock->check_peer_secret_event ((struct sockaddr_in*) peer))
		{
		  debug_printf ("connect from unauthorized client");
		  secret_check_failed = TRUE;
                }
            }

          if (secret_check_failed)
            {
              sock->close_secret_event ();
              if ((SOCKET) res != (SOCKET) INVALID_SOCKET)
                closesocket (res);
	      set_errno (ECONNABORTED);
              res = -1;
              goto done;
            }
        }

      SetResourceLock (LOCK_FD_LIST, WRITE_LOCK|READ_LOCK, "accept");

      int res_fd = cygheap->fdtab.find_unused_handle ();
      if (res_fd == -1)
	{
	  /* FIXME: what is correct errno? */
	  set_errno (EMFILE);
	  goto lock_done;
	}
      if ((SOCKET) res == (SOCKET) INVALID_SOCKET)
	set_winsock_errno ();
      else
	{
	  fhandler_socket* res_fh = fdsock (res_fd, sock->get_name (), res);
	  res_fh->set_addr_family (sock->get_addr_family ());
	  res = res_fd;
	}
    }
lock_done:
  ReleaseResourceLock (LOCK_FD_LIST, WRITE_LOCK|READ_LOCK, "accept");
done:
  syscall_printf ("%d = accept (%d, %x, %x)", res, fd, peer, len);
  return res;
}

/* exported as bind: standards? */
extern "C" int
cygwin_bind (int fd, const struct sockaddr *my_addr, int addrlen)
{
  int res = -1;

  fhandler_socket *sock = get (fd);
  if (sock)
    {
      if (my_addr->sa_family == AF_UNIX)
	{
#define un_addr ((struct sockaddr_un *) my_addr)
	  struct sockaddr_in sin;
	  int len = sizeof sin;
	  int fd;

	  if (strlen (un_addr->sun_path) >= UNIX_PATH_LEN)
	    {
	      set_errno (ENAMETOOLONG);
	      goto out;
	    }
	  sin.sin_family = AF_INET;
	  sin.sin_port = 0;
	  sin.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
	  if (bind (sock->get_socket (), (sockaddr *) &sin, len))
	    {
	      syscall_printf ("AF_UNIX: bind failed %d", get_errno ());
	      set_winsock_errno ();
	      goto out;
	    }
	  if (getsockname (sock->get_socket (), (sockaddr *) &sin, &len))
	    {
	      syscall_printf ("AF_UNIX: getsockname failed %d", get_errno ());
	      set_winsock_errno ();
	      goto out;
	    }

	  sin.sin_port = ntohs (sin.sin_port);
	  debug_printf ("AF_UNIX: socket bound to port %u", sin.sin_port);

	  /* bind must fail if file system socket object already exists
	     so _open () is called with O_EXCL flag. */
	  fd = _open (un_addr->sun_path,
		      O_WRONLY | O_CREAT | O_EXCL | O_BINARY,
		      0);
	  if (fd < 0)
	    {
	      if (get_errno () == EEXIST)
		set_errno (EADDRINUSE);
	      goto out;
	    }

          sock->set_connect_secret ();

	  char buf[sizeof (SOCKET_COOKIE) + 80];
	  __small_sprintf (buf, "%s%u ", SOCKET_COOKIE, sin.sin_port);
          sock->get_connect_secret (strchr (buf, '\0'));
	  len = strlen (buf) + 1;

	  /* Note that the terminating nul is written.  */
	  if (_write (fd, buf, len) != len)
	    {
	      save_errno here;
	      _close (fd);
	      _unlink (un_addr->sun_path);
	    }
	  else
	    {
	      _close (fd);
	      chmod (un_addr->sun_path,
		(S_IFSOCK | S_IRWXU | S_IRWXG | S_IRWXO) & ~cygheap->umask);
	      res = 0;
	    }
#undef un_addr
	}
      else if (bind (sock->get_socket (), my_addr, addrlen))
	set_winsock_errno ();
      else
	res = 0;
    }

out:
  syscall_printf ("%d = bind (%d, %x, %d)", res, fd, my_addr, addrlen);
  return res;
}

/* exported as getsockname: standards? */
extern "C" int
cygwin_getsockname (int fd, struct sockaddr *addr, int *namelen)
{
  int res = -1;

  fhandler_socket *sock = get (fd);
  if (sock)
    {
      res = getsockname (sock->get_socket (), addr, namelen);
      if (res)
	set_winsock_errno ();

    }
  syscall_printf ("%d = getsockname (%d, %x, %d)", res, fd, addr, namelen);
  return res;
}

/* exported as gethostbyaddr: standards? */
extern "C" struct hostent *
cygwin_gethostbyaddr (const char *addr, int len, int type)
{
  struct hostent *ptr = gethostbyaddr (addr, len, type);
  if (!ptr)
    {
      set_winsock_errno ();
      set_host_errno ();
    }
  else
    {
      debug_printf ("h_name %s", ptr->h_name);
      h_errno = 0;
    }
  return ptr;
}

/* exported as listen: standards? */
extern "C" int
cygwin_listen (int fd, int backlog)
{
  int res = -1;


  fhandler_socket *sock = get (fd);
  if (sock)
    {
      res = listen (sock->get_socket (), backlog);
      if (res)
	set_winsock_errno ();
    }
  syscall_printf ("%d = listen (%d, %d)", res, fd, backlog);
  return res;
}

/* exported as shutdown: standards? */
extern "C" int
cygwin_shutdown (int fd, int how)
{
  int res = -1;
  sigframe thisframe (mainthread);

  fhandler_socket *sock = get (fd);
  if (sock)
    {
      res = shutdown (sock->get_socket (), how);
      if (res)
	set_winsock_errno ();
    }
  syscall_printf ("%d = shutdown (%d, %d)", res, fd, how);
  return res;
}

/* exported as hstrerror: BSD 4.3  */
extern "C" const char *
cygwin_hstrerror (int err)
{
  int i;

  for (i = 0; host_errmap[i].e != 0; ++i)
    if (err == host_errmap[i].e)
      break;

  return host_errmap[i].s;
}

/* exported as herror: BSD 4.3  */
extern "C" void
cygwin_herror (const char *s)
{
  if (cygheap->fdtab.not_open (2))
    return;

  if (s)
    {
      write (2, s, strlen (s));
      write (2, ": ", 2);
    }

  const char *h_errstr = cygwin_hstrerror (h_errno);

  if (!h_errstr)
    switch (h_errno)
      {
      case NETDB_INTERNAL:
        h_errstr = "Resolver internal error";
        break;
      case NETDB_SUCCESS:
        h_errstr = "Resolver error 0 (no error)";
        break;
      default:
        h_errstr = "Unknown resolver error";
        break;
      }
  write (2, h_errstr, strlen (h_errstr));
  write (2, "\n", 1);
}

/* exported as getpeername: standards? */
extern "C" int
cygwin_getpeername (int fd, struct sockaddr *name, int *len)
{
  fhandler_socket *h = (fhandler_socket *) cygheap->fdtab[fd];

  debug_printf ("getpeername %d", h->get_socket ());
  int res = getpeername (h->get_socket (), name, len);
  if (res)
    set_winsock_errno ();

  debug_printf ("%d = getpeername %d", res, h->get_socket ());
  return res;
}

/* exported as recv: standards? */
extern "C" int
cygwin_recv (int fd, void *buf, int len, unsigned int flags)
{
  fhandler_socket *h = (fhandler_socket *) cygheap->fdtab[fd];
  sigframe thisframe (mainthread);

  int res = recv (h->get_socket (), (char *) buf, len, flags);
  if (res == SOCKET_ERROR)
    {
      set_winsock_errno ();
      res = -1;
    }

#if 0
  if (res > 0 && res < 200)
    for (int i=0; i < res; i++)
      system_printf ("%d %x %c", i, ((char *) buf)[i], ((char *) buf)[i]);
#endif

  syscall_printf ("%d = recv (%d, %x, %x, %x)", res, fd, buf, len, flags);

  return res;
}

/* exported as send: standards? */
extern "C" int
cygwin_send (int fd, const void *buf, int len, unsigned int flags)
{
  fhandler_socket *h = (fhandler_socket *) cygheap->fdtab[fd];
  sigframe thisframe (mainthread);

  int res = send (h->get_socket (), (const char *) buf, len, flags);
  if (res == SOCKET_ERROR)
    {
      set_winsock_errno ();
      res = -1;
    }

  syscall_printf ("%d = send (%d, %x, %d, %x)", res, fd, buf, len, flags);

  return res;
}

/* getdomainname: standards? */
extern "C" int
getdomainname (char *domain, int len)
{
  /*
   * This works for Win95 only if the machine is configured to use MS-TCP.
   * If a third-party TCP is being used this will fail.
   * FIXME: On Win95, is there a way to portably check the TCP stack
   * in use and include paths for the Domain name in each ?
   * Punt for now and assume MS-TCP on Win95.
   */
  reg_key r (HKEY_LOCAL_MACHINE, KEY_READ,
	     (os_being_run != winNT) ? "System" : "SYSTEM",
	     "CurrentControlSet", "Services",
	     (os_being_run != winNT) ? "MSTCP" : "Tcpip",
	     NULL);

  /* FIXME: Are registry keys case sensitive? */
  if (r.error () || r.get_string ("Domain", domain, len, "") != ERROR_SUCCESS)
    {
      __seterrno ();
      return -1;
    }

  return 0;
}

/* Cygwin internal */
/* Fill out an ifconf struct. */

/*
 * IFCONF 98/ME, NTSP4, W2K:
 * Use IP Helper Library
 */
static void
get_2k_ifconf (struct ifconf *ifc, int what)
{
  int cnt = 0;
  char eth[2] = "/", ppp[2] = "/", slp[2] = "/";

  /* Union maps buffer to correct struct */
  struct ifreq *ifr = ifc->ifc_req;

  DWORD if_cnt, ip_cnt, lip, lnp;
  DWORD siz_if_table = 0;
  DWORD siz_ip_table = 0;
  PMIB_IFTABLE ift;
  PMIB_IPADDRTABLE ipt;
  struct sockaddr_in *sa = NULL;
  struct sockaddr *so = NULL;

  if (GetIfTable(NULL, &siz_if_table, TRUE) == ERROR_INSUFFICIENT_BUFFER &&
      GetIpAddrTable(NULL, &siz_ip_table, TRUE) == ERROR_INSUFFICIENT_BUFFER &&
      (ift = (PMIB_IFTABLE) alloca (siz_if_table)) &&
      (ipt = (PMIB_IPADDRTABLE) alloca (siz_ip_table)) &&
      !GetIfTable(ift, &siz_if_table, TRUE) &&
      !GetIpAddrTable(ipt, &siz_ip_table, TRUE))
    {
      for (if_cnt = 0; if_cnt < ift->dwNumEntries; ++if_cnt)
        {
	  switch (ift->table[if_cnt].dwType)
	    {
	    case MIB_IF_TYPE_ETHERNET:
	      ++*eth;
	      strcpy (ifr->ifr_name, "eth");
	      strcat (ifr->ifr_name, eth);
	      break;
	    case MIB_IF_TYPE_PPP:
	      ++*ppp;
	      strcpy (ifr->ifr_name, "ppp");
	      strcat (ifr->ifr_name, ppp);
	      break;
	    case MIB_IF_TYPE_SLIP:
	      ++*slp;
	      strcpy (ifr->ifr_name, "slp");
	      strcat (ifr->ifr_name, slp);
	      break;
	    case MIB_IF_TYPE_LOOPBACK:
	      strcpy (ifr->ifr_name, "lo");
	      break;
	    default:
	      continue;
	    }
          for (ip_cnt = 0; ip_cnt < ipt->dwNumEntries; ++ip_cnt)
            if (ipt->table[ip_cnt].dwIndex == ift->table[if_cnt].dwIndex)
	      {
                switch (what)
                  {
                  case SIOCGIFCONF:
                  case SIOCGIFADDR:
                    sa = (struct sockaddr_in *) &ifr->ifr_addr;
                    sa->sin_addr.s_addr = ipt->table[ip_cnt].dwAddr;
		    sa->sin_family = AF_INET;
		    sa->sin_port = 0;
                    break;
                  case SIOCGIFBRDADDR:
                    sa = (struct sockaddr_in *) &ifr->ifr_broadaddr;
#if 0
		    /* Unfortunately, the field returns only crap. */
                    sa->sin_addr.s_addr = ipt->table[ip_cnt].dwBCastAddr;
#else
                    lip = ipt->table[ip_cnt].dwAddr;
                    lnp = ipt->table[ip_cnt].dwMask;
                    sa->sin_addr.s_addr = lip & lnp | ~lnp;
		    sa->sin_family = AF_INET;
		    sa->sin_port = 0;
#endif
                    break;
                  case SIOCGIFNETMASK:
                    sa = (struct sockaddr_in *) &ifr->ifr_netmask;
                    sa->sin_addr.s_addr = ipt->table[ip_cnt].dwMask;
		    sa->sin_family = AF_INET;
		    sa->sin_port = 0;
                    break;
		  case SIOCGIFHWADDR:
                    so = &ifr->ifr_hwaddr;
		    for (UINT i = 0; i < IFHWADDRLEN; ++i)
		      if (i >= ift->table[if_cnt].dwPhysAddrLen)
		        so->sa_data[i] = '\0';
		      else
                        so->sa_data[i] = ift->table[if_cnt].bPhysAddr[i];
		    so->sa_family = AF_INET;
		    break;
		  case SIOCGIFMETRIC:
		    ifr->ifr_metric = 1;
		    break;
		  case SIOCGIFMTU:
		    ifr->ifr_mtu = ift->table[if_cnt].dwMtu;
		    break;
                  }
                ++cnt;
	        if ((caddr_t) ++ifr >
		    ifc->ifc_buf + ifc->ifc_len - sizeof (struct ifreq))
		  goto done;
                break;
              }
	}
    }
done:
  /* Set the correct length */
  ifc->ifc_len = cnt * sizeof (struct ifreq);
}

/*
 * IFCONF Windows NT < SP4:
 * Look at the Bind value in
 * HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Tcpip\Linkage\
 * This is a REG_MULTI_SZ with strings of the form:
 * \Device\<Netcard>, where netcard is the name of the net device.
 * Then look under:
 * HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\<NetCard>\
 *							Parameters\Tcpip
 * at the IPAddress, Subnetmask and DefaultGateway values for the
 * required values.
 */
static void
get_nt_ifconf (struct ifconf *ifc, int what)
{
  HKEY key;
  unsigned long lip, lnp;
  struct sockaddr_in *sa = NULL;
  struct sockaddr *so = NULL;
  DWORD size;
  int cnt = 1;
  char *binding = (char *) 0;

  /* Union maps buffer to correct struct */
  struct ifreq *ifr = ifc->ifc_req;

  if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                    "SYSTEM\\"
                    "CurrentControlSet\\"
                    "Services\\"
                    "Tcpip\\"
                    "Linkage",
                    0, KEY_READ, &key) == ERROR_SUCCESS)
    {
      if (RegQueryValueEx (key, "Bind",
                           NULL, NULL,
                           NULL, &size) == ERROR_SUCCESS)
        {
          binding = (char *) alloca (size);
          if (RegQueryValueEx (key, "Bind",
                               NULL, NULL,
                               (unsigned char *) binding,
                               &size) != ERROR_SUCCESS)
            {
              binding = NULL;
            }
        }
      RegCloseKey (key);
    }

  if (binding)
    {
      char *bp, eth[2] = "/";
      char cardkey[256], ipaddress[256], netmask[256];

      for (bp = binding; *bp; bp += strlen (bp) + 1)
        {
          bp += strlen ("\\Device\\");
          strcpy (cardkey, "SYSTEM\\CurrentControlSet\\Services\\");
          strcat (cardkey, bp);
          strcat (cardkey, "\\Parameters\\Tcpip");

          if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, cardkey,
                            0, KEY_READ, &key) != ERROR_SUCCESS)
            continue;

          if (RegQueryValueEx (key, "IPAddress",
                               NULL, NULL,
                               (unsigned char *) ipaddress,
                               (size = 256, &size)) == ERROR_SUCCESS
              && RegQueryValueEx (key, "SubnetMask",
                                  NULL, NULL,
                                  (unsigned char *) netmask,
                                  (size = 256, &size)) == ERROR_SUCCESS)
            {
              char *ip, *np;
              char dhcpaddress[256], dhcpnetmask[256];

              for (ip = ipaddress, np = netmask;
                   *ip && *np;
                   ip += strlen (ip) + 1, np += strlen (np) + 1)
                {
                  if ((caddr_t) ++ifr > ifc->ifc_buf
                      + ifc->ifc_len
                      - sizeof (struct ifreq))
                    break;

                  if (! strncmp (bp, "NdisWan", 7))
                    {
                      strcpy (ifr->ifr_name, "ppp");
                      strcat (ifr->ifr_name, bp + 7);
                    }
                  else
                    {
                      ++*eth;
                      strcpy (ifr->ifr_name, "eth");
                      strcat (ifr->ifr_name, eth);
                    }
                  memset (&ifr->ifr_addr, '\0', sizeof ifr->ifr_addr);
                  if (cygwin_inet_addr (ip) == 0L
                      && RegQueryValueEx (key, "DhcpIPAddress",
                                          NULL, NULL,
                                          (unsigned char *) dhcpaddress,
                                          (size = 256, &size))
                      == ERROR_SUCCESS
                      && RegQueryValueEx (key, "DhcpSubnetMask",
                                          NULL, NULL,
                                          (unsigned char *) dhcpnetmask,
                                          (size = 256, &size))
                      == ERROR_SUCCESS)
                    {
                      switch (what)
                        {
                        case SIOCGIFCONF:
                        case SIOCGIFADDR:
                          sa = (struct sockaddr_in *) &ifr->ifr_addr;
                          sa->sin_addr.s_addr = cygwin_inet_addr (dhcpaddress);
			  sa->sin_family = AF_INET;
			  sa->sin_port = 0;
                          break;
                        case SIOCGIFBRDADDR:
                          lip = cygwin_inet_addr (dhcpaddress);
                          lnp = cygwin_inet_addr (dhcpnetmask);
                          sa = (struct sockaddr_in *) &ifr->ifr_broadaddr;
                          sa->sin_addr.s_addr = lip & lnp | ~lnp;
			  sa->sin_family = AF_INET;
			  sa->sin_port = 0;
                          break;
                        case SIOCGIFNETMASK:
                          sa = (struct sockaddr_in *) &ifr->ifr_netmask;
                          sa->sin_addr.s_addr =
                            cygwin_inet_addr (dhcpnetmask);
			  sa->sin_family = AF_INET;
			  sa->sin_port = 0;
                          break;
		        case SIOCGIFHWADDR:
			  so = &ifr->ifr_hwaddr;
			  memset (so->sa_data, 0, IFHWADDRLEN);
			  so->sa_family = AF_INET;
			  break;
		        case SIOCGIFMETRIC:
			  ifr->ifr_metric = 1;
			  break;
			case SIOCGIFMTU:
			  ifr->ifr_mtu = 1500;
			  break;
                        }
                    }
                  else
                    {
                      switch (what)
                        {
                        case SIOCGIFCONF:
                        case SIOCGIFADDR:
                          sa = (struct sockaddr_in *) &ifr->ifr_addr;
                          sa->sin_addr.s_addr = cygwin_inet_addr (ip);
			  sa->sin_family = AF_INET;
			  sa->sin_port = 0;
                          break;
                        case SIOCGIFBRDADDR:
                          lip = cygwin_inet_addr (ip);
                          lnp = cygwin_inet_addr (np);
                          sa = (struct sockaddr_in *) &ifr->ifr_broadaddr;
                          sa->sin_addr.s_addr = lip & lnp | ~lnp;
			  sa->sin_family = AF_INET;
			  sa->sin_port = 0;
                          break;
                        case SIOCGIFNETMASK:
                          sa = (struct sockaddr_in *) &ifr->ifr_netmask;
                          sa->sin_addr.s_addr = cygwin_inet_addr (np);
			  sa->sin_family = AF_INET;
			  sa->sin_port = 0;
                          break;
		        case SIOCGIFHWADDR:
			  so = &ifr->ifr_hwaddr;
			  memset (so->sa_data, 0, IFHWADDRLEN);
			  so->sa_family = AF_INET;
			  break;
		        case SIOCGIFMETRIC:
			  ifr->ifr_metric = 1;
			  break;
			case SIOCGIFMTU:
			  ifr->ifr_mtu = 1500;
			  break;
                        }
                    }
                  ++cnt;
                }
            }
          RegCloseKey (key);
        }
    }

  /* Set the correct length */
  ifc->ifc_len = cnt * sizeof (struct ifreq);
}

/*
 * IFCONF Windows 95:
 * HKLM/Enum/Network/MSTCP/"*"
 *         -> Value "Driver" enthält Subkey relativ zu
 *            HKLM/System/CurrentControlSet/Class/
 *         -> In Subkey "Bindings" die Values aufzählen
 *            -> Enthält Subkeys der Form "VREDIR\*"
 *               Das * ist ein Subkey relativ zu
 *               HKLM/System/CurrentControlSet/Class/Net/
 * HKLM/System/CurrentControlSet/Class/"Driver"
 *         -> Value "IPAddress"
 *         -> Value "IPMask"
 * HKLM/System/CurrentControlSet/Class/Net/"*"(aus "VREDIR\*")
 *         -> Wenn Value "AdapterName" == "MS$PPP" -> ppp interface
 *         -> Value "DriverDesc" enthält den Namen
 *
 */
static void
get_95_ifconf (struct ifconf *ifc, int what)
{
  HKEY key;
  unsigned long lip, lnp;
  struct sockaddr_in *sa = NULL;
  struct sockaddr *so = NULL;
  FILETIME update;
  LONG res;
  DWORD size;
  int cnt = 1;
  char ifname[256];
  char eth[2] = "/";
  char ppp[2] = "/";

  /* Union maps buffer to correct struct */
  struct ifreq *ifr = ifc->ifc_req;

  if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, "Enum\\Network\\MSTCP",
                    0, KEY_READ, &key) != ERROR_SUCCESS)
    {
      /* Set the correct length */
      ifc->ifc_len = cnt * sizeof (struct ifreq);
      return;
    }

  for (int i = 0;
       (res = RegEnumKeyEx (key, i, ifname,
                            (size = sizeof ifname, &size),
                            0, 0, 0, &update)) != ERROR_NO_MORE_ITEMS;
       ++i)
    {
      HKEY ifkey, subkey;
      char driver[256], classname[256], bindname[256], netname[256];
      char adapter[256], ip[256], np[256];

      if (res != ERROR_SUCCESS
          || RegOpenKeyEx (key, ifname, 0,
                           KEY_READ, &ifkey) != ERROR_SUCCESS)
        continue;

      if (RegQueryValueEx (ifkey, "Driver", 0,
                           NULL, (unsigned char *) driver,
                           (size = sizeof driver, &size)) != ERROR_SUCCESS)
        {
          RegCloseKey (ifkey);
          continue;
        }

      strcpy (classname, "System\\CurrentControlSet\\Services\\Class\\");
      strcat (classname, driver);
      if ((res = RegOpenKeyEx (HKEY_LOCAL_MACHINE, classname,
                               0, KEY_READ, &subkey)) != ERROR_SUCCESS)
        {
          RegCloseKey (ifkey);
          continue;
        }

      if (RegQueryValueEx (subkey, "IPAddress", 0,
                           NULL, (unsigned char *) ip,
                           (size = sizeof ip, &size)) == ERROR_SUCCESS
          && RegQueryValueEx (subkey, "IPMask", 0,
                              NULL, (unsigned char *) np,
                              (size = sizeof np, &size)) == ERROR_SUCCESS)
        {
          if ((caddr_t)++ifr > ifc->ifc_buf
              + ifc->ifc_len
              - sizeof (struct ifreq))
            goto out;

          switch (what)
            {
            case SIOCGIFCONF:
            case SIOCGIFADDR:
              sa = (struct sockaddr_in *) &ifr->ifr_addr;
              sa->sin_addr.s_addr = cygwin_inet_addr (ip);
	      sa->sin_family = AF_INET;
	      sa->sin_port = 0;
              break;
            case SIOCGIFBRDADDR:
              lip = cygwin_inet_addr (ip);
              lnp = cygwin_inet_addr (np);
              sa = (struct sockaddr_in *) &ifr->ifr_broadaddr;
              sa->sin_addr.s_addr = lip & lnp | ~lnp;
	      sa->sin_family = AF_INET;
	      sa->sin_port = 0;
              break;
            case SIOCGIFNETMASK:
              sa = (struct sockaddr_in *) &ifr->ifr_netmask;
              sa->sin_addr.s_addr = cygwin_inet_addr (np);
	      sa->sin_family = AF_INET;
	      sa->sin_port = 0;
              break;
	    case SIOCGIFHWADDR:
	      so = &ifr->ifr_hwaddr;
	      memset (so->sa_data, 0, IFHWADDRLEN);
	      so->sa_family = AF_INET;
	      break;
	    case SIOCGIFMETRIC:
	      ifr->ifr_metric = 1;
	      break;
	    case SIOCGIFMTU:
	      ifr->ifr_mtu = 1500;
	      break;
            }
        }

      RegCloseKey (subkey);

      if (RegOpenKeyEx (ifkey, "Bindings",
                         0, KEY_READ, &subkey) != ERROR_SUCCESS)
        {
          RegCloseKey (ifkey);
          --ifr;
          continue;
        }

      for (int j = 0;
           (res = RegEnumValue (subkey, j, bindname,
                                (size = sizeof bindname, &size),
                                0, NULL, NULL, NULL)) != ERROR_NO_MORE_ITEMS;
           ++j)
        if (!strncasecmp (bindname, "VREDIR\\", 7))
          break;

      RegCloseKey (subkey);

      if (res == ERROR_SUCCESS)
        {
          strcpy (netname, "System\\CurrentControlSet\\Services\\Class\\Net\\");
          strcat (netname, bindname + 7);

          if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, netname,
                            0, KEY_READ, &subkey) != ERROR_SUCCESS)
            {
              RegCloseKey (ifkey);
              --ifr;
              continue;
            }

          if (RegQueryValueEx (subkey, "AdapterName", 0,
                               NULL, (unsigned char *) adapter,
                               (size = sizeof adapter, &size)) == ERROR_SUCCESS
              && strcasematch (adapter, "MS$PPP"))
            {
              ++*ppp;
              strcpy (ifr->ifr_name, "ppp");
              strcat (ifr->ifr_name, ppp);
            }
          else
            {
              ++*eth;
              strcpy (ifr->ifr_name, "eth");
              strcat (ifr->ifr_name, eth);
            }

          RegCloseKey (subkey);

        }

      RegCloseKey (ifkey);

      ++cnt;
    }

out:

  RegCloseKey (key);

  /* Set the correct length */
  ifc->ifc_len = cnt * sizeof (struct ifreq);
}

int
get_ifconf (struct ifconf *ifc, int what)
{
  unsigned long lip, lnp;
  struct sockaddr_in *sa;

  /* Union maps buffer to correct struct */
  struct ifreq *ifr = ifc->ifc_req;

  /* Ensure we have space for two struct ifreqs, fail if not. */
  if (ifc->ifc_len < (int) (2 * sizeof (struct ifreq)))
    {
      set_errno (EFAULT);
      return -1;
    }

  /* Set up interface lo0 first */
  strcpy (ifr->ifr_name, "lo");
  memset (&ifr->ifr_addr, '\0', sizeof (ifr->ifr_addr));
  switch (what)
    {
    case SIOCGIFCONF:
    case SIOCGIFADDR:
      sa = (struct sockaddr_in *) &ifr->ifr_addr;
      sa->sin_addr.s_addr = htonl (INADDR_LOOPBACK);
      sa->sin_family = AF_INET;
      sa->sin_port = 0;
      break;
    case SIOCGIFBRDADDR:
      lip = htonl (INADDR_LOOPBACK);
      lnp = cygwin_inet_addr ("255.0.0.0");
      sa = (struct sockaddr_in *) &ifr->ifr_broadaddr;
      sa->sin_addr.s_addr = lip & lnp | ~lnp;
      sa->sin_family = AF_INET;
      sa->sin_port = 0;
      break;
    case SIOCGIFNETMASK:
      sa = (struct sockaddr_in *) &ifr->ifr_netmask;
      sa->sin_addr.s_addr = cygwin_inet_addr ("255.0.0.0");
      sa->sin_family = AF_INET;
      sa->sin_port = 0;
      break;
    case SIOCGIFHWADDR:
      ifr->ifr_hwaddr.sa_family = AF_INET;
      memset (ifr->ifr_hwaddr.sa_data, 0, IFHWADDRLEN);
      break;
    case SIOCGIFMETRIC:
      ifr->ifr_metric = 1;
      break;
    case SIOCGIFMTU:
      /* This funny value is returned by `ifconfig lo' on Linux 2.2 kernel. */
      ifr->ifr_mtu = 3924;
      break;
    default:
      set_errno (EINVAL);
      return -1;
    }

  OSVERSIONINFO os_version_info;
  memset (&os_version_info, 0, sizeof os_version_info);
  os_version_info.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
  GetVersionEx (&os_version_info);
  /* We have a win95 version... */
  if (os_version_info.dwPlatformId != VER_PLATFORM_WIN32_NT
      && (os_version_info.dwMajorVersion < 4
          || (os_version_info.dwMajorVersion == 4
              && os_version_info.dwMinorVersion == 0)))
    get_95_ifconf (ifc, what);
  /* ...and a NT <= SP3 version... */
  else if (os_version_info.dwPlatformId == VER_PLATFORM_WIN32_NT
           && (os_version_info.dwMajorVersion < 4
	       || (os_version_info.dwMajorVersion == 4
	           && strcmp (os_version_info.szCSDVersion, "Service Pack 4") < 0)))
    get_nt_ifconf (ifc, what);
  /* ...and finally a "modern" version for win98/ME, NT >= SP4 and W2K! */
  else
    get_2k_ifconf (ifc, what);

  return 0;
}

/* exported as rcmd: standards? */
extern "C" int
cygwin_rcmd (char **ahost, unsigned short inport, char *locuser,
	       char *remuser, char *cmd, int *fd2p)
{
  int res = -1;
  SOCKET fd2s;
  sigframe thisframe (mainthread);

  int res_fd = cygheap->fdtab.find_unused_handle ();
  if (res_fd == -1)
    goto done;

  if (fd2p)
    {
      *fd2p = cygheap->fdtab.find_unused_handle (res_fd + 1);
      if (*fd2p == -1)
	goto done;
    }

  res = rcmd (ahost, inport, locuser, remuser, cmd, fd2p? &fd2s: NULL);
  if (res == (int) INVALID_SOCKET)
    goto done;
  else
    {
      fdsock (res_fd, "/dev/tcp", res);
      res = res_fd;
    }
  if (fd2p)
    {
      fdsock (*fd2p, "/dev/tcp", fd2s);
    }
done:
  syscall_printf ("%d = rcmd (...)", res);
  return res;
}

/* exported as rresvport: standards? */
extern "C" int
cygwin_rresvport (int *port)
{
  int res = -1;
  sigframe thisframe (mainthread);

  int res_fd = cygheap->fdtab.find_unused_handle ();
  if (res_fd == -1)
    goto done;
  res = rresvport (port);

  if (res == (int) INVALID_SOCKET)
    goto done;
  else
    {
      fdsock (res_fd, "/dev/tcp", res);
      res = res_fd;
    }
done:
  syscall_printf ("%d = rresvport (%d)", res, port ? *port : 0);
  return res;
}

/* exported as rexec: standards? */
extern "C" int
cygwin_rexec (char **ahost, unsigned short inport, char *locuser,
	       char *password, char *cmd, int *fd2p)
{
  int res = -1;
  SOCKET fd2s;
  sigframe thisframe (mainthread);

  int res_fd = cygheap->fdtab.find_unused_handle ();
  if (res_fd == -1)
    goto done;
  if (fd2p)
    {
      *fd2p = cygheap->fdtab.find_unused_handle (res_fd + 1);
      if (*fd2p == -1)
	goto done;
    }
  res = rexec (ahost, inport, locuser, password, cmd, fd2p ? &fd2s : NULL);
  if (res == (int) INVALID_SOCKET)
    goto done;
  else
    {
      fdsock (res_fd, "/dev/tcp", res);
      res = res_fd;
    }
  if (fd2p)
    fdsock (*fd2p, "/dev/tcp", fd2s);

done:
  syscall_printf ("%d = rexec (...)", res);
  return res;
}

/* socketpair: standards? */
/* Win32 supports AF_INET only, so ignore domain and protocol arguments */
extern "C" int
socketpair (int, int type, int, int *sb)
{
  int res = -1;
  SOCKET insock, outsock, newsock;
  struct sockaddr_in sock_in;
  int len = sizeof (sock_in);

  SetResourceLock (LOCK_FD_LIST, WRITE_LOCK|READ_LOCK, "socketpair");

  sb[0] = cygheap->fdtab.find_unused_handle ();
  if (sb[0] == -1)
    {
      set_errno (EMFILE);
      goto done;
    }
  sb[1] = cygheap->fdtab.find_unused_handle (sb[0] + 1);
  if (sb[1] == -1)
    {
      set_errno (EMFILE);
      goto done;
    }

  /* create a listening socket */
  newsock = socket (AF_INET, type, 0);
  if (newsock == INVALID_SOCKET)
    {
      debug_printf ("first socket call failed");
      set_winsock_errno ();
      goto done;
    }

  /* bind the socket to any unused port */
  sock_in.sin_family = AF_INET;
  sock_in.sin_port = 0;
  sock_in.sin_addr.s_addr = INADDR_ANY;

  if (bind (newsock, (struct sockaddr *) &sock_in, sizeof (sock_in)) < 0)
    {
      debug_printf ("bind failed");
      set_winsock_errno ();
      closesocket (newsock);
      goto done;
    }

  if (getsockname (newsock, (struct sockaddr *) &sock_in, &len) < 0)
    {
      debug_printf ("getsockname error");
      set_winsock_errno ();
      closesocket (newsock);
      goto done;
    }

  listen (newsock, 2);

  /* create a connecting socket */
  outsock = socket (AF_INET, type, 0);
  if (outsock == INVALID_SOCKET)
    {
      debug_printf ("second socket call failed");
      set_winsock_errno ();
      closesocket (newsock);
      goto done;
    }

  sock_in.sin_addr.s_addr = htonl (INADDR_LOOPBACK);

  /* Do a connect and accept the connection */
  if (connect (outsock, (struct sockaddr *) &sock_in,
					   sizeof (sock_in)) < 0)
    {
      debug_printf ("connect error");
      set_winsock_errno ();
      closesocket (newsock);
      closesocket (outsock);
      goto done;
    }

  insock = accept (newsock, (struct sockaddr *) &sock_in, &len);
  if (insock == INVALID_SOCKET)
    {
      debug_printf ("accept error");
      set_winsock_errno ();
      closesocket (newsock);
      closesocket (outsock);
      goto done;
    }

  closesocket (newsock);
  res = 0;

  fdsock (sb[0], "/dev/tcp", insock);

  fdsock (sb[1], "/dev/tcp", outsock);

done:
  syscall_printf ("%d = socketpair (...)", res);
  ReleaseResourceLock (LOCK_FD_LIST, WRITE_LOCK|READ_LOCK, "socketpair");
  return res;
}

/* sethostent: standards? */
extern "C" void
sethostent (int)
{
}

/* endhostent: standards? */
extern "C" void
endhostent (void)
{
}

extern "C" void
wsock_init ()
{
  static LONG NO_COPY here = -1L;
  static int NO_COPY was_in_progress = 0;

  while (InterlockedIncrement (&here))
    {
      InterlockedDecrement (&here);
      Sleep (0);
    }
  if (!was_in_progress && (wsock32_handle || ws2_32_handle))
    {
      /* Don't use autoload to load WSAStartup to eliminate recursion. */
      int (*wsastartup) (int, WSADATA *);

      wsastartup = (int (*)(int, WSADATA *))
      		   GetProcAddress ((HMODULE) (wsock32_handle ?: ws2_32_handle),
				   "WSAStartup");
      if (wsastartup)
        {
	  int res = wsastartup ((2<<8) | 2, &wsadata);

	  debug_printf ("res %d", res);
	  debug_printf ("wVersion %d", wsadata.wVersion);
	  debug_printf ("wHighVersion %d", wsadata.wHighVersion);
	  debug_printf ("szDescription %s", wsadata.szDescription);
	  debug_printf ("szSystemStatus %s", wsadata.szSystemStatus);
	  debug_printf ("iMaxSockets %d", wsadata.iMaxSockets);
	  debug_printf ("iMaxUdpDg %d", wsadata.iMaxUdpDg);
	  debug_printf ("lpVendorInfo %d", wsadata.lpVendorInfo);

	  was_in_progress = 1;
        }
    }
  InterlockedDecrement (&here);
}


/* net.cc: network-related routines.

   Copyright 1996, 1997, 1998, 1999, 2000 Cygnus Solutions.

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
#include <sys/un.h>

#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include "autoload.h"
#include <winsock.h>
#include "cygerrno.h"
#include "fhandler.h"
#include "path.h"
#include "dtable.h"
#include "thread.h"
#include "sync.h"
#include "sigproc.h"
#include "pinfo.h"

/* We only want to initialize WinSock in a child process if socket
   handles are inheritted. This global allows us to know whether this
   should be done or not */
int number_of_sockets = 0;

extern "C"
{
int h_errno;

int __stdcall rcmd (char **ahost, unsigned short inport, char *locuser,
		    char *remuser, char *cmd, SOCKET *fd2p);
int __stdcall rexec (char **ahost, unsigned short inport, char *locuser,
		     char *password, char *cmd, SOCKET *fd2p);
int __stdcall rresvport (int *);
int sscanf (const char *, const char *, ...);
} /* End of "C" section */

/* Cygwin internal */
static SOCKET
duplicate_socket (SOCKET sock)
{
  /* Do not duplicate socket on Windows NT because of problems with
     MS winsock proxy server.
  */
  if (os_being_run == winNT)
    return sock;

  SOCKET newsock;
  if (DuplicateHandle (hMainProc, (HANDLE) sock, hMainProc, (HANDLE *) &newsock,
		       0, TRUE, DUPLICATE_SAME_ACCESS))
    {
      closesocket (sock);
      sock = newsock;
    }
  else
    small_printf ("DuplicateHandle failed %E");
  return sock;
}

/* htonl: standards? */
extern "C"
unsigned long int
htonl (unsigned long int x)
{
  MARK ();
  return ((((x & 0x000000ffU) << 24) |
	   ((x & 0x0000ff00U) <<  8) |
	   ((x & 0x00ff0000U) >>  8) |
	   ((x & 0xff000000U) >> 24)));
}

/* ntohl: standards? */
extern "C"
unsigned long int
ntohl (unsigned long int x)
{
  return htonl (x);
}

/* htons: standards? */
extern "C"
unsigned short
htons (unsigned short x)
{
  MARK ();
  return ((((x & 0x000000ffU) << 8) |
	   ((x & 0x0000ff00U) >> 8)));
}

/* ntohs: standards? */
extern "C"
unsigned short
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

/* exported as inet_ntoa: standards? */
extern "C"
char *
cygwin_inet_ntoa (struct in_addr in)
{
  char *res = inet_ntoa (in);
  return res;
}

/* exported as inet_addr: standards? */
extern "C"
unsigned long
cygwin_inet_addr (const char *cp)
{
  unsigned long res = inet_addr (cp);
  return res;
}

/* undocumented in wsock32.dll */
extern "C" unsigned int	WINAPI inet_network (const char *);

extern "C"
unsigned int 
cygwin_inet_network (const char *cp)
{
  unsigned int res = inet_network (cp);
  return res;
}

/* inet_netof is in the standard BSD sockets library.  It is useless
   for modern networks, since it assumes network values which are no
   longer meaningful, but some existing code calls it.  */

extern "C"
unsigned long
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

extern "C"
struct in_addr
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
 {0, NULL, 0}
};

/* Cygwin internal */
void
set_winsock_errno ()
{
  int i;
  int why = WSAGetLastError ();
  for (i = 0; errmap[i].w != 0; ++i)
    if (why == errmap[i].w)
      break;

  if (errmap[i].w != 0)
    {
      syscall_printf ("%d (%s) -> %d", why, errmap[i].s, errmap[i].e);
      set_errno (errmap[i].e);
    }
  else
    {
      syscall_printf ("unknown error %d", why);
      set_errno (EPERM);
    }
}

static struct tl host_errmap[] =
{
  {WSAHOST_NOT_FOUND, "WSAHOST_NOT_FOUND", HOST_NOT_FOUND},
  {WSATRY_AGAIN, "WSATRY_AGAIN", TRY_AGAIN},
  {WSANO_RECOVERY, "WSANO_RECOVERY", NO_RECOVERY},
  {WSANO_DATA, "WSANO_DATA", NO_DATA},
  {0, NULL, 0}
};

/* Cygwin internal */
static void
set_host_errno ()
{
  int i;

  int why = WSAGetLastError ();
  for (i = 0; i < host_errmap[i].w != 0; ++i)
    if (why == host_errmap[i].w)
      break;

  if (host_errmap[i].w != 0)
    h_errno = host_errmap[i].e;
  else
    h_errno = NETDB_INTERNAL;
}

/* exported as getprotobyname: standards? */
extern "C"
struct protoent *
cygwin_getprotobyname (const char *p)
{

  struct protoent *res = getprotobyname (p);
  if (!res)
    set_winsock_errno ();

  dump_protoent (res);
  return res;
}

/* exported as getprotobynumber: standards? */
extern "C"
struct protoent *
cygwin_getprotobynumber (int number)
{

  struct protoent *res = getprotobynumber (number);
  if (!res)
    set_winsock_errno ();

  dump_protoent (res);
  return res;
}

void
fdsock (int fd, const char *name, SOCKET soc)
{
  fhandler_base *fh = fdtab.build_fhandler(fd, FH_SOCKET, name);
  fh->set_io_handle ((HANDLE) soc);
  fh->set_flags (O_RDWR);
}

/* exported as socket: standards? */
extern "C"
int
cygwin_socket (int af, int type, int protocol)
{
  int res = -1;
  SetResourceLock(LOCK_FD_LIST,WRITE_LOCK|READ_LOCK," socket");

  SOCKET soc;

  int fd = fdtab.find_unused_handle ();

  if (fd < 0)
    {
      set_errno (ENMFILE);
    }
  else
    {
      debug_printf ("socket (%d, %d, %d)", af, type, protocol);

      soc = socket (AF_INET, type, 0);

      if (soc == INVALID_SOCKET)
        {
          set_winsock_errno ();
          goto done;
        }

      soc = duplicate_socket (soc);

      const char *name;
      if (af == AF_INET)
        name = (type == SOCK_STREAM ? "/dev/tcp" : "/dev/udp");
      else
        name = (type == SOCK_STREAM ? "/dev/streamsocket" : "/dev/dgsocket");

      fdsock (fd, name, soc);
      res = fd;
      fhandler_socket *h = (fhandler_socket *) fdtab[fd];

      h->set_addr_family (af);
    }

done:
  syscall_printf ("%d = socket (%d, %d, %d)", res, af, type, protocol);
  ReleaseResourceLock(LOCK_FD_LIST,WRITE_LOCK|READ_LOCK," socket");
  return res;
}

/* cygwin internal: map sockaddr into internet domain address */

static int get_inet_addr (const struct sockaddr *in, int inlen,
			   struct sockaddr_in *out, int *outlen)
{
  if (in->sa_family == AF_INET)
    {
      *out = * (sockaddr_in *)in;
      *outlen = inlen;
      return 1;
    }
  else if (in->sa_family == AF_UNIX)
    {
      sockaddr_in sin;
      char buf[32];

      memset (buf, 0, sizeof buf);
      int fd = open (in->sa_data, O_RDONLY);
      if (fd == -1)
	return 0;
      if (read (fd, buf, sizeof buf) == -1)
	return 0;
      sin.sin_family = AF_INET;
      sscanf (buf + strlen (SOCKET_COOKIE), "%hu", &sin.sin_port);
      sin.sin_port = htons (sin.sin_port);
      sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
      *out = sin;
      *outlen = sizeof sin;
      return 1;
    }
  else
    {
      set_errno (EAFNOSUPPORT);
      return 0;
    }
}

/* exported as sendto: standards? */
extern "C"
int
cygwin_sendto (int fd,
		 const void *buf,
		 int len,
		 unsigned int flags,
		 const struct sockaddr *to,
		 int tolen)
{
  fhandler_socket *h = (fhandler_socket *) fdtab[fd];
  sockaddr_in sin;
  sigframe thisframe (mainthread, 0);

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
extern "C"
int
cygwin_recvfrom (int fd,
		   char *buf,
		   int len,
		   int flags,
		   struct sockaddr *from,
		   int *fromlen)
{
  fhandler_socket *h = (fhandler_socket *) fdtab[fd];
  sigframe thisframe (mainthread, 0);

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
  if (fdtab.not_open (fd))
    {
      set_errno (EINVAL);
      return 0;
    }

  return fdtab[fd]->is_socket ();
}

/* exported as setsockopt: standards? */
extern "C"
int
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
extern "C"
int
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
	}

      res = getsockopt (h->get_socket (), level, optname,
				       (char *) optval, (int *) optlen);

      if (res)
	set_winsock_errno ();
    }

  syscall_printf ("%d = getsockopt (%d, %d, %x (%s), %x, %d)",
		  res, fd, level, optname, name, optval, optlen);
  return res;
}

/* exported as connect: standards? */
extern "C"
int
cygwin_connect (int fd,
		  const struct sockaddr *name,
		  int namelen)
{
  int res;
  fhandler_socket *sock = get (fd);
  sockaddr_in sin;
  sigframe thisframe (mainthread, 0);

  if (get_inet_addr (name, namelen, &sin, &namelen) == 0)
    return -1;

  if (!sock)
    {
      res = -1;
    }
  else
    {
      res = connect (sock->get_socket (), (sockaddr *) &sin, namelen);
      if (res)
	set_winsock_errno ();
    }
  return res;
}

/* exported as getservbyname: standards? */
extern "C"
struct servent *
cygwin_getservbyname (const char *name, const char *proto)
{
  struct servent *p = getservbyname (name, proto);
  if (!p)
    set_winsock_errno ();

  syscall_printf ("%x = getservbyname (%s, %s)", p, name, proto);
  return p;
}

/* exported as getservbyport: standards? */
extern "C"
struct servent *
cygwin_getservbyport (int port, const char *proto)
{
  struct servent *p = getservbyport (port, proto);
  if (!p)
    set_winsock_errno ();

  syscall_printf ("%x = getservbyport (%d, %s)", p, port, proto);
  return p;
}

extern "C"
int
cygwin_gethostname (char *name, size_t len)
{
  int PASCAL win32_gethostname(char*,int);

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
extern "C"
struct hostent *
cygwin_gethostbyname (const char *name)
{
  static unsigned char tmp_addr[4];
  static struct hostent tmp;
  static char *tmp_aliases[1] = {0};
  static char *tmp_addr_list[2] = {0,0};
  static int a, b, c, d;
  if (sscanf(name, "%d.%d.%d.%d", &a, &b, &c, &d) == 4)
    {
      /* In case you don't have DNS, at least x.x.x.x still works */
      memset(&tmp, 0, sizeof(tmp));
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
extern "C"
int
cygwin_accept (int fd, struct sockaddr *peer, int *len)
{
  int res = -1;
  sigframe thisframe (mainthread, 0);

  fhandler_socket *sock = get (fd);
  if (sock)
    {
      /* accept on NT fails if len < sizeof (sockaddr_in)
       * some programs set len to
       * sizeof(name.sun_family) + strlen(name.sun_path) for UNIX domain
       */
      if (len && ((unsigned) *len < sizeof (struct sockaddr_in)))
	*len = sizeof (struct sockaddr_in);

      res = accept (sock->get_socket (), peer, len);  // can't use a blocking call inside a lock

      SetResourceLock(LOCK_FD_LIST,WRITE_LOCK|READ_LOCK," accept");

      int res_fd = fdtab.find_unused_handle ();
      if (res_fd == -1)
	{
	  /* FIXME: what is correct errno? */
	  set_errno (EMFILE);
	  goto done;
	}
      if ((SOCKET) res == (SOCKET) INVALID_SOCKET)
	set_winsock_errno ();
      else
	{
	  res = duplicate_socket (res);

	  fdsock (res_fd, sock->get_name (), res);
	  res = res_fd;
	}
    }
done:
  syscall_printf ("%d = accept (%d, %x, %x)", res, fd, peer, len);
  ReleaseResourceLock(LOCK_FD_LIST,WRITE_LOCK|READ_LOCK," accept");
  return res;
}

/* exported as bind: standards? */
extern "C"
int
cygwin_bind (int fd, struct sockaddr *my_addr, int addrlen)
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
             so _open() is called with O_EXCL flag. */
	  fd = _open (un_addr->sun_path,
                      O_WRONLY | O_CREAT | O_EXCL | O_BINARY,
                      0);
	  if (fd < 0)
            {
              if (get_errno () == EEXIST)
                set_errno (EADDRINUSE);
	      goto out;
            }

          char buf[sizeof (SOCKET_COOKIE) + 10];
          __small_sprintf (buf, "%s%u", SOCKET_COOKIE, sin.sin_port);
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
                (S_IFSOCK | S_IRWXU | S_IRWXG | S_IRWXO) & ~myself->umask);
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
extern "C"
int
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
extern "C"
struct hostent *
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
extern "C"
int
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
extern "C"
int
cygwin_shutdown (int fd, int how)
{
  int res = -1;
  sigframe thisframe (mainthread, 0);

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

/* exported as herror: standards? */
extern "C"
void
cygwin_herror (const char *)
{
  debug_printf ("********%d*************", __LINE__);
}

/* exported as getpeername: standards? */
extern "C"
int
cygwin_getpeername (int fd, struct sockaddr *name, int *len)
{
  fhandler_socket *h = (fhandler_socket *) fdtab[fd];

  debug_printf ("getpeername %d", h->get_socket ());
  int res = getpeername (h->get_socket (), name, len);
  if (res)
    set_winsock_errno ();

  debug_printf ("%d = getpeername %d", res, h->get_socket ());
  return res;
}

/* exported as recv: standards? */
extern "C"
int
cygwin_recv (int fd, void *buf, int len, unsigned int flags)
{
  fhandler_socket *h = (fhandler_socket *) fdtab[fd];
  sigframe thisframe (mainthread, 0);

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
extern "C"
int
cygwin_send (int fd, const void *buf, int len, unsigned int flags)
{
  fhandler_socket *h = (fhandler_socket *) fdtab[fd];
  sigframe thisframe (mainthread, 0);

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
extern "C"
int
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
/* Fill out an ifconf struct.
 *
 * Windows NT:
 * Look at the Bind value in
 * HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Tcpip\Linkage\
 * This is a REG_MULTI_SZ with strings of the form:
 * \Device\<Netcard>, where netcard is the name of the net device.
 * Then look under:
 * HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\<NetCard>\
 *                                                  Parameters\Tcpip
 * at the IPAddress, Subnetmask and DefaultGateway values for the
 * required values.
 *
 * Windows 9x:
 * We originally just did a gethostbyname, assuming that it's pretty
 * unlikely Win9x will ever have more than one netcard.  When this
 * succeeded, we got the interface plus a loopback.
 * Currently, we read all
 * "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Class\NetTrans\*"
 * entries from the Registry and use all entries that have legal
 * "IPAddress" and "IPMask" values.
 */
static int
get_ifconf (struct ifconf *ifc, int what)
{
  if (os_being_run == winNT)
    {
      HKEY key;
      DWORD type, size;
      unsigned long lip, lnp;
      int cnt = 1;
      char *binding = (char *) 0;
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
      strcpy (ifr->ifr_name, "lo0");
      memset (&ifr->ifr_addr, '\0', sizeof (ifr->ifr_addr));
      switch (what)
	{
	case SIOCGIFCONF:
	case SIOCGIFADDR:
	  sa = (struct sockaddr_in *) &ifr->ifr_addr;
	  sa->sin_addr.s_addr = htonl (INADDR_LOOPBACK);
	  break;
	case SIOCGIFBRDADDR:
	  lip = htonl (INADDR_LOOPBACK);
	  lnp = cygwin_inet_addr ("255.0.0.0");
	  sa = (struct sockaddr_in *) &ifr->ifr_broadaddr;
	  sa->sin_addr.s_addr = lip & lnp | ~lnp;
	  break;
	case SIOCGIFNETMASK:
	  sa = (struct sockaddr_in *) &ifr->ifr_netmask;
	  sa->sin_addr.s_addr = cygwin_inet_addr ("255.0.0.0");
	  break;
	default:
	  set_errno (EINVAL);
	  return -1;
	}
      sa->sin_family = AF_INET;
      sa->sin_port = 0;

      if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,
			"SYSTEM\\"
			"CurrentControlSet\\"
			"Services\\"
			"Tcpip\\"
			"Linkage",
			0, KEY_READ, &key) == ERROR_SUCCESS)
	{
	  if (RegQueryValueEx (key, "Bind",
			       NULL, &type,
			       NULL, &size) == ERROR_SUCCESS)
	    {
	      binding = (char *) alloca (size);
	      if (RegQueryValueEx (key, "Bind",
				   NULL, &type,
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
	  char *bp, eth[2];
	  char cardkey[256], ipaddress[256], netmask[256];

	  eth[0] = '/';
	  eth[1] = '\0';
	  for (bp = binding; *bp; bp += strlen(bp) + 1)
	    {
	      bp += strlen ("\\Device\\");
	      strcpy (cardkey, "SYSTEM\\CurrentControlSet\\Services\\");
	      strcat (cardkey, bp);
	      strcat (cardkey, "\\Parameters\\Tcpip");

	      if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, cardkey,
				0, KEY_READ, &key) != ERROR_SUCCESS)
		continue;

	      if (RegQueryValueEx (key, "IPAddress",
				   NULL, &type,
				   (unsigned char *) &ipaddress,
				   (size = 256, &size)) == ERROR_SUCCESS
		  && RegQueryValueEx (key, "SubnetMask",
				      NULL, &type,
				      (unsigned char *) &netmask,
				      (size = 256, &size)) == ERROR_SUCCESS)
		{
		  char *ip, *np;
		  char sub[2];
		  char dhcpaddress[256], dhcpnetmask[256];

		  sub[0] = '/';
		  sub[1] = '\0';
		  if (strncmp (bp, "NdisWan", 7))
		    ++*eth;
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
			  strcpy (ifr->ifr_name, "eth");
			  strcat (ifr->ifr_name, eth);
			}
		      ++*sub;
		      if (*sub >= '1')
			strcat (ifr->ifr_name, sub);
		      memset (&ifr->ifr_addr, '\0', sizeof ifr->ifr_addr);
		      if (cygwin_inet_addr (ip) == 0L
			  && RegQueryValueEx (key, "DhcpIPAddress",
					      NULL, &type,
					      (unsigned char *) &dhcpaddress,
					      (size = 256, &size))
			  == ERROR_SUCCESS
			  && RegQueryValueEx (key, "DhcpSubnetMask",
					      NULL, &type,
					      (unsigned char *) &dhcpnetmask,
					      (size = 256, &size))
			  == ERROR_SUCCESS)
			{
			  switch (what)
			    {
			    case SIOCGIFCONF:
			    case SIOCGIFADDR:
			      sa = (struct sockaddr_in *) &ifr->ifr_addr;
			      sa->sin_addr.s_addr =
				cygwin_inet_addr (dhcpaddress);
			      break;
			    case SIOCGIFBRDADDR:
			      lip = cygwin_inet_addr (dhcpaddress);
			      lnp = cygwin_inet_addr (dhcpnetmask);
			      sa = (struct sockaddr_in *) &ifr->ifr_broadaddr;
			      sa->sin_addr.s_addr = lip & lnp | ~lnp;
			      break;
			    case SIOCGIFNETMASK:
			      sa = (struct sockaddr_in *) &ifr->ifr_netmask;
			      sa->sin_addr.s_addr =
				cygwin_inet_addr (dhcpnetmask);
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
			      break;
			    case SIOCGIFBRDADDR:
			      lip = cygwin_inet_addr (ip);
			      lnp = cygwin_inet_addr (np);
			      sa = (struct sockaddr_in *) &ifr->ifr_broadaddr;
			      sa->sin_addr.s_addr = lip & lnp | ~lnp;
			      break;
			    case SIOCGIFNETMASK:
			      sa = (struct sockaddr_in *) &ifr->ifr_netmask;
			      sa->sin_addr.s_addr = cygwin_inet_addr (np);
			      break;
			    }
			}
		      sa->sin_family = AF_INET;
		      sa->sin_port = 0;
		      ++cnt;
		    }
		}
	      RegCloseKey (key);
	    }
	}

      /* Set the correct length */
      ifc->ifc_len = cnt * sizeof (struct ifreq);
    }
  else /* Windows 9x */
    {
      HKEY key, subkey;
      FILETIME update;
      LONG res;
      DWORD type, size;
      unsigned long lip, lnp;
      char ifname[256], ip[256], np[256];
      int cnt = 1;
      struct sockaddr_in *sa;

      /* Union maps buffer to correct struct */
      struct ifreq *ifr = ifc->ifc_req;
      char eth[2];

      eth[0] = '/';
      eth[1] = '\0';

      /* Ensure we have space for two struct ifreqs, fail if not. */
      if (ifc->ifc_len < (int) (2 * sizeof (struct ifreq)))
	{
	  set_errno (EFAULT);
	  return -1;
	}

      /* Set up interface lo0 first */
      strcpy (ifr->ifr_name, "lo0");
      memset (&ifr->ifr_addr, '\0', sizeof ifr->ifr_addr);
      switch (what)
	{
	case SIOCGIFCONF:
	case SIOCGIFADDR:
	  sa = (struct sockaddr_in *) &ifr->ifr_addr;
	  sa->sin_addr.s_addr = htonl (INADDR_LOOPBACK);
	  break;
	case SIOCGIFBRDADDR:
	  lip = htonl(INADDR_LOOPBACK);
	  lnp = cygwin_inet_addr ("255.0.0.0");
	  sa = (struct sockaddr_in *) &ifr->ifr_broadaddr;
	  sa->sin_addr.s_addr = lip & lnp | ~lnp;
	  break;
	case SIOCGIFNETMASK:
	  sa = (struct sockaddr_in *) &ifr->ifr_netmask;
	  sa->sin_addr.s_addr = cygwin_inet_addr ("255.0.0.0");
	  break;
	default:
	  set_errno (EINVAL);
	  return -1;
	}
      sa->sin_family = AF_INET;
      sa->sin_port = 0;

      if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,
			"SYSTEM\\"
			"CurrentControlSet\\"
			"Services\\"
			"Class\\"
			"NetTrans",
			0, KEY_READ, &key) == ERROR_SUCCESS)
	{
	  for (int i = 0;
	       (res = RegEnumKeyEx (key, i, ifname,
				    (size = sizeof ifname, &size),
				    0, 0, 0, &update)) != ERROR_NO_MORE_ITEMS;
	       ++i)
	    {
	      if (res != ERROR_SUCCESS
		  || RegOpenKeyEx (key, ifname, 0,
				   KEY_READ, &subkey) != ERROR_SUCCESS)
		continue;
	      if (RegQueryValueEx (subkey, "IPAddress", 0,
				   &type, (unsigned char *) ip,
				   (size = sizeof ip, &size)) == ERROR_SUCCESS
		  || RegQueryValueEx (subkey, "IPMask", 0,
				      &type, (unsigned char *) np,
				      (size = sizeof np, &size)) == ERROR_SUCCESS)
		{
		  if ((caddr_t)++ifr > ifc->ifc_buf
		      + ifc->ifc_len
		      - sizeof(struct ifreq))
		    break;
		  ++*eth;
		  strcpy (ifr->ifr_name, "eth");
		  strcat (ifr->ifr_name, eth);
		  switch (what)
		    {
		    case SIOCGIFCONF:
		    case SIOCGIFADDR:
		      sa = (struct sockaddr_in *) &ifr->ifr_addr;
		      sa->sin_addr.s_addr = cygwin_inet_addr (ip);
		      break;
		    case SIOCGIFBRDADDR:
		      lip = cygwin_inet_addr (ip);
		      lnp = cygwin_inet_addr (np);
		      sa = (struct sockaddr_in *) &ifr->ifr_broadaddr;
		      sa->sin_addr.s_addr = lip & lnp | ~lnp;
		      break;
		    case SIOCGIFNETMASK:
		      sa = (struct sockaddr_in *) &ifr->ifr_netmask;
		      sa->sin_addr.s_addr = cygwin_inet_addr (np);
		      break;
		    }
		  sa->sin_family = AF_INET;
		  sa->sin_port = 0;
		  ++cnt;
		}
	      RegCloseKey (subkey);
	    }
	}

      /* Set the correct length */
      ifc->ifc_len = cnt * sizeof (struct ifreq);
    }

  return 0;
}

/* exported as rcmd: standards? */
extern "C"
int
cygwin_rcmd (char **ahost, unsigned short inport, char *locuser,
	       char *remuser, char *cmd, int *fd2p)
{
  int res = -1;
  SOCKET fd2s;
  sigframe thisframe (mainthread, 0);

  int res_fd = fdtab.find_unused_handle ();
  if (res_fd == -1)
    goto done;

  if (fd2p)
    {
      *fd2p = fdtab.find_unused_handle (res_fd + 1);
      if (*fd2p == -1)
	goto done;
    }

  res = rcmd (ahost, inport, locuser, remuser, cmd, fd2p? &fd2s: NULL);
  if (res == (int) INVALID_SOCKET)
    goto done;
  else
    {
      res = duplicate_socket (res);

      fdsock (res_fd, "/dev/tcp", res);
      res = res_fd;
    }
  if (fd2p)
    {
      fd2s = duplicate_socket (fd2s);

      fdsock (*fd2p, "/dev/tcp", fd2s);
    }
done:
  syscall_printf ("%d = rcmd (...)", res);
  return res;
}

/* exported as rresvport: standards? */
extern "C"
int
cygwin_rresvport (int *port)
{
  int res = -1;
  sigframe thisframe (mainthread, 0);

  int res_fd = fdtab.find_unused_handle ();
  if (res_fd == -1)
    goto done;
  res = rresvport (port);

  if (res == (int) INVALID_SOCKET)
    goto done;
  else
    {
      res = duplicate_socket (res);

      fdsock (res_fd, "/dev/tcp", res);
      res = res_fd;
    }
done:
  syscall_printf ("%d = rresvport (%d)", res, port ? *port : 0);
  return res;
}

/* exported as rexec: standards? */
extern "C"
int
cygwin_rexec (char **ahost, unsigned short inport, char *locuser,
	       char *password, char *cmd, int *fd2p)
{
  int res = -1;
  SOCKET fd2s;
  sigframe thisframe (mainthread, 0);

  int res_fd = fdtab.find_unused_handle ();
  if (res_fd == -1)
    goto done;
  if (fd2p)
    {
      *fd2p = fdtab.find_unused_handle (res_fd + 1);
      if (*fd2p == -1)
	goto done;
    }
  res = rexec (ahost, inport, locuser, password, cmd, fd2p ? &fd2s : NULL);
  if (res == (int) INVALID_SOCKET)
    goto done;
  else
    {
      res = duplicate_socket (res);

      fdsock (res_fd, "/dev/tcp", res);
      res = res_fd;
    }
  if (fd2p)
    {
      fd2s = duplicate_socket (fd2s);

      fdsock (*fd2p, "/dev/tcp", fd2s);
#if 0 /* ??? */
      fhandler_socket *h;
      p->hmap.vec[*fd2p].h = h =
	  new (&p->hmap.vec[*fd2p].item) fhandler_socket (fd2s, "/dev/tcp");
#endif
    }
done:
  syscall_printf ("%d = rexec (...)", res);
  return res;
}

/* socketpair: standards? */
/* Win32 supports AF_INET only, so ignore domain and protocol arguments */
extern "C"
int
socketpair (int, int type, int, int *sb)
{
  int res = -1;
  SOCKET insock, outsock, newsock;
  struct sockaddr_in sock_in;
  int len = sizeof (sock_in);

  SetResourceLock(LOCK_FD_LIST,WRITE_LOCK|READ_LOCK," socketpair");

  sb[0] = fdtab.find_unused_handle ();
  if (sb[0] == -1)
    {
      set_errno (EMFILE);
      goto done;
    }
  sb[1] = fdtab.find_unused_handle (sb[0] + 1);
  if (sb[1] == -1)
    {
      set_errno (EMFILE);
      goto done;
    }

  /* create a listening socket */
  newsock = socket (AF_INET, type, 0);
  if (newsock == INVALID_SOCKET)
    {
      set_winsock_errno ();
      goto done;
    }

  /* bind the socket to any unused port */
  sock_in.sin_family = AF_INET;
  sock_in.sin_port = 0;
  sock_in.sin_addr.s_addr = INADDR_ANY;

  if (bind (newsock, (struct sockaddr *) &sock_in, sizeof (sock_in)) < 0)
    {
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
      debug_printf ("can't create outsock");
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

  insock = duplicate_socket (insock);

  fdsock (sb[0], "/dev/tcp", insock);

  outsock = duplicate_socket (outsock);
  fdsock (sb[1], "/dev/tcp", outsock);

done:
  syscall_printf ("%d = socketpair (...)", res);
  ReleaseResourceLock(LOCK_FD_LIST,WRITE_LOCK|READ_LOCK," socketpair");
  return res;
}

/**********************************************************************/
/* fhandler_socket */

fhandler_socket::fhandler_socket (const char *name) :
	fhandler_base (FH_SOCKET, name)
{
  set_cb (sizeof *this);
  number_of_sockets++;
}

/* sethostent: standards? */
extern "C"
void
sethostent (int)
{
}

/* endhostent: standards? */
extern "C"
void
endhostent (void)
{
}

fhandler_socket::~fhandler_socket ()
{
  if (--number_of_sockets < 0)
    {
      number_of_sockets = 0;
      system_printf("socket count < 0");
    }
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

/* Cygwin internal */
/*
 * Return the flags settings for an interface.
 */
static int
get_if_flags (struct ifreq *ifr)
{
  struct sockaddr_in *sa = (struct sockaddr_in *) &ifr->ifr_addr;

  short flags = IFF_NOTRAILERS | IFF_UP | IFF_RUNNING;
  if (sa->sin_addr.s_addr == INADDR_LOOPBACK)
      flags |= IFF_LOOPBACK;
  else
      flags |= IFF_BROADCAST;

  ifr->ifr_flags = flags;
  return 0;
}

#define ASYNC_MASK (FD_READ|FD_WRITE|FD_OOB|FD_ACCEPT|FD_CONNECT)

/* Cygwin internal */
int
fhandler_socket::ioctl (unsigned int cmd, void *p)
{
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
      res = get_if_flags (ifr);
      break;
    case SIOCGIFBRDADDR:
    case SIOCGIFNETMASK:
    case SIOCGIFADDR:
      {
        char buf[2048];
        struct ifconf ifc;
        ifc.ifc_len = sizeof(buf);
        ifc.ifc_buf = buf;
        struct ifreq *ifrp;

        struct ifreq *ifr = (struct ifreq *) p;
        if (ifr == 0)
          {
            debug_printf("ifr == NULL\n");
            set_errno (EINVAL);
            return -1;
          }

	res = get_ifconf (&ifc, cmd);
        if (res)
	  {
	    debug_printf ("error in get_ifconf\n");
	    break;
	  }

        debug_printf("    name: %s\n", ifr->ifr_name);
        for (ifrp = ifc.ifc_req;
             (caddr_t) ifrp < ifc.ifc_buf + ifc.ifc_len;
             ++ifrp)
          {
            debug_printf("testname: %s\n", ifrp->ifr_name);
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
      /* We must cancel WSAAsyncSelect (if any) before settting socket to
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

extern "C" {
/* Initialize WinSock */
LoadDLLinitfunc (wsock32)
{
  WSADATA p;
  int res;
  HANDLE h;

  if ((h = LoadLibrary ("wsock32.dll")) != NULL)
    wsock32_handle = h;
  else if (!wsock32_handle)
    api_fatal ("could not load wsock32.dll.  Is TCP/IP installed?");
  else
    return 0;		/* Already done by another thread? */

  res = WSAStartup ((2<<8) | 2, &p);

  debug_printf ("res %d", res);
  debug_printf ("wVersion %d", p.wVersion);
  debug_printf ("wHighVersion %d", p.wHighVersion);
  debug_printf ("szDescription %s",p.szDescription);
  debug_printf ("szSystemStatus %s",p.szSystemStatus);
  debug_printf ("iMaxSockets %d", p.iMaxSockets);
  debug_printf ("iMaxUdpDg %d", p.iMaxUdpDg);
  debug_printf ("lpVendorInfo %d", p.lpVendorInfo);

  if (FIONBIO  != REAL_FIONBIO)
    debug_printf ("****************  FIONBIO  != REAL_FIONBIO");

  return 0;
}

LoadDLLinit (wsock32)

static void dummy_autoload (void) __attribute__ ((unused));
static void
dummy_autoload (void)
{
LoadDLLfunc (WSAAsyncSelect, 16, wsock32)
LoadDLLfunc (WSACleanup, 0, wsock32)
LoadDLLfunc (WSAGetLastError, 0, wsock32)
LoadDLLfunc (WSAStartup, 8, wsock32)
LoadDLLfunc (__WSAFDIsSet, 8, wsock32)
LoadDLLfunc (accept, 12, wsock32)
LoadDLLfunc (bind, 12, wsock32)
LoadDLLfunc (closesocket, 4, wsock32)
LoadDLLfunc (connect, 12, wsock32)
LoadDLLfunc (gethostbyaddr, 12, wsock32)
LoadDLLfunc (gethostbyname, 4, wsock32)
LoadDLLfunc (gethostname, 8, wsock32)
LoadDLLfunc (getpeername, 12, wsock32)
LoadDLLfunc (getprotobyname, 4, wsock32)
LoadDLLfunc (getprotobynumber, 4, wsock32)
LoadDLLfunc (getservbyname, 8, wsock32)
LoadDLLfunc (getservbyport, 8, wsock32)
LoadDLLfunc (getsockname, 12, wsock32)
LoadDLLfunc (getsockopt, 20, wsock32)
LoadDLLfunc (inet_addr, 4, wsock32)
LoadDLLfunc (inet_network, 4, wsock32)
LoadDLLfunc (inet_ntoa, 4, wsock32)
LoadDLLfunc (ioctlsocket, 12, wsock32)
LoadDLLfunc (listen, 8, wsock32)
LoadDLLfunc (rcmd, 24, wsock32)
LoadDLLfunc (recv, 16, wsock32)
LoadDLLfunc (recvfrom, 24, wsock32)
LoadDLLfunc (rexec, 24, wsock32)
LoadDLLfunc (rresvport, 4, wsock32)
LoadDLLfunc (select, 20, wsock32)
LoadDLLfunc (send, 16, wsock32)
LoadDLLfunc (sendto, 24, wsock32)
LoadDLLfunc (setsockopt, 20, wsock32)
LoadDLLfunc (shutdown, 8, wsock32)
LoadDLLfunc (socket, 12, wsock32)
}
}

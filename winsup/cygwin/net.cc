/* net.cc: network-related routines.

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.

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
#define USE_SYS_TYPES_FD_SET
#include <winsock2.h>
#include "cygerrno.h"
#include "security.h"
#include "fhandler.h"
#include "path.h"
#include "dtable.h"
#include "cygheap.h"
#include "sigproc.h"
#include "pinfo.h"
#include "registry.h"
#include "wsock_event.h"

extern "C" {
int h_errno;

int __stdcall rcmd (char **ahost, unsigned short inport, char *locuser,
		    char *remuser, char *cmd, SOCKET *fd2p);
int __stdcall rexec (char **ahost, unsigned short inport, char *locuser,
		     char *password, char *cmd, SOCKET *fd2p);
int __stdcall rresvport (int *);
int sscanf (const char *, const char *, ...);
} /* End of "C" section */

LPWSAOVERLAPPED
wsock_event::prepare ()
{
  LPWSAOVERLAPPED ret = NULL;

  SetLastError (0);
  if ((event = WSACreateEvent ()) != WSA_INVALID_EVENT)
    {
      memset (&ovr, 0, sizeof ovr);
      ovr.hEvent = event;
      ret = &ovr;
    }
  else if (GetLastError () == ERROR_PROC_NOT_FOUND) /* winsock2 not available */
    WSASetLastError (0);

  debug_printf ("%d = wsock_event::prepare ()", ret);
  return ret;
}

int
wsock_event::wait (int socket, LPDWORD flags)
{
  int ret = -1;
  WSAEVENT ev[2] = { event, signal_arrived };

  switch (WSAWaitForMultipleEvents(2, ev, FALSE, WSA_INFINITE, FALSE))
    {
    case WSA_WAIT_EVENT_0:
      DWORD len;
      if (WSAGetOverlappedResult(socket, &ovr, &len, FALSE, flags))
	ret = (int) len;
      break;
    case WSA_WAIT_EVENT_0 + 1:
      if (!CancelIo ((HANDLE)socket))
	{
	  debug_printf ("CancelIo() %E, fallback to blocking io");
	  WSAGetOverlappedResult(socket, &ovr, &len, TRUE, flags);
	}
      else
	WSASetLastError (WSAEINTR);
      break;
    case WSA_WAIT_FAILED:
      break;
    default: /* Should be impossible. *LOL* */
      WSASetLastError (WSAEFAULT);
      break;
    }
  WSACloseEvent (event);
  event = NULL;
  return ret;
}

WSADATA wsadata;

/* Cygwin internal */
static fhandler_socket *
get (const int fd)
{
  cygheap_fdget cfd (fd);
  if (cfd < 0)
    return 0;

  fhandler_socket *const fh = cfd->is_socket ();
  if (!fh)
    set_errno (ENOTSOCK);

  return fh;
}

/* Cygwin internal */
static SOCKET __stdcall
set_socket_inheritance (SOCKET sock)
{
  SOCKET osock = sock;
  if (!DuplicateHandle (hMainProc, (HANDLE) sock, hMainProc, (HANDLE *) &sock,
			0, TRUE, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE))
    system_printf ("DuplicateHandle failed %E");
  else
    debug_printf ("DuplicateHandle succeeded osock %p, sock %p", osock, sock);
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
#ifdef _MT_SAFE
#define ntoa_buf  _reent_winsup ()->_ntoa_buf
#else
  static char *ntoa_buf = NULL;
#endif

  char *res = inet_ntoa (in);
  if (ntoa_buf)
    {
      free (ntoa_buf);
      ntoa_buf = NULL;
    }
  if (res)
    ntoa_buf = strdup (res);
  return ntoa_buf;
}

/* exported as inet_addr: BSD 4.3 */
extern "C" unsigned long
cygwin_inet_addr (const char *cp)
{
  if (check_null_str_errno (cp))
    return INADDR_NONE;
  unsigned long res = inet_addr (cp);
  return res;
}

/* exported as inet_aton: BSD 4.3
   inet_aton is not exported by wsock32 and ws2_32,
   so it has to be implemented here. */
extern "C" int
cygwin_inet_aton (const char *cp, struct in_addr *inp)
{
  if (check_null_str_errno (cp) || check_null_invalid_struct_errno (inp))
    return 0;

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
  if (check_null_str_errno (cp))
    return INADDR_NONE;
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

static NO_COPY struct tl errmap[] =
{
 {WSAEINTR, "WSAEINTR", EINTR},
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
static NO_COPY struct tl host_errmap[] =
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

static void
free_char_list (char **clist)
{
  if (clist)
    {
      for (char **cl = clist; *cl; ++cl)
	free (*cl);
      free (clist);
    }
}

static char **
dup_char_list (char **src)
{
  char **dst;
  int cnt = 0;

  for (char **cl = src; *cl; ++cl)
    ++cnt;
  if (!(dst = (char **) calloc (cnt + 1, sizeof *dst)))
    return NULL;
  while (cnt-- > 0)
    if (!(dst[cnt] = strdup (src[cnt])))
      return NULL;
  return dst;
}

#define free_addr_list(addr_list)	free_char_list (addr_list)

static char **
dup_addr_list (char **src, unsigned int size)
{
  char **dst;
  int cnt = 0;

  for (char **cl = src; *cl; ++cl)
    ++cnt;
  if (!(dst = (char **) calloc (cnt + 1, sizeof *dst)))
    return NULL;
  while (cnt-- > 0)
    {
      if (!(dst[cnt] = (char *) malloc(size)))
	return NULL;
      memcpy(dst[cnt], src[cnt], size);
    }
  return dst;
}

static void
free_protoent_ptr (struct protoent *&p)
{
  if (p)
    {
      debug_printf ("protoent: %s", p->p_name);

      if (p->p_name)
	free (p->p_name);
      free_char_list (p->p_aliases);
      p = NULL;
    }
}

static struct protoent *
dup_protoent_ptr (struct protoent *src)
{
  if (!src)
    return NULL;

  struct protoent *dst = (struct protoent *) calloc (1, sizeof *dst);
  if (!dst)
    return NULL;

  debug_printf ("protoent: %s", src->p_name);

  dst->p_proto = src->p_proto;
  if (src->p_name && !(dst->p_name = strdup (src->p_name)))
    goto out;
  if (src->p_aliases && !(dst->p_aliases = dup_char_list (src->p_aliases)))
    goto out;

  debug_printf ("protoent: copied %s", dst->p_name);

  return dst;

out:
  free_protoent_ptr (dst);
  return NULL;
}

#ifdef _MT_SAFE
#define protoent_buf  _reent_winsup ()->_protoent_buf
#else
  static struct protoent *protoent_buf = NULL;
#endif

/* exported as getprotobyname: standards? */
extern "C" struct protoent *
cygwin_getprotobyname (const char *p)
{
  if (check_null_str_errno (p))
    return NULL;
  free_protoent_ptr (protoent_buf);
  protoent_buf = dup_protoent_ptr (getprotobyname (p));
  if (!protoent_buf)
    set_winsock_errno ();

  dump_protoent (protoent_buf);
  return protoent_buf;
}

/* exported as getprotobynumber: standards? */
extern "C" struct protoent *
cygwin_getprotobynumber (int number)
{
  free_protoent_ptr (protoent_buf);
  protoent_buf = dup_protoent_ptr (getprotobynumber (number));
  if (!protoent_buf)
    set_winsock_errno ();

  dump_protoent (protoent_buf);
  return protoent_buf;
}

fhandler_socket *
fdsock (int& fd, const char *name, SOCKET soc)
{
  if (!winsock2_active)
    soc = set_socket_inheritance (soc);
  else if (wincap.has_set_handle_information ())
    {
      /* NT systems apparently set sockets to inheritable by default */
      SetHandleInformation ((HANDLE)soc, HANDLE_FLAG_INHERIT, 0);
      debug_printf ("reset socket inheritance since winsock2_active %d", winsock2_active);
    }
  else
    debug_printf ("not setting socket inheritance since winsock2_active %d", winsock2_active);
  fhandler_socket *fh = (fhandler_socket *) cygheap->fdtab.build_fhandler (fd, FH_SOCKET, name);
  fh->set_io_handle ((HANDLE) soc);
  fh->set_flags (O_RDWR | O_BINARY);
  fh->set_r_no_interrupt (winsock2_active);
  debug_printf ("fd %d, name '%s', soc %p", fd, name, soc);
  return fh;
}

/* exported as socket: standards? */
extern "C" int
cygwin_socket (int af, int type, int protocol)
{
  int res = -1;
  SOCKET soc = 0;
  fhandler_socket* fh = NULL;

  cygheap_fdnew fd;

  if (fd >= 0)
    {
      debug_printf ("socket (%d, %d, %d)", af, type, protocol);

      soc = socket (AF_INET, type, af == AF_LOCAL ? 0 : protocol);

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

      fh = fdsock (fd, name, soc);
      if (fh)
	{
	  fh->set_addr_family (af);
	  fh->set_socket_type (type);
	}
      res = fd;
    }

done:
  syscall_printf ("%d = socket (%d, %d, %d)", res, af, type, protocol);
  return res;
}

/* exported as sendto: standards? */
extern "C" int
cygwin_sendto (int fd, const void *buf, int len, int flags,
	       const struct sockaddr *to, int tolen)
{
  int res;
  sigframe thisframe (mainthread);

  fhandler_socket *fh = get (fd);

  if ((len && __check_invalid_read_ptr_errno (buf, (unsigned) len))
      || (to &&__check_invalid_read_ptr_errno (to, tolen))
      || !fh)
    res = -1;
  else if ((res = len) != 0)
    res = fh->sendto (buf, len, flags, to, tolen);

  syscall_printf ("%d = sendto (%d, %p, %d, %x, %p, %d)",
		  res, fd, buf, len, flags, to, tolen);

  return res;
}

/* exported as recvfrom: standards? */
extern "C" int
cygwin_recvfrom (int fd, void *buf, int len, int flags,
		 struct sockaddr *from, int *fromlen)
{
  int res;
  sigframe thisframe (mainthread);

  fhandler_socket *fh = get (fd);

  if ((len && __check_null_invalid_struct_errno (buf, (unsigned) len))
      || (from
	  && (check_null_invalid_struct_errno (fromlen)
	      ||__check_null_invalid_struct_errno (from, (unsigned) *fromlen)))
      || !fh)
    res = -1;
  else if ((res = len) != 0)
    res = fh->recvfrom (buf, len, flags, from, fromlen);

  syscall_printf ("%d = recvfrom (%d, %p, %d, %x, %p, %p)",
		  res, fd, buf, len, flags, from, fromlen);

  return res;
}

/* exported as setsockopt: standards? */
extern "C" int
cygwin_setsockopt (int fd, int level, int optname, const void *optval,
		   int optlen)
{
  int res;
  fhandler_socket *fh = get (fd);
  const char *name = "error";

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

  if ((optval && __check_invalid_read_ptr_errno (optval, optlen)) || !fh)
    res = -1;
  else
    {
      res = setsockopt (fh->get_socket (), level, optname,
			(const char *) optval, optlen);

      if (optlen == 4)
	syscall_printf ("setsockopt optval=%x", *(long *) optval);

      if (res)
	set_winsock_errno ();
    }

  syscall_printf ("%d = setsockopt (%d, %d, %x (%s), %p, %d)",
		  res, fd, level, optname, name, optval, optlen);
  return res;
}

/* exported as getsockopt: standards? */
extern "C" int
cygwin_getsockopt (int fd, int level, int optname, void *optval, int *optlen)
{
  int res;
  fhandler_socket *fh = get (fd);
  const char *name = "error";

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

  if ((optval
       && (check_null_invalid_struct_errno (optlen)
	   || __check_null_invalid_struct_errno (optval, (unsigned) *optlen)))
      || !fh)
    res = -1;
  else
    {
      res = getsockopt (fh->get_socket (), level, optname, (char *) optval,
      			(int *) optlen);

      if (optname == SO_ERROR)
	{
	  int *e = (int *) optval;
	  *e = find_winsock_errno (*e);
	}

      if (res)
	set_winsock_errno ();
    }

  syscall_printf ("%d = getsockopt (%d, %d, %x (%s), %p, %p)",
		  res, fd, level, optname, name, optval, optlen);
  return res;
}

/* exported as connect: standards? */
extern "C" int
cygwin_connect (int fd, const struct sockaddr *name, int namelen)
{
  int res;
  sigframe thisframe (mainthread);

  fhandler_socket *fh = get (fd);

  if (__check_invalid_read_ptr_errno (name, namelen) || !fh)
    res = -1;
  else
    res = fh->connect (name, namelen);

  syscall_printf ("%d = connect (%d, %p, %d)", res, fd, name, namelen);

  return res;
}

static void
free_servent_ptr (struct servent *&p)
{
  if (p)
    {
      debug_printf ("servent: %s", p->s_name);

      if (p->s_name)
	free (p->s_name);
      if (p->s_proto)
	free (p->s_proto);
      free_char_list (p->s_aliases);
      p = NULL;
    }
}

#pragma pack(push,2)
struct pservent
{
  char *s_name;
  char **s_aliases;
  short s_port;
  char *s_proto;
};
#pragma pack(pop)
static struct servent *
dup_servent_ptr (struct servent *src)
{
  if (!src)
    return NULL;

  struct servent *dst = (struct servent *) calloc (1, sizeof *dst);
  if (!dst)
    return NULL;

  debug_printf ("servent: %s", src->s_name);

  dst->s_port = src->s_port;
  if (src->s_name && !(dst->s_name = strdup (src->s_name)))
    goto out;
  if (src->s_aliases && !(dst->s_aliases = dup_char_list (src->s_aliases)))
    goto out;
  char *s_proto;
  if (IsBadReadPtr (src->s_proto, sizeof (src->s_proto))
      && !IsBadReadPtr (((pservent *) src)->s_proto, sizeof (src->s_proto)))
    s_proto = ((pservent *)src)->s_proto;
  else
    s_proto = src->s_proto;

  if (s_proto && !(dst->s_proto = strdup (s_proto)))
    goto out;

  debug_printf ("servent: copied %s", dst->s_name);

  return dst;

out:
  free_servent_ptr (dst);
  return NULL;
}

#ifdef _MT_SAFE
#define servent_buf  _reent_winsup ()->_servent_buf
#else
  static struct servent *servent_buf = NULL;
#endif

/* exported as getservbyname: standards? */
extern "C" struct servent *
cygwin_getservbyname (const char *name, const char *proto)
{
  if (check_null_str_errno (name)
      || (proto != NULL && check_null_str_errno (proto)))
    return NULL;

  free_servent_ptr (servent_buf);
  servent_buf = dup_servent_ptr (getservbyname (name, proto));
  if (!servent_buf)
    set_winsock_errno ();

  syscall_printf ("%x = getservbyname (%s, %s)", servent_buf, name, proto);
  return servent_buf;
}

/* exported as getservbyport: standards? */
extern "C" struct servent *
cygwin_getservbyport (int port, const char *proto)
{
  if (proto != NULL && check_null_str_errno (proto))
    return NULL;

  free_servent_ptr (servent_buf);
  servent_buf = dup_servent_ptr (getservbyport (port, proto));
  if (!servent_buf)
    set_winsock_errno ();

  syscall_printf ("%x = getservbyport (%d, %s)", servent_buf, port, proto);
  return servent_buf;
}

extern "C" int
cygwin_gethostname (char *name, size_t len)
{
  int PASCAL win32_gethostname (char*, int);

  if (__check_null_invalid_struct_errno (name, len))
    return -1;

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

static void
free_hostent_ptr (struct hostent *&p)
{
  if (p)
    {
      debug_printf ("hostent: %s", p->h_name);

      if (p->h_name)
	free ((void *)p->h_name);
      free_char_list (p->h_aliases);
      free_addr_list (p->h_addr_list);
      p = NULL;
    }
}

static struct hostent *
dup_hostent_ptr (struct hostent *src)
{
  if (!src)
    return NULL;

  struct hostent *dst = (struct hostent *) calloc (1, sizeof *dst);
  if (!dst)
    return NULL;

  debug_printf ("hostent: %s", src->h_name);

  dst->h_addrtype = src->h_addrtype;
  dst->h_length = src->h_length;
  if (src->h_name && !(dst->h_name = strdup (src->h_name)))
    goto out;
  if (src->h_aliases && !(dst->h_aliases = dup_char_list (src->h_aliases)))
    goto out;
  if (src->h_addr_list
      && !(dst->h_addr_list = dup_addr_list(src->h_addr_list, src->h_length)))
    goto out;

  debug_printf ("hostent: copied %s", dst->h_name);

  return dst;

out:
  free_hostent_ptr (dst);
  return NULL;
}

#ifdef _MT_SAFE
#define hostent_buf  _reent_winsup ()->_hostent_buf
#else
  static struct hostent *hostent_buf = NULL;
#endif

/* exported as gethostbyname: standards? */
extern "C" struct hostent *
cygwin_gethostbyname (const char *name)
{
  static unsigned char tmp_addr[4];
  static struct hostent tmp;
  static char *tmp_aliases[1];
  static char *tmp_addr_list[2];
  static int a, b, c, d;

  if (check_null_str_errno (name))
    return NULL;

  if (sscanf (name, "%d.%d.%d.%d", &a, &b, &c, &d) == 4)
    {
      /* In case you don't have DNS, at least x.x.x.x still works */
      memset (&tmp, 0, sizeof (tmp));
      tmp_addr[0] = a;
      tmp_addr[1] = b;
      tmp_addr[2] = c;
      tmp_addr[3] = d;
      tmp_addr_list[0] = (char *) tmp_addr;
      tmp.h_name = name;
      tmp.h_aliases = tmp_aliases;
      tmp.h_addrtype = 2;
      tmp.h_length = 4;
      tmp.h_addr_list = tmp_addr_list;
      return &tmp;
    }

  free_hostent_ptr (hostent_buf);
  hostent_buf = dup_hostent_ptr (gethostbyname (name));
  if (!hostent_buf)
    {
      set_winsock_errno ();
      set_host_errno ();
    }
  else
    {
      debug_printf ("h_name %s", hostent_buf->h_name);
      h_errno = 0;
    }
  return hostent_buf;
}

/* exported as gethostbyaddr: standards? */
extern "C" struct hostent *
cygwin_gethostbyaddr (const char *addr, int len, int type)
{
  if (__check_invalid_read_ptr_errno (addr, len))
    return NULL;

  free_hostent_ptr (hostent_buf);
  hostent_buf = dup_hostent_ptr (gethostbyaddr (addr, len, type));
  if (!hostent_buf)
    {
      set_winsock_errno ();
      set_host_errno ();
    }
  else
    {
      debug_printf ("h_name %s", hostent_buf->h_name);
      h_errno = 0;
    }
  return hostent_buf;
}

/* exported as accept: standards? */
extern "C" int
cygwin_accept (int fd, struct sockaddr *peer, int *len)
{
  int res;
  sigframe thisframe (mainthread);

  fhandler_socket *fh = get (fd);

  if ((peer && (check_null_invalid_struct_errno (len)
		|| __check_null_invalid_struct_errno (peer, (unsigned) *len)))
      || !fh)
    res = -1;
  else
    res = fh->accept (peer, len);

  syscall_printf ("%d = accept (%d, %p, %p)", res, fd, peer, len);
  return res;
}

/* exported as bind: standards? */
extern "C" int
cygwin_bind (int fd, const struct sockaddr *my_addr, int addrlen)
{
  int res;
  fhandler_socket *fh = get (fd);

  if (__check_invalid_read_ptr_errno (my_addr, addrlen) || !fh)
    res = -1;
  else
    res = fh->bind (my_addr, addrlen);

  syscall_printf ("%d = bind (%d, %p, %d)", res, fd, my_addr, addrlen);
  return res;
}

/* exported as getsockname: standards? */
extern "C" int
cygwin_getsockname (int fd, struct sockaddr *addr, int *namelen)
{
  int res;
  sigframe thisframe (mainthread);

  fhandler_socket *fh = get (fd);

  if (check_null_invalid_struct_errno (namelen)
      || __check_null_invalid_struct_errno (addr, (unsigned) *namelen)
      || !fh)
    res = -1;
  else
    res = fh->getsockname (addr, namelen);

  syscall_printf ("%d = getsockname (%d, %p, %p)", res, fd, addr, namelen);
  return res;
}

/* exported as listen: standards? */
extern "C" int
cygwin_listen (int fd, int backlog)
{
  int res;
  fhandler_socket *fh = get (fd);

  if (!fh)
    res = -1;
  else
    res = fh->listen (backlog);

  syscall_printf ("%d = listen (%d, %d)", res, fd, backlog);
  return res;
}

/* exported as shutdown: standards? */
extern "C" int
cygwin_shutdown (int fd, int how)
{
  int res;
  sigframe thisframe (mainthread);

  fhandler_socket *fh = get (fd);

  if (!fh)
    res = -1;
  else
    res = fh->shutdown (how);

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
  if (s && check_null_str (s))
    return;
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
  int res;
  sigframe thisframe (mainthread);

  fhandler_socket *fh = get (fd);

  if (check_null_invalid_struct_errno (len)
      || __check_null_invalid_struct_errno (name, (unsigned) *len)
      || !fh)
    res = -1;
  else
    res = fh->getpeername (name, len);

  syscall_printf ("%d = getpeername %d", res, (fh ? fh->get_socket () : -1));
  return res;
}

/* exported as recv: standards? */
extern "C" int
cygwin_recv (int fd, void *buf, int len, int flags)
{
  return cygwin_recvfrom (fd, buf, len, flags, NULL, NULL);
}

/* exported as send: standards? */
extern "C" int
cygwin_send (int fd, const void *buf, int len, int flags)
{
  return cygwin_sendto (fd, buf, len, flags, NULL, 0);
}

/* getdomainname: standards? */
extern "C" int
getdomainname (char *domain, size_t len)
{
  /*
   * This works for Win95 only if the machine is configured to use MS-TCP.
   * If a third-party TCP is being used this will fail.
   * FIXME: On Win95, is there a way to portably check the TCP stack
   * in use and include paths for the Domain name in each ?
   * Punt for now and assume MS-TCP on Win95.
   */
  if (__check_null_invalid_struct_errno (domain, len))
    return -1;

  reg_key r (HKEY_LOCAL_MACHINE, KEY_READ,
	     (!wincap.is_winnt ()) ? "System" : "SYSTEM",
	     "CurrentControlSet", "Services",
	     (!wincap.is_winnt ()) ? "VxD" : "Tcpip",
	     (!wincap.is_winnt ()) ? "MSTCP" : "Parameters",
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
  char eth[2] = "/", ppp[2] = "/", slp[2] = "/", sub[2] = "0", tok[2] = "/";

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
      /* Iterate over all known interfaces */
      for (if_cnt = 0; if_cnt < ift->dwNumEntries; ++if_cnt)
	{
	  *sub = '0';
	  /* Iterate over all configured IP-addresses */
	  for (ip_cnt = 0; ip_cnt < ipt->dwNumEntries; ++ip_cnt)
	    {
	      /* Does the IP address belong to the interface? */
	      if (ipt->table[ip_cnt].dwIndex == ift->table[if_cnt].dwIndex)
		{
		  /* Setup the interface name */
		  switch (ift->table[if_cnt].dwType)
		    {
		      case MIB_IF_TYPE_TOKENRING:
			++*tok;
			strcpy (ifr->ifr_name, "tok");
			strcat (ifr->ifr_name, tok);
			break;
		      case MIB_IF_TYPE_ETHERNET:
			if (*sub == '0')
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
		  if (*sub > '0')
		    {
		      strcat (ifr->ifr_name, ":");
		      strcat (ifr->ifr_name, sub);
		    }
		  ++*sub;
		  /* setup sockaddr struct */
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
		}
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
 *	  -> Value "Driver" enthält Subkey relativ zu
 *	    HKLM/System/CurrentControlSet/Class/
 *	  -> In Subkey "Bindings" die Values aufzählen
 *	    -> Enthält Subkeys der Form "VREDIR\*"
 *	       Das * ist ein Subkey relativ zu
 *	       HKLM/System/CurrentControlSet/Class/Net/
 * HKLM/System/CurrentControlSet/Class/"Driver"
 *	  -> Value "IPAddress"
 *	  -> Value "IPMask"
 * HKLM/System/CurrentControlSet/Class/Net/"*"(aus "VREDIR\*")
 *	  -> Wenn Value "AdapterName" == "MS$PPP" -> ppp interface
 *	  -> Value "DriverDesc" enthält den Namen
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
      char driver[256], classname[256], netname[256];
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

      strcpy (netname, "System\\CurrentControlSet\\Services\\Class\\Net\\");
      strcat (netname, ifname);

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

  if (check_null_invalid_struct_errno (ifc))
    return -1;

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
  if (wincap.has_ip_helper_lib ())
    get_2k_ifconf (ifc, what);
  else if (wincap.is_winnt ())
    get_nt_ifconf (ifc, what);
  else
    get_95_ifconf (ifc, what);
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

  if (check_null_invalid_struct_errno (ahost) ||
      check_null_empty_str_errno (*ahost) ||
      (locuser && check_null_empty_str_errno (locuser)) ||
      (remuser && check_null_str_errno (remuser)))
    return (int) INVALID_SOCKET;

  cygheap_fdnew res_fd;
  if (res_fd < 0)
    goto done;

  if (fd2p)
    {
      cygheap_fdnew newfd (res_fd, false);
      if (*fd2p < 0)
	goto done;
      *fd2p = newfd;
    }

  res = rcmd (ahost, inport, locuser, remuser, cmd, fd2p ? &fd2s : NULL);
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
  syscall_printf ("%d = rcmd (...)", res);
  return res;
}

/* exported as rresvport: standards? */
extern "C" int
cygwin_rresvport (int *port)
{
  int res;
  sigframe thisframe (mainthread);

  if (check_null_invalid_struct_errno (port))
    return -1;

  cygheap_fdnew res_fd;
  if (res_fd < 0)
    res = -1;
  else
    {
      res = rresvport (port);

      if (res != (int) INVALID_SOCKET)
	{
	  fdsock (res_fd, "/dev/tcp", res);
	  res = res_fd;
	}
    }

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

  if (check_null_invalid_struct_errno (ahost) ||
      check_null_empty_str_errno (*ahost) ||
      (locuser && check_null_empty_str_errno (locuser)) ||
      (password && check_null_str_errno (password)))
    return (int) INVALID_SOCKET;

  cygheap_fdnew res_fd;
  if (res_fd < 0)
    goto done;
  if (fd2p)
    {
      cygheap_fdnew newfd (res_fd);
      if (newfd < 0)
	goto done;
      *fd2p = newfd;
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
socketpair (int family, int type, int protocol, int *sb)
{
  int res = -1;
  SOCKET insock, outsock, newsock;
  struct sockaddr_in sock_in, sock_out;
  int len;
  cygheap_fdnew sb0;
  fhandler_socket *fh;

  if (__check_null_invalid_struct_errno (sb, 2 * sizeof(int)))
    return -1;

  if (family != AF_LOCAL && family != AF_INET)
    {
      set_errno (EAFNOSUPPORT);
      goto done;
    }
  if (type != SOCK_STREAM && type != SOCK_DGRAM)
    {
      set_errno (EPROTOTYPE);
      goto done;
    }
  if ((family == AF_LOCAL && protocol != PF_UNSPEC && protocol != PF_LOCAL)
      || (family == AF_INET && protocol != PF_UNSPEC && protocol != PF_INET))
    {
      set_errno (EPROTONOSUPPORT);
      goto done;
    }

  if (sb0 < 0)
    goto done;
  else
    {
      sb[0] = sb0;
      cygheap_fdnew sb1 (sb0, false);
      if (sb1 < 0)
	goto done;

      sb[1] = sb1;
    }

  /* create the first socket */
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
  len = sizeof (sock_in);
  if (getsockname (newsock, (struct sockaddr *) &sock_in, &len) < 0)
    {
      debug_printf ("getsockname error");
      set_winsock_errno ();
      closesocket (newsock);
      goto done;
    }

  /* For stream sockets, create a listener */
  if (type == SOCK_STREAM)
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

  /* For datagram sockets, bind the 2nd socket to an unused address, too */
  if (type == SOCK_DGRAM)
    {
      sock_out.sin_family = AF_INET;
      sock_out.sin_port = 0;
      sock_out.sin_addr.s_addr = INADDR_ANY;
      if (bind (outsock, (struct sockaddr *) &sock_out, sizeof (sock_out)) < 0)
	{
	  debug_printf ("bind failed");
	  set_winsock_errno ();
	  closesocket (newsock);
	  closesocket (outsock);
	  goto done;
	}
      len = sizeof (sock_out);
      if (getsockname (outsock, (struct sockaddr *) &sock_out, &len) < 0)
	{
	  debug_printf ("getsockname error");
	  set_winsock_errno ();
	  closesocket (newsock);
	  closesocket (outsock);
	  goto done;
	}
    }

  /* Force IP address to loopback */
  sock_in.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
  if (type == SOCK_DGRAM)
    sock_out.sin_addr.s_addr = htonl (INADDR_LOOPBACK);

  /* Do a connect */
  if (connect (outsock, (struct sockaddr *) &sock_in,
					   sizeof (sock_in)) < 0)
    {
      debug_printf ("connect error");
      set_winsock_errno ();
      closesocket (newsock);
      closesocket (outsock);
      goto done;
    }

  if (type == SOCK_STREAM)
    {
      /* For stream sockets, accept the connection and close the listener */
      len = sizeof (sock_in);
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
    }
  else
    {
      /* For datagram sockets, connect the 2nd socket */
      if (connect (newsock, (struct sockaddr *) &sock_out,
					       sizeof (sock_out)) < 0)
	{
	  debug_printf ("connect error");
	  set_winsock_errno ();
	  closesocket (newsock);
	  closesocket (outsock);
	  goto done;
	}
      insock = newsock;
    }

  res = 0;

  if (family == AF_LOCAL)
    {

      fh = fdsock (sb[0],
		   type == SOCK_STREAM ? "/dev/streamsocket" : "/dev/dgsocket",
		   insock);
      fh->set_sun_path ("");
      fh->set_addr_family (AF_LOCAL);
      fh->set_socket_type (type);
      fh = fdsock (sb[1],
		   type == SOCK_STREAM ? "/dev/streamsocket" : "/dev/dgsocket",
		   outsock);
      fh->set_sun_path ("");
      fh->set_addr_family (AF_LOCAL);
      fh->set_socket_type (type);
    }
  else
    {
      fh = fdsock (sb[0], type == SOCK_STREAM ? "/dev/tcp" : "/dev/udp",
		   insock);
      fh->set_addr_family (AF_INET);
      fh->set_socket_type (type);
      fh = fdsock (sb[1], type == SOCK_STREAM ? "/dev/tcp" : "/dev/udp",
		   outsock);
      fh->set_addr_family (AF_INET);
      fh->set_socket_type (type);
    }

done:
  syscall_printf ("%d = socketpair (...)", res);
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

/* exported as recvmsg: standards? */
extern "C" int
cygwin_recvmsg (int fd, struct msghdr *msg, int flags)
{
  int res;
  sigframe thisframe (mainthread);

  fhandler_socket *fh = get (fd);

  if (check_null_invalid_struct_errno (msg)
      || (msg->msg_name
	  && __check_null_invalid_struct_errno (msg->msg_name,
						(unsigned) msg->msg_namelen))
      || !fh)
    res = -1;
  else
    {
      res = check_iovec_for_read (msg->msg_iov, msg->msg_iovlen);
      if (res > 0)
	res = fh->recvmsg (msg, flags, res); // res == iovec tot
    }

  syscall_printf ("%d = recvmsg (%d, %p, %x)", res, fd, msg, flags);
  return res;
}

/* exported as sendmsg: standards? */
extern "C" int
cygwin_sendmsg (int fd, const struct msghdr *msg, int flags)
{
  int res;
  sigframe thisframe (mainthread);

  fhandler_socket *fh = get (fd);

  if (__check_invalid_read_ptr_errno (msg, sizeof msg)
      || (msg->msg_name
	  && __check_invalid_read_ptr_errno (msg->msg_name,
					     (unsigned) msg->msg_namelen))
      || !fh)
    res = -1;
  else 
    {
      res = check_iovec_for_write (msg->msg_iov, msg->msg_iovlen);
      if (res > 0)
	res = fh->sendmsg (msg, flags, res); // res == iovec tot
    }

  syscall_printf ("%d = sendmsg (%d, %p, %x)", res, fd, msg, flags);
  return res;
}

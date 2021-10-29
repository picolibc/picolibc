/* net.cc: network-related routines.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* #define DEBUG_NEST_ON 1 */

#define  __INSIDE_CYGWIN_NET__
#define USE_SYS_TYPES_FD_SET
#define __WSA_ERR_MACROS_DEFINED
#include "winsup.h"
#ifdef __x86_64__
/* 2014-04-24: Current Mingw headers define sockaddr_in6 using u_long (8 byte)
   because a redefinition for LP64 systems is missing.  This leads to a wrong
   definition and size of sockaddr_in6 when building with winsock headers. */
#undef u_long
#define u_long __ms_u_long
#endif
#include <w32api/ws2tcpip.h>
#include <w32api/mswsock.h>
#include <w32api/iphlpapi.h>
#define gethostname cygwin_gethostname
#include <unistd.h>
#undef gethostname
#include <ifaddrs.h>
#include <netdb.h>
#include <asm/byteorder.h>
#include "shared_info.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "tls_pbuf.h"

/* Unfortunately defined in Windows header files and arpa/nameser_compat.h. */
#undef NOERROR
#undef DELETE
#define _CYGWIN_IN_H
#include <resolv.h>

extern "C"
{
  int h_errno;

  int sscanf (const char *, const char *, ...);
  int cygwin_inet_pton(int, const char *, void *);
  const char *cygwin_inet_ntop (int, const void *, char *, socklen_t);
  int dn_length1(const unsigned char *, const unsigned char *,
		 const unsigned char *);
}				/* End of "C" section */

const struct in6_addr in6addr_any = IN6ADDR_ANY_INIT;
const struct in6_addr in6addr_loopback = IN6ADDR_LOOPBACK_INIT;

static fhandler_socket *
get (const int fd)
{
  cygheap_fdget cfd (fd);

  if (cfd < 0)
    return NULL;

  fhandler_socket *const fh = cfd->is_socket ();

  if (!fh || (fh->get_flags () & O_PATH))
    {
      set_errno (ENOTSOCK);
      return NULL;
    }

  return fh;
}

/* exported as inet_ntoa: BSD 4.3 */
extern "C" char *
cygwin_inet_ntoa (struct in_addr in)
{
  char buf[20];
  const char *res = cygwin_inet_ntop (AF_INET, &in, buf, sizeof buf);

  if (_my_tls.locals.ntoa_buf)
    {
      free (_my_tls.locals.ntoa_buf);
      _my_tls.locals.ntoa_buf = NULL;
    }
  if (res)
    _my_tls.locals.ntoa_buf = strdup (res);
  return _my_tls.locals.ntoa_buf;
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

static const errmap_t wsock_errmap[] = {
  {WSA_INVALID_HANDLE, "WSA_INVALID_HANDLE", EBADF},
  {WSA_NOT_ENOUGH_MEMORY, "WSA_NOT_ENOUGH_MEMORY", ENOMEM},
  {WSA_INVALID_PARAMETER, "WSA_INVALID_PARAMETER", EINVAL},
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
  {WSAEBADF, "WSAEBADF", EBADF},
  {WSAEACCES, "WSAEACCES", EACCES},
  {WSAEMFILE, "WSAEMFILE", EMFILE},
  {0, "NOERROR", 0},
  {0, NULL, 0}
};

int
find_winsock_errno (DWORD why)
{
  for (int i = 0; wsock_errmap[i].s != NULL; ++i)
    if (why == wsock_errmap[i].w)
      return wsock_errmap[i].e;

  return EACCES;
}

void
__set_winsock_errno (const char *fn, int ln)
{
  DWORD werr = WSAGetLastError ();
  int err = find_winsock_errno (werr);

  set_errno (err);
  syscall_printf ("%s:%d - winsock error %u -> errno %d", fn, ln, werr, err);
}

/*
 * Since the member `s' isn't used for debug output we can use it
 * for the error text returned by herror and hstrerror.
 */
static const errmap_t host_errmap[] = {
  {WSAHOST_NOT_FOUND, "Unknown host", HOST_NOT_FOUND},
  {WSATRY_AGAIN, "Host name lookup failure", TRY_AGAIN},
  {WSANO_RECOVERY, "Unknown server error", NO_RECOVERY},
  {WSANO_DATA, "No address associated with name", NO_DATA},
  {0, NULL, 0}
};

static void
set_host_errno ()
{
  int i;

  DWORD why = WSAGetLastError ();

  for (i = 0; host_errmap[i].w != 0; ++i)
    if (why == host_errmap[i].w)
      break;

  if (host_errmap[i].w != 0)
    h_errno = host_errmap[i].e;
  else
    h_errno = NETDB_INTERNAL;
}

inline int
DWORD_round (int n)
{
  return sizeof (DWORD) * (((n + sizeof (DWORD) - 1)) / sizeof (DWORD));
}

inline int
strlen_round (const char *s)
{
  if (!s)
    return 0;
  return DWORD_round (strlen (s) + 1);
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

static const char *entnames[] = {"host", "proto", "serv"};

static unionent *
realloc_ent (unionent *&dst, int sz)
{
  /* Allocate the storage needed.  Allocate a rounded size to attempt to force
     reuse of this buffer so that a poorly-written caller will not be using
     a freed buffer. */
  unsigned rsz = 256 * ((sz + 255) / 256);
  unionent * ptr;
  if ((ptr = (unionent *) realloc (dst, rsz)))
    dst = ptr;
  return ptr;
}

static inline hostent *
realloc_ent (int sz, hostent *)
{
  return (hostent *) realloc_ent (_my_tls.locals.hostent_buf, sz);
}

/* Generic "dup a {host,proto,serv}ent structure" function.
   This is complicated because we need to be able to free the
   structure at any point and we can't rely on the pointer contents
   being untouched by callers.  So, we allocate a chunk of memory
   large enough to hold the structure and all of the stuff it points
   to then we copy the source into this new block of memory.
   The 'unionent' struct is a union of all of the currently used
   *ent structure.  */

#ifdef __x86_64__
/* For some baffling reason, somebody at Microsoft decided that it would be
   a good idea to exchange the s_port and s_proto members in the servent
   structure. */
struct win64_servent
{
  char  *s_name;
  char **s_aliases;
  char  *s_proto;
  short  s_port;
};
#define WIN_SERVENT(x)	((win64_servent *)(x))
#else
#define WIN_SERVENT(x)	((servent *)(x))
#endif

#ifdef DEBUGGING
static void *
#else
static inline void *
#endif
dup_ent (unionent *&dst, unionent *src, unionent::struct_type type)
{
  if (dst)
    debug_printf ("old %sent structure \"%s\" %p\n", entnames[type],
		  dst->name, dst);

  if (!src)
    {
      set_winsock_errno ();
      return NULL;
    }

  debug_printf ("duping %sent \"%s\", %p", entnames[type], src->name, src);

  /* Find the size of the raw structure minus any character strings, etc. */
  int sz, struct_sz;
  switch (type)
    {
    case unionent::t_protoent:
      struct_sz = sizeof (protoent);
      break;
    case unionent::t_servent:
      struct_sz = sizeof (servent);
      break;
    case unionent::t_hostent:
      struct_sz = sizeof (hostent);
      break;
    default:
      api_fatal ("called with invalid value %d", type);
      break;
    }

  /* Every *ent begins with a name.  Calculate its length. */
  int namelen = strlen_round (src->name);
  sz = struct_sz + namelen;

  char **av;
  /* The next field in every *ent is an argv list of "something".
     Calculate the number of components and how much space the
     character strings will take.  */
  int list_len = 0;
  for (av = src->list; av && *av; av++)
    {
      list_len++;
      sz += sizeof (char **) + strlen_round (*av);
    }

  /* NULL terminate if there actually was a list */
  if (av)
    {
      sz += sizeof (char **);
      list_len++;
    }

  /* Do servent/hostent specific processing */
  int protolen = 0;
  int addr_list_len = 0;
  if (type == unionent::t_servent)
    {
      if (WIN_SERVENT (src)->s_proto)
	sz += (protolen = strlen_round (WIN_SERVENT (src)->s_proto));
    }
  else if (type == unionent::t_hostent)
    {
      /* Calculate the length and storage used for h_addr_list */
      for (av = src->h_addr_list; av && *av; av++)
	{
	  addr_list_len++;
	  sz += sizeof (char **) + DWORD_round (src->h_len);
	}
      if (av)
	{
	  sz += sizeof (char **);
	  addr_list_len++;
	}
    }

  /* Allocate the storage needed.  */
  if (realloc_ent (dst, sz))
    {
      memset (dst, 0, sz);
      /* This field is common to all *ent structures but named differently
	 in each, of course.  Also, take 64 bit Windows servent weirdness
	 into account. */
      if (type == unionent::t_servent)
	dst->port_proto_addrtype = WIN_SERVENT (src)->s_port;
      else
	dst->port_proto_addrtype = src->port_proto_addrtype;

      char *dp = ((char *) dst) + struct_sz;
      if (namelen)
	{
	  /* Copy the name field to dst, using space just beyond the end of
	     the dst structure. */
	  strcpy (dst->name = dp, src->name);
	  dp += namelen;
	}

      /* Copy the 'list' type to dst, using space beyond end of structure
	 + storage for name. */
      if (src->list)
	{
	  char **dav = dst->list = (char **) dp;
	  dp += sizeof (char **) * list_len;
	  for (av = src->list; av && *av; av++)
	    {
	      int len = strlen (*av) + 1;
	      memcpy (*dav++ = dp, *av, len);
	      dp += DWORD_round (len);
	    }
	}

      /* Do servent/protoent/hostent specific processing. */
      if (type == unionent::t_protoent)
	debug_printf ("protoent %s %p %y", dst->name, dst->list, dst->port_proto_addrtype);
      else if (type == unionent::t_servent)
	{
	  if (WIN_SERVENT (src)->s_proto)
	    {
	      strcpy (dst->s_proto = dp, WIN_SERVENT (src)->s_proto);
	      dp += protolen;
	    }
	}
      else if (type == unionent::t_hostent)
	{
	  /* Transfer h_len and duplicate contents of h_addr_list, using
	     memory after 'list' allocation. */
	  dst->h_len = src->h_len;
	  char **dav = dst->h_addr_list = (char **) dp;
	  dp += sizeof (char **) * addr_list_len;
	  for (av = src->h_addr_list; av && *av; av++)
	    {
	      memcpy (*dav++ = dp, *av, src->h_len);
	      dp += DWORD_round (src->h_len);
	    }
	}
    }
  debug_printf ("duped %sent \"%s\", %p", entnames[type], dst ? dst->name : "<null!>", dst);
  return dst;
}

static inline hostent *
dup_ent (hostent *src)
{
  return (hostent *) dup_ent (_my_tls.locals.hostent_buf, (unionent *) src, unionent::t_hostent);
}

static inline protoent *
dup_ent (protoent *src)
{
  return (protoent *) dup_ent (_my_tls.locals.protoent_buf, (unionent *) src, unionent::t_protoent);
}

static inline servent *
dup_ent (servent *src)
{
  return (servent *) dup_ent (_my_tls.locals.servent_buf, (unionent *) src, unionent::t_servent);
}

/* exported as getprotobyname: POSIX.1-2001, POSIX.1-2008, 4.3BSD */
extern "C" struct protoent *
cygwin_getprotobyname (const char *p)
{
  __try
    {
      return dup_ent (getprotobyname (p));
    }
  __except (EFAULT) {}
  __endtry
  return NULL;
}

/* exported as getprotobynumber: POSIX.1-2001, POSIX.1-2008, 4.3BSD */
extern "C" struct protoent *
cygwin_getprotobynumber (int number)
{
  return dup_ent (getprotobynumber (number));
}

/* exported as socket: POSIX.1-2001, POSIX.1-2008, 4.4BSD */
extern "C" int
cygwin_socket (int af, int type, int protocol)
{
  int res = -1;
  const device *dev;
  fhandler_socket *fh;

  int flags = type & _SOCK_FLAG_MASK;
  type &= ~_SOCK_FLAG_MASK;

  debug_printf ("socket (%d, %d (flags %y), %d)", af, type, flags, protocol);

  switch (af)
    {
    case AF_LOCAL:
#ifndef __WITH_AF_UNIX
      dev = af_local_dev;
#else
    case AF_UNIX:
      dev = (af == AF_LOCAL) ? af_local_dev : af_unix_dev;
#endif /* __WITH_AF_UNIX */
      break;
    case AF_INET:
    case AF_INET6:
      dev = af_inet_dev;
      break;
    default:
      set_errno (EAFNOSUPPORT);
      goto done;
    }

  if ((flags & ~(SOCK_NONBLOCK | SOCK_CLOEXEC)) != 0)
    {
      set_errno (EINVAL);
      goto done;
    }

    {
      cygheap_fdnew fd;

      if (fd < 0)
	goto done;
      fh = (fhandler_socket *) build_fh_dev (*dev);
      if (fh && fh->socket (af, type, protocol, flags) == 0)
	{
	  fd = fh;
	  if (fd <= 2)
	    set_std_handle (fd);
	  res = fd;
	}
      else
	delete fh;
    }

done:
  syscall_printf ("%R = socket(%d, %d (flags %y), %d)",
		  res, af, type, flags, protocol);
  return res;
}

/* exported as sendto: 4.4BSD, SVr4, POSIX.1-2001 */
extern "C" ssize_t
cygwin_sendto (int fd, const void *buf, size_t len, int flags,
	       const struct sockaddr *to, socklen_t tolen)
{
  ssize_t res = -1;

  pthread_testcancel ();

  __try
    {
      fhandler_socket *fh = get (fd);
      if (fh)
	res = fh->sendto (buf, len, flags, to, tolen);
    }
  __except (EFAULT) {}
  __endtry
  syscall_printf ("%lR = sendto(%d, %p, %ld, %y, %p, %d)",
		  res, fd, buf, len, flags, to, tolen);
  return res;
}

/* exported as recvfrom: 4.4BSD, SVr4, POSIX.1-2001 */
extern "C" ssize_t
cygwin_recvfrom (int fd, void *buf, size_t len, int flags,
		 struct sockaddr *from, socklen_t *fromlen)
{
  ssize_t res = -1;

  pthread_testcancel ();

  __try
    {
      fhandler_socket *fh = get (fd);
      if (fh)
	/* Originally we shortcircuited here if res == 0.
	   Allow 0 bytes buffer.  This is valid in POSIX and handled in
	   fhandler_socket::recv_internal.  If we shortcircuit, we fail
	   to deliver valid error conditions and peer address. */
	res = fh->recvfrom (buf, len, flags, from, fromlen);
    }
  __except (EFAULT) {}
  __endtry
  syscall_printf ("%lR = recvfrom(%d, %p, %ld, %y, %p, %p)",
		  res, fd, buf, len, flags, from, fromlen);
  return res;
}

/* exported as setsockopt: POSIX.1-2001,  POSIX.1-2008,  SVr4,  4.4BSD */
extern "C" int
cygwin_setsockopt (int fd, int level, int optname, const void *optval,
		   socklen_t optlen)
{
  int ret = -1;

  __try
    {
      fhandler_socket *fh = get (fd);
      if (fh)
	ret = fh->setsockopt (level, optname, optval, optlen);
    }
  __except (EFAULT) {}
  __endtry
  syscall_printf ("%R = setsockopt(%d, %d, %y, %p, %d)",
                  ret, fd, level, optname, optval, optlen);
  return ret;
}

/* exported as getsockopt: POSIX.1-2001,  POSIX.1-2008,  SVr4,  4.4BSD */
extern "C" int
cygwin_getsockopt (int fd, int level, int optname, void *optval,
		   socklen_t *optlen)
{
  int ret = -1;

  __try
    {
      fhandler_socket *fh = get (fd);
      if (fh)
	ret = fh->getsockopt (level, optname, optval, optlen);
    }
  __except (EFAULT) {}
  __endtry
  syscall_printf ("%R = getsockopt(%d, %d, %y, %p, %p)",
                  ret, fd, level, optname, optval, optlen);
  return ret;
}

/* POSIX.1-2001 */
extern "C" int
sockatmark (int fd)
{
  int ret;
  cygheap_fdget cfd (fd);

  if (cfd < 0)
    return -1;

  fhandler_socket *const fh = cfd->is_socket ();
  if (!fh)
    set_errno (ENOTSOCK);
  else if (fh->get_flags () & O_PATH)
    set_errno (EBADF);
  else if (fh->ioctl (SIOCATMARK, &ret) != -1)
    return ret;
  return -1;
}

/* BSD */
extern "C" int
getpeereid (int fd, uid_t *euid, gid_t *egid)
{
  fhandler_socket *fh = get (fd);
  if (fh)
    return fh->getpeereid (NULL, euid, egid);
  return -1;
}

/* exported as connect: POSIX.1-2001, POSIX.1-2008, SVr4, 4.4BSD */
extern "C" int
cygwin_connect (int fd, const struct sockaddr *name, socklen_t namelen)
{
  int res = -1;

  pthread_testcancel ();

  __try
    {
      fhandler_socket *fh = get (fd);
      if (fh)
	res = fh->connect (name, namelen);
    }
  __except (EFAULT) {}
  __endtry
  syscall_printf ("%R = connect(%d, %p, %d)", res, fd, name, namelen);
  return res;
}

/* exported as getservbyname: POSIX.1-2001, POSIX.1-2008, 4.3BSD */
extern "C" struct servent *
cygwin_getservbyname (const char *name, const char *proto)
{
  servent *res = NULL;

  __try
    {
      res = dup_ent (getservbyname (name, proto));
    }
  __except (EFAULT) {}
  __endtry
  syscall_printf ("%p = getservbyname (%s, %s)", res, name, proto);
  return res;
}

/* exported as getservbyport: POSIX.1-2001, POSIX.1-2008, 4.3BSD */
extern "C" struct servent *
cygwin_getservbyport (int port, const char *proto)
{
  servent *res = NULL;

  __try
    {
      res = dup_ent (getservbyport (port, proto));
    }
  __except (EFAULT) {}
  __endtry
  syscall_printf ("%p = getservbyport (%d, %s)", res, port, proto);
  return res;
}

extern "C" int
cygwin_gethostname (char *name, size_t len)
{
  __try
    {
      PFIXED_INFO info = NULL;
      ULONG size = 0;

      if (GetNetworkParams(info, &size) == ERROR_BUFFER_OVERFLOW
	  && (info = (PFIXED_INFO) alloca(size))
	  && GetNetworkParams(info, &size) == ERROR_SUCCESS)
	{
	  strncpy(name, info->HostName, len);
	  debug_printf ("gethostname %s", name);
	  return 0;
	}
      __seterrno ();
    }
  __except (EFAULT)
  __endtry
  return -1;
}

extern "C" int
sethostname (const char *name, size_t len)
{
  WCHAR wname[MAX_COMPUTERNAME_LENGTH + 1];

  sys_mbstowcs (wname, MAX_COMPUTERNAME_LENGTH + 1, name, len);
  if (!SetComputerNameExW (ComputerNamePhysicalDnsHostname, wname))
    {
      __seterrno ();
      return -1;
    }
  return 0;
}

/* getdomainname: 4.4BSD */
extern "C" int
getdomainname (char *domain, size_t len)
{
  __try
    {
      PFIXED_INFO info = NULL;
      ULONG size = 0;

      if (GetNetworkParams(info, &size) == ERROR_BUFFER_OVERFLOW
	  && (info = (PFIXED_INFO) alloca(size))
	  && GetNetworkParams(info, &size) == ERROR_SUCCESS)
	{
	  strncpy(domain, info->DomainName, len);
	  debug_printf ("getdomainname %s", domain);
	  return 0;
	}
      __seterrno ();
    }
  __except (EFAULT)
  __endtry
  return -1;
}

/* exported as gethostbyname: POSIX.1-2001 */
extern "C" struct hostent *
cygwin_gethostbyname (const char *name)
{
  unsigned char tmp_addr[4];
  struct hostent tmp, *h;
  char *tmp_aliases[1] = {0};
  char *tmp_addr_list[2] = {0,0};
  unsigned int a, b, c, d;
  char dummy;
  hostent *res = NULL;

  __try
    {
      if (sscanf (name, "%u.%u.%u.%u%c", &a, &b, &c, &d, &dummy) != 4
	  || a >= 256 || b >= 256 || c >= 256 || d >= 256)
	h = gethostbyname (name);
      else
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
	  h = &tmp;
	}

      res = dup_ent (h);
      if (res)
	debug_printf ("h_name %s", res->h_name);
      else
	{
	  debug_printf ("dup_ent returned NULL for name %s, h %p", name, h);
	  set_host_errno ();
	}
    }
  __except (EFAULT)
    {
      res = NULL;
    }
  __endtry
  return res;
}

/* exported as gethostbyaddr: POSIX.1-2001 */
extern "C" struct hostent *
cygwin_gethostbyaddr (const void *addr, socklen_t len, int type)
{
  hostent *res = NULL;

  __try
    {
      res = dup_ent (gethostbyaddr ((const char *) addr, len, type));
      if (res)
	debug_printf ("h_name %s", res->h_name);
      else
	set_host_errno ();
    }
  __except (EFAULT)
    {
      res = NULL;
    }
  __endtry
  return res;
}

static void
memcpy4to6 (char *dst, const u_int8_t *src)
{
  const unsigned int h[] = {0, 0, htonl (0xFFFF)};
  memcpy (dst, h, 12);
  memcpy (dst + 12, src, NS_INADDRSZ);
}

/* gethostby_specials: RFC 6761
   Handles numerical addresses and special names for gethostbyname2 */
static hostent *
gethostby_specials (const char *name, const int af,
		    const int addrsize_in, const int addrsize_out)
{
  int namelen = strlen (name);
  /* Ignore a final '.' */
  if ((namelen == 0) || ((namelen -= (name[namelen - 1] == '.')) == 0))
    {
      set_errno (EINVAL);
      h_errno = NETDB_INTERNAL;
      return NULL;
    }

  int res;
  u_int8_t address[NS_IN6ADDRSZ];
  /* Test for numerical addresses */
  res = cygwin_inet_pton(af, name, address);
  /* Test for special domain names */
  if (res != 1)
    {
      {
	char const match[] = "invalid";
	int const matchlen = sizeof(match) - 1;
	int start = namelen - matchlen;
	if ((start >= 0) && ((start == 0) || (name[start-1] == '.'))
	    && (strncasecmp (&name[start], match , matchlen) == 0))
	  {
	    h_errno = HOST_NOT_FOUND;
	    return NULL;
	  }
      }
      {
	char const match[] = "localhost";
	int const matchlen = sizeof(match) - 1;
	int start = namelen - matchlen;
	if ((start >= 0) && ((start == 0) || (name[start-1] == '.'))
	    && (strncasecmp (&name[start], match , matchlen) == 0))
	  {
	    res = 1;
	    if (af == AF_INET)
	      {
		address[0] = 127;
		address[1] = address[2] = 0;
		address[3] = 1;
	      }
	    else
	      {
		memset (address, 0, NS_IN6ADDRSZ);
		address[NS_IN6ADDRSZ-1] = 1;
	      }
	  }
      }
    }
  if (res != 1)
    return NULL;

  int const alias_count = 0, address_count = 1;
  char * string_ptr;
  int sz = DWORD_round (sizeof(hostent))
    + sizeof (char *) * (alias_count + address_count + 2)
    + namelen + 1
    + address_count * addrsize_out;
  hostent *ret = realloc_ent (sz,  (hostent *) NULL);
  if (!ret)
    {
      /* errno is already set */
      h_errno = NETDB_INTERNAL;
      return NULL;
    }

  ret->h_addrtype = af;
  ret->h_length = addrsize_out;
  ret->h_aliases = (char **) (((char *) ret) + DWORD_round (sizeof(hostent)));
  ret->h_addr_list = ret->h_aliases + alias_count + 1;
  string_ptr = (char *) (ret->h_addr_list + address_count + 1);
  ret->h_name = string_ptr;

  memcpy (string_ptr, name, namelen);
  string_ptr[namelen] = 0;
  string_ptr += namelen + 1;

  ret->h_addr_list[0] = string_ptr;
  if (addrsize_in != addrsize_out)
    {
      memcpy4to6 (string_ptr, address);
      ret->h_addrtype = AF_INET6;
    }
  else
    memcpy (string_ptr, address, addrsize_out);

  ret->h_aliases[alias_count] = NULL;
  ret->h_addr_list[address_count] = NULL;

  return ret;
}

static hostent *
gethostby_helper (const char *name, const int af, const int type,
		  const int addrsize_in, const int addrsize_out)
{
  /* Get the data from the name server */
  const int maxcount = 3;
  int old_errno, ancount = 0, anlen = 1024, msgsize = 0;
  unsigned char *ptr, *msg = NULL;
  int sz;
  hostent *ret;
  char *string_ptr;

  while ((anlen > msgsize) && (ancount++ < maxcount))
    {
      msgsize = anlen;
      ptr = (unsigned char *) realloc (msg, msgsize);
      if (ptr == NULL )
	{
	  old_errno = errno;
	  free (msg);
	  set_errno (old_errno);
	  h_errno = NETDB_INTERNAL;
	  return NULL;
	}
      msg = ptr;
      anlen = res_search (name, ns_c_in, type, msg, msgsize);
    }

  if (ancount >= maxcount)
    {
      free (msg);
      h_errno = NO_RECOVERY;
      return NULL;
    }
  if (anlen < 0) /* errno and h_errno are set */
    {
      old_errno = errno;
      free (msg);
      set_errno (old_errno);
      return NULL;
    }
  unsigned char *eomsg = msg + anlen - 1;

  /* We scan the answer records to determine the required memory size.
     They can be corrupted and we don't fully trust that the message
     follows the standard exactly. glibc applies some checks that
     we emulate.
     The answers are copied in the hostent structure in a second scan.
     To simplify the second scan we store information as follows:
     - "class" is replaced by the compressed name size
     - the first 16 bits of the "ttl" store the expanded name size + 1
     - the last 16 bits of the "ttl" store the offset to the next valid record.
     Note that "type" is rewritten in host byte order. */

  class record {
  public:
    unsigned type: 16;		// type
    unsigned complen: 16;       // class or compressed length
    unsigned namelen1: 16;      // expanded length (with final 0)
    unsigned next_o: 16;        // offset to next valid
    unsigned size: 16;          // data size
    unsigned char data[];       // data
    record * next () { return (record *) (((char *) this) + next_o); }
    void set_next ( record * nxt) { next_o = ((char *) nxt) - ((char *) this); }
    unsigned char *name () { return (unsigned char *)
				    (((char *) this) - complen); }
  };

  record * anptr = NULL, * prevptr = NULL, * curptr;
  int i, alias_count = 0, string_size = 0, address_count = 0;
  int namelen1 = 0, address_len = 0, antype, anclass, ansize;
  unsigned complen;

  /* Get the count of answers */
  ancount = ntohs (((HEADER *) msg)->ancount);

  /* Skip the question, it was verified by res_send */
  ptr = msg + sizeof (HEADER);
  if ((complen = dn_skipname (ptr, eomsg)) < 0)
    goto corrupted;
  /* Point to the beginning of the answer section */
  ptr += complen + NS_QFIXEDSZ;

  /* Scan the answer records to determine the sizes */
  for (i = 0; i < ancount; i++, ptr = curptr->data + ansize)
    {
      if ((complen = dn_skipname (ptr, eomsg)) < 0)
	goto corrupted;

      curptr = (record *) (ptr + complen);
      antype = ntohs (curptr->type);
      anclass = ntohs (curptr->complen);
      ansize = ntohs (curptr->size);
      /* Class must be internet */
      if (anclass != ns_c_in)
	continue;

      curptr->complen = complen;
      if ((namelen1 = dn_length1 (msg, eomsg, curptr-> name())) <= 0)
	goto corrupted;

      if (antype == ns_t_cname)
	{
	  alias_count++;
	  string_size += namelen1;
	}
      else if (antype == type)
	{
	  ansize = ntohs (curptr->size);
	  if (ansize != addrsize_in)
	    continue;
	  if (address_count == 0)
	    {
	      address_len = namelen1;
	      string_size += namelen1;
	    }
	  else if (address_len != namelen1)
	    continue;
	  address_count++;
	}
      /* Update the records */
      curptr->type = antype; /* Host byte order */
      curptr->namelen1 = namelen1;
      if (! anptr)
	anptr = prevptr = curptr;
      else
	{
	  prevptr->set_next (curptr);
	  prevptr = curptr;
	}
    }

  /* If there is no address, quit */
  if (address_count == 0)
    {
      free (msg);
      h_errno = NO_DATA;
      return NULL;
    }

  /* Determine the total size */
  sz = DWORD_round (sizeof(hostent))
       + sizeof (char *) * (alias_count + address_count + 2)
       + string_size
       + address_count * addrsize_out;

  ret = realloc_ent (sz,  (hostent *) NULL);
  if (! ret)
    {
      old_errno = errno;
      free (msg);
      set_errno (old_errno);
      h_errno = NETDB_INTERNAL;
      return NULL;
    }

  ret->h_addrtype = af;
  ret->h_length = addrsize_out;
  ret->h_aliases = (char **) (((char *) ret) + DWORD_round (sizeof(hostent)));
  ret->h_addr_list = ret->h_aliases + alias_count + 1;
  string_ptr = (char *) (ret->h_addr_list + address_count + 1);

  /* Rescan the answers */
  alias_count = address_count = 0;
  prevptr->set_next (prevptr + 1);

  for (curptr = anptr; curptr <= prevptr; curptr = curptr->next ())
    {
      antype = curptr->type;
      if (antype == ns_t_cname)
	{
	  dn_expand (msg, eomsg, curptr->name (), string_ptr, curptr->namelen1);
	  ret->h_aliases[alias_count++] = string_ptr;
	  string_ptr += curptr->namelen1;
	}
      else
	{
	  if (address_count == 0)
	    {
	      dn_expand (msg, eomsg, curptr->name (), string_ptr,
			 curptr->namelen1);
	      ret->h_name = string_ptr;
	      string_ptr += curptr->namelen1;
	    }
	  ret->h_addr_list[address_count++] = string_ptr;
	  if (addrsize_in != addrsize_out)
	    {
	      memcpy4to6 (string_ptr, curptr->data);
	      ret->h_addrtype =  AF_INET6;
	    }
	  else
	    memcpy (string_ptr, curptr->data, addrsize_in);
	  string_ptr += addrsize_out;
	}
    }

  free (msg);

  ret->h_aliases[alias_count] = NULL;
  ret->h_addr_list[address_count] = NULL;

  return ret;

corrupted:
  free (msg);
  /* Hopefully message corruption errors are temporary.
     Should it be NO_RECOVERY ? */
  h_errno = TRY_AGAIN;
  return NULL;
}

/* gethostbyname2: GNU extension */
extern "C" struct hostent *
gethostbyname2 (const char *name, int af)
{
  hostent *res = NULL;

  __try
    {
      if (!(_res.options & RES_INIT))
	res_init();

      bool v4to6 = _res.options & RES_USE_INET6;
      int type, addrsize_in, addrsize_out;

      switch (af)
	{
	case AF_INET:
	  addrsize_in = NS_INADDRSZ;
	  addrsize_out = (v4to6) ? NS_IN6ADDRSZ : NS_INADDRSZ;
	  type = ns_t_a;
	  break;
	case AF_INET6:
	  addrsize_in = addrsize_out = NS_IN6ADDRSZ;
	  type = ns_t_aaaa;
	  break;
	default:
	  set_errno (EAFNOSUPPORT);
	  h_errno = NETDB_INTERNAL;
	  __leave;
	}

      h_errno = NETDB_SUCCESS;
      res = gethostby_specials (name, af, addrsize_in, addrsize_out);
      if ((res == NULL) && (h_errno == NETDB_SUCCESS))
	  res = gethostby_helper (name, af, type, addrsize_in, addrsize_out);
    }
  __except (EFAULT) {}
  __endtry
  return res;
}

/* exported as accept: POSIX.1-2001,  POSIX.1-2008,  SVr4,  4.4BSD */
extern "C" int
cygwin_accept (int fd, struct sockaddr *peer, socklen_t *len)
{
  int res = -1;

  pthread_testcancel ();

  __try
    {
      fhandler_socket *fh = get (fd);
      if (fh)
	res = fh->accept4 (peer, len,
			   fh->is_nonblocking () ? SOCK_NONBLOCK : 0);
    }
  __except (EFAULT) {}
  __endtry
  syscall_printf ("%R = accept(%d, %p, %p)", res, fd, peer, len);
  return res;
}

/* accept4: GNU extension */
extern "C" int
accept4 (int fd, struct sockaddr *peer, socklen_t *len, int flags)
{
  int res = -1;

  pthread_testcancel ();

  __try
    {
      fhandler_socket *fh = get (fd);
      if (!fh)
	__leave;
      if ((flags & ~(SOCK_NONBLOCK | SOCK_CLOEXEC)) != 0)
	  set_errno (EINVAL);
      else
	res = fh->accept4 (peer, len, flags);
    }
  __except (EFAULT) {}
  __endtry
  syscall_printf ("%R = accept4(%d, %p, %p, %y)", res, fd, peer, len, flags);
  return res;
}

/* exported as bind: POSIX.1-2001, POSIX.1-2008, SVr4,  4.4BSD */
extern "C" int
cygwin_bind (int fd, const struct sockaddr *my_addr, socklen_t addrlen)
{
  int res = -1;

  __try
    {
      fhandler_socket *fh = get (fd);
      if (fh)
	res = fh->bind (my_addr, addrlen);
    }
  __except (EFAULT) {}
  __endtry
  syscall_printf ("%R = bind(%d, %p, %d)", res, fd, my_addr, addrlen);
  return res;
}

/* exported as getsockname: POSIX.1-2001, POSIX.1-2008, SVr4,  4.4BSD */
extern "C" int
cygwin_getsockname (int fd, struct sockaddr *addr, socklen_t *namelen)
{
  int res = -1;

  __try
    {
      fhandler_socket *fh = get (fd);
      if (fh)
	res = fh->getsockname (addr, namelen);
    }
  __except (EFAULT) {}
  __endtry
  syscall_printf ("%R =getsockname (%d, %p, %p)", res, fd, addr, namelen);
  return res;
}

/* exported as listen: POSIX.1-2001, POSIX.1-2008, SVr4,  4.4BSD */
extern "C" int
cygwin_listen (int fd, int backlog)
{
  int res = -1;

  __try
    {
      fhandler_socket *fh = get (fd);
      if (fh)
	res = fh->listen (backlog);
    }
  __except (EFAULT) {}
  __endtry
  syscall_printf ("%R = listen(%d, %d)", res, fd, backlog);
  return res;
}

/* exported as shutdown: POSIX.1-2001, POSIX.1-2008, SVr4,  4.4BSD */
extern "C" int
cygwin_shutdown (int fd, int how)
{
  int res = -1;

  fhandler_socket *fh = get (fd);
  if (fh)
    res = fh->shutdown (how);
  syscall_printf ("%R = shutdown(%d, %d)", res, fd, how);
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
  __try
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
  __except (NO_ERROR) {}
  __endtry
}

/* exported as getpeername: POSIX.1-2001, POSIX.1-2008, SVr4,  4.4BSD */
extern "C" int
cygwin_getpeername (int fd, struct sockaddr *name, socklen_t *len)
{
  int res = -1;
  fhandler_socket *fh = NULL;

  __try
    {
      fh = get (fd);
      if (fh)
	res = fh->getpeername (name, len);
    }
  __except (EFAULT) {}
  __endtry
  syscall_printf ("%R = getpeername(%d)", res, fd);
  return res;
}

/* exported as recv: POSIX.1-2001, POSIX.1-2008, SVr4,  4.4BSD */
extern "C" ssize_t
cygwin_recv (int fd, void *buf, size_t len, int flags)
{
  ssize_t res = -1;

  pthread_testcancel ();

  __try
    {
      fhandler_socket *fh = get (fd);
      if (fh)
	/* Originally we shortcircuited here if res == 0.
	   Allow 0 bytes buffer.  This is valid in POSIX and handled in
	   fhandler_socket::recv_internal.  If we shortcircuit, we fail
	   to deliver valid error conditions. */
	res = fh->recvfrom (buf, len, flags, NULL, NULL);
    }
  __except (EFAULT) {}
  __endtry
  syscall_printf ("%lR = recv(%d, %p, %ld, %y)", res, fd, buf, len, flags);
  return res;
}

/* exported as send: POSIX.1-2001, POSIX.1-2008, SVr4,  4.4BSD */
extern "C" ssize_t
cygwin_send (int fd, const void *buf, size_t len, int flags)
{
  ssize_t res = -1;

  pthread_testcancel ();

  __try
    {
      fhandler_socket *fh = get (fd);
      if (fh)
	res = fh->sendto (buf, len, flags, NULL, 0);
    }
  __except (EFAULT)
  __endtry
  syscall_printf ("%lR = send(%d, %p, %ld, %y)", res, fd, buf, len, flags);
  return res;
}

/* Fill out an ifconf struct. */

struct gaa_wa {
  ULONG family;
  PIP_ADAPTER_ADDRESSES *pa_ret;
};

DWORD WINAPI
call_gaa (LPVOID param)
{
  DWORD ret, size = 0;
  gaa_wa *p = (gaa_wa *) param;
  PIP_ADAPTER_ADDRESSES pa0 = NULL;

  if (!p->pa_ret)
    return GetAdaptersAddresses (p->family, GAA_FLAG_INCLUDE_PREFIX
					    | GAA_FLAG_INCLUDE_ALL_INTERFACES,
				 NULL, NULL, &size);
  do
    {
      ret = GetAdaptersAddresses (p->family, GAA_FLAG_INCLUDE_PREFIX
					     | GAA_FLAG_INCLUDE_ALL_INTERFACES,
				  NULL, pa0, &size);
      if (ret == ERROR_BUFFER_OVERFLOW
	  && !(pa0 = (PIP_ADAPTER_ADDRESSES) realloc (pa0, size)))
	break;
    }
  while (ret == ERROR_BUFFER_OVERFLOW);
  if (pa0)
    {
      if (ret != ERROR_SUCCESS)
	{
	  free (pa0);
	  *p->pa_ret = NULL;
	}
      else
	*p->pa_ret = pa0;
    }
  return ret;
}

bool
get_adapters_addresses (PIP_ADAPTER_ADDRESSES *pa_ret, ULONG family)
{
  DWORD ret;
  gaa_wa param = { family, pa_ret };

  if (wincap.has_gaa_largeaddress_bug ()
      && (uintptr_t) &param >= (uintptr_t) 0x80000000L)
    {
      /* In Windows 7 under WOW64, GetAdaptersAddresses fails if it's running
	 in a thread with a stack located in the large address area.  If we're
	 running in a pthread with such a stack, we call GetAdaptersAddresses
	 in a child thread with an OS-allocated stack.  The OS allocates stacks
	 bottom up, so chances are good that the new stack will be located in
	 the lower address area. */
      HANDLE thr = CreateThread (NULL, 0, call_gaa, &param, 0, NULL);
      SetThreadName (GetThreadId (thr), "__call_gaa");
      if (!thr)
	{
	  debug_printf ("CreateThread: %E");
	  return false;
	}
      WaitForSingleObject (thr, INFINITE);
      GetExitCodeThread (thr, &ret);
      CloseHandle (thr);
    }
  else
    ret = call_gaa (&param);
  return ret == ERROR_SUCCESS || (!pa_ret && ret == ERROR_BUFFER_OVERFLOW);
}

static in_addr_t
get_routedst (DWORD if_index)
{
  PMIB_IPFORWARDTABLE pift;
  ULONG size = 0;
  if (GetIpForwardTable (NULL, &size, FALSE) == ERROR_INSUFFICIENT_BUFFER
      && (pift = (PMIB_IPFORWARDTABLE) alloca (size))
      && GetIpForwardTable (pift, &size, FALSE) == NO_ERROR)
    for (DWORD i = 0; i < pift->dwNumEntries; ++i)
      {
	if (pift->table[i].dwForwardIfIndex == if_index
	    && pift->table[i].dwForwardMask == INADDR_BROADCAST)
	  return pift->table[i].dwForwardDest;
      }
  return INADDR_ANY;
}

struct ifall {
  struct ifaddrs          ifa_ifa;
  char                    ifa_name[IFNAMSIZ];
  struct sockaddr_storage ifa_addr;
  struct sockaddr_storage ifa_brddstaddr;
  struct sockaddr_storage ifa_netmask;
  struct ifaddrs_hwdata   ifa_hwdata;
};

static unsigned int
get_flags (PIP_ADAPTER_ADDRESSES pap)
{
  unsigned int flags = IFF_UP;
  if (pap->IfType == IF_TYPE_SOFTWARE_LOOPBACK)
    flags |= IFF_LOOPBACK;
  else if (pap->IfType == IF_TYPE_PPP
	   || pap->IfType == IF_TYPE_SLIP)
    flags |= IFF_POINTOPOINT | IFF_NOARP;
  if (!(pap->Flags & IP_ADAPTER_NO_MULTICAST))
    flags |= IFF_MULTICAST;
  if (pap->OperStatus == IfOperStatusUp
      || pap->OperStatus == IfOperStatusUnknown)
    flags |= IFF_RUNNING;
  if (pap->OperStatus != IfOperStatusLowerLayerDown)
    flags |= IFF_LOWER_UP;
  if (pap->OperStatus == IfOperStatusDormant)
    flags |= IFF_DORMANT;
  return flags;
}

static ULONG
get_ipv4fromreg_ipcnt (const char *name)
{
  WCHAR regkey[256], *c;

  c = wcpcpy (regkey, L"Tcpip\\Parameters\\Interfaces\\");
  sys_mbstowcs (c, 220, name);
  if (!NT_SUCCESS (RtlCheckRegistryKey (RTL_REGISTRY_SERVICES, regkey)))
    return 0;

  ULONG ifs = 1;
  DWORD dhcp = 0;
  UNICODE_STRING uipa = { 0, 0, NULL };
  RTL_QUERY_REGISTRY_TABLE tab[3] = {
    { NULL, RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_NOSTRING,
      L"EnableDHCP", &dhcp, REG_NONE, NULL, 0 },
    { NULL, RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_NOEXPAND,
      L"IPAddress", &uipa, REG_NONE, NULL, 0 },
    { NULL, 0, NULL, NULL, 0, NULL, 0 }
  };

  /* If DHCP is used, we have only one address. */
  if (NT_SUCCESS (RtlQueryRegistryValues (RTL_REGISTRY_SERVICES, regkey, tab,
					  NULL, NULL))
      && uipa.Buffer)
    {
      if (dhcp == 0)
      for (ifs = 0, c = uipa.Buffer; *c; c += wcslen (c) + 1)
	ifs++;
      RtlFreeUnicodeString (&uipa);
    }
  return ifs;
}

static void
get_ipv4fromreg (struct ifall *ifp, const char *name, DWORD idx)
{
  WCHAR regkey[256], *c;
  bool got_addr = false, got_mask = false;

  c = wcpcpy (regkey, L"Tcpip\\Parameters\\Interfaces\\");
  sys_mbstowcs (c, 220, name);
  if (!NT_SUCCESS (RtlCheckRegistryKey (RTL_REGISTRY_SERVICES, regkey)))
    return;

  ULONG ifs;
  DWORD dhcp = 0;
  UNICODE_STRING udipa = { 0, 0, NULL };
  UNICODE_STRING udsub = { 0, 0, NULL };
  UNICODE_STRING uipa = { 0, 0, NULL };
  UNICODE_STRING usub = { 0, 0, NULL };
  RTL_QUERY_REGISTRY_TABLE tab[6] = {
    { NULL, RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_NOSTRING,
      L"EnableDHCP", &dhcp, REG_NONE, NULL, 0 },
    { NULL, RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_NOEXPAND,
      L"DhcpIPAddress", &udipa, REG_NONE, NULL, 0 },
    { NULL, RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_NOEXPAND,
      L"DhcpSubnetMask", &udsub, REG_NONE, NULL, 0 },
    { NULL, RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_NOEXPAND,
      L"IPAddress", &uipa, REG_NONE, NULL, 0 },
    { NULL, RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_NOEXPAND,
      L"SubnetMask", &usub, REG_NONE, NULL, 0 },
    { NULL, 0, NULL, NULL, 0, NULL, 0 }
  };

  if (NT_SUCCESS (RtlQueryRegistryValues (RTL_REGISTRY_SERVICES, regkey, tab,
					  NULL, NULL)))
    {
#     define addr ((struct sockaddr_in *) &ifp->ifa_addr)
#     define mask ((struct sockaddr_in *) &ifp->ifa_netmask)
#     define brdc ((struct sockaddr_in *) &ifp->ifa_brddstaddr)
#     define inet_uton(u, a) \
	{ \
	  char t[64]; \
	  sys_wcstombs (t, 64, (u)); \
	  cygwin_inet_pton (AF_INET, t, (a)); \
	}
      /* If DHCP is used, we have only one address. */
      if (dhcp)
	{
	  if (udipa.Buffer)
	    {
	      inet_uton (udipa.Buffer, &addr->sin_addr);
	      got_addr = true;
	    }
	  if (udsub.Buffer)
	    {
	      inet_uton (udsub.Buffer, &mask->sin_addr);
	      got_mask = true;
	    }
	}
      else
	{
	  if (uipa.Buffer)
	    {
	      for (ifs = 0, c = uipa.Buffer; *c && ifs < idx;
		   c += wcslen (c) + 1)
		ifs++;
	      if (*c)
		{
		  inet_uton (c, &addr->sin_addr);
		  got_addr = true;
		}
	    }
	  if (usub.Buffer)
	    {
	      for (ifs = 0, c = usub.Buffer; *c && ifs < idx;
		   c += wcslen (c) + 1)
		ifs++;
	      if (*c)
		{
		  inet_uton (c, &mask->sin_addr);
		  got_mask = true;
		}
	    }
	}
      if (got_addr)
	ifp->ifa_ifa.ifa_addr = (struct sockaddr *) addr;
      if (got_mask)
	ifp->ifa_ifa.ifa_netmask = (struct sockaddr *) mask;
      if (ifp->ifa_ifa.ifa_flags & IFF_BROADCAST)
	brdc->sin_addr.s_addr = (addr->sin_addr.s_addr
				 & mask->sin_addr.s_addr)
				| ~mask->sin_addr.s_addr;
#undef addr
#undef mask
#undef brdc
#undef inet_uton
      if (udipa.Buffer)
	RtlFreeUnicodeString (&udipa);
      if (udsub.Buffer)
	RtlFreeUnicodeString (&udsub);
      if (uipa.Buffer)
	RtlFreeUnicodeString (&uipa);
      if (usub.Buffer)
	RtlFreeUnicodeString (&usub);
    }
}

static void
get_friendlyname (struct ifall *ifp, PIP_ADAPTER_ADDRESSES pap)
{
  struct ifreq_frndlyname *iff = (struct ifreq_frndlyname *)
				 &ifp->ifa_hwdata.ifa_frndlyname;
  iff->ifrf_len = sys_wcstombs (iff->ifrf_friendlyname,
				IFRF_FRIENDLYNAMESIZ,
				pap->FriendlyName) + 1;
}

static void
get_hwaddr (struct ifall *ifp, PIP_ADAPTER_ADDRESSES pap)
{
  for (UINT i = 0; i < IFHWADDRLEN; ++i)
    if (i >= pap->PhysicalAddressLength)
      ifp->ifa_hwdata.ifa_hwaddr.sa_data[i] = '\0';
    else
      ifp->ifa_hwdata.ifa_hwaddr.sa_data[i] = pap->PhysicalAddress[i];
}

/*
 * Generate short, unique interface name for usage with aged
 * applications still using the old pre-1.7 ifreq structure.
 */
static void
gen_old_if_name (char *name, PIP_ADAPTER_ADDRESSES pap, DWORD idx)
{
  /* Note: The returned name must be < 16 chars. */
  const char *prefix;

  switch (pap->IfType)
    {
      case IF_TYPE_ISO88025_TOKENRING:
	prefix = "tok";
	break;
      case IF_TYPE_PPP:
	prefix = "ppp";
	break;
      case IF_TYPE_SOFTWARE_LOOPBACK:
	prefix = "lo";
	break;
      case IF_TYPE_ATM:
	prefix = "atm";
	break;
      case IF_TYPE_IEEE80211:
	prefix = "wlan";
	break;
      case IF_TYPE_SLIP:
      case IF_TYPE_RS232:
      case IF_TYPE_MODEM:
	prefix = "slp";
	break;
      case IF_TYPE_TUNNEL:
	prefix = "tun";
	break;
      default:
	prefix = "eth";
	break;
    }
  if (idx)
    __small_sprintf (name, "%s%u:%u", prefix, pap->IfIndex, idx);
  else
    __small_sprintf (name, "%s%u", prefix, pap->IfIndex, idx);
}

/*
 * Get network interfaces.  Use IP Helper function GetAdaptersAddresses.
 */
static struct ifall *
get_ifs (ULONG family)
{
  PIP_ADAPTER_ADDRESSES pa0 = NULL, pap;
  PIP_ADAPTER_UNICAST_ADDRESS pua;
  int cnt = 0;
  struct ifall *ifret = NULL, *ifp;
  struct sockaddr_in *if_sin;
  struct sockaddr_in6 *if_sin6;

  if (!get_adapters_addresses (&pa0, family))
    goto done;

  for (pap = pa0; pap; pap = pap->Next)
    if (!pap->FirstUnicastAddress)
      {
	/* FirstUnicastAddress is NULL for interfaces which are disconnected.
	   Fetch number of configured IPv4 addresses from registry and
	   store in an unused member of the adapter addresses structure. */
	pap->Ipv6IfIndex = get_ipv4fromreg_ipcnt (pap->AdapterName);
	cnt += pap->Ipv6IfIndex;
      }
    else for (pua = pap->FirstUnicastAddress; pua; pua = pua->Next)
      ++cnt;

  if (!(ifret = (struct ifall *) calloc (cnt, sizeof (struct ifall))))
    goto done;
  ifp = ifret;

  for (pap = pa0; pap; pap = pap->Next)
    {
      DWORD idx = 0;
      if (!pap->FirstUnicastAddress)
	for (idx = 0; idx < pap->Ipv6IfIndex; ++idx)
	  {
	    /* Next in chain */
	    ifp->ifa_ifa.ifa_next = (struct ifaddrs *) &ifp[1].ifa_ifa;
	    /* Interface name */

	    if (CYGWIN_VERSION_CHECK_FOR_OLD_IFREQ)
	      gen_old_if_name (ifp->ifa_name, pap, idx);
	    else if (idx)
	      __small_sprintf (ifp->ifa_name, "%s:%u", pap->AdapterName, idx);
	    else
	      strcpy (ifp->ifa_name, pap->AdapterName);
	    ifp->ifa_ifa.ifa_name = ifp->ifa_name;
	    /* Flags */
	    ifp->ifa_ifa.ifa_flags = get_flags (pap);
	    if (pap->IfType != IF_TYPE_PPP
		&& pap->IfType != IF_TYPE_SOFTWARE_LOOPBACK)
	      ifp->ifa_ifa.ifa_flags |= IFF_BROADCAST;
	    /* Address */
	    ifp->ifa_addr.ss_family = AF_INET;
	    ifp->ifa_ifa.ifa_addr = NULL;
	    /* Broadcast/Destination address */
	    ifp->ifa_brddstaddr.ss_family = AF_INET;
	    ifp->ifa_ifa.ifa_dstaddr = NULL;
	    /* Netmask */
	    ifp->ifa_netmask.ss_family = AF_INET;
	    ifp->ifa_ifa.ifa_netmask = NULL;
	    /* Try to fetch real IPv4 address information from registry. */
	    get_ipv4fromreg (ifp, pap->AdapterName, idx);
	    /* Hardware address */
	    get_hwaddr (ifp, pap);
	    /* Metric */
	    ifp->ifa_hwdata.ifa_metric = 1;
	    /* MTU */
	    ifp->ifa_hwdata.ifa_mtu = pap->Mtu;
	    /* Interface index */
	    ifp->ifa_hwdata.ifa_ifindex = pap->IfIndex;
	    /* Friendly name */
	    get_friendlyname (ifp, pap);
	    /* Let ifa_data member point to "ifaddrs_hwdata" data. */
	    ifp->ifa_ifa.ifa_data = &ifp->ifa_hwdata;
	    ++ifp;
	  }
      else
	for (idx = 0, pua = pap->FirstUnicastAddress; pua; pua = pua->Next)
	  {
	    struct sockaddr *sa = (struct sockaddr *) pua->Address.lpSockaddr;
#         define sin	((struct sockaddr_in *) sa)
#         define sin6	((struct sockaddr_in6 *) sa)
	    size_t sa_size = (sa->sa_family == AF_INET6
			      ? sizeof *sin6 : sizeof *sin);
	    /* Next in chain */
	    ifp->ifa_ifa.ifa_next = (struct ifaddrs *) &ifp[1].ifa_ifa;
	    /* Interface name */
	    if (CYGWIN_VERSION_CHECK_FOR_OLD_IFREQ)
	      gen_old_if_name (ifp->ifa_name, pap, idx);
	    else if (sa->sa_family == AF_INET && idx)
	      __small_sprintf (ifp->ifa_name, "%s:%u", pap->AdapterName, idx);
	    else
	      strcpy (ifp->ifa_name, pap->AdapterName);
	    if (sa->sa_family == AF_INET)
	      ++idx;
	    ifp->ifa_ifa.ifa_name = ifp->ifa_name;
	    /* Flags */
	    ifp->ifa_ifa.ifa_flags = get_flags (pap);
	    if (sa->sa_family == AF_INET
		&& pap->IfType != IF_TYPE_SOFTWARE_LOOPBACK
		&& pap->IfType != IF_TYPE_PPP)
	      ifp->ifa_ifa.ifa_flags |= IFF_BROADCAST;
	    /* Address */
	    memcpy (&ifp->ifa_addr, sa, sa_size);
	    ifp->ifa_ifa.ifa_addr = (struct sockaddr *) &ifp->ifa_addr;
	    /* Netmask */
	    int prefix = pua->OnLinkPrefixLength;
	    switch (sa->sa_family)
	      {
	      case AF_INET:
		if_sin = (struct sockaddr_in *) &ifp->ifa_netmask;
		if_sin->sin_addr.s_addr = htonl (UINT32_MAX << (32 - prefix));
		if_sin->sin_family = AF_INET;
		break;
	      case AF_INET6:
		if_sin6 = (struct sockaddr_in6 *) &ifp->ifa_netmask;
		for (cnt = 0; cnt < 4 && prefix > 0; ++cnt, prefix -= 32)
		  {
		    if_sin6->sin6_addr.s6_addr32[cnt] = UINT32_MAX;
		    if (prefix < 32)
		      if_sin6->sin6_addr.s6_addr32[cnt] <<= 32 - prefix;
		  }
		if_sin6->sin6_family = AF_INET6;
		break;
	      }
	    ifp->ifa_ifa.ifa_netmask = (struct sockaddr *) &ifp->ifa_netmask;
	    if (pap->IfType == IF_TYPE_PPP)
	      {
		/* Destination address */
		if (sa->sa_family == AF_INET)
		  {
		    if_sin = (struct sockaddr_in *) &ifp->ifa_brddstaddr;
		    if_sin->sin_addr.s_addr = get_routedst (pap->IfIndex);
		    if_sin->sin_family = AF_INET;
		  }
		else
		  /* FIXME: No official way to get the dstaddr for ipv6? */
		  memcpy (&ifp->ifa_addr, sa, sa_size);
		ifp->ifa_ifa.ifa_dstaddr = (struct sockaddr *)
					   &ifp->ifa_brddstaddr;
	      }
	    else
	      {
		/* Broadcast address  */
		if (sa->sa_family == AF_INET)
		  {
		    if_sin = (struct sockaddr_in *) &ifp->ifa_brddstaddr;
		    uint32_t mask =
		    ((struct sockaddr_in *) &ifp->ifa_netmask)->sin_addr.s_addr;
		    if_sin->sin_addr.s_addr = (sin->sin_addr.s_addr & mask)
					      | ~mask;
		    if_sin->sin_family = AF_INET;
		    ifp->ifa_ifa.ifa_broadaddr = (struct sockaddr *)
						 &ifp->ifa_brddstaddr;
		  }
		else /* No IPv6 broadcast */
		  ifp->ifa_ifa.ifa_broadaddr = NULL;
	      }
	    /* Hardware address */
	    get_hwaddr (ifp, pap);
	    /* Metric */
	    ifp->ifa_hwdata.ifa_metric = (sa->sa_family == AF_INET
					 ? pap->Ipv4Metric : pap->Ipv6Metric);
	    /* MTU */
	    ifp->ifa_hwdata.ifa_mtu = pap->Mtu;
	    /* Interface index */
	    ifp->ifa_hwdata.ifa_ifindex = pap->IfIndex;
	    /* Friendly name */
	    get_friendlyname (ifp, pap);
	    /* Let ifa_data member point to "ifaddrs_hwdata" data. */
	    ifp->ifa_ifa.ifa_data = &ifp->ifa_hwdata;
	    ++ifp;
#         undef sin
#         undef sin6
	  }
    }
  /* Since every entry is set to the next entry, the last entry points to an
     invalid next entry now.  Fix it retroactively. */
  if (ifp > ifret)
    ifp[-1].ifa_ifa.ifa_next = NULL;

done:
  if (pa0)
    free (pa0);
  return ifret;
}

extern "C" int
getifaddrs (struct ifaddrs **ifap)
{
  if (!ifap)
    {
      set_errno (EINVAL);
      return -1;
    }
  struct ifall *ifp;
  ifp = get_ifs (AF_UNSPEC);
  *ifap = &ifp->ifa_ifa;
  return ifp ? 0 : -1;
}

extern "C" void
freeifaddrs (struct ifaddrs *ifp)
{
  if (ifp)
    free (ifp);
}

int
get_ifconf (struct ifconf *ifc, int what)
{
  __try
    {
      /* Ensure we have space for at least one struct ifreqs, fail if not. */
      if (ifc->ifc_len < (int) sizeof (struct ifreq))
	{
	  set_errno (EINVAL);
	  __leave;
	}

      struct ifall *ifret, *ifp;
      ifret = get_ifs (AF_INET);
      if (!ifret)
	__leave;

      struct sockaddr_in *sin;
      struct ifreq *ifr = ifc->ifc_req;
      int cnt = 0;
      for (ifp = ifret; ifp; ifp = (struct ifall *) ifp->ifa_ifa.ifa_next)
	{
	  ++cnt;
	  strcpy (ifr->ifr_name, ifp->ifa_name);
	  switch (what)
	    {
	    case SIOCGIFFLAGS:
	      ifr->ifr_flags = ifp->ifa_ifa.ifa_flags;
	      break;
	    case SIOCGIFCONF:
	    case SIOCGIFADDR:
	      sin = (struct sockaddr_in *) &ifr->ifr_addr;
	      memcpy (sin, &ifp->ifa_addr, sizeof *sin);
	      break;
	    case SIOCGIFNETMASK:
	      sin = (struct sockaddr_in *) &ifr->ifr_netmask;
	      memcpy (sin, &ifp->ifa_netmask, sizeof *sin);
	      break;
	    case SIOCGIFDSTADDR:
	      sin = (struct sockaddr_in *) &ifr->ifr_dstaddr;
	      if (ifp->ifa_ifa.ifa_flags & IFF_POINTOPOINT)
		memcpy (sin, &ifp->ifa_brddstaddr, sizeof *sin);
	      else /* Return addr as on Linux. */
		memcpy (sin, &ifp->ifa_addr, sizeof *sin);
	      break;
	    case SIOCGIFBRDADDR:
	      sin = (struct sockaddr_in *) &ifr->ifr_broadaddr;
	      if (!(ifp->ifa_ifa.ifa_flags & IFF_POINTOPOINT))
		memcpy (sin, &ifp->ifa_brddstaddr, sizeof *sin);
	      else
		{
		  sin->sin_addr.s_addr = INADDR_ANY;
		  sin->sin_family = AF_INET;
		  sin->sin_port = 0;
		}
	      break;
	    case SIOCGIFHWADDR:
	      memcpy (&ifr->ifr_hwaddr, &ifp->ifa_hwdata.ifa_hwaddr,
		      sizeof ifr->ifr_hwaddr);
	      break;
	    case SIOCGIFMETRIC:
	      ifr->ifr_metric = ifp->ifa_hwdata.ifa_metric;
	      break;
	    case SIOCGIFMTU:
	      ifr->ifr_mtu = ifp->ifa_hwdata.ifa_mtu;
	      break;
	    case SIOCGIFINDEX:
	      ifr->ifr_ifindex = ifp->ifa_hwdata.ifa_ifindex;
	      break;
	    case SIOCGIFFRNDLYNAM:
	      memcpy (ifr->ifr_frndlyname, &ifp->ifa_hwdata.ifa_frndlyname,
		      sizeof (struct ifreq_frndlyname));
	    }
	  if ((caddr_t) ++ifr >
	      ifc->ifc_buf + ifc->ifc_len - sizeof (struct ifreq))
	    break;
	}
      /* Set the correct length */
      ifc->ifc_len = cnt * sizeof (struct ifreq);
      free (ifret);
      return 0;
    }
  __except (EFAULT) {}
  __endtry
  return -1;
}

extern "C" unsigned
cygwin_if_nametoindex (const char *name)
{
  return (unsigned) ::if_nametoindex (name);
}

extern "C" char *
cygwin_if_indextoname (unsigned ifindex, char *ifname)
{
  return ::if_indextoname (ifindex, ifname);
}

extern "C" struct if_nameindex *
if_nameindex (void)
{
  PIP_ADAPTER_ADDRESSES pa0 = NULL, pap;
  struct if_nameindex *iflist = NULL;
  char (*ifnamelist)[IF_NAMESIZE];

  __try
    {
      if (get_adapters_addresses (&pa0, AF_UNSPEC))
	{
	  int cnt = 0;
	  for (pap = pa0; pap; pap = pap->Next)
	    ++cnt;
	  iflist = (struct if_nameindex *)
		   malloc ((cnt + 1) * sizeof (struct if_nameindex)
			   + cnt * IF_NAMESIZE);
	  if (!iflist)
	    set_errno (ENOBUFS);
	  else
	    {
	      ifnamelist = (char (*)[IF_NAMESIZE]) (iflist + cnt + 1);
	      for (pap = pa0, cnt = 0; pap; pap = pap->Next)
		{
		  for (int i = 0; i < cnt; ++i)
		    if (iflist[i].if_index
			== (pap->Ipv6IfIndex ?: pap->IfIndex))
		      goto outer_loop;
		  iflist[cnt].if_index = pap->Ipv6IfIndex ?: pap->IfIndex;
		  strcpy (iflist[cnt].if_name = ifnamelist[cnt],
			  pap->AdapterName);
		  /* See comment in if_indextoname. */
		  if (pap->IfIndex == 1 && pap->Ipv6IfIndex == 0)
		    for (PIP_ADAPTER_ADDRESSES pap2 = pa0;
			 pap2;
			 pap2 = pap2->Next)
		      if (pap2->Ipv6IfIndex == 1)
			{
			  strcpy (ifnamelist[cnt], pap2->AdapterName);
			  break;
			}
		  ++cnt;
		outer_loop:
		  ;
		}
	      iflist[cnt].if_index = 0;
	      iflist[cnt].if_name = NULL;
	    }
	  free (pa0);
	}
      else
	set_errno (ENXIO);
    }
  __except (EFAULT) {}
  __endtry
  return iflist;
}

extern "C" void
if_freenameindex (struct if_nameindex *ptr)
{
  free (ptr);
}

#define PORT_LOW	(IPPORT_EFSSERVER + 1)
#define PORT_HIGH	(IPPORT_RESERVED - 1)
#define NUM_PORTS	(PORT_HIGH - PORT_LOW + 1)

/* port is in host byte order */
static in_port_t
next_free_port (sa_family_t family, in_port_t in_port)
{
  DWORD ret;
  ULONG size = 0;
  char *tab = NULL;
  PMIB_TCPTABLE tab4 = NULL;
  PMIB_TCP6TABLE tab6 = NULL;

  /* Start testing with incoming port number. */
  in_port_t tst_port = in_port;
  in_port_t res_port = 0;
  in_port_t tab_port;

  do
    {
      if (family == AF_INET)
	ret = GetTcpTable ((PMIB_TCPTABLE) tab, &size, TRUE);
      else
	ret = GetTcp6Table ((PMIB_TCP6TABLE) tab, &size, TRUE);

      if (ret == ERROR_INSUFFICIENT_BUFFER)
	tab = (char *) realloc (tab, size);
    }
  while (ret == ERROR_INSUFFICIENT_BUFFER);

  tab4 = (PMIB_TCPTABLE) tab;
  tab6 = (PMIB_TCP6TABLE) tab;

  /* dwNumEntries has offset 0 in both structs. */
  for (int idx = tab4->dwNumEntries - 1; idx >= 0; --idx)
    {
      if (family == AF_INET)
	tab_port = ntohs (tab4->table[idx].dwLocalPort);
      else
	tab_port = ntohs (tab6->table[idx].dwLocalPort);
      /* Skip table entries with too high port number. */
      if (tab_port > tst_port)
	continue;
      /* Is the current port number free? */
      if (tab_port < tst_port)
	{
	  res_port = tst_port;
	  break;
	}
      /* Decrement port and handle underflow of the reserved area. */
      if (--tst_port < PORT_LOW)
	{
	  tst_port = PORT_HIGH;
	  idx = tab4->dwNumEntries;
	}
      /* Check if we're round to the incoming port. */
      if (tst_port == in_port)
	break;
    }
  free (tab);
  return res_port;
}

extern "C" int
cygwin_bindresvport_sa (int fd, struct sockaddr *sa)
{
  struct sockaddr_storage sst;
  struct sockaddr_in *sin = NULL;
  struct sockaddr_in6 *sin6 = NULL;
  socklen_t salen;
  int ret = -1;
  LONG port, next_port;

  __try
    {
      fhandler_socket *fh = get (fd);
      if (!fh)
	__leave;

      if (!sa)
	{
	  sa = (struct sockaddr *) &sst;
	  memset (&sst, 0, sizeof sst);
	  sa->sa_family = fh->get_addr_family ();
	}
      else if (sa->sa_family != fh->get_addr_family ())
	{
	  set_errno (EPFNOSUPPORT);
	  __leave;
	}

      switch (sa->sa_family)
	{
	case AF_INET:
	  salen = sizeof (struct sockaddr_in);
	  sin = (struct sockaddr_in *) sa;
	  break;
	case AF_INET6:
	  salen = sizeof (struct sockaddr_in6);
	  sin6 = (struct sockaddr_in6 *) sa;
	  break;
	default:
	  set_errno (EPFNOSUPPORT);
	  __leave;
	}

      /* Originally we tried to use the incoming port number first here,
	 but that may lead to EADDRINUSE scenarios when calling bindresvport
	 on the client side.  So we ignore any port value that the caller
	 supplies, just like glibc. */

      /* Note that repeating this loop NUM_PORTS times is arbitrary.  The
         job is mainly done by next_free_port() but it doesn't cover bound
	 sockets.  And calling and checking GetTcpTable and subsequently
	 calling bind is inevitably racy.  We have to continue if bind fails
	 with EADDRINUSE. */
      for (int i = 0; i < NUM_PORTS; i++)
	{
	  while ((port = InterlockedExchange (
			    &cygwin_shared->last_used_bindresvport, -1)) == -1)
	    yield ();
	  next_port = port;
	  if (next_port == 0 || --next_port < PORT_LOW)
	    next_port = PORT_HIGH;
	  /* Returns 0 if no reserved port is free. */
	  next_port = next_free_port (sa->sa_family, next_port);
	  if (next_port)
	    port = next_port;
	  InterlockedExchange (&cygwin_shared->last_used_bindresvport, port);
	  if (next_port == 0)
	    {
	      set_errno (EADDRINUSE);
	      break;
	    }
	  if (sa->sa_family == AF_INET6)
	    sin6->sin6_port = htons (port);
	  else
	    sin->sin_port = htons (port);
	  if (!(ret = fh->bind (sa, salen)))
	    break;
	  if (get_errno () != EADDRINUSE && get_errno () != EINVAL)
	    break;
	}

    }
  __except (EFAULT) {}
  __endtry
  return ret;
}

extern "C" int
cygwin_bindresvport (int fd, struct sockaddr_in *sin)
{
  if (sin && sin->sin_family != AF_INET)
    {
      set_errno (EAFNOSUPPORT);
      return -1;
    }
  return cygwin_bindresvport_sa (fd, (struct sockaddr *) sin);
}

/* socketpair: POSIX.1-2001, POSIX.1-2008, 4.4BSD. */
extern "C" int
socketpair (int af, int type, int protocol, int sv[2])
{
  int res = -1;
  const device *dev;
  fhandler_socket *fh_in, *fh_out;

  int flags = type & _SOCK_FLAG_MASK;
  type &= ~_SOCK_FLAG_MASK;

  debug_printf ("socket (%d, %d (flags %y), %d)", af, type, flags, protocol);

  switch (af)
    {
    case AF_LOCAL:
#ifndef __WITH_AF_UNIX
      dev = af_local_dev;
#else
    case AF_UNIX:
      dev = (af == AF_LOCAL) ? af_local_dev : af_unix_dev;
#endif /* __WITH_AF_UNIX */
      break;
    default:
      set_errno (EAFNOSUPPORT);
      goto done;
    }

  if ((flags & ~(SOCK_NONBLOCK | SOCK_CLOEXEC)) != 0)
    {
      set_errno (EINVAL);
      goto done;
    }

    {
      cygheap_fdnew fd_in;
      if (fd_in < 0)
	goto done;

      cygheap_fdnew fd_out (fd_in, false);
      if (fd_out < 0)
	goto done;

      fh_in = reinterpret_cast<fhandler_socket *> (build_fh_dev (*dev));
      fh_out = reinterpret_cast<fhandler_socket *> (build_fh_dev (*dev));
      if (fh_in && fh_out
	  && fh_in->socketpair (af, type, protocol, flags, fh_out) == 0)
	{
	  fd_in = fh_in;
	  fd_out = fh_out;
	  if (fd_in <= 2)
	    set_std_handle (fd_in);
	  if (fd_out <= 2)
	    set_std_handle (fd_out);
	  __try
	    {
	      sv[0] = fd_in;
	      sv[1] = fd_out;
	      res = 0;
	    }
	  __except (EFAULT) {}
	  __endtry
	}
      else
	{
	  delete fh_in;
	  delete fh_out;
	}
    }

done:
  syscall_printf ("%R = socketpair(...)", res);
  return res;
}

/* sethostent: POSIX.1-2001 */
extern "C" void
sethostent (int)
{
}

/* endhostent: POSIX.1-2001 */
extern "C" void
endhostent (void)
{
}

/* exported as recvmsg: POSIX.1-2001, POSIX.1-2008, 4.4BSD */
extern "C" ssize_t
cygwin_recvmsg (int fd, struct msghdr *msg, int flags)
{
  ssize_t res = -1;

  pthread_testcancel ();

  __try
    {
      fhandler_socket *fh = get (fd);
      if (fh)
	{
	  res = check_iovec_for_read (msg->msg_iov, msg->msg_iovlen);
	  /* Originally we shortcircuited here if res == 0.
	     Allow 0 bytes buffer.  This is valid in POSIX and handled in
	     fhandler_socket::recv_internal.  If we shortcircuit, we fail
	     to deliver valid error conditions and peer address. */
	  if (res >= 0)
	    res = fh->recvmsg (msg, flags);
	}
    }
  __except (EFAULT)
    {
      res = -1;
    }
  __endtry
  syscall_printf ("%lR = recvmsg(%d, %p, %y)", res, fd, msg, flags);
  return res;
}

/* exported as sendmsg: POSIX.1-2001, POSIX.1-2008, 4.4BSD */
extern "C" ssize_t
cygwin_sendmsg (int fd, const struct msghdr *msg, int flags)
{
  ssize_t res = -1;

  pthread_testcancel ();

  __try
    {
      fhandler_socket *fh = get (fd);
      if (fh)
	{
	  res = check_iovec_for_write (msg->msg_iov, msg->msg_iovlen);
	  if (res >= 0)
	    res = fh->sendmsg (msg, flags);
	}
    }
  __except (EFAULT)
    {
      res = -1;
    }
  __endtry
  syscall_printf ("%lR = sendmsg(%d, %p, %y)", res, fd, msg, flags);
  return res;
}

/* This is from the BIND 4.9.4 release, modified to compile by itself */

/* Copyright (c) 1996 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/* int
 * inet_pton4(src, dst)
 *	like inet_aton() but without all the hexadecimal and shorthand.
 * return:
 *	1 if `src' is a valid dotted quad, else 0.
 * notice:
 *	does not touch `dst' unless it's returning 1.
 * author:
 *	Paul Vixie, 1996.
 */
static int
inet_pton4 (const char *src, u_int8_t *dst)
{
  static const char digits[] = "0123456789";
  int saw_digit, octets, ch;
  u_int8_t tmp[INADDRSZ], *tp;

  saw_digit = 0;
  octets = 0;
  *(tp = tmp) = 0;
  while ((ch = *src++) != '\0')
    {
      const char *pch;

      if ((pch = strchr(digits, ch)) != NULL)
	{
	  u_int32_t ret = *tp * 10 + (pch - digits);

	  if (ret > 255)
	    return (0);
	  *tp = ret;
	  if (! saw_digit)
	    {
	      if (++octets > 4)
		return (0);
	      saw_digit = 1;
	    }
	}
      else if (ch == '.' && saw_digit)
	{
	  if (octets == 4)
	    return (0);
	  *++tp = 0;
	  saw_digit = 0;
	}
      else
	return (0);
    }
  if (octets < 4)
    return (0);

  memcpy(dst, tmp, INADDRSZ);
  return (1);
}

/* int
 * inet_pton6(src, dst)
 *	convert presentation level address to network order binary form.
 * return:
 *	1 if `src' is a valid [RFC1884 2.2] address, else 0.
 * notice:
 *	(1) does not touch `dst' unless it's returning 1.
 *	(2) :: in a full address is silently ignored.
 * credit:
 *	inspired by Mark Andrews.
 * author:
 *	Paul Vixie, 1996.
 */
static int
inet_pton6 (const char *src, u_int8_t *dst)
{
  static const char xdigits_l[] = "0123456789abcdef",
		    xdigits_u[] = "0123456789ABCDEF";
  u_int8_t tmp[IN6ADDRSZ], *tp, *endp, *colonp;
  const char *xdigits, *curtok;
  int ch, saw_xdigit;
  u_int32_t val;

  memset((tp = tmp), 0, IN6ADDRSZ);
  endp = tp + IN6ADDRSZ;
  colonp = NULL;
  /* Leading :: requires some special handling. */
  if (*src == ':')
    if (*++src != ':')
      return (0);
  curtok = src;
  saw_xdigit = 0;
  val = 0;
  while ((ch = *src++) != '\0')
    {
      const char *pch;

      if ((pch = strchr((xdigits = xdigits_l), ch)) == NULL)
	pch = strchr((xdigits = xdigits_u), ch);
      if (pch != NULL)
	{
	  val <<= 4;
	  val |= (pch - xdigits);
	  if (val > 0xffff)
	    return (0);
	  saw_xdigit = 1;
	  continue;
	}
      if (ch == ':')
	{
	  curtok = src;
	  if (!saw_xdigit)
	    {
	      if (colonp)
		return (0);
	      colonp = tp;
	      continue;
	    }
	  if (tp + INT16SZ > endp)
	    return (0);
	  *tp++ = (u_int8_t) (val >> 8) & 0xff;
	  *tp++ = (u_int8_t) val & 0xff;
	  saw_xdigit = 0;
	  val = 0;
	  continue;
	}
      if (ch == '.' && ((tp + INADDRSZ) <= endp) && inet_pton4(curtok, tp) > 0)
	{
	  tp += INADDRSZ;
	  saw_xdigit = 0;
	  break;	/* '\0' was seen by inet_pton4(). */
	}
      return (0);
    }
  if (saw_xdigit)
    {
      if (tp + INT16SZ > endp)
	return (0);
      *tp++ = (u_int8_t) (val >> 8) & 0xff;
      *tp++ = (u_int8_t) val & 0xff;
    }
  if (colonp != NULL)
    {
      /*
       * Since some memmove()'s erroneously fail to handle
       * overlapping regions, we'll do the shift by hand.
       */
      const int n = tp - colonp;
      int i;

      for (i = 1; i <= n; i++)
	{
	  endp[- i] = colonp[n - i];
	  colonp[n - i] = 0;
	}
      tp = endp;
    }
  if (tp != endp)
    return (0);

  memcpy(dst, tmp, IN6ADDRSZ);
  return (1);
}

/* int
 * inet_pton(af, src, dst)
 *	convert from presentation format (which usually means ASCII printable)
 *	to network format (which is usually some kind of binary format).
 * return:
 *	1 if the address was valid for the specified address family
 *	0 if the address wasn't valid (`dst' is untouched in this case)
 *	-1 if some other error occurred (`dst' is untouched in this case, too)
 * author:
 *	Paul Vixie, 1996.
 */
extern "C" int
cygwin_inet_pton (int af, const char *src, void *dst)
{
  switch (af)
    {
    case AF_INET:
      return (inet_pton4(src, (u_int8_t *) dst));
    case AF_INET6:
      return (inet_pton6(src, (u_int8_t *) dst));
    default:
      errno = EAFNOSUPPORT;
      return (-1);
    }
  /* NOTREACHED */
}

/* const char *
 * inet_ntop4(src, dst, size)
 *	format an IPv4 address, more or less like inet_ntoa()
 * return:
 *	`dst' (as a const)
 * notes:
 *	(1) uses no statics
 *	(2) takes a u_int8_t* not an in_addr as input
 * author:
 *	Paul Vixie, 1996.
 */
static const char *
inet_ntop4 (const u_int8_t *src, char *dst, size_t size)
{
  static const char fmt[] = "%u.%u.%u.%u";
  char tmp[sizeof "255.255.255.255"];

  __small_sprintf(tmp, fmt, src[0], src[1], src[2], src[3]);
  if (strlen(tmp) > size)
    {
      errno = ENOSPC;
      return (NULL);
    }
  strcpy(dst, tmp);
  return (dst);
}

/* const char *
 * inet_ntop6(src, dst, size)
 *	convert IPv6 binary address into presentation (printable) format
 * author:
 *	Paul Vixie, 1996.
 */
static const char *
inet_ntop6 (const u_int8_t *src, char *dst, size_t size)
{
  /*
   * Note that int32_t and int16_t need only be "at least" large enough
   * to contain a value of the specified size.  On some systems, like
   * Crays, there is no such thing as an integer variable with 16 bits.
   * Keep this in mind if you think this function should have been coded
   * to use pointer overlays.  All the world's not a VAX.
   */
  char tmp[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"], *tp;
  struct { int base, len; } best, cur;
  u_int32_t words[IN6ADDRSZ / INT16SZ];
  int i;

  /*
   * Preprocess:
   *	Copy the input (bytewise) array into a wordwise array.
   *	Find the longest run of 0x00's in src[] for :: shorthanding.
   */
  memset(words, 0, sizeof words);
  for (i = 0; i < IN6ADDRSZ; i++)
    words[i / 2] |= (src[i] << ((1 - (i % 2)) << 3));
  best.base = -1;
  cur.base = -1;
  best.len = 0;
  cur.len = 0;
  for (i = 0; i < (IN6ADDRSZ / INT16SZ); i++)
    {
      if (words[i] == 0)
	{
	  if (cur.base == -1)
	    cur.base = i, cur.len = 1;
	  else
	    cur.len++;
	}
      else
	{
	  if (cur.base != -1)
	    {
	      if (best.base == -1 || cur.len > best.len)
		best = cur;
	      cur.base = -1;
	    }
	}
    }
  if (cur.base != -1)
    {
      if (best.base == -1 || cur.len > best.len)
	best = cur;
    }
  if (best.base != -1 && best.len < 2)
    best.base = -1;

  /*
   * Format the result.
   */
  tp = tmp;
  for (i = 0; i < (IN6ADDRSZ / INT16SZ); i++)
    {
      /* Are we inside the best run of 0x00's? */
      if (best.base != -1 && i >= best.base && i < (best.base + best.len))
	{
	  if (i == best.base)
	    *tp++ = ':';
	  continue;
	}
      /* Are we following an initial run of 0x00s or any real hex? */
      if (i != 0)
	*tp++ = ':';
      /* Is this address an encapsulated IPv4? */
      if (i == 6 && best.base == 0 &&
	  (best.len == 6 || (best.len == 5 && words[5] == 0xffff)))
	{
	  if (!inet_ntop4(src+12, tp, sizeof tmp - (tp - tmp)))
	    return (NULL);
	  tp += strlen(tp);
	  break;
	}
      __small_sprintf(tp, "%x", words[i]);
      while (*tp)
	{
	  if (isupper (*tp))
	    *tp = _tolower (*tp);
	  ++tp;
	}
    }
  /* Was it a trailing run of 0x00's? */
  if (best.base != -1 && (best.base + best.len) == (IN6ADDRSZ / INT16SZ))
    *tp++ = ':';
  *tp++ = '\0';

  /*
   * Check for overflow, copy, and we're done.
   */
  if ((size_t) (tp - tmp) > size)
    {
      errno = ENOSPC;
      return (NULL);
    }
  strcpy(dst, tmp);
  return (dst);
}

/* char *
 * inet_ntop(af, src, dst, size)
 *	convert a network format address to presentation format.
 * return:
 *	pointer to presentation format address (`dst'), or NULL (see errno).
 * author:
 *	Paul Vixie, 1996.
 */
extern "C" const char *
cygwin_inet_ntop (int af, const void *src, char *dst, socklen_t size)
{
  switch (af)
    {
    case AF_INET:
      return (inet_ntop4((const u_int8_t *) src, dst, size));
    case AF_INET6:
      return (inet_ntop6((const u_int8_t *) src, dst, size));
    default:
      errno = EAFNOSUPPORT;
      return (NULL);
    }
  /* NOTREACHED */
}

/* Exported as freeaddrinfo: POSIX.1-2001, POSIX.1-2008, RFC 2553 */
extern "C" void
cygwin_freeaddrinfo (struct addrinfo *addr)
{
  struct addrinfo *ai, *ainext;

  for (ai = addr; ai != NULL; ai = ainext)
    {
      if (ai->ai_addr != NULL)
	free (ai->ai_addr);	/* socket address structure */

      if (ai->ai_canonname != NULL)
	free (ai->ai_canonname);

      ainext = ai->ai_next;	/* can't fetch ai_next after free() */
      free (ai);		/* the addrinfo{} itself */
    }
}

static struct addrinfo *
ga_dup (struct addrinfoW *ai, bool v4mapped, int idn_flags, int &err)
{
  struct addrinfo *nai;

  if ((nai = (struct addrinfo *) calloc (1, sizeof (struct addrinfo))) == NULL)
    {
      err = EAI_MEMORY;
      return NULL;
    }

  nai->ai_family = v4mapped ? AF_INET6 : ai->ai_family;
  nai->ai_socktype = ai->ai_socktype;
  nai->ai_protocol = ai->ai_protocol;
  if (ai->ai_canonname)
    {
      tmp_pathbuf tp;
      wchar_t *canonname = ai->ai_canonname;

      if (idn_flags & AI_CANONIDN)
	{
	  /* Map flags to equivalent IDN_* flags. */
	  wchar_t *canonbuf = tp.w_get ();
	  if (IdnToUnicode (idn_flags >> 16, canonname, -1,
			    canonbuf, NT_MAX_PATH))
	    canonname = canonbuf;
	  else if (GetLastError () != ERROR_PROC_NOT_FOUND)
	    {
	      free (nai);
	      err = EAI_IDN_ENCODE;
	      return NULL;
	    }
	}
      size_t len = wcstombs (NULL, canonname, 0);
      if (len == (size_t) -1)
	{
	  free (nai);
	  err = EAI_IDN_ENCODE;
	  return NULL;
	}
      nai->ai_canonname = (char *) malloc (len + 1);
      if (!nai->ai_canonname)
	{
	  free (nai);
	  err = EAI_MEMORY;
	  return NULL;
	}
      wcstombs (nai->ai_canonname, canonname, len + 1);
    }

  nai->ai_addrlen = v4mapped ? sizeof (struct sockaddr_in6) : ai->ai_addrlen;
  if ((nai->ai_addr = (struct sockaddr *) malloc (v4mapped
						  ? sizeof (struct sockaddr_in6)
						  : ai->ai_addrlen)) == NULL)
    {
      if (nai->ai_canonname)
	free (nai->ai_canonname);
      free (nai);
      err = EAI_MEMORY;
      return NULL;
    }
  if (v4mapped)
    {
      struct sockaddr_in6 *in = (struct sockaddr_in6 *) nai->ai_addr;
      in->sin6_family = AF_INET6;
      in->sin6_port = ((struct sockaddr_in *) ai->ai_addr)->sin_port;
      in->sin6_flowinfo = 0;
      in->sin6_addr.s6_addr32[0] = 0;
      in->sin6_addr.s6_addr32[1] = 0;
      in->sin6_addr.s6_addr32[2] = htonl (0xffff);
      in->sin6_addr.s6_addr32[3] = ((struct sockaddr_in *) ai->ai_addr)->sin_addr.s_addr;
      in->sin6_scope_id = 0;
    }
  else
    memcpy (nai->ai_addr, ai->ai_addr, ai->ai_addrlen);

  return nai;
}

static struct addrinfo *
ga_duplist (struct addrinfoW *ai, bool v4mapped, int idn_flags, int &err)
{
  struct addrinfo *tmp, *nai = NULL, *nai0 = NULL;

  for (; ai; ai = ai->ai_next)
    {
      /* Workaround for a Windows weirdness.  If a service is supported as
	 TCP and UDP service, GetAddrInfo does not return two entries, one
	 for TCP, one for UDP, as on Linux.  Rather, it just returns a single
	 entry with ai_socktype and ai_protocol set to 0, kind of like a
	 placeholder.  If the service only exists as TCP or UDP service, then
	 ai->ai_socktype is set, but ai_protocol isn't.  Fix up the fields,
	 and in case ai_socktype and ai_protocol are 0 duplicate the entry
	 with valid values for ai_socktype and ai_protocol. */
      switch (ai->ai_socktype)
	{
	case SOCK_STREAM:
	  if (ai->ai_protocol == 0)
	    ai->ai_protocol = IPPROTO_TCP;
	  break;
	case SOCK_DGRAM:
	  if (ai->ai_protocol == 0)
	    ai->ai_protocol = IPPROTO_UDP;
	  break;
	case 0:
	  switch (ai->ai_protocol)
	    {
	    case IPPROTO_TCP:
	      ai->ai_socktype = SOCK_STREAM;
	      break;
	    case IPPROTO_UDP:
	      ai->ai_socktype = SOCK_DGRAM;
	      break;
	    case 0:
	      ai->ai_socktype = SOCK_STREAM;
	      ai->ai_protocol = IPPROTO_TCP;
	      if (!(tmp = ga_dup (ai, v4mapped, idn_flags, err)))
		goto bad;
	      if (!nai0)
		nai0 = tmp;
	      if (nai)
		nai->ai_next = tmp;
	      nai = tmp;
	      ai->ai_socktype = SOCK_DGRAM;
	      ai->ai_protocol = IPPROTO_UDP;
	      break;
	    default:
	      break;
	    }
	  break;
	default:
	  break;
	}
      if (!(tmp = ga_dup (ai, v4mapped, idn_flags, err)))
	goto bad;
      if (!nai0)
	nai0 = tmp;
      if (nai)
	nai->ai_next = tmp;
      nai = tmp;
    }
  return nai0;

bad:
  cygwin_freeaddrinfo (nai0);
  return NULL;
}

/* Cygwin specific wrappers around the gai functions. */
static struct gai_errmap_t
{
  int w32_errval;
  const char *errtxt;
} gai_errmap[] =
{
  {0,			  "Success"},
  /* EAI_ADDRFAMILY */
  {0,			  "Address family for hostname not supported"},
  /* EAI_AGAIN */
  {WSATRY_AGAIN,	  "Temporary failure in name resolution"},
  /* EAI_BADFLAGS */
  {WSAEINVAL,		  "Bad value for ai_flags"},
  /* EAI_FAIL */
  {WSANO_RECOVERY,	  "Non-recoverable failure in name resolution"},
  /* EAI_FAMILY */
  {WSAEAFNOSUPPORT,	  "ai_family not supported"},
  /* EAI_MEMORY */
  {WSA_NOT_ENOUGH_MEMORY, "Memory allocation failure"},
  /* EAI_NODATA */
  {WSANO_DATA,		  "No address associated with hostname"},
  /* EAI_NONAME */
  {WSAHOST_NOT_FOUND,	  "Name or service not known"},
  /* EAI_SERVICE */
  {WSATYPE_NOT_FOUND,	  "Servname not supported for ai_socktype"},
  /* EAI_SOCKTYPE */
  {WSAESOCKTNOSUPPORT,	  "ai_socktype not supported"},
  /* EAI_SYSTEM */
  {0,			  "System error"},
  /* EAI_BADHINTS */
  {0,                     "Invalid value for hints"},
  /* EAI_PROTOCOL */
  {0,			  "Resolved protocol is unknown"},
  /* EAI_OVERFLOW */
  {WSAEFAULT,		  "An argument buffer overflowed"},
  /* EAI_IDN_ENCODE */
  {0,			  "Parameter string not correctly encoded"}
};

/* Exported as gai_strerror: POSIX.1-2001, POSIX.1-2008 */
extern "C" const char *
cygwin_gai_strerror (int err)
{
  if (err >= 0 && err < (int) (sizeof gai_errmap / sizeof *gai_errmap))
    return gai_errmap[err].errtxt;
  return "Unknown error";
}

static int
w32_to_gai_err (int w32_err)
{
  if (w32_err >= WSABASEERR)
    for (unsigned i = 0; i < sizeof gai_errmap / sizeof *gai_errmap; ++i)
      if (gai_errmap[i].w32_errval == w32_err)
	return i;
  return w32_err;
}

/* Exported as getaddrinfo: POSIX.1-2001, POSIX.1-2008, RFC 2553 */
extern "C" int
cygwin_getaddrinfo (const char *hostname, const char *servname,
		    const struct addrinfo *hints, struct addrinfo **res)
{
  int ret = 0;

  /* Windows' getaddrinfo implementations lets all possible values
     in ai_flags slip through and just ignores unknown values.  So we
     check manually here. */
#define AI_IDN_MASK (AI_IDN | \
		     AI_CANONIDN | \
		     AI_IDN_ALLOW_UNASSIGNED | \
		     AI_IDN_USE_STD3_ASCII_RULES)
#ifndef AI_DISABLE_IDN_ENCODING
#define AI_DISABLE_IDN_ENCODING 0x80000
#endif
  __try
    {
      if (hints && (hints->ai_flags
		    & ~(AI_PASSIVE | AI_CANONNAME | AI_NUMERICHOST | AI_ALL
			| AI_NUMERICSERV | AI_ADDRCONFIG | AI_V4MAPPED
			| AI_IDN_MASK)))
	return EAI_BADFLAGS;
      /* AI_NUMERICSERV is not supported "by Microsoft providers".  We just
	 check the servname parameter by ourselves here. */
      if (hints && (hints->ai_flags & AI_NUMERICSERV))
	{
	  char *p;
	  if (servname && *servname && (strtoul (servname, &p, 10), *p))
	    return EAI_NONAME;
	}

      int idn_flags = hints ? (hints->ai_flags & AI_IDN_MASK) : 0;
      const char *src;
      mbstate_t ps;
      tmp_pathbuf tp;
      wchar_t *whost = NULL, *wserv = NULL;
      struct addrinfoW whints, *wres;

      if (hostname)
	{
	  memset (&ps, 0, sizeof ps);
	  src = hostname;
	  whost = tp.w_get ();
	  if (mbsrtowcs (whost, &src, NT_MAX_PATH, &ps) == (size_t) -1)
	    return EAI_IDN_ENCODE;
	  if (src)
	    return EAI_MEMORY;
	  if (idn_flags & AI_IDN)
	    {
	      /* Map flags to equivalent IDN_* flags. */
	      wchar_t *ascbuf = tp.w_get ();
	      if (IdnToAscii (idn_flags >> 16, whost, -1, ascbuf, NT_MAX_PATH))
		whost = ascbuf;
	      else if (GetLastError () != ERROR_PROC_NOT_FOUND)
		return EAI_IDN_ENCODE;
	    }
	}
      if (servname)
	{
	  memset (&ps, 0, sizeof ps);
	  src = servname;
	  wserv = tp.w_get ();
	  if (mbsrtowcs (wserv, &src, NT_MAX_PATH, &ps) == (size_t) -1)
	    return EAI_IDN_ENCODE;
	  if (src)
	    return EAI_MEMORY;
	}

      if (!hints)
	{
	  /* Default settings per glibc man page. */
	  memset (&whints, 0, sizeof whints);
	  whints.ai_family = PF_UNSPEC;
	  whints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG;
	}
      else
	{
	  /* sizeof addrinfo == sizeof addrinfoW */
	  memcpy (&whints, hints, sizeof whints);
	  whints.ai_flags &= ~AI_IDN_MASK;
#ifdef __x86_64__
	  /* ai_addrlen is socklen_t (4 bytes) in POSIX but size_t (8 bytes) in
	     Winsock.  Sert upper 4 bytes explicitely to 0 to avoid EAI_FAIL. */
	  whints.ai_addrlen &= UINT32_MAX;
#endif
	  /* On Windows, the default behaviour is as if AI_ADDRCONFIG is set,
	     apparently for performance reasons.  To get the POSIX default
	     behaviour, the AI_ALL flag has to be set. */
	  if (whints.ai_family == PF_UNSPEC
	      && !(whints.ai_flags & AI_ADDRCONFIG))
	    whints.ai_flags |= AI_ALL;
	}
      /* Disable automatic IDN conversion on W8 and later. */
      whints.ai_flags |= AI_DISABLE_IDN_ENCODING;
      ret = GetAddrInfoW (whost, wserv, &whints, &wres);
      /* Try to workaround an apparent shortcoming in Winsock's getaddrinfo
	 implementation.  See this link for details:
	 https://communities.vmware.com/message/2577858#2577858 */
      if (ret == WSANO_RECOVERY && (whints.ai_flags & AI_ALL))
	{
	  whints.ai_flags &= ~AI_ALL;
	  ret = GetAddrInfoW (whost, wserv, &whints, &wres);
	}
      ret = w32_to_gai_err (ret);
      /* Always copy over to self-allocated memory. */
      if (!ret)
	{
	  *res = ga_duplist (wres, false, idn_flags, ret);
	  FreeAddrInfoW (wres);
	  if (!*res)
	    __leave;
	}
    }
  __except (EFAULT)
    {
      ret = EAI_SYSTEM;
    }
  __endtry
  return ret;
}

/* Exported as getnameinfo: POSIX.1-2001, POSIX.1-2008, RFC 2553 */
extern "C" int
cygwin_getnameinfo (const struct sockaddr *sa, socklen_t salen,
		    char *host, size_t hostlen, char *serv,
		    size_t servlen, int flags)
{
  int ret = 0;

  __try
    {
      /* We call GetNameInfoW with local buffers and convert to locale
	 charset to allow RFC 3490 IDNs like glibc 2.3.4 and later. */
#define NI_IDN_MASK (NI_IDN | \
		     NI_IDN_ALLOW_UNASSIGNED | \
		     NI_IDN_USE_STD3_ASCII_RULES)
      int idn_flags = flags & NI_IDN_MASK;
      flags &= ~NI_IDN_MASK;
      tmp_pathbuf tp;
      wchar_t *whost = NULL, *wserv = NULL;
      DWORD whlen = 0, wslen = 0;

      if (host && hostlen)
	{
	  whost = tp.w_get ();
	  whlen = NT_MAX_PATH;
	}
      if (serv && servlen)
	{
	  wserv = tp.w_get ();
	  wslen = NT_MAX_PATH;
	}

      ret = w32_to_gai_err (GetNameInfoW (sa, salen, whost, whlen,
					  wserv, wslen, flags));
      if (!ret)
	{
	  const wchar_t *src;

	  if (whost)
	    {
	      if (idn_flags & NI_IDN)
		{
		  /* Map flags to equivalent IDN_* flags. */
		  wchar_t *idnbuf = tp.w_get ();
		  if (IdnToUnicode (idn_flags >> 16, whost, -1,
				    idnbuf, NT_MAX_PATH))
		    whost = idnbuf;
		  else if (GetLastError () != ERROR_PROC_NOT_FOUND)
		    return EAI_IDN_ENCODE;
		}
	      src = whost;
	      if (wcsrtombs (host, &src, hostlen, NULL) == (size_t) -1)
		return EAI_IDN_ENCODE;
	      if (src)
		return EAI_OVERFLOW;
	    }
	  if (wserv)
	    {
	      src = wserv;
	      if (wcsrtombs (serv, &src, servlen, NULL) == (size_t) -1)
		return EAI_IDN_ENCODE;
	      if (src)
		return EAI_OVERFLOW;
	    }
	}
      else if (ret == EAI_SYSTEM)
	set_winsock_errno ();
    }
  __except (EFAULT)
    {
      ret = EAI_SYSTEM;
    }
  __endtry
  return ret;
}

/* These functions are stick to the end of this file so that the
   optimization in asm/byteorder.h can be used even here in net.cc. */

#undef htonl
#undef ntohl
#undef htons
#undef ntohs

/* htonl: POSIX.1-2001, POSIX.1-2008 */
extern "C" uint32_t
htonl (uint32_t x)
{
  return __htonl (x);
}

/* ntohl: POSIX.1-2001, POSIX.1-2008 */
extern "C" uint32_t
ntohl (uint32_t x)
{
  return __ntohl (x);
}

/* htons: POSIX.1-2001, POSIX.1-2008 */
extern "C" uint16_t
htons (uint16_t x)
{
  return __htons (x);
}

/* ntohs: POSIX.1-2001, POSIX.1-2008 */
extern "C" uint16_t
ntohs (uint16_t x)
{
  return __ntohs (x);
}

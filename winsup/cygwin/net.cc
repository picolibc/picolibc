/* net.cc: network-related routines.

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005, 2006 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* #define DEBUG_NEST_ON 1 */

#define  __INSIDE_CYGWIN_NET__

#include "winsup.h"
#include <ctype.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <syslog.h>

#include <stdlib.h>
#define gethostname cygwin_gethostname
#include <unistd.h>
#undef gethostname
#include <netdb.h>
#define USE_SYS_TYPES_FD_SET
#include <winsock2.h>
#include <iphlpapi.h>
#include <assert.h>
#include "cygerrno.h"
#include "security.h"
#include "cygwin/version.h"
#include "perprocess.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "sigproc.h"
#include "pinfo.h"
#include "registry.h"
#include "cygtls.h"

extern "C"
{
  int h_errno;

  int __stdcall rcmd (char **ahost, unsigned short inport, char *locuser,
		      char *remuser, char *cmd, SOCKET * fd2p);
  int __stdcall rexec (char **ahost, unsigned short inport, char *locuser,
		       char *password, char *cmd, SOCKET * fd2p);
  int sscanf (const char *, const char *, ...);
}				/* End of "C" section */

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

/* htonl: standards? */
extern "C" unsigned long int
htonl (unsigned long int x)
{
  return ((((x & 0x000000ffU) << 24) |
	   ((x & 0x0000ff00U) << 8) |
	   ((x & 0x00ff0000U) >> 8) |
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

/* exported as inet_ntoa: BSD 4.3 */
extern "C" char *
cygwin_inet_ntoa (struct in_addr in)
{
  char *res = inet_ntoa (in);

  if (_my_tls.locals.ntoa_buf)
    {
      free (_my_tls.locals.ntoa_buf);
      _my_tls.locals.ntoa_buf = NULL;
    }
  if (res)
    _my_tls.locals.ntoa_buf = strdup (res);
  return _my_tls.locals.ntoa_buf;
}

/* exported as inet_addr: BSD 4.3 */
extern "C" unsigned long
cygwin_inet_addr (const char *cp)
{
  myfault efault;
  if (efault.faulted (EFAULT))
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
  myfault efault;
  if (efault.faulted (EFAULT))
    return 0;

  unsigned long res = inet_addr (cp);

  if (res == INADDR_NONE && strcmp (cp, "255.255.255.255"))
    return 0;
  if (inp)
    inp->s_addr = res;
  return 1;
}

/* undocumented in wsock32.dll */
extern "C" unsigned int WINAPI inet_network (const char *);

extern "C" unsigned int
cygwin_inet_network (const char *cp)
{
  myfault efault;
  if (efault.faulted (EFAULT))
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

static NO_COPY struct tl errmap[] = {
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
static NO_COPY struct tl host_errmap[] = {
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

  int why = WSAGetLastError ();

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

struct unionent
{
  char *name;
  char **list;
  short port_proto_addrtype;
  short h_len;
  union
  {
    char *s_proto;
    char **h_addr_list;
  };
};

enum struct_type
{
  t_hostent, t_protoent, t_servent
};

static const char *entnames[] = {"host", "proto", "serv"};

/* Generic "dup a {host,proto,serv}ent structure" function.
   This is complicated because we need to be able to free the
   structure at any point and we can't rely on the pointer contents
   being untouched by callers.  So, we allocate a chunk of memory
   large enough to hold the structure and all of the stuff it points
   to then we copy the source into this new block of memory.
   The 'unionent' struct is a union of all of the currently used
   *ent structure.  */

#define dup_ent(old, src, type) __dup_ent ((unionent *&) (_my_tls.locals.old), (unionent *) (src), type)
#ifdef DEBUGGING
static void *
#else
static inline void *
#endif
__dup_ent (unionent *&dst, unionent *src, struct_type type)
{
  if (dst)
    debug_printf ("old %sent structure \"%s\" %p\n", entnames[type],
		  ((unionent *) dst)->name, dst);

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
    case t_protoent:
      struct_sz = sizeof (protoent);
      break;
    case t_servent:
      struct_sz = sizeof (servent);
      break;
    case t_hostent:
      struct_sz = sizeof (hostent);
      break;
    default:
      api_fatal ("called with invalid value %d", type);
      break;
    }

  /* Every *ent begins with a name.  Calculate it's length. */
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
  char *s_proto = NULL;
  if (type == t_servent)
    {
      if (src->s_proto)
	{
	  /* Windows 95 idiocy.  Structure is misaligned on Windows 95.
	     Kludge around this by trying a different pointer alignment.  */
	  if (!IsBadStringPtr (src->s_proto, INT32_MAX))
	    s_proto = src->s_proto;
	  else if (!IsBadStringPtr (((pservent *) src)->s_proto, INT32_MAX))
	    s_proto = ((pservent *) src)->s_proto;
	  sz += (protolen = strlen_round (s_proto));
	}
    }
  else if (type == t_hostent)
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

  /* Allocate the storage needed.  Allocate a rounded size to attempt to force
     reuse of this buffer so that a poorly-written caller will not be using
     a freed buffer. */
  unsigned rsz = 256 * ((sz + 255) / 256);
  dst = (unionent *) realloc (dst, rsz);

  /* Hopefully, this worked. */
  if (dst)
    {
      memset (dst, 0, sz);
      /* This field is common to all *ent structures but named differently
	 in each, of course.  */
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
      if (type == t_protoent)
	debug_printf ("protoent %s %x %x", dst->name, dst->list, dst->port_proto_addrtype);
      else if (type == t_servent)
	{
	  if (s_proto)
	    {
	      strcpy (dst->s_proto = dp, s_proto);
	      dp += protolen;
	    }
	}
      else if (type == t_hostent)
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
      /* Sanity check that we did our bookkeeping correctly. */
      assert ((dp - (char *) dst) == sz);
    }
  debug_printf ("duped %sent \"%s\", %p", entnames[type], dst ? dst->name : "<null!>", dst);
  return dst;
}

/* exported as getprotobyname: standards? */
extern "C" struct protoent *
cygwin_getprotobyname (const char *p)
{
  myfault efault;
  if (efault.faulted (EFAULT))
    return NULL;
  return (protoent *) dup_ent (protoent_buf, getprotobyname (p), t_protoent);
}

/* exported as getprotobynumber: standards? */
extern "C" struct protoent *
cygwin_getprotobynumber (int number)
{
  return (protoent *) dup_ent (protoent_buf, getprotobynumber (number), t_protoent);
}

bool
fdsock (cygheap_fdmanip& fd, const device *dev, SOCKET soc)
{
  if (wincap.has_set_handle_information ())
    {
      /* NT systems apparently set sockets to inheritable by default */
      SetHandleInformation ((HANDLE) soc, HANDLE_FLAG_INHERIT, 0);
      debug_printf ("reset socket inheritance");
    }
  else
    debug_printf ("not setting socket inheritance");
  fd = build_fh_dev (*dev);
  if (!fd.isopen ())
    return false;
  fd->set_io_handle ((HANDLE) soc);
  fd->set_flags (O_RDWR | O_BINARY);
  fd->uninterruptible_io (true);
  cygheap->fdtab.inc_need_fixup_before ();
  debug_printf ("fd %d, name '%s', soc %p", (int) fd, dev->name, soc);

  /* Same default buffer sizes as on Linux (instead of WinSock default 8K). */
  int rmem = dev == tcp_dev ? 87380 : 120832;
  int wmem = dev == tcp_dev ? 16384 : 120832;
  if (::setsockopt (soc, SOL_SOCKET, SO_RCVBUF, (char *) &rmem, sizeof (int)))
    debug_printf ("setsockopt(SO_RCVBUF) failed, %lu", WSAGetLastError ());
  if (::setsockopt (soc, SOL_SOCKET, SO_SNDBUF, (char *) &wmem, sizeof (int)))
    debug_printf ("setsockopt(SO_SNDBUF) failed, %lu", WSAGetLastError ());

  return true;
}

/* exported as socket: standards? */
extern "C" int
cygwin_socket (int af, int type, int protocol)
{
  int res = -1;
  SOCKET soc = 0;

  debug_printf ("socket (%d, %d, %d)", af, type, protocol);

  soc = socket (AF_INET, type, af == AF_LOCAL ? 0 : protocol);

  if (soc == INVALID_SOCKET)
    {
      set_winsock_errno ();
      goto done;
    }

  const device *dev;

  if (af == AF_INET)
    dev = type == SOCK_STREAM ? tcp_dev : udp_dev;
  else
    dev = type == SOCK_STREAM ? stream_dev : dgram_dev;

  {
    cygheap_fdnew fd;
    if (fd < 0 || !fdsock (fd, dev, soc))
      closesocket (soc);
    else
      {
	((fhandler_socket *) fd)->set_addr_family (af);
	((fhandler_socket *) fd)->set_socket_type (type);
	res = fd;
      }
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
  sig_dispatch_pending ();

  fhandler_socket *fh = get (fd);

  myfault efault;
  if (efault.faulted (EFAULT) || !fh)
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
  sig_dispatch_pending ();

  fhandler_socket *fh = get (fd);

  myfault efault;
  if (efault.faulted (EFAULT) || !fh)
    res = -1;
  else if ((res = len) != 0)
    res = fh->recvfrom (buf, len, flags, from, fromlen);

  syscall_printf ("%d = recvfrom (%d, %p, %d, %x, %p, %p)",
		  res, fd, buf, len, flags, from, fromlen);

  return res;
}

static int
convert_ws1_ip_optname (int optname)
{
  static int ws2_optname[] =
  {
    0,
    IP_OPTIONS,
    IP_MULTICAST_IF,
    IP_MULTICAST_TTL,
    IP_MULTICAST_LOOP,
    IP_ADD_MEMBERSHIP,
    IP_DROP_MEMBERSHIP,
    IP_TTL,
    IP_TOS,
    IP_DONTFRAGMENT
  };
  return (optname < 1 || optname > _WS1_IP_DONTFRAGMENT)
  	 ? optname
	 : ws2_optname[optname];
}

/* exported as setsockopt: standards? */
extern "C" int
cygwin_setsockopt (int fd, int level, int optname, const void *optval,
		   int optlen)
{
  int res;
  fhandler_socket *fh = get (fd);

  myfault efault;
  if (efault.faulted (EFAULT) || !fh)
    res = -1;
  else
    {
      /* Old applications still use the old Winsock1 IPPROTO_IP values. */
      if (level == IPPROTO_IP && CYGWIN_VERSION_CHECK_FOR_USING_WINSOCK1_VALUES)
	optname = convert_ws1_ip_optname (optname);

      res = setsockopt (fh->get_socket (), level, optname,
			(const char *) optval, optlen);

      if (optlen == 4)
	syscall_printf ("setsockopt optval=%x", *(long *) optval);

      if (res)
        {
	  /* KB 248611:
	  
	     Windows 2000 and above don't support setting the IP_TOS field
	     with setsockopt.  Additionally, TOS was always (also under 9x
	     and NT) only implemented for UDP and ICMP, never for TCP.

	     The difference is that beginning with Windows 2000 the
	     setsockopt call returns WinSock error 10022, WSAEINVAL when
	     trying to set the IP_TOS field, instead of just ignoring the
	     call.  This is *not* explained in KB 248611, but only in KB
	     258978.

	     Either case, the official workaround is to add a new registry
	     DWORD value HKLM/System/CurrentControlSet/Services/Tcpip/...
	     ...  Parameters/DisableUserTOSSetting, set to 0, and reboot.

	     Sidenote: The reasoning for dropping ToS in Win2K is that ToS
	     per RFC 1349 is incompatible with DiffServ per RFC 2474/2475.

	     We just ignore the return value of setting IP_TOS under Windows
	     2000 and above entirely. */
	  if (level == IPPROTO_IP && optname == IP_TOS
	      && WSAGetLastError () == WSAEINVAL
	      && wincap.has_disabled_user_tos_setting ())
	    {
	      debug_printf ("Faked IP_TOS success");
	      res = 0;
	    }
	  else
	    set_winsock_errno ();
	}
      else if (level == SOL_SOCKET && optname == SO_REUSEADDR)
        fh->saw_reuseaddr (*(int *) optval);
    }

  syscall_printf ("%d = setsockopt (%d, %d, %x, %p, %d)",
		  res, fd, level, optname, optval, optlen);
  return res;
}

/* exported as getsockopt: standards? */
extern "C" int
cygwin_getsockopt (int fd, int level, int optname, void *optval, int *optlen)
{
  int res;
  fhandler_socket *fh = get (fd);

  myfault efault;
  if (efault.faulted (EFAULT) || !fh)
    res = -1;
  else if (optname == SO_PEERCRED)
    {
      struct ucred *cred = (struct ucred *) optval;
      res = fh->getpeereid (&cred->pid, &cred->uid, &cred->gid);
    }
  else
    {
      /* Old applications still use the old Winsock1 IPPROTO_IP values. */
      if (level == IPPROTO_IP && CYGWIN_VERSION_CHECK_FOR_USING_WINSOCK1_VALUES)
	optname = convert_ws1_ip_optname (optname);
      res = getsockopt (fh->get_socket (), level, optname, (char *) optval,
			(int *) optlen);

      if (optname == SO_ERROR)
	{
	  int *e = (int *) optval;

	  debug_printf ("WinSock SO_ERROR = %d", *e);
	  *e = find_winsock_errno (*e);
	}

      if (res)
	set_winsock_errno ();
    }

  syscall_printf ("%d = getsockopt (%d, %d, 0x%x, %p, %p)",
		  res, fd, level, optname, optval, optlen);
  return res;
}

extern "C" int
getpeereid (int fd, __uid32_t *euid, __gid32_t *egid)
{
  sig_dispatch_pending ();
  fhandler_socket *fh = get (fd);
  if (fh)
    return fh->getpeereid (NULL, euid, egid);
  return -1;
}

/* exported as connect: standards? */
extern "C" int
cygwin_connect (int fd, const struct sockaddr *name, int namelen)
{
  int res;
  sig_dispatch_pending ();

  fhandler_socket *fh = get (fd);

  myfault efault;
  if (efault.faulted (EFAULT) || !fh)
    res = -1;
  else
    res = fh->connect (name, namelen);

  syscall_printf ("%d = connect (%d, %p, %d)", res, fd, name, namelen);

  return res;
}

/* exported as getservbyname: standards? */
extern "C" struct servent *
cygwin_getservbyname (const char *name, const char *proto)
{
  sig_dispatch_pending ();
  myfault efault;
  if (efault.faulted (EFAULT))
    return NULL;

  servent *res = (servent *) dup_ent (servent_buf, getservbyname (name, proto), t_servent);
  syscall_printf ("%p = getservbyname (%s, %s)", res, name, proto);
  return res;
}

/* exported as getservbyport: standards? */
extern "C" struct servent *
cygwin_getservbyport (int port, const char *proto)
{
  sig_dispatch_pending ();
  myfault efault;
  if (efault.faulted (EFAULT))
    return NULL;

  servent *res = (servent *) dup_ent (servent_buf, getservbyport (port, proto), t_servent);
  syscall_printf ("%p = getservbyport (%d, %s)", _my_tls.locals.servent_buf, port, proto);
  return res;
}

extern "C" int
cygwin_gethostname (char *name, size_t len)
{
  sig_dispatch_pending ();
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;

  if (gethostname (name, len))
    {
      DWORD local_len = len;

      if (!GetComputerNameA (name, &local_len))
	{
	  set_winsock_errno ();
	  return -1;
	}
    }
  debug_printf ("name %s", name);
  return 0;
}

/* exported as gethostbyname: standards? */
extern "C" struct hostent *
cygwin_gethostbyname (const char *name)
{
  sig_dispatch_pending ();
  myfault efault;
  if (efault.faulted (EFAULT))
      return NULL;

  unsigned char tmp_addr[4];
  struct hostent tmp, *h;
  char *tmp_aliases[1] = {0};
  char *tmp_addr_list[2] = {0,0};
  unsigned int a, b, c, d;
  char dummy;

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

  hostent *res = (hostent *) dup_ent (hostent_buf, h, t_hostent);
  if (res)
    debug_printf ("h_name %s", res->h_name);
  else
    {
      debug_printf ("dup_ent returned NULL for name %s, h %p", name, h);
      set_host_errno ();
    }
  return res;
}

/* exported as gethostbyaddr: standards? */
extern "C" struct hostent *
cygwin_gethostbyaddr (const char *addr, int len, int type)
{
  sig_dispatch_pending ();
  myfault efault;
  if (efault.faulted (EFAULT))
    return NULL;

  hostent *res = (hostent *) dup_ent (hostent_buf, gethostbyaddr (addr, len, type), t_hostent);
  if (res)
    debug_printf ("h_name %s", _my_tls.locals.hostent_buf->h_name);
  else
    set_host_errno ();
  return res;
}

/* exported as accept: standards? */
extern "C" int
cygwin_accept (int fd, struct sockaddr *peer, int *len)
{
  int res;
  sig_dispatch_pending ();

  fhandler_socket *fh = get (fd);

  myfault efault;
  if (efault.faulted (EFAULT) || !fh)
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
  sig_dispatch_pending ();
  fhandler_socket *fh = get (fd);

  myfault efault;
  if (efault.faulted (EFAULT) || !fh)
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
  sig_dispatch_pending ();

  fhandler_socket *fh = get (fd);

  myfault efault;
  if (efault.faulted (EFAULT) || !fh)
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
  sig_dispatch_pending ();
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
  sig_dispatch_pending ();

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
  myfault efault;
  if (efault.faulted ())
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
  sig_dispatch_pending ();

  fhandler_socket *fh = get (fd);

  myfault efault;
  if (efault.faulted (EFAULT) || !fh)
    res = -1;
  else
    res = fh->getpeername (name, len);

  syscall_printf ("%d = getpeername (%d) %d", res, fd, (fh ? fh->get_socket () : -1));
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
  sig_dispatch_pending ();
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;

  PFIXED_INFO info = NULL;
  ULONG size = 0;

  if (GetNetworkParams(info, &size) == ERROR_BUFFER_OVERFLOW
      && (info = (PFIXED_INFO) alloca(size))
      && GetNetworkParams(info, &size) == ERROR_SUCCESS)
    {
      strncpy(domain, info->DomainName, len);
      return 0;
    }

  /* This is only used by Win95 and NT <=  4.0.
     The registry names are language independent.
     FIXME: Handle DHCP on Win95. The DhcpDomain(s) may be available
     in ..VxD\DHCP\DhcpInfoXX\OptionInfo, RFC 1533 format */

  reg_key r (HKEY_LOCAL_MACHINE, KEY_READ,
	     (!wincap.is_winnt ()) ? "System" : "SYSTEM",
	     "CurrentControlSet", "Services",
	     (!wincap.is_winnt ()) ? "VxD" : "Tcpip",
	     (!wincap.is_winnt ()) ? "MSTCP" : "Parameters", NULL);

  if (!r.error ())
    {
      int res1, res2 = 0; /* Suppress compiler warning */
      res1 = r.get_string ("Domain", domain, len, "");
      if (res1 != ERROR_SUCCESS || !domain[0])
	res2 = r.get_string ("DhcpDomain", domain, len, "");
      if (res1 == ERROR_SUCCESS || res2 == ERROR_SUCCESS)
	return 0;
    }
  __seterrno ();
  return -1;
}

/* Fill out an ifconf struct. */

/*
 * IFCONF 98/ME, NTSP4, W2K:
 * Use IP Helper Library
 */
static void
get_2k_ifconf (struct ifconf *ifc, int what)
{
  int cnt = 0;
  int ethId = 0, pppId = 0, slpId = 0, tokId = 0;

  /* Union maps buffer to correct struct */
  struct ifreq *ifr = ifc->ifc_req;

  DWORD ip_cnt, lip, lnp;
  DWORD siz_ip_table = 0;
  PMIB_IPADDRTABLE ipt;
  PMIB_IFROW ifrow;
  struct sockaddr_in *sa = NULL;
  struct sockaddr *so = NULL;

  typedef struct ifcount_t
  {
    DWORD ifIndex;
    size_t count;
    unsigned int enumerated;	// for eth0:1
    unsigned int classId;	// for eth0, tok0 ...

  };
  ifcount_t *iflist, *ifEntry;

  if (GetIpAddrTable (NULL, &siz_ip_table, TRUE) == ERROR_INSUFFICIENT_BUFFER
      && (ifrow = (PMIB_IFROW) alloca (sizeof (MIB_IFROW)))
      && (ipt = (PMIB_IPADDRTABLE) alloca (siz_ip_table))
      && !GetIpAddrTable (ipt, &siz_ip_table, TRUE))
    {
      iflist =
	(ifcount_t *) alloca (sizeof (ifcount_t) * (ipt->dwNumEntries + 1));
      memset (iflist, 0, sizeof (ifcount_t) * (ipt->dwNumEntries + 1));
      for (ip_cnt = 0; ip_cnt < ipt->dwNumEntries; ++ip_cnt)
	{
	  ifEntry = iflist;
	  /* search for matching entry (and stop at first free entry) */
	  while (ifEntry->count != 0)
	    {
	      if (ifEntry->ifIndex == ipt->table[ip_cnt].dwIndex)
		break;
	      ifEntry++;
	    }
	  if (ifEntry->count == 0)
	    {
	      ifEntry->count = 1;
	      ifEntry->ifIndex = ipt->table[ip_cnt].dwIndex;
	    }
	  else
	    {
	      ifEntry->count++;
	    }
	}
      // reset the last element. This is just the stopper for the loop.
      iflist[ipt->dwNumEntries].count = 0;

      /* Iterate over all configured IP-addresses */
      for (ip_cnt = 0; ip_cnt < ipt->dwNumEntries; ++ip_cnt)
	{
	  memset (ifrow, 0, sizeof (MIB_IFROW));
	  ifrow->dwIndex = ipt->table[ip_cnt].dwIndex;
	  if (GetIfEntry (ifrow) != NO_ERROR)
	    continue;

	  ifcount_t *ifEntry = iflist;

	  /* search for matching entry (and stop at first free entry) */
	  while (ifEntry->count != 0)
	    {
	      if (ifEntry->ifIndex == ipt->table[ip_cnt].dwIndex)
		break;
	      ifEntry++;
	    }

	  /* Setup the interface name */
	  switch (ifrow->dwType)
	    {
	      case MIB_IF_TYPE_TOKENRING:
		if (ifEntry->enumerated == 0)
		  {
		    ifEntry->classId = tokId++;
		    __small_sprintf (ifr->ifr_name, "tok%u",
				     ifEntry->classId);
		  }
		else
		  {
		    __small_sprintf (ifr->ifr_name, "tok%u:%u",
				     ifEntry->classId,
				     ifEntry->enumerated - 1);
		  }
		ifEntry->enumerated++;
		break;
	      case MIB_IF_TYPE_ETHERNET:
		if (ifEntry->enumerated == 0)
		  {
		    ifEntry->classId = ethId++;
		    __small_sprintf (ifr->ifr_name, "eth%u",
				     ifEntry->classId);
		  }
		else
		  {
		    __small_sprintf (ifr->ifr_name, "eth%u:%u",
				     ifEntry->classId,
				     ifEntry->enumerated - 1);
		  }
		ifEntry->enumerated++;
		break;
	      case MIB_IF_TYPE_PPP:
		if (ifEntry->enumerated == 0)
		  {
		    ifEntry->classId = pppId++;
		    __small_sprintf (ifr->ifr_name, "ppp%u",
				     ifEntry->classId);
		  }
		else
		  {
		    __small_sprintf (ifr->ifr_name, "ppp%u:%u",
				     ifEntry->classId,
				     ifEntry->enumerated - 1);
		  }
		ifEntry->enumerated++;
		break;
	      case MIB_IF_TYPE_SLIP:
		if (ifEntry->enumerated == 0)
		  {
		    ifEntry->classId = slpId++;
		    __small_sprintf (ifr->ifr_name, "slp%u",
				     ifEntry->classId);
		  }
		else
		  {
		    __small_sprintf (ifr->ifr_name, "slp%u:%u",
				     ifEntry->classId,
				     ifEntry->enumerated - 1);
		  }
		ifEntry->enumerated++;
		break;
	      case MIB_IF_TYPE_LOOPBACK:
		strcpy (ifr->ifr_name, "lo");
		break;
	      default:
		continue;
	    }
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
		  if (i >= ifrow->dwPhysAddrLen)
		    so->sa_data[i] = '\0';
		  else
		    so->sa_data[i] = ifrow->bPhysAddr[i];
		so->sa_family = AF_INET;
		break;
	      case SIOCGIFMETRIC:
		ifr->ifr_metric = 1;
		break;
	      case SIOCGIFMTU:
		ifr->ifr_mtu = ifrow->dwMtu;
		break;
	    }
	  ++cnt;
	  if ((caddr_t)++ ifr >
	      ifc->ifc_buf + ifc->ifc_len - sizeof (struct ifreq))
	    goto done;
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
		    "Tcpip\\" "Linkage",
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
		  if ((caddr_t)++ ifr > ifc->ifc_buf
		      + ifc->ifc_len - sizeof (struct ifreq))
		    break;

		  if (!strncmp (bp, "NdisWan", 7))
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
			    sa->sin_addr.s_addr =
			      cygwin_inet_addr (dhcpaddress);
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
	  || RegOpenKeyEx (key, ifname, 0, KEY_READ, &ifkey) != ERROR_SUCCESS)
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
	  if ((caddr_t)++ ifr > ifc->ifc_buf
	      + ifc->ifc_len - sizeof (struct ifreq))
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

  sig_dispatch_pending ();
  myfault efault;
  if (efault.faulted (EFAULT))
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

  sig_dispatch_pending ();

  myfault efault;
  if (efault.faulted (EFAULT))
    return (int) INVALID_SOCKET;
  if (!*locuser)
    {
      set_errno (EINVAL);
      return (int) INVALID_SOCKET;
    }

  res = rcmd (ahost, inport, locuser, remuser, cmd, fd2p ? &fd2s : NULL);
  if (res != (int) INVALID_SOCKET)
    {
      cygheap_fdnew res_fd;

      if (res_fd >= 0 && fdsock (res_fd, tcp_dev, res))
	{
	  ((fhandler_socket *) res_fd)->connect_state (connected);
	  res = res_fd;
	}
      else
	{
	  closesocket (res);
	  res = -1;
	}

      if (res >= 0 && fd2p)
	{
	  cygheap_fdnew newfd (res_fd, false);
	  cygheap_fdget fd (*fd2p);

	  if (newfd >= 0 && fdsock (newfd, tcp_dev, fd2s))
	    {
	      *fd2p = newfd;
	      ((fhandler_socket *) fd2p)->connect_state (connected);
	    }
	  else
	    {
	      closesocket (res);
	      closesocket (fd2s);
	      res = -1;
	    }
	}
    }

  syscall_printf ("%d = rcmd (...)", res);
  return res;
}

/* The below implementation of rresvport looks pretty ugly, but there's
   a problem in Winsock.  The bind(2) call does not fail if a local
   address is still in TIME_WAIT state, and there's no way to get this
   behaviour.  Unfortunately the first time when this is detected is when
   the calling application tries to connect. 
   
   One (also not really foolproof) way around this problem would be to use
   the iphlpapi function GetTcpTable and to check if the port in question is
   in TIME_WAIT state and if so, choose another port number.  But this method
   is as prone to races as the below one, or any other method using random
   port numbers, etc.  The below method at least tries to avoid races between
   multiple applications using rrecvport.

   As for the question "why don't you just use the Winsock rresvport?"...
   For some reason I do NOT understand, the call to WinSocks rresvport
   corrupts the stack when Cygwin is built using -fomit-frame-pointers.
   And then again, the Winsock rresvport function has the exact same
   problem with reusing ports in the TIME_WAIT state as the socket/bind
   method has.  So there's no gain in using that function. */

#define PORT_LOW	(IPPORT_EFSSERVER + 1)
#define PORT_HIGH	(IPPORT_RESERVED - 1)
#define NUM_PORTS	(PORT_HIGH - PORT_LOW + 1)

LONG last_used_rrecvport __attribute__((section (".cygwin_dll_common"), shared)) = IPPORT_RESERVED;

/* exported as rresvport: standards? */
extern "C" int
cygwin_rresvport (int *port)
{
  int res;
  sig_dispatch_pending ();

  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;

  res = socket (AF_INET, SOCK_STREAM, 0);
  if (res != (int) INVALID_SOCKET)
    {
      LONG myport;
      int ret = SOCKET_ERROR;
      struct sockaddr_in sin;
      sin.sin_family = AF_INET;
      sin.sin_addr.s_addr = INADDR_ANY;

      for (int i = 0; i < NUM_PORTS; i++)
	{
	  while ((myport = InterlockedExchange (&last_used_rrecvport, 0)) == 0)
	    low_priority_sleep (0);
	  if (--myport < PORT_LOW)
	    myport = PORT_HIGH;
	  InterlockedExchange (&last_used_rrecvport, myport);

	  sin.sin_port = htons (myport);
	  if (!(ret = bind (res, (struct sockaddr *) &sin, sizeof sin)))
	    break;
	  int err = WSAGetLastError ();
	  if (err != WSAEADDRINUSE && err != WSAEINVAL)
	    break;
	}
      if (ret == SOCKET_ERROR)
	{
	  closesocket (res);
	  res = (int) INVALID_SOCKET;
	}
      else if (port)
        *port = myport;
    }

  if (res != (int) INVALID_SOCKET)
    {
      cygheap_fdnew res_fd;

      if (res_fd >= 0 && fdsock (res_fd, tcp_dev, res))
	res = res_fd;
      else
	res = -1;
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
  sig_dispatch_pending ();

  myfault efault;
  if (efault.faulted (EFAULT))
    return (int) INVALID_SOCKET;

  res = rexec (ahost, inport, locuser, password, cmd, fd2p ? &fd2s : NULL);
  if (res != (int) INVALID_SOCKET)
    {
      cygheap_fdnew res_fd;

      if (res_fd >= 0 && fdsock (res_fd, tcp_dev, res))
	{
	  ((fhandler_socket *) res_fd)->connect_state (connected);
	  res = res_fd;
	}
      else
	{
	  closesocket (res);
	  res = -1;
	}

      if (res >= 0 && fd2p)
	{
	  cygheap_fdnew newfd (res_fd, false);
	  cygheap_fdget fd (*fd2p);

	  if (newfd >= 0 && fdsock (newfd, tcp_dev, fd2s))
	    {
	      ((fhandler_socket *) fd2p)->connect_state (connected);
	      *fd2p = newfd;
	    }
	  else
	    {
	      closesocket (res);
	      closesocket (fd2s);
	      res = -1;
	    }
	}
    }

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

  sig_dispatch_pending ();
  myfault efault;
  if (efault.faulted (EFAULT))
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
  if (connect (outsock, (struct sockaddr *) &sock_in, sizeof (sock_in)) < 0)
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

  {
    cygheap_fdnew sb0;
    const device *dev;

    if (family == AF_INET)
      dev = (type == SOCK_STREAM ? tcp_dev : udp_dev);
    else
      dev = (type == SOCK_STREAM ? stream_dev : dgram_dev);

    if (sb0 >= 0 && fdsock (sb0, dev, insock))
      {
	((fhandler_socket *) sb0)->set_sun_path ("");
	((fhandler_socket *) sb0)->set_addr_family (family);
	((fhandler_socket *) sb0)->set_socket_type (type);
	((fhandler_socket *) sb0)->connect_state (connected);
	if (family == AF_LOCAL && type == SOCK_STREAM)
	  ((fhandler_socket *) sb0)->af_local_set_sockpair_cred ();

	cygheap_fdnew sb1 (sb0, false);

	if (sb1 >= 0 && fdsock (sb1, dev, outsock))
	  {
	    ((fhandler_socket *) sb1)->set_sun_path ("");
	    ((fhandler_socket *) sb1)->set_addr_family (family);
	    ((fhandler_socket *) sb1)->set_socket_type (type);
	    ((fhandler_socket *) sb1)->connect_state (connected);
	    if (family == AF_LOCAL && type == SOCK_STREAM)
	      ((fhandler_socket *) sb1)->af_local_set_sockpair_cred ();

	    sb[0] = sb0;
	    sb[1] = sb1;
	    res = 0;
	  }
      }

    if (res == -1)
      {
	closesocket (insock);
	closesocket (outsock);
      }
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
  sig_dispatch_pending ();

  fhandler_socket *fh = get (fd);

  myfault efault;
  if (efault.faulted (EFAULT) || !fh)
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
  sig_dispatch_pending ();

  fhandler_socket *fh = get (fd);

  myfault efault;
  if (efault.faulted (EFAULT) || !fh)
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

/* See "UNIX Network Programming, Networing APIs: Sockets and XTI",
   W. Richard Stevens, Prentice Hall PTR, 1998. */
extern "C" int
cygwin_inet_pton (int family, const char *strptr, void *addrptr)
{
  if (family == AF_INET)
    {
      struct in_addr in_val;

      if (cygwin_inet_aton (strptr, &in_val))
	{
	  memcpy (addrptr, &in_val, sizeof (struct in_addr));
	  return 1;
	}
      return 0;
    }
  set_errno (EAFNOSUPPORT);
  return -1;
}

/* See "UNIX Network Programming, Networing APIs: Sockets and XTI",
   W. Richard Stevens, Prentice Hall PTR, 1998. */
extern "C" const char *
cygwin_inet_ntop (int family, const void *addrptr, char *strptr, socklen_t len)
{
  const u_char *p = (const u_char *) addrptr;

  myfault efault;
  if (efault.faulted (EFAULT))
    return NULL;
  if (family == AF_INET)
    {
      char temp[64]; /* Big enough for 4 ints ... */

      __small_sprintf (temp, "%u.%u.%u.%u", p[0], p[1], p[2], p[3]);
      if (strlen (temp) >= (size_t) len)
	{
	  set_errno (ENOSPC);
	  return NULL;
	}
      strcpy (strptr, temp);
      return strptr;
    }
  set_errno (EAFNOSUPPORT);
  return NULL;
}

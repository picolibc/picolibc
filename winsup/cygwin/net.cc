/* net.cc: network-related routines.

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005, 2006, 2007, 2008, 2009 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* #define DEBUG_NEST_ON 1 */

#define  __INSIDE_CYGWIN_NET__

#include "winsup.h"

/* unfortunately defined in windows header file but used in
   cygwin header files too */
#undef NOERROR
#undef DELETE

#include "miscfuncs.h"
#include <ctype.h>
#include <wchar.h>

#include <stdlib.h>
#define gethostname cygwin_gethostname
#include <unistd.h>
#undef gethostname
#include <netdb.h>
#include <asm/byteorder.h>
#define USE_SYS_TYPES_FD_SET
#include <winsock2.h>
#include <iphlpapi.h>
#include <assert.h>
#include "cygerrno.h"
#include "security.h"
#include "cygwin/version.h"
#include "shared_info.h"
#include "perprocess.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "sigproc.h"
#include "registry.h"
#include "cygtls.h"
#include "cygwin/in6.h"
#include "ifaddrs.h"
#include "tls_pbuf.h"
#define _CYGWIN_IN_H
#include <resolv.h>

extern "C"
{
  int h_errno;

  int __stdcall rcmd (char **ahost, unsigned short inport, char *locuser,
		      char *remuser, char *cmd, SOCKET * fd2p);
  int sscanf (const char *, const char *, ...);
  int cygwin_inet_aton(const char *, struct in_addr *);
  const char *cygwin_inet_ntop (int, const void *, char *, socklen_t);
  int dn_length1(const unsigned char *, const unsigned char *,
		 const unsigned char *);
}				/* End of "C" section */

const struct in6_addr in6addr_any = {{IN6ADDR_ANY_INIT}};
const struct in6_addr in6addr_loopback = {{IN6ADDR_LOOPBACK_INIT}};

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
      if (src->s_proto)
	sz += (protolen = strlen_round (src->s_proto));
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
      if (type == unionent::t_protoent)
	debug_printf ("protoent %s %x %x", dst->name, dst->list, dst->port_proto_addrtype);
      else if (type == unionent::t_servent)
	{
	  if (src->s_proto)
	    {
	      strcpy (dst->s_proto = dp, src->s_proto);
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
      /* Sanity check that we did our bookkeeping correctly. */
      assert ((dp - (char *) dst) == sz);
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

/* exported as getprotobyname: standards? */
extern "C" struct protoent *
cygwin_getprotobyname (const char *p)
{
  myfault efault;
  if (efault.faulted (EFAULT))
    return NULL;
  return dup_ent (getprotobyname (p));
}

/* exported as getprotobynumber: standards? */
extern "C" struct protoent *
cygwin_getprotobynumber (int number)
{
  return dup_ent (getprotobynumber (number));
}

bool
fdsock (cygheap_fdmanip& fd, const device *dev, SOCKET soc)
{
  int size;

  fd = build_fh_dev (*dev);
  if (!fd.isopen ())
    return false;
  fd->set_io_handle ((HANDLE) soc);
  if (!((fhandler_socket *) fd)->init_events ())
    return false;
  fd->set_flags (O_RDWR | O_BINARY);
  fd->uninterruptible_io (true);
  debug_printf ("fd %d, name '%s', soc %p", (int) fd, dev->name, soc);

  /* Usually sockets are inheritable IFS objects.  Unfortunately some virus
     scanners or other network-oriented software replace normal sockets
     with their own kind, which is running through a filter driver.
     
     The result is that these new sockets are not normal kernel objects
     anymore.  They are typically not marked as inheritable, nor are they
     IFS handles, as normal OS sockets are.  They are in fact not inheritable
     to child processes, and subsequent socket calls in the child process
     will fail with error 10038, WSAENOTSOCK.  And worse, while DuplicateHandle
     on these sockets mostly works in the process which created the socket,
     DuplicateHandle does quite often not work anymore in a child process.
     It does not help to mark them inheritable via SetHandleInformation.

     The only way to make these sockets usable in child processes is to
     duplicate them via WSADuplicateSocket/WSASocket calls.  This requires
     some incredible amount of extra processing so we only do this on
     affected systems.  If we recognize a non-inheritable socket, or if
     the XP1_IFS_HANDLES flag is not set in a call to WSADuplicateSocket,
     we switch to inheritance/dup via WSADuplicateSocket/WSASocket for
     that socket. */
  DWORD flags;
  WSAPROTOCOL_INFOW wpi;
  if (!GetHandleInformation ((HANDLE) soc, &flags)
      || !(flags & HANDLE_FLAG_INHERIT)
      || WSADuplicateSocketW (soc, GetCurrentProcessId (), &wpi)
      || !(wpi.dwServiceFlags1 & XP1_IFS_HANDLES))
    ((fhandler_socket *) fd)->init_fixup_before ();

  /* Raise default buffer sizes (instead of WinSock default 8K).

     64K appear to have the best size/performance ratio for a default
     value.  Tested with ssh/scp on Vista over Gigabit LAN.

     NOTE.  If the SO_RCVBUF size exceeds 65535(*), and if the socket is
     connected to a remote machine, then calling WSADuplicateSocket on
     fork/exec fails with WinSock error 10022, WSAEINVAL.  Fortunately
     we don't use WSADuplicateSocket anymore, rather we just utilize
     handle inheritance.  An explanation for this weird behaviour would
     be nice, though.

     (*) Maximum normal TCP window size.  Coincidence?  */
  ((fhandler_socket *) fd)->rmem () = 65535;
  ((fhandler_socket *) fd)->wmem () = 65535;
  if (::setsockopt (soc, SOL_SOCKET, SO_RCVBUF,
		    (char *) &((fhandler_socket *) fd)->rmem (), sizeof (int)))
    {
      debug_printf ("setsockopt(SO_RCVBUF) failed, %lu", WSAGetLastError ());
      if (::getsockopt (soc, SOL_SOCKET, SO_RCVBUF,
			(char *) &((fhandler_socket *) fd)->rmem (),
			(size = sizeof (int), &size)))
	system_printf ("getsockopt(SO_RCVBUF) failed, %lu", WSAGetLastError ());
    }
  if (::setsockopt (soc, SOL_SOCKET, SO_SNDBUF,
		    (char *) &((fhandler_socket *) fd)->wmem (), sizeof (int)))
    {
      debug_printf ("setsockopt(SO_SNDBUF) failed, %lu", WSAGetLastError ());
      if (::getsockopt (soc, SOL_SOCKET, SO_SNDBUF,
			(char *) &((fhandler_socket *) fd)->wmem (),
			(size = sizeof (int), &size)))
	system_printf ("getsockopt(SO_SNDBUF) failed, %lu", WSAGetLastError ());
    }

  return true;
}

/* exported as socket: standards? */
extern "C" int
cygwin_socket (int af, int type, int protocol)
{
  int res = -1;
  SOCKET soc = 0;

  debug_printf ("socket (%d, %d, %d)", af, type, protocol);

  soc = socket (af == AF_LOCAL ? AF_INET : af, type,
		af == AF_LOCAL ? 0 : protocol);

  if (soc == INVALID_SOCKET)
    {
      set_winsock_errno ();
      goto done;
    }

  const device *dev;

  if (af == AF_LOCAL)
    dev = type == SOCK_STREAM ? stream_dev : dgram_dev;
  else
    dev = type == SOCK_STREAM ? tcp_dev : udp_dev;

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
cygwin_sendto (int fd, const void *buf, size_t len, int flags,
	       const struct sockaddr *to, socklen_t tolen)
{
  int res;

  fhandler_socket *fh = get (fd);

  myfault efault;
  if (efault.faulted (EFAULT) || !fh)
    res = -1;
  else
    res = fh->sendto (buf, len, flags, to, tolen);

  syscall_printf ("%d = sendto (%d, %p, %d, %x, %p, %d)",
		  res, fd, buf, len, flags, to, tolen);

  return res;
}

/* exported as recvfrom: standards? */
extern "C" int
cygwin_recvfrom (int fd, void *buf, size_t len, int flags,
		 struct sockaddr *from, socklen_t *fromlen)
{
  int res;

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
		   socklen_t optlen)
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

      /* On systems supporting "enhanced socket security (2K3 and later),
	 the default behaviour of socket binding is equivalent to the POSIX
	 behaviour with SO_REUSEADDR.  Setting SO_REUSEADDR would only result
	 in wrong behaviour.  See also fhandler_socket::bind(). */
      if (level == SOL_SOCKET && optname == SO_REUSEADDR
	  && wincap.has_enhanced_socket_security ())
	res = 0;
      else
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
      else if (level == SOL_SOCKET)
	switch (optname)
	  {
	  case SO_REUSEADDR:
	    fh->saw_reuseaddr (*(int *) optval);
	    break;
	  case SO_RCVBUF:
	    fh->rmem (*(int *) optval);
	    break;
	  case SO_SNDBUF:
	    fh->wmem (*(int *) optval);
	    break;
	  default:
	    break;
	  }
    }

  syscall_printf ("%d = setsockopt (%d, %d, %x, %p, %d)",
		  res, fd, level, optname, optval, optlen);
  return res;
}

/* exported as getsockopt: standards? */
extern "C" int
cygwin_getsockopt (int fd, int level, int optname, void *optval,
		   socklen_t *optlen)
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
cygwin_connect (int fd, const struct sockaddr *name, socklen_t namelen)
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

  servent *res = dup_ent (getservbyname (name, proto));
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

  servent *res = dup_ent (getservbyport (port, proto));
  syscall_printf ("%p = getservbyport (%d, %s)", res, port, proto);
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

  hostent *res = dup_ent (h);
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

  hostent *res = dup_ent (gethostbyaddr (addr, len, type));
  if (res)
    debug_printf ("h_name %s", res->h_name);
  else
    set_host_errno ();
  return res;
}

static void
memcpy4to6 (char *dst, const u_char *src)
{
  const unsigned int h[] = {0, 0, htonl (0xFFFF)};
  memcpy (dst, h, 12);
  memcpy (dst + 12, src, NS_INADDRSZ);
}

static hostent *
gethostby_helper (const char *name, const int af, const int type,
		  const int addrsize_in, const int addrsize_out)
{
  /* Get the data from the name server */
  const int maxcount = 3;
  int old_errno, ancount = 0, anlen = 1024, msgsize = 0;
  u_char *ptr, *msg = NULL;
  int sz;
  hostent *ret;
  char *string_ptr;

  while ((anlen > msgsize) && (ancount++ < maxcount))
    {
      msgsize = anlen;
      ptr = (u_char *) realloc (msg, msgsize);
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
  u_char *eomsg = msg + anlen - 1;


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
    u_char data[];              // data
    record * next () { return (record *) (((char *) this) + next_o); }
    void set_next ( record * nxt) { next_o = ((char *) nxt) - ((char *) this); }
    u_char * name () { return (u_char *) (((char *) this) - complen); }
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
  ancount = alias_count + address_count; /* Valid records */
  alias_count = address_count = 0;

  for (i = 0, curptr = anptr; i < ancount; i++, curptr = curptr->next ())
    {
      antype = curptr->type;
      if (antype == ns_t_cname)
	{
	  complen = dn_expand (msg, eomsg, curptr->name (), string_ptr, string_size);
#ifdef DEBUGGING
	  if (complen != curptr->complen)
	    goto debugging;
#endif
	  ret->h_aliases[alias_count++] = string_ptr;
	  namelen1 = curptr->namelen1;
	  string_ptr += namelen1;
	  string_size -= namelen1;
	  continue;
	}
      if (antype == type)
	    {
	      if (address_count == 0)
		{
		  complen = dn_expand (msg, eomsg, curptr->name(), string_ptr, string_size);
#ifdef DEBUGGING
		  if (complen != curptr->complen)
		    goto debugging;
#endif
		  ret->h_name = string_ptr;
		  namelen1 = curptr->namelen1;
		  string_ptr += namelen1;
		  string_size -= namelen1;
		}
	      ret->h_addr_list[address_count++] = string_ptr;
	      if (addrsize_in != addrsize_out)
		memcpy4to6 (string_ptr, curptr->data);
	      else
		memcpy (string_ptr, curptr->data, addrsize_in);
	      string_ptr += addrsize_out;
	      string_size -= addrsize_out;
	      continue;
	    }
#ifdef DEBUGGING
      /* Should not get here */
      goto debugging;
#endif
    }
#ifdef DEBUGGING
  if (string_size < 0)
    goto debugging;
#endif

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


#ifdef DEBUGGING
 debugging:
   system_printf ("Please debug.");
   free (msg);
   free (ret);
   h_errno = NO_RECOVERY;
   return NULL;
#endif
}

/* gethostbyname2: standards? */
extern "C" struct hostent *
gethostbyname2 (const char *name, int af)
{
  sig_dispatch_pending ();
  myfault efault;
  if (efault.faulted (EFAULT))
    return NULL;

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
      return NULL;
    }

  return gethostby_helper (name, af, type, addrsize_in, addrsize_out);
}

/* exported as accept: standards? */
extern "C" int
cygwin_accept (int fd, struct sockaddr *peer, socklen_t *len)
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
cygwin_bind (int fd, const struct sockaddr *my_addr, socklen_t addrlen)
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
cygwin_getsockname (int fd, struct sockaddr *addr, socklen_t *namelen)
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
cygwin_getpeername (int fd, struct sockaddr *name, socklen_t *len)
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
cygwin_recv (int fd, void *buf, size_t len, int flags)
{
  return cygwin_recvfrom (fd, buf, len, flags, NULL, NULL);
}

/* exported as send: standards? */
extern "C" int
cygwin_send (int fd, const void *buf, size_t len, int flags)
{
  return cygwin_sendto (fd, buf, len, flags, NULL, 0);
}

/* getdomainname: standards? */
extern "C" int
getdomainname (char *domain, size_t len)
{
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

  /* This is only used by NT4.
     The registry names are language independent. */
  reg_key r (HKEY_LOCAL_MACHINE, KEY_READ,
	     "SYSTEM", "CurrentControlSet", "Services",
	     "Tcpip", "Parameters", NULL);

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

/* Vista/Longhorn: unicast address has additional OnLinkPrefixLength member. */
typedef struct _IP_ADAPTER_UNICAST_ADDRESS_LH {
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
} IP_ADAPTER_UNICAST_ADDRESS_LH, *PIP_ADAPTER_UNICAST_ADDRESS_LH;

/* Vista/Longhorn: IP_ADAPTER_ADDRESSES has a lot more info.  We pick only
   what we need for now. */
typedef struct _IP_ADAPTER_ADDRESSES_LH {
  _ANONYMOUS_UNION union {
    ULONGLONG Alignment;
    _ANONYMOUS_STRUCT struct {
      ULONG Length;
      DWORD IfIndex;
    } DUMMYSTRUCTNAME;
  } DUMMYUNIONNAME;
  struct _IP_ADAPTER_ADDRESSES* Next;
  PCHAR AdapterName;
  PIP_ADAPTER_UNICAST_ADDRESS FirstUnicastAddress;
  PIP_ADAPTER_ANYCAST_ADDRESS FirstAnycastAddress;
  PIP_ADAPTER_MULTICAST_ADDRESS FirstMulticastAddress;
  PIP_ADAPTER_DNS_SERVER_ADDRESS FirstDnsServerAddress;
  PWCHAR DnsSuffix;
  PWCHAR Description;
  PWCHAR FriendlyName;
  BYTE PhysicalAddress[MAX_ADAPTER_ADDRESS_LENGTH];
  DWORD PhysicalAddressLength;
  DWORD Flags;
  DWORD Mtu;
  DWORD IfType;
  IF_OPER_STATUS OperStatus;
  DWORD Ipv6IfIndex;
  DWORD ZoneIndices[16];
  PIP_ADAPTER_PREFIX FirstPrefix;

  ULONG64 TransmitLinkSpeed;
  ULONG64 ReceiveLinkSpeed;
  PVOID FirstWinsServerAddress;
  PVOID FirstGatewayAddress;
  ULONG Ipv4Metric;
  ULONG Ipv6Metric;
} IP_ADAPTER_ADDRESSES_LH,*PIP_ADAPTER_ADDRESSES_LH;

/* We can't include ws2tcpip.h. */

#define SIO_GET_INTERFACE_LIST  _IOR('t', 127, u_long)

struct sockaddr_in6_old {
  short   sin6_family;
  u_short sin6_port;
  u_long  sin6_flowinfo;
  struct in6_addr sin6_addr;
};

typedef union sockaddr_gen{
  struct sockaddr	  Address;
  struct sockaddr_in	  AddressIn;
  struct sockaddr_in6_old AddressIn6;
} sockaddr_gen;

typedef struct _INTERFACE_INFO {
  u_long	  iiFlags;
  sockaddr_gen	  iiAddress;
  sockaddr_gen	  iiBroadcastAddress;
  sockaddr_gen	  iiNetmask;
} INTERFACE_INFO, *LPINTERFACE_INFO;

#ifndef IN_LOOPBACK
#define IN_LOOPBACK(a)	((((long int) (a)) & 0xff000000) == 0x7f000000)
#endif

static int in6_are_prefix_equal (struct in6_addr *, struct in6_addr *, int);

static int in_are_prefix_equal (struct in_addr *p1, struct in_addr *p2, int len)
{
  if (0 > len || len > 32)
    return 0;
  uint32_t pfxmask = 0xffffffff << (32 - len);
  return (ntohl (p1->s_addr) & pfxmask) == (ntohl (p2->s_addr) & pfxmask);
}

extern "C" int
ip_addr_prefix (PIP_ADAPTER_UNICAST_ADDRESS pua, PIP_ADAPTER_PREFIX pap)
{
  if (wincap.has_gaa_on_link_prefix ())
    return (int) ((PIP_ADAPTER_UNICAST_ADDRESS_LH) pua)->OnLinkPrefixLength;
  switch (pua->Address.lpSockaddr->sa_family)
    {
    case AF_INET:
      /* Prior to Vista, the loopback prefix is not available. */
      if (IN_LOOPBACK (ntohl (((struct sockaddr_in *)
			      pua->Address.lpSockaddr)->sin_addr.s_addr)))
	return 8;
      for ( ; pap; pap = pap->Next)
	if (in_are_prefix_equal (
	      &((struct sockaddr_in *) pua->Address.lpSockaddr)->sin_addr,
	      &((struct sockaddr_in *) pap->Address.lpSockaddr)->sin_addr,
	      pap->PrefixLength))
	  return pap->PrefixLength;
      break;
    case AF_INET6:
      /* Prior to Vista, the loopback prefix is not available. */
      if (IN6_IS_ADDR_LOOPBACK (&((struct sockaddr_in6 *)
				  pua->Address.lpSockaddr)->sin6_addr))
	return 128;
      for ( ; pap; pap = pap->Next)
	if (in6_are_prefix_equal (
	      &((struct sockaddr_in6 *) pua->Address.lpSockaddr)->sin6_addr,
	      &((struct sockaddr_in6 *) pap->Address.lpSockaddr)->sin6_addr,
	      pap->PrefixLength))
	  return pap->PrefixLength;
      break;
    default:
      break;
    }
  return 0;
}

#ifndef GAA_FLAG_INCLUDE_ALL_INTERFACES
#define GAA_FLAG_INCLUDE_ALL_INTERFACES 0x0100
#endif

bool
get_adapters_addresses (PIP_ADAPTER_ADDRESSES *pa_ret, ULONG family)
{
  DWORD ret, size = 0;
  PIP_ADAPTER_ADDRESSES pa0 = NULL;

  if (!pa_ret)
    return ERROR_BUFFER_OVERFLOW
	   == GetAdaptersAddresses (family, GAA_FLAG_INCLUDE_PREFIX
					    | GAA_FLAG_INCLUDE_ALL_INTERFACES,
				    NULL, NULL, &size);
  do
    {
      ret = GetAdaptersAddresses (family, GAA_FLAG_INCLUDE_PREFIX
					  | GAA_FLAG_INCLUDE_ALL_INTERFACES,
				  NULL, pa0, &size);
      if (ret == ERROR_BUFFER_OVERFLOW
	  && !(pa0 = (PIP_ADAPTER_ADDRESSES) realloc (pa0, size)))
	break;
    }
  while (ret == ERROR_BUFFER_OVERFLOW);
  if (ret != ERROR_SUCCESS)
    {
      if (pa0)
	free (pa0);
      *pa_ret = NULL;
      return false;
    }
  *pa_ret = pa0;
  return true;
}

#define WS_IFF_UP	     1
#define WS_IFF_BROADCAST     2
#define WS_IFF_LOOPBACK	     4
#define WS_IFF_POINTTOPOINT  8
#define WS_IFF_MULTICAST    16

static inline short
convert_ifr_flags (u_long ws_flags)
{
  return (ws_flags & (WS_IFF_UP | WS_IFF_BROADCAST))
	 | ((ws_flags & (WS_IFF_LOOPBACK | WS_IFF_POINTTOPOINT)) << 1)
	 | ((ws_flags & WS_IFF_MULTICAST) << 8);
}

static u_long
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
  struct sockaddr         ifa_hwaddr;
  int                     ifa_metric;
  int                     ifa_mtu;
  int                     ifa_ifindex;
  struct ifreq_frndlyname ifa_frndlyname;
};

static unsigned int
get_flags (PIP_ADAPTER_ADDRESSES pap)
{
  unsigned int flags = IFF_UP;
  if (pap->IfType == IF_TYPE_SOFTWARE_LOOPBACK)
    flags |= IFF_LOOPBACK;
  else if (pap->IfType == IF_TYPE_PPP)
    flags |= IFF_POINTOPOINT;
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
  HKEY key;
  LONG ret;
  char regkey[256], *c;
  ULONG ifs = 1;
  DWORD dhcp, size;

  c = stpcpy (regkey, "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\"
		      "Parameters\\Interfaces\\");
  stpcpy (c, name);
  if ((ret = RegOpenKeyEx (HKEY_LOCAL_MACHINE, regkey, 0, KEY_READ,
			   &key)) != ERROR_SUCCESS)
    {
      debug_printf ("RegOpenKeyEx(%s), win32 error %ld", ret);
      return 0;
    }
  /* If DHCP is used, we have only one address. */
  if ((ret = RegQueryValueEx (key, "EnableDHCP", NULL, NULL, (PBYTE) &dhcp,
			      (size = sizeof dhcp, &size))) == ERROR_SUCCESS
      && dhcp == 0
      && (ret = RegQueryValueEx (key, "IPAddress", NULL, NULL, NULL,
				 &size)) == ERROR_SUCCESS)
    {
      char *ipa = (char *) alloca (size);
      RegQueryValueEx (key, "IPAddress", NULL, NULL, (PBYTE) ipa, &size);
      for (ifs = 0, c = ipa; *c; c += strlen (c) + 1)
	ifs++;
    }
  RegCloseKey (key);
  return ifs;
}

static void
get_ipv4fromreg (struct ifall *ifp, const char *name, DWORD idx)
{
  HKEY key;
  LONG ret;
  char regkey[256], *c;
  DWORD ifs;
  DWORD dhcp, size;

  c = stpcpy (regkey, "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\"
		      "Parameters\\Interfaces\\");
  stpcpy (c, name);
  if ((ret = RegOpenKeyEx (HKEY_LOCAL_MACHINE, regkey, 0, KEY_READ, &key))
      != ERROR_SUCCESS)
    {
      debug_printf ("RegOpenKeyEx(%s), win32 error %ld", ret);
      return;
    }
  /* If DHCP is used, we have only one address. */
  if ((ret = RegQueryValueEx (key, "EnableDHCP", NULL, NULL, (PBYTE) &dhcp,
			      (size = sizeof dhcp, &size))) == ERROR_SUCCESS)
    {
#define addr ((struct sockaddr_in *) &ifp->ifa_addr)
#define mask ((struct sockaddr_in *) &ifp->ifa_netmask)
#define brdc ((struct sockaddr_in *) &ifp->ifa_brddstaddr)
      if (dhcp)
	{
	  if ((ret = RegQueryValueEx (key, "DhcpIPAddress", NULL, NULL,
				      (PBYTE) regkey, (size = 256, &size)))
	      == ERROR_SUCCESS)
	    cygwin_inet_aton (regkey, &addr->sin_addr);
	  if ((ret = RegQueryValueEx (key, "DhcpSubnetMask", NULL, NULL,
				      (PBYTE) regkey, (size = 256, &size)))
	      == ERROR_SUCCESS)
	    cygwin_inet_aton (regkey, &mask->sin_addr);
	}
      else
	{
	  if ((ret = RegQueryValueEx (key, "IPAddress", NULL, NULL, NULL,
				     &size)) == ERROR_SUCCESS)
	    {
	      char *ipa = (char *) alloca (size);
	      RegQueryValueEx (key, "IPAddress", NULL, NULL, (PBYTE) ipa, &size);
	      for (ifs = 0, c = ipa; *c && ifs < idx; c += strlen (c) + 1)
		ifs++;
	      if (*c)
		cygwin_inet_aton (c, &addr->sin_addr);
	    }
	  if ((ret = RegQueryValueEx (key, "SubnetMask", NULL, NULL, NULL,
				     &size)) == ERROR_SUCCESS)
	    {
	      char *ipa = (char *) alloca (size);
	      RegQueryValueEx (key, "SubnetMask", NULL, NULL, (PBYTE) ipa, &size);
	      for (ifs = 0, c = ipa; *c && ifs < idx; c += strlen (c) + 1)
		ifs++;
	      if (*c)
		cygwin_inet_aton (c, &mask->sin_addr);
	    }
	}
      if (ifp->ifa_ifa.ifa_flags & IFF_BROADCAST)
	brdc->sin_addr.s_addr = (addr->sin_addr.s_addr
				 & mask->sin_addr.s_addr)
				| ~mask->sin_addr.s_addr;
#undef addr
#undef mask
#undef brdc
    }
  RegCloseKey (key);
}

static void
get_friendlyname (struct ifall *ifp, PIP_ADAPTER_ADDRESSES pap)
{
  struct ifreq_frndlyname *iff = (struct ifreq_frndlyname *)
				 &ifp->ifa_frndlyname;
  iff->ifrf_len = sys_wcstombs (iff->ifrf_friendlyname,
				IFRF_FRIENDLYNAMESIZ,
				pap->FriendlyName);
}

static void
get_hwaddr (struct ifall *ifp, PIP_ADAPTER_ADDRESSES pap)
{
  for (UINT i = 0; i < IFHWADDRLEN; ++i)
    if (i >= pap->PhysicalAddressLength)
      ifp->ifa_hwaddr.sa_data[i] = '\0';
    else
      ifp->ifa_hwaddr.sa_data[i] = pap->PhysicalAddress[i];
}
/*
 * Get network interfaces XP SP1 and above.
 * Use IP Helper function GetAdaptersAddresses.
 */
static struct ifall *
get_xp_ifs (ULONG family)
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
	    if (idx)
	      __small_sprintf (ifp->ifa_name, "%s:%u", pap->AdapterName, idx);
	    else
	      strcpy (ifp->ifa_name, pap->AdapterName);
	    ifp->ifa_ifa.ifa_name = ifp->ifa_name;
	    /* Flags */
	    ifp->ifa_ifa.ifa_flags = get_flags (pap);
	    if (pap->IfType != IF_TYPE_PPP)
	      ifp->ifa_ifa.ifa_flags |= IFF_BROADCAST;
	    /* Address */
	    ifp->ifa_addr.ss_family = AF_INET;
	    ifp->ifa_ifa.ifa_addr = (struct sockaddr *) &ifp->ifa_addr;
	    /* Broadcast/Destination address */
	    ifp->ifa_brddstaddr.ss_family = AF_INET;
	    ifp->ifa_ifa.ifa_dstaddr = NULL;
	    /* Netmask */
	    ifp->ifa_netmask.ss_family = AF_INET;
	    ifp->ifa_ifa.ifa_netmask = (struct sockaddr *) &ifp->ifa_netmask;
	    /* Try to fetch real IPv4 address information from registry. */
	    get_ipv4fromreg (ifp, pap->AdapterName, idx);
	    /* Hardware address */
	    get_hwaddr (ifp, pap);
	    /* Metric */
	    ifp->ifa_metric = 1;
	    /* MTU */
	    ifp->ifa_mtu = pap->Mtu;
	    /* Interface index */
	    ifp->ifa_ifindex = pap->IfIndex;
	    /* Friendly name */
	    get_friendlyname (ifp, pap);
	    ++ifp;
	  }
      else
	for (idx = 0, pua = pap->FirstUnicastAddress; pua;
	     ++idx, pua = pua->Next)
	  {
	    struct sockaddr *sa = (struct sockaddr *) pua->Address.lpSockaddr;
#         define sin	((struct sockaddr_in *) sa)
#         define sin6	((struct sockaddr_in6 *) sa)
	    size_t sa_size = (sa->sa_family == AF_INET6
			      ? sizeof *sin6 : sizeof *sin);
	    /* Next in chain */
	    ifp->ifa_ifa.ifa_next = (struct ifaddrs *) &ifp[1].ifa_ifa;
	    /* Interface name */
	    if (idx && sa->sa_family == AF_INET)
	      __small_sprintf (ifp->ifa_name, "%s:%u", pap->AdapterName, idx);
	    else
	      strcpy (ifp->ifa_name, pap->AdapterName);
	    ifp->ifa_ifa.ifa_name = ifp->ifa_name;
	    /* Flags */
	    ifp->ifa_ifa.ifa_flags = get_flags (pap);
	    if (sa->sa_family == AF_INET
		&& pap->IfType != IF_TYPE_SOFTWARE_LOOPBACK
		&& pap->IfType != IF_TYPE_PPP)
	      ifp->ifa_ifa.ifa_flags |= IFF_BROADCAST;
	    if (sa->sa_family == AF_INET)
	      {
		ULONG hwaddr[2], hwlen = 6;
		if (SendARP (sin->sin_addr.s_addr, 0, hwaddr, &hwlen))
		  ifp->ifa_ifa.ifa_flags |= IFF_NOARP;
	      }
	    /* Address */
	    memcpy (&ifp->ifa_addr, sa, sa_size);
	    ifp->ifa_ifa.ifa_addr = (struct sockaddr *) &ifp->ifa_addr;
	    /* Netmask */
	    int prefix = ip_addr_prefix (pua, pap->FirstPrefix);
	    switch (sa->sa_family)
	      {
	      case AF_INET:
		if_sin = (struct sockaddr_in *) &ifp->ifa_netmask;
		if_sin->sin_addr.s_addr = htonl (UINT32_MAX << (32 - prefix));
		if_sin->sin_family = AF_INET;
		break;
	      case AF_INET6:
		if_sin6 = (struct sockaddr_in6 *) &ifp->ifa_netmask;
		for (cnt = 0; cnt < 4 && prefix; ++cnt, prefix -= 32)
		  if_sin6->sin6_addr.s6_addr32[cnt] = UINT32_MAX;
		  if (prefix < 32)
		    if_sin6->sin6_addr.s6_addr32[cnt] <<= 32 - prefix;
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
	    if (wincap.has_gaa_on_link_prefix ())
	      ifp->ifa_metric = (sa->sa_family == AF_INET
				? ((PIP_ADAPTER_ADDRESSES_LH) pap)->Ipv4Metric
				: ((PIP_ADAPTER_ADDRESSES_LH) pap)->Ipv6Metric);
	    else
	      ifp->ifa_metric = 1;
	    /* MTU */
	    ifp->ifa_mtu = pap->Mtu;
	    /* Interface index */
	    ifp->ifa_ifindex = pap->IfIndex;
	    /* Friendly name */
	    get_friendlyname (ifp, pap);
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

/*
 * Get network interfaces NTSP4, W2K, XP w/o service packs.
 * Use IP Helper Library
 */
static struct ifall *
get_2k_ifs ()
{
  int ethId = 0, pppId = 0, slpId = 0, tokId = 0;

  DWORD ip_cnt;
  DWORD siz_ip_table = 0;
  PMIB_IPADDRTABLE ipt;
  PMIB_IFROW ifrow;
  struct ifall *ifret = NULL, *ifp = NULL;
  struct sockaddr_in *if_sin;

  struct ifcount_t
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
      if (!(ifret = (struct ifall *) calloc (ipt->dwNumEntries, sizeof (struct ifall))))
	goto done;
      ifp = ifret;

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
      /* reset the last element. This is just the stopper for the loop. */
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

	  /* Next in chain */
	  ifp->ifa_ifa.ifa_next = (struct ifaddrs *) &ifp[1].ifa_ifa;
	  /* Interface name */
	  if (ifrow->dwType == IF_TYPE_SOFTWARE_LOOPBACK)
	    strcpy (ifp->ifa_name, "lo");
	  else
	    {
	      const char *name = "";
	      switch (ifrow->dwType)
		{
		  case IF_TYPE_ISO88025_TOKENRING:
		    name = "tok";
		    if (ifEntry->enumerated == 0)
		      ifEntry->classId = tokId++;
		    break;
		  case IF_TYPE_ETHERNET_CSMACD:
		    name = "eth";
		    if (ifEntry->enumerated == 0)
		      ifEntry->classId = ethId++;
		    break;
		  case IF_TYPE_PPP:
		    name = "ppp";
		    if (ifEntry->enumerated == 0)
		      ifEntry->classId = pppId++;
		    break;
		  case IF_TYPE_SLIP:
		    name = "slp";
		    if (ifEntry->enumerated == 0)
		      ifEntry->classId = slpId++;
		    break;
		  default:
		    continue;
		}
	      __small_sprintf (ifp->ifa_name,
			       ifEntry->enumerated ? "%s%u:%u" : "%s%u",
			       name, ifEntry->classId, ifEntry->enumerated);
	      ifEntry->enumerated++;
	    }
	  ifp->ifa_ifa.ifa_name = ifp->ifa_name;
	  /* Flags */
	  if (ifrow->dwType == IF_TYPE_SOFTWARE_LOOPBACK)
	    ifp->ifa_ifa.ifa_flags |= IFF_LOOPBACK;
	  else if (ifrow->dwType == IF_TYPE_PPP
		   || ifrow->dwType == IF_TYPE_SLIP)
	    ifp->ifa_ifa.ifa_flags |= IFF_POINTOPOINT | IFF_NOARP;
	  else
	    ifp->ifa_ifa.ifa_flags |= IFF_BROADCAST;
	  if (ifrow->dwAdminStatus == IF_ADMIN_STATUS_UP)
	    {
	      ifp->ifa_ifa.ifa_flags |= IFF_UP | IFF_LOWER_UP;
	      /* Bug in NT4's IP Helper lib.  The dwOperStatus has just
		 two values, 0 or 1, non operational, operational. */
	      if (ifrow->dwOperStatus >= (wincap.has_broken_if_oper_status ()
					  ? 1 : IF_OPER_STATUS_CONNECTED))
		ifp->ifa_ifa.ifa_flags |= IFF_RUNNING;
	    }
	  /* Address */
	  if_sin = (struct sockaddr_in *) &ifp->ifa_addr;
	  if_sin->sin_addr.s_addr = ipt->table[ip_cnt].dwAddr;
	  if_sin->sin_family = AF_INET;
	  ifp->ifa_ifa.ifa_addr = (struct sockaddr *) &ifp->ifa_addr;
	  /* Netmask */
	  if_sin = (struct sockaddr_in *) &ifp->ifa_netmask;
	  if_sin->sin_addr.s_addr = ipt->table[ip_cnt].dwMask;
	  if_sin->sin_family = AF_INET;
	  ifp->ifa_ifa.ifa_netmask = (struct sockaddr *) &ifp->ifa_netmask;
	  if_sin = (struct sockaddr_in *) &ifp->ifa_brddstaddr;
	  if (ifrow->dwType == IF_TYPE_PPP
	      || ifrow->dwType == IF_TYPE_SLIP)
	    {
	      /* Destination address */
	      if_sin->sin_addr.s_addr =
		get_routedst (ipt->table[ip_cnt].dwIndex);
	      ifp->ifa_ifa.ifa_dstaddr = (struct sockaddr *)
					 &ifp->ifa_brddstaddr;
	    }
	  else
	    {
	      /* Broadcast address */
#if 0
	      /* Unfortunately, the field returns only crap. */
	      if_sin->sin_addr.s_addr = ipt->table[ip_cnt].dwBCastAddr;
#else
	      uint32_t mask = ipt->table[ip_cnt].dwMask;
	      if_sin->sin_addr.s_addr = (ipt->table[ip_cnt].dwAddr & mask) | ~mask;
#endif
	      ifp->ifa_ifa.ifa_broadaddr = (struct sockaddr *)
					   &ifp->ifa_brddstaddr;
	    }
	  if_sin->sin_family = AF_INET;
	  /* Hardware address */
	  for (UINT i = 0; i < IFHWADDRLEN; ++i)
	    if (i >= ifrow->dwPhysAddrLen)
	      ifp->ifa_hwaddr.sa_data[i] = '\0';
	    else
	      ifp->ifa_hwaddr.sa_data[i] = ifrow->bPhysAddr[i];
	  /* Metric */
	  ifp->ifa_metric = 1;
	  /* MTU */
	  ifp->ifa_mtu = ifrow->dwMtu;
	  /* Interface index */
	  ifp->ifa_ifindex = ifrow->dwIndex;
	  /* Friendly name */
	  struct ifreq_frndlyname *iff = (struct ifreq_frndlyname *)
					 &ifp->ifa_frndlyname;
	  iff->ifrf_len = sys_wcstombs (iff->ifrf_friendlyname,
					IFRF_FRIENDLYNAMESIZ,
					ifrow->wszName);
	  ++ifp;
	}
    }
  /* Since every entry is set to the next entry, the last entry points to an
     invalid next entry now.  Fix it retroactively. */
  if (ifp > ifret)
    ifp[-1].ifa_ifa.ifa_next = NULL;

done:
  return ifret;
}

/*
 * Get network interfaces Windows NT < SP4:
 * Look at the Bind value in
 * HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Tcpip\Linkage\
 * This is a REG_MULTI_SZ with strings of the form:
 * \Device\<Netcard>, where netcard is the name of the net device.
 * Then look under:
 * HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\<NetCard>\
 *							Parameters\Tcpip
 * at the IPAddress, Subnetmask and DefaultGateway values for the
 * required values.
 * Also fake "lo" since there's no representation in the registry.
 */
static struct ifall *
get_nt_ifs ()
{
  HKEY key;
  LONG ret;
  struct ifall *ifret = NULL, *ifp;
  unsigned long lip, lnp;
  struct sockaddr_in *sin = NULL;
  DWORD size;
  int cnt = 0, idx;
  char *binding = NULL;

  if ((ret = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
			   "SYSTEM\\"
			   "CurrentControlSet\\"
			   "Services\\"
			   "Tcpip\\" "Linkage",
			   0, KEY_READ, &key)) == ERROR_SUCCESS)
    {
      if ((ret = RegQueryValueEx (key, "Bind", NULL, NULL,
				  NULL, &size)) == ERROR_SUCCESS)
	{
	  binding = (char *) alloca (size);
	  if ((ret = RegQueryValueEx (key, "Bind", NULL, NULL,
				      (unsigned char *) binding,
				      &size)) != ERROR_SUCCESS)
	    binding = NULL;
	}
      RegCloseKey (key);
    }

  if (!binding)
    {
      __seterrno_from_win_error (ret);
      return NULL;
    }

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
	++cnt;
      RegCloseKey (key);
    }
  ++cnt; /* loopback */
  if (!(ifret = (struct ifall *) malloc (cnt * sizeof (struct ifall))))
    return NULL;
  /* Set up lo interface first */
  idx = 0;
  ifp = ifret + idx;
  memset (ifp, 0, sizeof *ifp);
  /* Next in chain */
  ifp->ifa_ifa.ifa_next = (struct ifaddrs *) &ifp[1].ifa_ifa;
  /* Interface name */
  strcpy (ifp->ifa_name, "lo");
  ifp->ifa_ifa.ifa_name = ifp->ifa_name;
  /* Flags */
  ifp->ifa_ifa.ifa_flags = IFF_UP | IFF_LOWER_UP | IFF_RUNNING | IFF_LOOPBACK;
  /* Address */
  sin = (struct sockaddr_in *) &ifp->ifa_addr;
  sin->sin_addr.s_addr = htonl (INADDR_LOOPBACK);
  sin->sin_family = AF_INET;
  ifp->ifa_ifa.ifa_addr = (struct sockaddr *) &ifp->ifa_addr;
  /* Netmask */
  sin = (struct sockaddr_in *) &ifp->ifa_netmask;
  sin->sin_addr.s_addr = htonl (IN_CLASSA_NET);
  sin->sin_family = AF_INET;
  ifp->ifa_ifa.ifa_netmask = (struct sockaddr *) &ifp->ifa_netmask;
  /* Broadcast address  */
  sin = (struct sockaddr_in *) &ifp->ifa_brddstaddr;
  sin->sin_addr.s_addr = htonl (INADDR_LOOPBACK | IN_CLASSA_HOST);
  sin->sin_family = AF_INET;
  ifp->ifa_ifa.ifa_broadaddr = (struct sockaddr *) &ifp->ifa_brddstaddr;
  /* Hardware address */
  ; // Nothing to do... */
  /* Metric */
  ifp->ifa_metric = 1;
  /* MTU */
  ifp->ifa_mtu = 1520; /* Default value for MS TCP Loopback interface. */
  /* Interface index */
  ifp->ifa_ifindex = -1;
  /* Friendly name */
  struct ifreq_frndlyname *iff = (struct ifreq_frndlyname *)
				  &ifp->ifa_frndlyname;
  strcpy (iff->ifrf_friendlyname, "Default loopback");
  iff->ifrf_len = 16;

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
	  bool ppp = false;

	  for (ip = ipaddress, np = netmask;
	       *ip && *np;
	       ip += strlen (ip) + 1, np += strlen (np) + 1)
	    {
	      bool dhcp = false;
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
		dhcp = true;
	      if (++idx == cnt
		  && !(ifp = (struct ifall *)
			     realloc (ifret, ++cnt * sizeof (struct ifall))))
		  {
		    free (ifret);
		    return NULL;
		  }
	      ifp = ifret + idx;
	      memset (ifp, 0, sizeof *ifp);
	      /* Next in chain */
	      ifp->ifa_ifa.ifa_next = (struct ifaddrs *) &ifp[1].ifa_ifa;
	      /* Interface name */
	      if (!strncmp (bp, "NdisWan", 7))
		{
		  strcpy (ifp->ifa_name, "ppp");
		  strcat (ifp->ifa_name, bp + 7);
		  ppp = true;
		}
	      else
		{
		  ++*eth;
		  strcpy (ifp->ifa_name, "eth");
		  strcat (ifp->ifa_name, eth);
		}
	      ifp->ifa_ifa.ifa_name = ifp->ifa_name;
	      /* Flags */
	      ifp->ifa_ifa.ifa_flags = IFF_UP | IFF_LOWER_UP | IFF_RUNNING;
	      if (ppp)
		ifp->ifa_ifa.ifa_flags |= IFF_POINTOPOINT | IFF_NOARP;
	      else
		ifp->ifa_ifa.ifa_flags |= IFF_BROADCAST;
	      /* Address */
	      sin = (struct sockaddr_in *) &ifp->ifa_addr;
	      sin->sin_addr.s_addr = cygwin_inet_addr (dhcp ? dhcpaddress : ip);
	      sin->sin_family = AF_INET;
	      ifp->ifa_ifa.ifa_addr = (struct sockaddr *) &ifp->ifa_addr;
	      /* Netmask */
	      sin = (struct sockaddr_in *) &ifp->ifa_netmask;
	      sin->sin_addr.s_addr = cygwin_inet_addr (dhcp ? dhcpnetmask : np);
	      sin->sin_family = AF_INET;
	      ifp->ifa_ifa.ifa_netmask = (struct sockaddr *) &ifp->ifa_netmask;
	      if (ppp)
		{
		  /* Destination address */
		  sin = (struct sockaddr_in *) &ifp->ifa_brddstaddr;
		  sin->sin_addr.s_addr =
		    cygwin_inet_addr (dhcp ? dhcpaddress : ip);
		  sin->sin_family = AF_INET;
		  ifp->ifa_ifa.ifa_dstaddr = (struct sockaddr *)
					     &ifp->ifa_brddstaddr;
		}
	      else
		{
		  /* Broadcast address */
		  lip = cygwin_inet_addr (dhcp ? dhcpaddress : ip);
		  lnp = cygwin_inet_addr (dhcp ? dhcpnetmask : np);
		  sin = (struct sockaddr_in *) &ifp->ifa_brddstaddr;
		  sin->sin_addr.s_addr = (lip & lnp) | ~lnp;
		  sin->sin_family = AF_INET;
		  ifp->ifa_ifa.ifa_broadaddr = (struct sockaddr *)
					       &ifp->ifa_brddstaddr;
		}
	      /* Hardware address */
	      ; // Nothing to do... */
	      /* Metric */
	      ifp->ifa_metric = 1;
	      /* MTU */
	      ifp->ifa_mtu = 1500;
	      /* Interface index */
	      ifp->ifa_ifindex = -1;
	      /* Friendly name */
	      struct ifreq_frndlyname *iff = (struct ifreq_frndlyname *)
					     &ifp->ifa_frndlyname;
	      strcpy (iff->ifrf_friendlyname, bp);
	      iff->ifrf_len = strlen (iff->ifrf_friendlyname);
	    }
	}
      RegCloseKey (key);
    }
  /* Since every entry is set to the next entry, the last entry points to an
     invalid next entry now.  Fix it retroactively. */
  if (ifp > ifret)
    ifp->ifa_ifa.ifa_next = NULL;
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
  if (wincap.has_gaa_prefixes () && !CYGWIN_VERSION_CHECK_FOR_OLD_IFREQ)
    ifp = get_xp_ifs (AF_UNSPEC);
  else if (wincap.has_ip_helper_lib ())
    ifp = get_2k_ifs ();
  else
    ifp = get_nt_ifs ();
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
  sig_dispatch_pending ();
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;

  /* Ensure we have space for at least one struct ifreqs, fail if not. */
  if (ifc->ifc_len < (int) sizeof (struct ifreq))
    {
      set_errno (EINVAL);
      return -1;
    }

  struct ifall *ifret, *ifp;
  if (wincap.has_gaa_prefixes () && !CYGWIN_VERSION_CHECK_FOR_OLD_IFREQ)
    ifret = get_xp_ifs (AF_INET);
  else if (wincap.has_ip_helper_lib ())
    ifret = get_2k_ifs ();
  else
    ifret = get_nt_ifs ();
  if (!ifret)
    return -1;

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
	  memcpy (&ifr->ifr_hwaddr, &ifp->ifa_hwaddr, sizeof ifr->ifr_hwaddr);
	  break;
	case SIOCGIFMETRIC:
	  ifr->ifr_metric = ifp->ifa_metric;
	  break;
	case SIOCGIFMTU:
	  ifr->ifr_mtu = ifp->ifa_mtu;
	  break;
	case SIOCGIFINDEX:
	  ifr->ifr_ifindex = ifp->ifa_ifindex;
	  break;
	case SIOCGIFFRNDLYNAM:
	  memcpy (ifr->ifr_frndlyname, &ifp->ifa_frndlyname,
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

extern "C" unsigned
if_nametoindex (const char *name)
{
  PIP_ADAPTER_ADDRESSES pa0 = NULL, pap;
  unsigned index = 0;

  myfault efault;
  if (efault.faulted (EFAULT))
    return 0;

  if (wincap.has_gaa_prefixes ()
      && get_adapters_addresses (&pa0, AF_UNSPEC))
    {
      char lname[IF_NAMESIZE], *c;

      lname[0] = '\0';
      strncat (lname, name, IF_NAMESIZE - 1);
      if (lname[0] == '{' && (c = strchr (lname, ':')))
	*c = '\0';
      for (pap = pa0; pap; pap = pap->Next)
	if (strcasematch (lname, pap->AdapterName))
	  {
	    index = pap->Ipv6IfIndex ?: pap->IfIndex;
	    break;
	  }
      free (pa0);
    }
  return index;
}

extern "C" char *
if_indextoname (unsigned ifindex, char *ifname)
{
  PIP_ADAPTER_ADDRESSES pa0 = NULL, pap;
  char *name = NULL;

  myfault efault;
  if (efault.faulted (EFAULT))
    return NULL;

  if (wincap.has_gaa_prefixes ()
      && get_adapters_addresses (&pa0, AF_UNSPEC))
    {
      for (pap = pa0; pap; pap = pap->Next)
	if (ifindex == (pap->Ipv6IfIndex ?: pap->IfIndex))
	  {
	    /* Unfortunately the pre-Vista IPv6 stack has a distinct loopback
	       device with the same Ipv6IfIndex as the IfIndex of the IPv4
	       loopback device, but with a different adapter name.
	       For consistency with /proc/net/if_inet6, try to find the
	       IPv6 loopback device and use that adapter name instead.
	       We identify the loopback device by its IfIndex of 1. */
	    if (pap->IfIndex == 1 && pap->Ipv6IfIndex == 0)
	      for (PIP_ADAPTER_ADDRESSES pap2 = pa0; pap2; pap2 = pap2->Next)
		if (pap2->Ipv6IfIndex == 1)
		  {
		    pap = pap2;
		    break;
		  }
	    name = strcpy (ifname, pap->AdapterName);
	    break;
	  }
      free (pa0);
    }
  else
    set_errno (ENXIO);
  return name;
}

extern "C" struct if_nameindex *
if_nameindex (void)
{
  PIP_ADAPTER_ADDRESSES pa0 = NULL, pap;
  struct if_nameindex *iflist = NULL;
  char (*ifnamelist)[IF_NAMESIZE];

  myfault efault;
  if (efault.faulted (EFAULT))
    return NULL;

  if (wincap.has_gaa_prefixes ()
      && get_adapters_addresses (&pa0, AF_UNSPEC))
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
		if (iflist[i].if_index == (pap->Ipv6IfIndex ?: pap->IfIndex))
		  goto outer_loop;
	      iflist[cnt].if_index = pap->Ipv6IfIndex ?: pap->IfIndex;
	      strcpy (iflist[cnt].if_name = ifnamelist[cnt], pap->AdapterName);
	      /* See comment in if_indextoname. */
	      if (pap->IfIndex == 1 && pap->Ipv6IfIndex == 0)
		for (PIP_ADAPTER_ADDRESSES pap2 = pa0; pap2; pap2 = pap2->Next)
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
  return iflist;
}

extern "C" void
if_freenameindex (struct if_nameindex *ptr)
{
  myfault efault;
  if (efault.faulted (EFAULT))
    return;
  free (ptr);
}

#define PORT_LOW	(IPPORT_EFSSERVER + 1)
#define PORT_HIGH	(IPPORT_RESERVED - 1)
#define NUM_PORTS	(PORT_HIGH - PORT_LOW + 1)

extern "C" int
cygwin_bindresvport_sa (int fd, struct sockaddr *sa)
{
  struct sockaddr_storage sst;
  struct sockaddr_in *sin = NULL;
  struct sockaddr_in6 *sin6 = NULL;
  in_port_t port;
  socklen_t salen;
  int ret;

  sig_dispatch_pending ();

  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;

  fhandler_socket *fh = get (fd);
  if (!fh)
    return -1;

  if (!sa)
    {
      sa = (struct sockaddr *) &sst;
      memset (&sst, 0, sizeof sst);
      sa->sa_family = fh->get_addr_family ();
    }

  switch (sa->sa_family)
    {
    case AF_INET:
      salen = sizeof (struct sockaddr_in);
      sin = (struct sockaddr_in *) sa;
      port = sin->sin_port;
      break;
    case AF_INET6:
      salen = sizeof (struct sockaddr_in6);
      sin6 = (struct sockaddr_in6 *) sa;
      port = sin6->sin6_port;
      break;
    default:
      set_errno (EPFNOSUPPORT);
      return -1;
    }

  /* If a non-zero port number is given, try this first.  If that succeeds,
     or if the error message is serious, return. */
  if (port)
    {
      ret = fh->bind (sa, salen);
      if (!ret || (get_errno () != EADDRINUSE && get_errno () != EINVAL))
	return ret;
    }

  LONG myport;

  for (int i = 0; i < NUM_PORTS; i++)
    {
      while ((myport = InterlockedExchange (&cygwin_shared->last_used_bindresvport, -1)) == -1)
	low_priority_sleep (0);
      if (myport == 0 || --myport < PORT_LOW)
	myport = PORT_HIGH;
      InterlockedExchange (&cygwin_shared->last_used_bindresvport, myport);

      if (sa->sa_family == AF_INET6)
	sin6->sin6_port = htons (myport);
      else
	sin->sin_port = htons (myport);
      if (!(ret = fh->bind (sa, salen)))
	break;
      if (get_errno () != EADDRINUSE && get_errno () != EINVAL)
	break;
    }

  return ret;
}

extern "C" int
cygwin_bindresvport (int fd, struct sockaddr_in *sin)
{
  return cygwin_bindresvport_sa (fd, (struct sockaddr *) sin);
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
      sock_out.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
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
	((fhandler_socket *) sb0)->set_addr_family (family);
	((fhandler_socket *) sb0)->set_socket_type (type);
	((fhandler_socket *) sb0)->connect_state (connected);
	if (family == AF_LOCAL && type == SOCK_STREAM)
	  ((fhandler_socket *) sb0)->af_local_set_sockpair_cred ();

	cygheap_fdnew sb1 (sb0, false);

	if (sb1 >= 0 && fdsock (sb1, dev, outsock))
	  {
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

  fhandler_socket *fh = get (fd);

  myfault efault;
  if (efault.faulted (EFAULT) || !fh)
    res = -1;
  else
    {
      res = check_iovec_for_read (msg->msg_iov, msg->msg_iovlen);
      if (res > 0)
	res = fh->recvmsg (msg, flags);
    }

  syscall_printf ("%d = recvmsg (%d, %p, %x)", res, fd, msg, flags);
  return res;
}

/* exported as sendmsg: standards? */
extern "C" int
cygwin_sendmsg (int fd, const struct msghdr *msg, int flags)
{
  int res;

  fhandler_socket *fh = get (fd);

  myfault efault;
  if (efault.faulted (EFAULT) || !fh)
    res = -1;
  else
    {
      res = check_iovec_for_write (msg->msg_iov, msg->msg_iovlen);
      if (res >= 0)
	res = fh->sendmsg (msg, flags);
    }

  syscall_printf ("%d = sendmsg (%d, %p, %x)", res, fd, msg, flags);
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
inet_pton4 (const char *src, u_char *dst)
{
  static const char digits[] = "0123456789";
  int saw_digit, octets, ch;
  u_char tmp[INADDRSZ], *tp;

  saw_digit = 0;
  octets = 0;
  *(tp = tmp) = 0;
  while ((ch = *src++) != '\0')
    {
      const char *pch;

      if ((pch = strchr(digits, ch)) != NULL)
	{
	  u_int ret = *tp * 10 + (pch - digits);

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
inet_pton6 (const char *src, u_char *dst)
{
  static const char xdigits_l[] = "0123456789abcdef",
		    xdigits_u[] = "0123456789ABCDEF";
  u_char tmp[IN6ADDRSZ], *tp, *endp, *colonp;
  const char *xdigits, *curtok;
  int ch, saw_xdigit;
  u_int val;

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
	  *tp++ = (u_char) (val >> 8) & 0xff;
	  *tp++ = (u_char) val & 0xff;
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
      *tp++ = (u_char) (val >> 8) & 0xff;
      *tp++ = (u_char) val & 0xff;
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
      return (inet_pton4(src, (u_char *) dst));
    case AF_INET6:
      return (inet_pton6(src, (u_char *) dst));
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
 *	(2) takes a u_char* not an in_addr as input
 * author:
 *	Paul Vixie, 1996.
 */
static const char *
inet_ntop4 (const u_char *src, char *dst, size_t size)
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
inet_ntop6 (const u_char *src, char *dst, size_t size)
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
  u_int words[IN6ADDRSZ / INT16SZ];
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
      return (inet_ntop4((const u_char *) src, dst, size));
    case AF_INET6:
      return (inet_ntop6((const u_char *) src, dst, size));
    default:
      errno = EAFNOSUPPORT;
      return (NULL);
    }
  /* NOTREACHED */
}

/* W. Richard STEVENS libgai implementation, slightly tweaked for inclusion
   into Cygwin as pure IPv4 replacement.  Please note that the code is
   kept intact as much as possible.  Especially the IPv6 and AF_UNIX code
   is kept in, even though we can support neither of them.  Please don't
   activate them, they won't work correctly. */

#define IPv4
#undef IPv6
#undef UNIXdomain

#undef HAVE_SOCKADDR_SA_LEN
#define gethostbyname2(host,family) cygwin_gethostbyname((host))

#define AI_CLONE 0x8000		/* Avoid collision with AI_ values in netdb.h */

/*
 * Create and fill in an addrinfo{}.
 */

/* include ga_aistruct1 */
static int
ga_aistruct (struct addrinfo ***paipnext, const struct addrinfo *hintsp,
	     const void *addr, int family)
{
  struct addrinfo *ai;

  if ((ai = (struct addrinfo *) calloc (1, sizeof (struct addrinfo))) == NULL)
    return (EAI_MEMORY);
  ai->ai_next = NULL;
  ai->ai_canonname = NULL;
  **paipnext = ai;
  *paipnext = &ai->ai_next;

  if ((ai->ai_socktype = hintsp->ai_socktype) == 0)
    ai->ai_flags |= AI_CLONE;

  ai->ai_protocol = hintsp->ai_protocol;
/* end ga_aistruct1 */

/* include ga_aistruct2 */
  switch ((ai->ai_family = family))
    {
#ifdef	IPv4
    case AF_INET:
      {
	struct sockaddr_in *sinptr;

	/* 4allocate sockaddr_in{} and fill in all but port */
	if ((sinptr = (struct sockaddr_in *)
		      calloc (1, sizeof (struct sockaddr_in))) == NULL)
	  return (EAI_MEMORY);
#ifdef	HAVE_SOCKADDR_SA_LEN
	sinptr->sin_len = sizeof (struct sockaddr_in);
#endif
	sinptr->sin_family = AF_INET;
	memcpy (&sinptr->sin_addr, addr, sizeof (struct in_addr));
	ai->ai_addr = (struct sockaddr *) sinptr;
	ai->ai_addrlen = sizeof (struct sockaddr_in);
	break;
      }
#endif /* IPV4 */
#ifdef	IPv6
    case AF_INET6:
      {
	struct sockaddr_in6 *sin6ptr;

	/* 4allocate sockaddr_in6{} and fill in all but port */
	if ((sin6ptr = calloc (1, sizeof (struct sockaddr_in6))) == NULL)
	  return (EAI_MEMORY);
#ifdef	HAVE_SOCKADDR_SA_LEN
	sin6ptr->sin6_len = sizeof (struct sockaddr_in6);
#endif
	sin6ptr->sin6_family = AF_INET6;
	memcpy (&sin6ptr->sin6_addr, addr, sizeof (struct in6_addr));
	ai->ai_addr = (struct sockaddr *) sin6ptr;
	ai->ai_addrlen = sizeof (struct sockaddr_in6);
	break;
      }
#endif /* IPV6 */
#ifdef	UNIXdomain
    case AF_LOCAL:
      {
	struct sockaddr_un *unp;

	/* 4allocate sockaddr_un{} and fill in */
/* *INDENT-OFF* */
			if (strlen(addr) >= sizeof(unp->sun_path))
				return(EAI_SERVICE);
			if ( (unp = calloc(1, sizeof(struct sockaddr_un))) == NULL)
				return(EAI_MEMORY);
/* *INDENT-ON* */
	unp->sun_family = AF_LOCAL;
	strcpy (unp->sun_path, addr);
#ifdef	HAVE_SOCKADDR_SA_LEN
	unp->sun_len = SUN_LEN (unp);
#endif
	ai->ai_addr = (struct sockaddr *) unp;
	ai->ai_addrlen = sizeof (struct sockaddr_un);
	if (hintsp->ai_flags & AI_PASSIVE)
	  unlink (unp->sun_path);	/* OK if this fails */
	break;
      }
#endif /* UNIXDOMAIN */
    }
  return (0);
}

/* end ga_aistruct2 */

/*
 * Clone a new addrinfo structure from an existing one.
 */

/* include ga_clone */

/* Cygwin specific: The ga_clone function is split up to allow an easy
   duplication of addrinfo structs.  This is used to duplicate the
   structures from Winsock, so that we have the allocation of the structs
   returned to the application under control.  This is especially helpful
   for the AI_V4MAPPED case prior to Vista. */
static struct addrinfo *
ga_dup (struct addrinfo *ai, bool v4mapped)
{
  struct addrinfo *nai;

  if ((nai = (struct addrinfo *) calloc (1, sizeof (struct addrinfo))) == NULL)
    return (NULL);

  nai->ai_flags = 0;		/* make sure AI_CLONE is off */
  nai->ai_family = v4mapped ? AF_INET6 : ai->ai_family;
  nai->ai_socktype = ai->ai_socktype;
  nai->ai_protocol = ai->ai_protocol;
  nai->ai_canonname = NULL;
  if (!(ai->ai_flags & AI_CLONE) && ai->ai_canonname
      && !(nai->ai_canonname = strdup (ai->ai_canonname)))
    {
      free (nai);
      return NULL;
    }
  nai->ai_addrlen = v4mapped ? sizeof (struct sockaddr_in6) : ai->ai_addrlen;
  if ((nai->ai_addr = (struct sockaddr *) malloc (v4mapped
						  ? sizeof (struct sockaddr_in6)
						  : ai->ai_addrlen)) == NULL)
    {
      if (nai->ai_canonname)
	free (nai->ai_canonname);
      free (nai);
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
ga_clone (struct addrinfo *ai)
{
  struct addrinfo *nai;

  if ((nai = ga_dup (ai, false)))
    {
      nai->ai_next = ai->ai_next;
      ai->ai_next = nai;
    }
  return nai;
}

static struct addrinfo *
ga_duplist (struct addrinfo *ai, bool v4mapped)
{
  void ipv4_freeaddrinfo (struct addrinfo *aihead);
  struct addrinfo *tmp, *nai = NULL, *nai0 = NULL;

  for (; ai; ai = ai->ai_next, nai = tmp)
    {
      if (!(tmp = ga_dup (ai, v4mapped)))
	goto bad;
      if (!nai0)
	nai0 = tmp;
      if (nai)
	nai->ai_next = tmp;
    }
  return nai0;

bad:
  ipv4_freeaddrinfo (nai0);
  return NULL;
}

/* end ga_clone */

/*
 * Basic error checking of flags, family, socket type, and protocol.
 */

/* include ga_echeck */
static int
ga_echeck (const char *hostname, const char *servname,
	   int flags, int family, int socktype, int protocol)
{
#if 0
  if (flags & ~(AI_PASSIVE | AI_CANONNAME))
    return (EAI_BADFLAGS);	/* unknown flag bits */
#endif
  if (hostname == NULL || hostname[0] == '\0')
    {
      if (servname == NULL || servname[0] == '\0')
	return (EAI_NONAME);	/* host or service must be specified */
    }

  switch (family)
    {
    case AF_UNSPEC:
      break;
#ifdef	IPv4
    case AF_INET:
      if (socktype != 0 &&
	  (socktype != SOCK_STREAM &&
	   socktype != SOCK_DGRAM && socktype != SOCK_RAW))
	return (EAI_SOCKTYPE);	/* invalid socket type */
      break;
#endif
#ifdef	IPv6
    case AF_INET6:
      if (socktype != 0 &&
	  (socktype != SOCK_STREAM &&
	   socktype != SOCK_DGRAM && socktype != SOCK_RAW))
	return (EAI_SOCKTYPE);	/* invalid socket type */
      break;
#endif
#ifdef	UNIXdomain
    case AF_LOCAL:
      if (socktype != 0 &&
	  (socktype != SOCK_STREAM && socktype != SOCK_DGRAM))
	return (EAI_SOCKTYPE);	/* invalid socket type */
      break;
#endif
    default:
      return (EAI_FAMILY);	/* unknown protocol family */
    }
  return (0);
}

/* end ga_echeck */

struct search {
  const char *host;  /* hostname or address string */
  int        family; /* AF_xxx */
};

/*
 * Set up the search[] array with the hostnames and address families
 * that we are to look up.
 */

/* include ga_nsearch1 */
static int
ga_nsearch (const char *hostname, const struct addrinfo *hintsp,
	    struct search *search)
{
  int nsearch = 0;

  if (hostname == NULL || hostname[0] == '\0')
    {
      if (hintsp->ai_flags & AI_PASSIVE)
	{
	  /* 4no hostname and AI_PASSIVE: implies wildcard bind */
	  switch (hintsp->ai_family)
	    {
#ifdef	IPv4
	    case AF_INET:
	      search[nsearch].host = "0.0.0.0";
	      search[nsearch].family = AF_INET;
	      nsearch++;
	      break;
#endif
#ifdef	IPv6
	    case AF_INET6:
	      search[nsearch].host = "0::0";
	      search[nsearch].family = AF_INET6;
	      nsearch++;
	      break;
#endif
	    case AF_UNSPEC:
#ifdef	IPv6
	      search[nsearch].host = "0::0";	/* IPv6 first, then IPv4 */
	      search[nsearch].family = AF_INET6;
	      nsearch++;
#endif
#ifdef	IPv4
	      search[nsearch].host = "0.0.0.0";
	      search[nsearch].family = AF_INET;
	      nsearch++;
#endif
	      break;
	    }
/* end ga_nsearch1 */
/* include ga_nsearch2 */
	}
      else
	{
	  /* 4no host and not AI_PASSIVE: connect to local host */
	  switch (hintsp->ai_family)
	    {
#ifdef	IPv4
	    case AF_INET:
	      search[nsearch].host = "localhost";	/* 127.0.0.1 */
	      search[nsearch].family = AF_INET;
	      nsearch++;
	      break;
#endif
#ifdef	IPv6
	    case AF_INET6:
	      search[nsearch].host = "0::1";
	      search[nsearch].family = AF_INET6;
	      nsearch++;
	      break;
#endif
	    case AF_UNSPEC:
#ifdef	IPv6
	      search[nsearch].host = "0::1";	/* IPv6 first, then IPv4 */
	      search[nsearch].family = AF_INET6;
	      nsearch++;
#endif
#ifdef	IPv4
	      search[nsearch].host = "localhost";
	      search[nsearch].family = AF_INET;
	      nsearch++;
#endif
	      break;
	    }
	}
/* end ga_nsearch2 */
/* include ga_nsearch3 */
    }
  else
    {				/* host is specified */
      switch (hintsp->ai_family)
	{
#ifdef	IPv4
	case AF_INET:
	  search[nsearch].host = hostname;
	  search[nsearch].family = AF_INET;
	  nsearch++;
	  break;
#endif
#ifdef	IPv6
	case AF_INET6:
	  search[nsearch].host = hostname;
	  search[nsearch].family = AF_INET6;
	  nsearch++;
	  break;
#endif
	case AF_UNSPEC:
#ifdef	IPv6
	  search[nsearch].host = hostname;
	  search[nsearch].family = AF_INET6;	/* IPv6 first */
	  nsearch++;
#endif
#ifdef	IPv4
	  search[nsearch].host = hostname;
	  search[nsearch].family = AF_INET;	/* then IPv4 */
	  nsearch++;
#endif
	  break;
	}
    }
  if (nsearch < 1 || nsearch > 2)
    return -1;
  return (nsearch);
}

/* end ga_nsearch3 */

/*
 * Go through all the addrinfo structures, checking for a match of the
 * socket type and filling in the socket type, and then the port number
 * in the corresponding socket address structures.
 *
 * The AI_CLONE flag works as follows.  Consider a multihomed host with
 * two IP addresses and no socket type specified by the caller.  After
 * the "host" search there are two addrinfo structures, one per IP address.
 * Assuming a service supported by both TCP and UDP (say the daytime
 * service) we need to return *four* addrinfo structures:
 *		IP#1, SOCK_STREAM, TCP port,
 *		IP#1, SOCK_DGRAM, UDP port,
 *		IP#2, SOCK_STREAM, TCP port,
 *		IP#2, SOCK_DGRAM, UDP port.
 * To do this, when the "host" loop creates an addrinfo structure, if the
 * caller has not specified a socket type (hintsp->ai_socktype == 0), the
 * AI_CLONE flag is set.  When the following function finds an entry like
 * this it is handled as follows: If the entry's ai_socktype is still 0,
 * this is the first use of the structure, and the ai_socktype field is set.
 * But, if the entry's ai_socktype is nonzero, then we clone a new addrinfo
 * structure and set it's ai_socktype to the new value.  Although we only
 * need two socket types today (SOCK_STREAM and SOCK_DGRAM) this algorithm
 * will handle any number.  Also notice that Posix.1g requires all socket
 * types to be nonzero.
 */

/* include ga_port */
static int
ga_port (struct addrinfo *aihead, int port, int socktype)
		/* port must be in network byte order */
{
  int nfound = 0;
  struct addrinfo *ai;

  for (ai = aihead; ai != NULL; ai = ai->ai_next)
    {
      if (ai->ai_flags & AI_CLONE)
	{
	  if (ai->ai_socktype != 0)
	    {
	      if ((ai = ga_clone (ai)) == NULL)
		return (-1);	/* memory allocation error */
	      /* ai points to newly cloned entry, which is what we want */
	    }
	}
      else if (ai->ai_socktype != socktype)
	continue;		/* ignore if mismatch on socket type */

      ai->ai_socktype = socktype;

      switch (ai->ai_family)
	{
#ifdef	IPv4
	case AF_INET:
	  ((struct sockaddr_in *) ai->ai_addr)->sin_port = port;
	  nfound++;
	  break;
#endif
#ifdef	IPv6
	case AF_INET6:
	  ((struct sockaddr_in6 *) ai->ai_addr)->sin6_port = port;
	  nfound++;
	  break;
#endif
	}
    }
  return (nfound);
}

/* end ga_port */

/*
 * This function handles the service string.
 */

/* include ga_serv */
static int
ga_serv (struct addrinfo *aihead, const struct addrinfo *hintsp,
	 const char *serv)
{
  int port, rc, nfound;
  struct servent *sptr;

  nfound = 0;
  if (isdigit (serv[0]))
    {				/* check for port number string first */
      port = htons (atoi (serv));
      if (hintsp->ai_socktype)
	{
	  /* 4caller specifies socket type */
	  if ((rc = ga_port (aihead, port, hintsp->ai_socktype)) < 0)
	    return (EAI_MEMORY);
	  nfound += rc;
	}
      else
	{
	  /* 4caller does not specify socket type */
	  if ((rc = ga_port (aihead, port, SOCK_STREAM)) < 0)
	    return (EAI_MEMORY);
	  nfound += rc;
	  if ((rc = ga_port (aihead, port, SOCK_DGRAM)) < 0)
	    return (EAI_MEMORY);
	  nfound += rc;
	}
    }
  else
    {
      /* 4try service name, TCP then UDP */
      if (hintsp->ai_socktype == 0 || hintsp->ai_socktype == SOCK_STREAM)
	{
	  if ((sptr = cygwin_getservbyname (serv, "tcp")) != NULL)
	    {
	      if ((rc = ga_port (aihead, sptr->s_port, SOCK_STREAM)) < 0)
		return (EAI_MEMORY);
	      nfound += rc;
	    }
	}
      if (hintsp->ai_socktype == 0 || hintsp->ai_socktype == SOCK_DGRAM)
	{
	  if ((sptr = cygwin_getservbyname (serv, "udp")) != NULL)
	    {
	      if ((rc = ga_port (aihead, sptr->s_port, SOCK_DGRAM)) < 0)
		return (EAI_MEMORY);
	      nfound += rc;
	    }
	}
    }

  if (nfound == 0)
    {
      if (hintsp->ai_socktype == 0)
	return (EAI_NONAME);	/* all calls to getservbyname() failed */
      else
	return (EAI_SERVICE);	/* service not supported for socket type */
    }
  return (0);
}

/* end ga_serv */

#ifdef	UNIXdomain
/* include ga_unix */
static int
ga_unix (const char *path, struct addrinfo *hintsp, struct addrinfo **result)
{
  int rc;
  struct addrinfo *aihead, **aipnext;

  aihead = NULL;
  aipnext = &aihead;

  if (hintsp->ai_family != AF_UNSPEC && hintsp->ai_family != AF_LOCAL)
    return (EAI_ADDRFAMILY);

  if (hintsp->ai_socktype == 0)
    {
      /* 4no socket type specified: return stream then dgram */
      hintsp->ai_socktype = SOCK_STREAM;
      if ((rc = ga_aistruct (&aipnext, hintsp, path, AF_LOCAL)) != 0)
	return (rc);
      hintsp->ai_socktype = SOCK_DGRAM;
    }

  if ((rc = ga_aistruct (&aipnext, hintsp, path, AF_LOCAL)) != 0)
    return (rc);

  if (hintsp->ai_flags & AI_CANONNAME)
    {
      struct utsname myname;

      if (uname (&myname) < 0)
	return (EAI_SYSTEM);
      if ((aihead->ai_canonname = strdup (myname.nodename)) == NULL)
	return (EAI_MEMORY);
    }

  *result = aihead;		/* pointer to first structure in linked list */
  return (0);
}

/* end ga_unix */
#endif /* UNIXdomain */

/* include gn_ipv46 */
static int
gn_ipv46 (char *host, size_t hostlen, char *serv, size_t servlen,
	  void *aptr, size_t alen, int family, int port, int flags)
{
  char *ptr;
  struct hostent *hptr;
  struct servent *sptr;

  if (host && hostlen > 0)
    {
      if (flags & NI_NUMERICHOST)
	{
	  if (cygwin_inet_ntop (family, aptr, host, hostlen) == NULL)
	    return (1);
	}
      else
	{
	  hptr = cygwin_gethostbyaddr ((const char *) aptr, alen, family);
	  if (hptr != NULL && hptr->h_name != NULL)
	    {
	      if (flags & NI_NOFQDN)
		{
		  if ((ptr = strchr (hptr->h_name, '.')) != NULL)
		    *ptr = 0;	/* overwrite first dot */
		}
	      //snprintf (host, hostlen, "%s", hptr->h_name);
	      *host = '\0';
	      strncat (host, hptr->h_name, hostlen - 1);
	    }
	  else
	    {
	      if (flags & NI_NAMEREQD)
		return (1);
	      if (cygwin_inet_ntop (family, aptr, host, hostlen) == NULL)
		return (1);
	    }
	}
    }

  if (serv && servlen > 0)
    {
      if (flags & NI_NUMERICSERV)
	{
	  //snprintf (serv, servlen, "%d", ntohs (port));
	  char buf[32];
	  __small_sprintf (buf, "%d", ntohs (port));
	  *serv = '\0';
	  strncat (serv, buf, servlen - 1);
	}
      else
	{
	  sptr = cygwin_getservbyport (port, (flags & NI_DGRAM) ? "udp" : NULL);
	  if (sptr != NULL && sptr->s_name != NULL)
	    {
	      //snprintf (serv, servlen, "%s", sptr->s_name);
	      *serv = '\0';
	      strncat (serv, sptr->s_name, servlen - 1);
	    }
	  else
	    {
	      //snprintf (serv, servlen, "%d", ntohs (port));
	      char buf[32];
	      __small_sprintf (buf, "%d", ntohs (port));
	      *serv = '\0';
	      strncat (serv, buf, servlen - 1);
	    }
	}
    }
  return (0);
}

/* end gn_ipv46 */

/* include freeaddrinfo */
void
ipv4_freeaddrinfo (struct addrinfo *aihead)
{
  struct addrinfo *ai, *ainext;

  for (ai = aihead; ai != NULL; ai = ainext)
    {
      if (ai->ai_addr != NULL)
	free (ai->ai_addr);	/* socket address structure */

      if (ai->ai_canonname != NULL)
	free (ai->ai_canonname);

      ainext = ai->ai_next;	/* can't fetch ai_next after free() */
      free (ai);		/* the addrinfo{} itself */
    }
}

/* end freeaddrinfo */

/* include ga1 */

int
ipv4_getaddrinfo (const char *hostname, const char *servname,
		  const struct addrinfo *hintsp, struct addrinfo **result)
{
  int rc, error, nsearch;
  char **ap, *canon;
  struct hostent *hptr;
  struct search search[3], *sptr;
  struct addrinfo hints, *aihead, **aipnext;

  /*
   * If we encounter an error we want to free() any dynamic memory
   * that we've allocated.  This is our hack to simplify the code.
   */
#define	error(e) { error = (e); goto bad; }

  aihead = NULL;		/* initialize automatic variables */
  aipnext = &aihead;
  canon = NULL;

  if (hintsp == NULL)
    {
      bzero (&hints, sizeof (hints));
      hints.ai_family = AF_UNSPEC;
    }
  else
    hints = *hintsp;		/* struct copy */

  /* 4first some basic error checking */
  if ((rc = ga_echeck (hostname, servname, hints.ai_flags, hints.ai_family,
		       hints.ai_socktype, hints.ai_protocol)) != 0)
    error (rc);

#ifdef	UNIXdomain
  /* 4special case Unix domain first */
  if (hostname != NULL &&
      (strcmp (hostname, "/local") == 0 || strcmp (hostname, "/unix") == 0) &&
      (servname != NULL && servname[0] == '/'))
    return (ga_unix (servname, &hints, result));
#endif
/* end ga1 */

/* include ga3 */
  /* 4remainder of function for IPv4/IPv6 */
  nsearch = ga_nsearch (hostname, &hints, &search[0]);
  if (nsearch == -1)
    error (EAI_FAMILY);
  for (sptr = &search[0]; sptr < &search[nsearch]; sptr++)
    {
#ifdef	IPv4
      /* 4check for an IPv4 dotted-decimal string */
      if (isdigit (sptr->host[0]))
	{
	  struct in_addr inaddr;

	  if (inet_pton4 (sptr->host, (u_char *) &inaddr) == 1)
	    {
	      if (hints.ai_family != AF_UNSPEC && hints.ai_family != AF_INET)
		error (EAI_ADDRFAMILY);
	      if (sptr->family != AF_INET)
		continue;	/* ignore */
	      rc = ga_aistruct (&aipnext, &hints, &inaddr, AF_INET);
	      if (rc != 0)
		error (rc);
	      continue;
	    }
	}
#endif

#ifdef	IPv6
      /* 4check for an IPv6 hex string */
      if ((isxdigit (sptr->host[0]) || sptr->host[0] == ':') &&
	  (strchr (sptr->host, ':') != NULL))
	{
	  struct in6_addr in6addr;

	  if (inet_pton6 (sptr->host, &in6addr) == 1)
	    {
	      if (hints.ai_family != AF_UNSPEC && hints.ai_family != AF_INET6)
		error (EAI_ADDRFAMILY);
	      if (sptr->family != AF_INET6)
		continue;	/* ignore */
	      rc = ga_aistruct (&aipnext, &hints, &in6addr, AF_INET6);
	      if (rc != 0)
		error (rc);
	      continue;
	    }
	}
#endif
/* end ga3 */
/* include ga4 */
#ifdef	IPv6
      /* 4remainder of for() to look up hostname */
      if ((_res.options & RES_INIT) == 0)
	res_init ();		/* need this to set _res.options */
#endif

      if (nsearch == 2)
	{
#ifdef	IPv6
	  _res.options &= ~RES_USE_INET6;
#endif
	  hptr = gethostbyname2 (sptr->host, sptr->family);
	}
      else
	{
#ifdef  IPv6
	  if (sptr->family == AF_INET6)
	    _res.options |= RES_USE_INET6;
	  else
	    _res.options &= ~RES_USE_INET6;
#endif
	  hptr = gethostbyname (sptr->host);
	}
      if (hptr == NULL)
	{
	  if (nsearch == 2)
	    continue;		/* failure OK if multiple searches */

	  switch (h_errno)
	    {
	    case HOST_NOT_FOUND:
	      error (EAI_NONAME);
	    case TRY_AGAIN:
	      error (EAI_AGAIN);
	    case NO_RECOVERY:
	      error (EAI_FAIL);
	    case NO_DATA:
	      error (EAI_NODATA);
	    default:
	      error (EAI_NONAME);
	    }
	}

      /* 4check for address family mismatch if one specified */
      if (hints.ai_family != AF_UNSPEC && hints.ai_family != hptr->h_addrtype)
	error (EAI_ADDRFAMILY);

      /* 4save canonical name first time */
      if (hostname != NULL && hostname[0] != '\0' &&
	  (hints.ai_flags & AI_CANONNAME) && canon == NULL)
	{
	  if ((canon = strdup (hptr->h_name)) == NULL)
	    error (EAI_MEMORY);
	}

      /* 4create one addrinfo{} for each returned address */
      for (ap = hptr->h_addr_list; *ap != NULL; ap++)
	{
	  rc = ga_aistruct (&aipnext, &hints, *ap, hptr->h_addrtype);
	  if (rc != 0)
	    error (rc);
	}
    }
  if (aihead == NULL)
    error (EAI_NONAME);		/* nothing found */
/* end ga4 */

/* include ga5 */
  /* 4return canonical name */
  if (hostname != NULL && hostname[0] != '\0' &&
      hints.ai_flags & AI_CANONNAME)
    {
      if (canon != NULL)
	aihead->ai_canonname = canon;	/* strdup'ed earlier */
      else
	{
	  if ((aihead->ai_canonname = strdup (search[0].host)) == NULL)
	    error (EAI_MEMORY);
	}
    }

  /* 4now process the service name */
  if (servname != NULL && servname[0] != '\0')
    {
      if ((rc = ga_serv (aihead, &hints, servname)) != 0)
	error (rc);
    }

  *result = aihead;		/* pointer to first structure in linked list */
  return (0);

bad:
  ipv4_freeaddrinfo (aihead);	/* free any alloc'ed memory */
  return (error);
}

/* end ga5 */

/* include getnameinfo */
int
ipv4_getnameinfo (const struct sockaddr *sa, socklen_t salen,
		  char *host, size_t hostlen,
		  char *serv, size_t servlen, int flags)
{

  switch (sa->sa_family)
    {
#ifdef	IPv4
    case AF_INET:
      {
	struct sockaddr_in *sain = (struct sockaddr_in *) sa;

	return (gn_ipv46 (host, hostlen, serv, servlen,
			  &sain->sin_addr, sizeof (struct in_addr),
			  AF_INET, sain->sin_port, flags));
      }
#endif

#ifdef	IPv6
    case AF_INET6:
      {
	struct sockaddr_in6 *sain = (struct sockaddr_in6 *) sa;

	return (gn_ipv46 (host, hostlen, serv, servlen,
			  &sain->sin6_addr, sizeof (struct in6_addr),
			  AF_INET6, sain->sin6_port, flags));
      }
#endif

#ifdef	UNIXdomain
    case AF_LOCAL:
      {
	struct sockaddr_un *un = (struct sockaddr_un *) sa;

	if (hostlen > 0)
	  snprintf (host, hostlen, "%s", "/local");
	if (servlen > 0)
	  snprintf (serv, servlen, "%s", un->sun_path);
	return (0);
      }
#endif

    default:
      return (EAI_FAMILY);
    }
}

/* end getnameinfo */

/* Start of cygwin specific wrappers around the gai functions. */

struct gai_errmap_t
{
  int w32_errval;
  const char *errtxt;
};

static gai_errmap_t gai_errmap[] =
{
  {0,			  "Success"},
  {0,			  "Address family for hostname not supported"},
  {WSATRY_AGAIN,	  "Temporary failure in name resolution"},
  {WSAEINVAL,		  "Invalid value for ai_flags"},
  {WSANO_RECOVERY,	  "Non-recoverable failure in name resolution"},
  {WSAEAFNOSUPPORT,	  "ai_family not supported"},
  {WSA_NOT_ENOUGH_MEMORY, "Memory allocation failure"},
  {WSANO_DATA,		  "No address associated with hostname"},
  {WSAHOST_NOT_FOUND,	  "hostname nor servname provided, or not known"},
  {WSATYPE_NOT_FOUND,	  "servname not supported for ai_socktype"},
  {WSAESOCKTNOSUPPORT,	  "ai_socktype not supported"},
  {0,			  "System error returned in errno"},
  {0,			  "Invalid value for hints"},
  {0,			  "Resolved protocol is unknown"},
  {WSAEFAULT,		  "An argument buffer overflowed"}
};

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

/* We can't use autoload here because we don't know where the functions
   are loaded from.  On Win2K, the functions are available in the
   ipv6 technology preview lib called wship6.dll, in XP and above they
   are implemented in ws2_32.dll.  For older systems we use the ipv4-only
   version above. */

static void (WINAPI *freeaddrinfo)(const struct addrinfo *);
static int (WINAPI *getaddrinfo)(const char *, const char *,
				  const struct addrinfo *,
				  struct addrinfo **);
static int (WINAPI *getnameinfo)(const struct sockaddr *, socklen_t,
				  char *, size_t, char *, size_t, int);
static bool
get_ipv6_funcs (HMODULE lib)
{
  return ((freeaddrinfo = (void (WINAPI *)(const struct addrinfo *))
			  GetProcAddress (lib, "freeaddrinfo"))
	  && (getaddrinfo = (int (WINAPI *)(const char *, const char *,
					    const struct addrinfo *,
					    struct addrinfo **))
			    GetProcAddress (lib, "getaddrinfo"))
	  && (getnameinfo = (int (WINAPI *)(const struct sockaddr *,
					    socklen_t, char *, size_t,
					    char *, size_t, int))
			    GetProcAddress (lib, "getnameinfo")));
}

static NO_COPY muto load_ipv6_guard;
static NO_COPY bool ipv6_inited = false;
#define load_ipv6()	if (!ipv6_inited) load_ipv6_funcs ();

static void
load_ipv6_funcs ()
{
  tmp_pathbuf tp;
  PWCHAR lib_name = tp.w_get ();
  size_t len;
  HMODULE lib;

  load_ipv6_guard.init ("klog_guard")->acquire ();
  if (ipv6_inited)
    goto out;
  WSAGetLastError ();	/* Kludge.  Enforce WSAStartup call. */
  if (GetSystemDirectoryW (lib_name, NT_MAX_PATH))
    {
      len = wcslen (lib_name);
      wcpcpy (lib_name + len, L"\\ws2_32.dll");
      if ((lib = LoadLibraryW (lib_name)))
	{
	  if (get_ipv6_funcs (lib))
	    goto out;
	  FreeLibrary (lib);
	}
      wcpcpy (lib_name + len, L"\\wship6.dll");
      if ((lib = LoadLibraryW (lib_name)))
	{
	  if (get_ipv6_funcs (lib))
	    goto out;
	  FreeLibrary (lib);
	}
      freeaddrinfo = NULL;
      getaddrinfo = NULL;
      getnameinfo = NULL;
    }
out:
  ipv6_inited = true;
  load_ipv6_guard.release ();
}

extern "C" void
cygwin_freeaddrinfo (struct addrinfo *addr)
{
  myfault efault;
  if (efault.faulted (EFAULT))
    return;
  ipv4_freeaddrinfo (addr);
}

extern "C" int
cygwin_getaddrinfo (const char *hostname, const char *servname,
		    const struct addrinfo *hints, struct addrinfo **res)
{
  myfault efault;
  if (efault.faulted (EFAULT))
    return EAI_SYSTEM;
  /* Both subsequent getaddrinfo implementations let all possible values
     in ai_flags slip through and just ignore unknowen values.  So we have
     to check manually here. */
  if (hints && (hints->ai_flags
		& ~(AI_PASSIVE | AI_CANONNAME | AI_NUMERICHOST | AI_ALL
		    | AI_NUMERICSERV | AI_ADDRCONFIG | AI_V4MAPPED)))
    return EAI_BADFLAGS;
  /* AI_NUMERICSERV is not supported in our replacement getaddrinfo, nor
     is it supported by Winsock prior to Vista.  We just check the servname
     parameter by ourselves here. */
  if (hints && (hints->ai_flags & AI_NUMERICSERV))
    {
      char *p;
      if (servname && *servname && (strtoul (servname, &p, 10), *p))
	return EAI_NONAME;
    }
  load_ipv6 ();
  if (!getaddrinfo)
    return ipv4_getaddrinfo (hostname, servname, hints, res);

  struct addrinfo nhints, *dupres;

  /* AI_ADDRCONFIG is not supported prior to Vista.  Rather it's
     the default and only possible setting.
     On Vista, the default behaviour is as if AI_ADDRCONFIG is set,
     apparently for performance reasons.  To get the POSIX default
     behaviour, the AI_ALL flag has to be set. */
  if (wincap.supports_all_posix_ai_flags ()
      && hints && hints->ai_family == PF_UNSPEC)
    {
      nhints = *hints;
      hints = &nhints;
      nhints.ai_flags |= AI_ALL;
    }
  int ret = w32_to_gai_err (getaddrinfo (hostname, servname, hints, res));
  /* Always copy over to self-allocated memory. */
  if (!ret)
    {
      dupres = ga_duplist (*res, false);
      freeaddrinfo (*res);
      *res = dupres;
      if (!dupres)
	return EAI_MEMORY;
    }
  /* AI_V4MAPPED and AI_ALL are not supported prior to Vista.  So, what
     we do here is to emulate AI_V4MAPPED.  If no IPv6 addresses are
     returned, or the AI_ALL flag is set, we try with AF_INET again, and
     convert the returned IPv4 addresses into v4-in-v6 entries.  This
     is done in ga_dup if the v4mapped flag is set. */
  if (!wincap.supports_all_posix_ai_flags ()
      && hints
      && hints->ai_family == AF_INET6
      && (hints->ai_flags & AI_V4MAPPED)
      && (ret == EAI_NODATA || ret == EAI_NONAME
	  || (hints->ai_flags & AI_ALL)))
    {
      struct addrinfo *v4res;
      nhints = *hints;
      nhints.ai_family = AF_INET;
      int ret2 = w32_to_gai_err (getaddrinfo (hostname, servname,
					      &nhints, &v4res));
      if (!ret2)
	{
	  dupres = ga_duplist (v4res, true);
	  freeaddrinfo (v4res);
	  if (!dupres)
	    {
	      if (!ret)
		ipv4_freeaddrinfo (*res);
	      return EAI_MEMORY;
	    }
	  /* If a list of v6 addresses exists, append the v4-in-v6 address
	     list.  Otherwise just return the v4-in-v6 address list. */
	  if (!ret)
	    {
	      struct addrinfo *ptr;
	      for (ptr = *res; ptr->ai_next; ptr = ptr->ai_next)
		;
	      ptr->ai_next = dupres;
	    }
	  else
	    *res = dupres;
	  ret = 0;
	}
    }
  return ret;
}

extern "C" int
cygwin_getnameinfo (const struct sockaddr *sa, socklen_t salen,
		    char *host, size_t hostlen, char *serv,
		    size_t servlen, int flags)
{
  myfault efault;
  if (efault.faulted (EFAULT))
    return EAI_SYSTEM;
  load_ipv6 ();
  if (!getnameinfo)
    return ipv4_getnameinfo (sa, salen, host, hostlen, serv, servlen, flags);

  /* When the incoming port number does not resolve to a well-known service,
     Winsock's getnameinfo up to Windows 2003 returns with error WSANO_DATA
     instead of setting `serv' to the numeric port number string, as required
     by RFC 3493.  This is fixed on Vista and later.  To avoid the error on
     systems up to Windows 2003, we check if the port number resolves
     to a well-known service.  If not, we set the NI_NUMERICSERV flag. */
  if (!wincap.supports_all_posix_ai_flags ())
    {
      int port = 0;

      switch (sa->sa_family)
	{
	case AF_INET:
	  port = ((struct sockaddr_in *) sa)->sin_port;
	  break;
	case AF_INET6:
	  port = ((struct sockaddr_in6 *) sa)->sin6_port;
	  break;
	}
      if (!port || !getservbyport (port, flags & NI_DGRAM ? "udp" : "tcp"))
	flags |= NI_NUMERICSERV;
    }
  int ret = w32_to_gai_err (getnameinfo (sa, salen, host, hostlen, serv,
					 servlen, flags));
  if (ret)
    set_winsock_errno ();
  return ret;
}

/* The below function in6_are_prefix_equal has been taken from OpenBSD's
   src/sys/netinet6/in6.c. */

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
in6_are_prefix_equal (struct in6_addr *p1, struct in6_addr *p2, int len)
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

/* These functions are stick to the end of this file so that the
   optimization in asm/byteorder.h can be used even here in net.cc. */

#undef htonl
#undef ntohl
#undef htons
#undef ntohs

/* htonl: standards? */
extern "C" uint32_t
htonl (uint32_t x)
{
  return __htonl (x);
}

/* ntohl: standards? */
extern "C" uint32_t
ntohl (uint32_t x)
{
  return __ntohl (x);
}

/* htons: standards? */
extern "C" uint16_t
htons (uint16_t x)
{
  return __htons (x);
}

/* ntohs: standards? */
extern "C" uint16_t
ntohs (uint16_t x)
{
  return __ntohs (x);
}

/* net.cc: network-related routines.

   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
   2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* #define DEBUG_NEST_ON 1 */

#define  __INSIDE_CYGWIN_NET__
#define USE_SYS_TYPES_FD_SET
#define __WSA_ERR_MACROS_DEFINED
/* FIXME: Collision with different declarations of if_nametoindex and
	  if_indextoname functions in iphlpapi.h since Vista.
   TODO:  Convert if_nametoindex to cygwin_if_nametoindex and call
	  system functions on Vista and later. */
#define _INC_NETIOAPI
#include "winsup.h"
#ifdef __x86_64__
/* 2014-04-24: Current Mingw headers define sockaddr_in6 using u_long (8 byte)
   because a redefinition for LP64 systems is missing.  This leads to a wrong
   definition and size of sockaddr_in6 when building with winsock headers.
   This definition is also required to use the right u_long type in subsequent
   function calls. */
#undef u_long
#define u_long __ms_u_long
#endif
#include <ws2tcpip.h>
#include <mswsock.h>
#include <iphlpapi.h>
#include "miscfuncs.h"
#include <ctype.h>
#include <wchar.h>
#include <stdlib.h>
#define gethostname cygwin_gethostname
#include <unistd.h>
#undef gethostname
#include <netdb.h>
#include <cygwin/in.h>
#include <asm/byteorder.h>
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
#include "ifaddrs.h"
#include "tls_pbuf.h"
#include "ntdll.h"

/* Unfortunately defined in Windows header files and arpa/nameser_compat.h. */
#undef NOERROR
#undef DELETE
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

static const struct tl errmap[] = {
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
  syscall_printf ("%s:%d - winsock error %u -> errno %d", fn, ln, werr, err);
}

/*
 * Since the member `s' isn't used for debug output we can use it
 * for the error text returned by herror and hstrerror.
 */
static const struct tl host_errmap[] = {
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

#ifndef SIO_BASE_HANDLE
#define SIO_BASE_HANDLE _WSAIOR(IOC_WS2,34)
#endif

bool
fdsock (cygheap_fdmanip& fd, const device *dev, SOCKET soc)
{
  int size;

  fd = build_fh_dev (*dev);
  if (!fd.isopen ())
    return false;

  /* Usually sockets are inheritable IFS objects.  Unfortunately some virus
     scanners or other network-oriented software replace normal sockets
     with their own kind, which is running through a filter driver called
     "layered service provider" (LSP).

     LSP sockets are not kernel objects.  They are typically not marked as
     inheritable, nor are they IFS handles.  They are in fact not inheritable
     to child processes, and it does not help to mark them inheritable via
     SetHandleInformation.  Subsequent socket calls in the child process fail
     with error 10038, WSAENOTSOCK.

     The only way up to Windows Server 2003 to make these sockets usable in
     child processes is to duplicate them via WSADuplicateSocket/WSASocket
     calls.  This requires to start the child process in SUSPENDED state so
     we only do this on affected systems.  If we recognize a non-inheritable
     socket we switch to inheritance/dup via WSADuplicateSocket/WSASocket for
     that socket.

     Starting with Vista there's another neat way to workaround these annoying
     LSP sockets.  WSAIoctl allows to fetch the underlying base socket, which
     is a normal, inheritable IFS handle.  So we fetch the base socket,
     duplicate it, and close the original socket.  Now we have a standard IFS
     socket which (hopefully) works as expected. */
  DWORD flags;
  bool fixup = false;
  if (!GetHandleInformation ((HANDLE) soc, &flags)
      || !(flags & HANDLE_FLAG_INHERIT))
    {
      int ret;
      SOCKET base_soc;
      DWORD bret;

      fixup = true;
      debug_printf ("LSP handle: %p", soc);
      ret = WSAIoctl (soc, SIO_BASE_HANDLE, NULL, 0, (void *) &base_soc,
		      sizeof (base_soc), &bret, NULL, NULL);
      if (ret)
	debug_printf ("WSAIoctl: %u", WSAGetLastError ());
      else if (base_soc != soc)
	{
	  /* LSPs are often BLODAs as well.  So we print an info about
	     detecting an LSP if BLODA detection is desired. */
	  if (detect_bloda)
	    {
	      WSAPROTOCOL_INFO prot;

	      memset (&prot, 0, sizeof prot);
	      ::getsockopt (soc, SOL_SOCKET, SO_PROTOCOL_INFO, (char *) &prot,
			    (size = sizeof prot, &size));
	      small_printf ("\n\nPotential BLODA detected!  Layered Socket "
			    "Service Provider:\n  %s\n\n", prot.szProtocol);
	    }
	  if (GetHandleInformation ((HANDLE) base_soc, &flags)
	      && (flags & HANDLE_FLAG_INHERIT))
	    {
	      if (!DuplicateHandle (GetCurrentProcess (), (HANDLE) base_soc,
				    GetCurrentProcess (), (PHANDLE) &base_soc,
				    0, TRUE, DUPLICATE_SAME_ACCESS))
		debug_printf ("DuplicateHandle failed, %E");
	      else
		{
		  closesocket (soc);
		  soc = base_soc;
		  fixup = false;
		}
	    }
	}
    }
  fd->set_io_handle ((HANDLE) soc);
  if (!((fhandler_socket *) fd)->init_events ())
    return false;
  if (fixup)
    ((fhandler_socket *) fd)->init_fixup_before ();
  fd->set_flags (O_RDWR | O_BINARY);
  debug_printf ("fd %d, name '%s', soc %p", (int) fd, dev->name, soc);

  /* Raise default buffer sizes (instead of WinSock default 8K).

     64K appear to have the best size/performance ratio for a default
     value.  Tested with ssh/scp on Vista over Gigabit LAN.

     NOTE.  If the SO_RCVBUF size exceeds 65535(*), and if the socket is
     connected to a remote machine, then calling WSADuplicateSocket on
     fork/exec fails with WinSock error 10022, WSAEINVAL.  Fortunately
     we don't use WSADuplicateSocket anymore, rather we just utilize
     handle inheritance.  An explanation for this weird behaviour would
     be nice, though.

     NOTE 2.  Testing on x86_64 (XP, Vista, 2008 R2, W8) indicates that
     this is no problem on 64 bit.  So we set the default buffer size to
     the default values in current 3.x Linux versions.

     (*) Maximum normal TCP window size.  Coincidence?  */
#ifdef __x86_64__
  ((fhandler_socket *) fd)->rmem () = 212992;
  ((fhandler_socket *) fd)->wmem () = 212992;
#else
  ((fhandler_socket *) fd)->rmem () = 65535;
  ((fhandler_socket *) fd)->wmem () = 65535;
#endif
  if (::setsockopt (soc, SOL_SOCKET, SO_RCVBUF,
		    (char *) &((fhandler_socket *) fd)->rmem (), sizeof (int)))
    {
      debug_printf ("setsockopt(SO_RCVBUF) failed, %u", WSAGetLastError ());
      if (::getsockopt (soc, SOL_SOCKET, SO_RCVBUF,
			(char *) &((fhandler_socket *) fd)->rmem (),
			(size = sizeof (int), &size)))
	system_printf ("getsockopt(SO_RCVBUF) failed, %u", WSAGetLastError ());
    }
  if (::setsockopt (soc, SOL_SOCKET, SO_SNDBUF,
		    (char *) &((fhandler_socket *) fd)->wmem (), sizeof (int)))
    {
      debug_printf ("setsockopt(SO_SNDBUF) failed, %u", WSAGetLastError ());
      if (::getsockopt (soc, SOL_SOCKET, SO_SNDBUF,
			(char *) &((fhandler_socket *) fd)->wmem (),
			(size = sizeof (int), &size)))
	system_printf ("getsockopt(SO_SNDBUF) failed, %u", WSAGetLastError ());
    }

  /* A unique ID is necessary to recognize fhandler entries which are
     duplicated by dup(2) or fork(2).  This is used in BSD flock calls
     to identify the descriptor. */
  ((fhandler_socket *) fd)->set_unique_id ();

  return true;
}

/* exported as socket: standards? */
extern "C" int
cygwin_socket (int af, int type, int protocol)
{
  int res = -1;
  SOCKET soc = 0;

  int flags = type & _SOCK_FLAG_MASK;
  type &= ~_SOCK_FLAG_MASK;

  debug_printf ("socket (%d, %d (flags %y), %d)", af, type, flags, protocol);

  if ((flags & ~(SOCK_NONBLOCK | SOCK_CLOEXEC)) != 0)
    {
      set_errno (EINVAL);
      goto done;
    }

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
	if (flags & SOCK_NONBLOCK)
	  ((fhandler_socket *) fd)->set_nonblocking (true);
	if (flags & SOCK_CLOEXEC)
	  ((fhandler_socket *) fd)->set_close_on_exec (true);
	if (type == SOCK_DGRAM)
	  {
	    /* Workaround the problem that a missing listener on a UDP socket
	       in a call to sendto will result in select/WSAEnumNetworkEvents
	       reporting that the socket has pending data and a subsequent call
	       to recvfrom will return -1 with error set to WSAECONNRESET.

	       This problem is a regression introduced in Windows 2000.
	       Instead of fixing the problem, a new socket IOCTL code has
	       been added, see http://support.microsoft.com/kb/263823 */
	    BOOL cr = FALSE;
	    DWORD blen;
	    if (WSAIoctl (soc, SIO_UDP_CONNRESET, &cr, sizeof cr, NULL, 0,
			  &blen, NULL, NULL) == SOCKET_ERROR)
	      debug_printf ("Reset SIO_UDP_CONNRESET: WinSock error %u",
			    WSAGetLastError ());
	  }
	res = fd;
      }
  }

done:
  syscall_printf ("%R = socket(%d, %d (flags %y), %d)",
		  res, af, type, flags, protocol);
  return res;
}

/* exported as sendto: standards? */
extern "C" ssize_t
cygwin_sendto (int fd, const void *buf, size_t len, int flags,
	       const struct sockaddr *to, socklen_t tolen)
{
  ssize_t res;

  pthread_testcancel ();

  fhandler_socket *fh = get (fd);

  myfault efault;
  if (efault.faulted (EFAULT) || !fh)
    res = -1;
  else
    res = fh->sendto (buf, len, flags, to, tolen);

  syscall_printf ("%lR = sendto(%d, %p, %ld, %y, %p, %d)",
		  res, fd, buf, len, flags, to, tolen);
  return res;
}

/* exported as recvfrom: standards? */
extern "C" ssize_t
cygwin_recvfrom (int fd, void *buf, size_t len, int flags,
		 struct sockaddr *from, socklen_t *fromlen)
{
  ssize_t res;

  pthread_testcancel ();

  fhandler_socket *fh = get (fd);

  myfault efault;
  if (efault.faulted (EFAULT) || !fh)
    res = -1;
  else
    /* Originally we shortcircuited here if res == 0.
       Allow 0 bytes buffer.  This is valid in POSIX and handled in
       fhandler_socket::recv_internal.  If we shortcircuit, we fail
       to deliver valid error conditions and peer address. */
    res = fh->recvfrom (buf, len, flags, from, fromlen);

  syscall_printf ("%lR = recvfrom(%d, %p, %ld, %y, %p, %p)",
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
      /* Old applications still use the old WinSock1 IPPROTO_IP values. */
      if (level == IPPROTO_IP && CYGWIN_VERSION_CHECK_FOR_USING_WINSOCK1_VALUES)
	optname = convert_ws1_ip_optname (optname);

      /* Per POSIX we must not be able to reuse a complete duplicate of a
	 local TCP address (same IP, same port), even if SO_REUSEADDR has been
	 set.  That's unfortunately possible in WinSock, and this has never
	 been changed to maintain backward compatibility.  Instead, the
	 SO_EXCLUSIVEADDRUSE option has been added to allow an application to
	 request POSIX standard behaviour in the non-SO_REUSEADDR case.

	 However, the WinSock standard behaviour of stream socket binding
	 is equivalent to the POSIX behaviour as if SO_REUSEADDR has been set.
	 So what we do here is to note that SO_REUSEADDR has been set, but not
	 actually hand over the request to WinSock.  This is tested in
	 fhandler_socket::bind(), so that SO_EXCLUSIVEADDRUSE can be set if
	 the application did not set SO_REUSEADDR.  This should reflect the
	 POSIX socket binding behaviour as close as possible with WinSock. */
      if (level == SOL_SOCKET && optname == SO_REUSEADDR
	  && fh->get_socket_type () == SOCK_STREAM)
	res = 0;
      else
	res = setsockopt (fh->get_socket (), level, optname,
			  (const char *) optval, optlen);

      if (optlen == sizeof (int))
	syscall_printf ("setsockopt optval=%x", *(int *) optval);

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

	     We just ignore the return value of setting IP_TOS entirely.
	     
	     CV 2014-04-16: Same for IPV6_TCLASS
	     FIXME:         Same for IPV6_RECVTCLASS? */
	  if (level == IPPROTO_IP && optname == IP_TOS
	      && WSAGetLastError () == WSAEINVAL)
	    {
	      debug_printf ("Faked IP_TOS success");
	      res = 0;
	    }
	  else if (level == IPPROTO_IPV6 && optname == IPV6_TCLASS
		   && WSAGetLastError () == WSAENOPROTOOPT)
	    {
	      debug_printf ("Faked IPV6_TCLASS success");
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

  syscall_printf ("%R = setsockopt(%d, %d, %y, %p, %d)",
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
  else if (optname == SO_PEERCRED && level == SOL_SOCKET)
    {
      struct ucred *cred = (struct ucred *) optval;
      res = fh->getpeereid (&cred->pid, &cred->uid, &cred->gid);
    }
  else
    {
      /* Old applications still use the old WinSock1 IPPROTO_IP values. */
      if (level == IPPROTO_IP && CYGWIN_VERSION_CHECK_FOR_USING_WINSOCK1_VALUES)
	optname = convert_ws1_ip_optname (optname);
      res = getsockopt (fh->get_socket (), level, optname, (char *) optval,
			(int *) optlen);
      if (res == SOCKET_ERROR)
	set_winsock_errno ();
      else if (level == SOL_SOCKET && optname == SO_ERROR)
	{
	  int *e = (int *) optval;
	  debug_printf ("WinSock SO_ERROR = %d", *e);
	  *e = find_winsock_errno (*e);
	}
      else if (*optlen == 1)
      	{
	  /* Regression in Vista and later:  instead of a 4 byte BOOL value,
	     a 1 byte BOOLEAN value is returned, in contrast to older systems
	     and the documentation.  Since an int type is expected by the
	     calling application, we convert the result here.  For some reason
	     only three BSD-compatible socket options seem to be affected. */
	  if ((level == SOL_SOCKET
	       && (optname == SO_KEEPALIVE || optname == SO_DONTROUTE))
	      || (level == IPPROTO_TCP && optname == TCP_NODELAY))
	    {
	      BOOLEAN *in = (BOOLEAN *) optval;
	      int *out = (int *) optval;
	      *out = *in;
	      *optlen = 4;
	    }
	}
    }

  syscall_printf ("%R = getsockopt(%d, %d, %y, %p, %p)",
		  res, fd, level, optname, optval, optlen);
  return res;
}

extern "C" int
getpeereid (int fd, uid_t *euid, gid_t *egid)
{
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

  pthread_testcancel ();

  fhandler_socket *fh = get (fd);

  myfault efault;
  if (efault.faulted (EFAULT) || !fh)
    res = -1;
  else
    res = fh->connect (name, namelen);

  syscall_printf ("%R = connect(%d, %p, %d)", res, fd, name, namelen);

  return res;
}

/* exported as getservbyname: standards? */
extern "C" struct servent *
cygwin_getservbyname (const char *name, const char *proto)
{
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
  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;

  if (gethostname (name, len))
    {
      DWORD local_len = len;

      if (!GetComputerNameExA (ComputerNameDnsFullyQualified, name, &local_len))
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
	      dn_expand (msg, eomsg, curptr->name (), string_ptr, curptr->namelen1);
	      ret->h_name = string_ptr;
	      string_ptr += curptr->namelen1;
	    }
	  ret->h_addr_list[address_count++] = string_ptr;
	  if (addrsize_in != addrsize_out)
	    memcpy4to6 (string_ptr, curptr->data);
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

/* gethostbyname2: standards? */
extern "C" struct hostent *
gethostbyname2 (const char *name, int af)
{
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

  pthread_testcancel ();

  fhandler_socket *fh = get (fd);

  myfault efault;
  if (efault.faulted (EFAULT) || !fh)
    res = -1;
  else
    res = fh->accept4 (peer, len, fh->is_nonblocking () ? SOCK_NONBLOCK : 0);

  syscall_printf ("%R = accept(%d, %p, %p)", res, fd, peer, len);
  return res;
}

extern "C" int
accept4 (int fd, struct sockaddr *peer, socklen_t *len, int flags)
{
  int res;

  pthread_testcancel ();

  fhandler_socket *fh = get (fd);

  myfault efault;
  if (efault.faulted (EFAULT) || !fh)
    res = -1;
  else if ((flags & ~(SOCK_NONBLOCK | SOCK_CLOEXEC)) != 0)
    {
      set_errno (EINVAL);
      res = -1;
    }
  else
    res = fh->accept4 (peer, len, flags);

  syscall_printf ("%R = accept4(%d, %p, %p, %y)", res, fd, peer, len, flags);
  return res;
}

/* exported as bind: standards? */
extern "C" int
cygwin_bind (int fd, const struct sockaddr *my_addr, socklen_t addrlen)
{
  int res;
  fhandler_socket *fh = get (fd);

  myfault efault;
  if (efault.faulted (EFAULT) || !fh)
    res = -1;
  else
    res = fh->bind (my_addr, addrlen);

  syscall_printf ("%R = bind(%d, %p, %d)", res, fd, my_addr, addrlen);
  return res;
}

/* exported as getsockname: standards? */
extern "C" int
cygwin_getsockname (int fd, struct sockaddr *addr, socklen_t *namelen)
{
  int res;

  fhandler_socket *fh = get (fd);

  myfault efault;
  if (efault.faulted (EFAULT) || !fh)
    res = -1;
  else
    res = fh->getsockname (addr, namelen);

  syscall_printf ("%R =getsockname (%d, %p, %p)", res, fd, addr, namelen);
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

  syscall_printf ("%R = listen(%d, %d)", res, fd, backlog);
  return res;
}

/* exported as shutdown: standards? */
extern "C" int
cygwin_shutdown (int fd, int how)
{
  int res;

  fhandler_socket *fh = get (fd);

  if (!fh)
    res = -1;
  else
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

  fhandler_socket *fh = get (fd);

  myfault efault;
  if (efault.faulted (EFAULT) || !fh)
    res = -1;
  else
    res = fh->getpeername (name, len);

  syscall_printf ("%R = getpeername(%d) %p", res, fd,
  		  (fh ? fh->get_socket () : (SOCKET) -1));
  return res;
}

/* exported as recv: standards? */
extern "C" ssize_t
cygwin_recv (int fd, void *buf, size_t len, int flags)
{
  ssize_t res;

  pthread_testcancel ();

  fhandler_socket *fh = get (fd);

  myfault efault;
  if (efault.faulted (EFAULT) || !fh)
    res = -1;
  else
    /* Originally we shortcircuited here if res == 0.
       Allow 0 bytes buffer.  This is valid in POSIX and handled in
       fhandler_socket::recv_internal.  If we shortcircuit, we fail
       to deliver valid error conditions. */
    res = fh->recvfrom (buf, len, flags, NULL, NULL);

  syscall_printf ("%lR = recv(%d, %p, %ld, %y)", res, fd, buf, len, flags);
  return res;
}

/* exported as send: standards? */
extern "C" ssize_t
cygwin_send (int fd, const void *buf, size_t len, int flags)
{
  ssize_t res;

  pthread_testcancel ();

  fhandler_socket *fh = get (fd);

  myfault efault;
  if (efault.faulted (EFAULT) || !fh)
    res = -1;
  else
    res = fh->sendto (buf, len, flags, NULL, 0);

  syscall_printf ("%lR = send(%d, %p, %ld, %y)", res, fd, buf, len, flags);
  return res;
}

/* getdomainname: standards? */
extern "C" int
getdomainname (char *domain, size_t len)
{
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
  __seterrno ();
  return -1;
}

/* Fill out an ifconf struct. */

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

  if ((uintptr_t) &param >= (uintptr_t) 0x80000000L
      && wincap.has_gaa_largeaddress_bug ())
    {
      /* In Windows Vista and Windows 7 under WOW64, GetAdaptersAddresses fails
	 if it's running in a thread with a stack located in the large address
	 area.  So, if we're running in a pthread with such a stack, we call
	 GetAdaptersAddresses in a child thread with an OS-allocated stack.
	 The OS allocates stacks bottom up, so chances are good that the new
	 stack will be located in the lower address area. */
      HANDLE thr = CreateThread (NULL, 0, call_gaa, &param, 0, NULL);
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
	  cygwin_inet_aton (t, (a)); \
	}
      /* If DHCP is used, we have only one address. */
      if (dhcp)
	{
	  if (udipa.Buffer)
	    inet_uton (udipa.Buffer, &addr->sin_addr);
	  if (udsub.Buffer)
	    inet_uton (udsub.Buffer, &mask->sin_addr);
	}
      else
	{
	  if (uipa.Buffer)
	    {
	      for (ifs = 0, c = uipa.Buffer; *c && ifs < idx;
		   c += wcslen (c) + 1)
		ifs++;
	      if (*c)
		inet_uton (c, &addr->sin_addr);
	    }
	  if (usub.Buffer)
	    {
	      for (ifs = 0, c = usub.Buffer; *c && ifs < idx;
		   c += wcslen (c) + 1)
		ifs++;
	      if (*c)
		inet_uton (c, &mask->sin_addr);
	    }
	}
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
		for (cnt = 0; cnt < 4 && prefix > 0; ++cnt, prefix -= 32)
		  {
		    if_sin6->sin6_addr.s6_addr32[cnt] = UINT32_MAX;
		    if (prefix < 32)
		      if_sin6->sin6_addr.s6_addr32[cnt] <<= 32 - prefix;
		  }
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
  ifret = get_ifs (AF_INET);
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

  if (get_adapters_addresses (&pa0, AF_UNSPEC))
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

  if (get_adapters_addresses (&pa0, AF_UNSPEC))
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
	yield ();
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

  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;

  int flags = type & _SOCK_FLAG_MASK;
  type &= ~_SOCK_FLAG_MASK;

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
  if ((flags & ~(SOCK_NONBLOCK | SOCK_CLOEXEC)) != 0)
    {
      set_errno (EINVAL);
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
  sock_in.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
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
	if (flags & SOCK_NONBLOCK)
	  ((fhandler_socket *) sb0)->set_nonblocking (true);
	if (flags & SOCK_CLOEXEC)
	  ((fhandler_socket *) sb0)->set_close_on_exec (true);
	if (family == AF_LOCAL && type == SOCK_STREAM)
	  ((fhandler_socket *) sb0)->af_local_set_sockpair_cred ();

	cygheap_fdnew sb1 (sb0, false);

	if (sb1 >= 0 && fdsock (sb1, dev, outsock))
	  {
	    ((fhandler_socket *) sb1)->set_addr_family (family);
	    ((fhandler_socket *) sb1)->set_socket_type (type);
	    ((fhandler_socket *) sb1)->connect_state (connected);
	    if (flags & SOCK_NONBLOCK)
	      ((fhandler_socket *) sb1)->set_nonblocking (true);
	    if (flags & SOCK_CLOEXEC)
	      ((fhandler_socket *) sb1)->set_close_on_exec (true);
	    if (family == AF_LOCAL && type == SOCK_STREAM)
	      ((fhandler_socket *) sb1)->af_local_set_sockpair_cred ();

	    sb[0] = sb0;
	    sb[1] = sb1;
	    res = 0;
	  }
	else
	  sb0.release ();
      }

    if (res == -1)
      {
	closesocket (insock);
	closesocket (outsock);
      }
  }

done:
  syscall_printf ("%R = socketpair(...)", res);
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
extern "C" ssize_t
cygwin_recvmsg (int fd, struct msghdr *msg, int flags)
{
  ssize_t res;

  pthread_testcancel ();

  fhandler_socket *fh = get (fd);

  myfault efault;
  if (efault.faulted (EFAULT) || !fh)
    res = -1;
  else
    {
      res = check_iovec_for_read (msg->msg_iov, msg->msg_iovlen);
      /* Originally we shortcircuited here if res == 0.
	 Allow 0 bytes buffer.  This is valid in POSIX and handled in
	 fhandler_socket::recv_internal.  If we shortcircuit, we fail
	 to deliver valid error conditions and peer address. */
      if (res >= 0)
	res = fh->recvmsg (msg, flags);
    }

  syscall_printf ("%lR = recvmsg(%d, %p, %y)", res, fd, msg, flags);
  return res;
}

/* exported as sendmsg: standards? */
extern "C" ssize_t
cygwin_sendmsg (int fd, const struct msghdr *msg, int flags)
{
  ssize_t res;

  pthread_testcancel ();

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
      return (inet_ntop4((const u_char *) src, dst, size));
    case AF_INET6:
      return (inet_ntop6((const u_char *) src, dst, size));
    default:
      errno = EAFNOSUPPORT;
      return (NULL);
    }
  /* NOTREACHED */
}

extern "C" void
cygwin_freeaddrinfo (struct addrinfo *addr)
{
  struct addrinfo *ai, *ainext;

  myfault efault;
  if (efault.faulted (EFAULT))
    return;

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

  for (; ai; ai = ai->ai_next, nai = tmp)
    {
      if (!(tmp = ga_dup (ai, v4mapped, idn_flags, err)))
	goto bad;
      if (!nai0)
	nai0 = tmp;
      if (nai)
	nai->ai_next = tmp;
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

extern "C" int
cygwin_getaddrinfo (const char *hostname, const char *servname,
		    const struct addrinfo *hints, struct addrinfo **res)
{
  myfault efault;
  if (efault.faulted (EFAULT))
    return EAI_SYSTEM;
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
  if (hints && (hints->ai_flags
		& ~(AI_PASSIVE | AI_CANONNAME | AI_NUMERICHOST | AI_ALL
		    | AI_NUMERICSERV | AI_ADDRCONFIG | AI_V4MAPPED
		    | AI_IDN_MASK)))
    return EAI_BADFLAGS;
  /* AI_NUMERICSERV is not supported prior to Windows Vista.  We just check
     the servname parameter by ourselves here. */
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
      /* AI_ADDRCONFIG is not supported prior to Vista.  Rather it's
	 the default and only possible setting.
	 On Vista, the default behaviour is as if AI_ADDRCONFIG is set,
	 apparently for performance reasons.  To get the POSIX default
	 behaviour, the AI_ALL flag has to be set. */
      if (wincap.supports_all_posix_ai_flags ()
	  && whints.ai_family == PF_UNSPEC
	  && !(whints.ai_flags & AI_ADDRCONFIG))
	whints.ai_flags |= AI_ALL;
    }
  /* Disable automatic IDN conversion on W8 and later. */
  whints.ai_flags |= AI_DISABLE_IDN_ENCODING;
  int ret = w32_to_gai_err (GetAddrInfoW (whost, wserv, &whints, &wres));
  /* Always copy over to self-allocated memory. */
  if (!ret)
    {
      *res = ga_duplist (wres, false, idn_flags, ret);
      FreeAddrInfoW (wres);
      if (!*res)
	return ret;
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
      /* sizeof addrinfo == sizeof addrinfoW */
      memcpy (&whints, hints, sizeof whints);
      whints.ai_family = AF_INET;
#ifdef __x86_64__
      /* ai_addrlen is socklen_t (4 bytes) in POSIX but size_t (8 bytes) in
         Winsock.  Sert upper 4 bytes explicitely to 0 to avoid EAI_FAIL. */
      whints.ai_addrlen &= UINT32_MAX;
#endif
      int ret2 = w32_to_gai_err (GetAddrInfoW (whost, wserv, &whints, &wres));
      if (!ret2)
	{
	  struct addrinfo *v4res = ga_duplist (wres, true, idn_flags, ret);
	  FreeAddrInfoW (wres);
	  if (!v4res)
	    {
	      if (!ret)
		cygwin_freeaddrinfo (*res);
	      return ret;
	    }
	  /* If a list of v6 addresses exists, append the v4-in-v6 address
	     list.  Otherwise just return the v4-in-v6 address list. */
	  if (!ret)
	    {
	      struct addrinfo *ptr;
	      for (ptr = *res; ptr->ai_next; ptr = ptr->ai_next)
		;
	      ptr->ai_next = v4res;
	    }
	  else
	    *res = v4res;
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

  /* When the incoming port number does not resolve to a well-known service,
     WinSock's getnameinfo up to Windows 2003 returns with error WSANO_DATA
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

  int ret = w32_to_gai_err (GetNameInfoW (sa, salen, whost, whlen,
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

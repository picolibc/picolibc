/* fhandler_socket.cc. See fhandler.h for a description of the fhandler classes.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#define  __INSIDE_CYGWIN_NET__
#define USE_SYS_TYPES_FD_SET

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
#include <ntsecapi.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <iphlpapi.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include <asm/byteorder.h>
#include "cygwin/version.h"
#include "perprocess.h"
#include "shared_info.h"
#include "sigproc.h"
#include "wininfo.h"
#include <unistd.h>
#include <sys/param.h>
#include <sys/statvfs.h>
#include <cygwin/acl.h>
#include "cygtls.h"
#include <sys/un.h>
#include "ntdll.h"
#include "miscfuncs.h"
#include "tls_pbuf.h"

#define ASYNC_MASK (FD_READ|FD_WRITE|FD_OOB|FD_ACCEPT|FD_CONNECT)
#define EVENT_MASK (FD_READ|FD_WRITE|FD_OOB|FD_ACCEPT|FD_CONNECT|FD_CLOSE)

extern "C" {
  int sscanf (const char *, const char *, ...);
} /* End of "C" section */

/**********************************************************************/
/* fhandler_socket */

fhandler_socket::fhandler_socket () :
  fhandler_base (),
  uid (myself->uid),
  gid (myself->gid),
  mode (S_IFSOCK | S_IRWXU | S_IRWXG | S_IRWXO),
  wsock_events (NULL),
  wsock_mtx (NULL),
  wsock_evt (NULL),
  _rcvtimeo (INFINITE),
  _sndtimeo (INFINITE),
  prot_info_ptr (NULL),
  status ()
{
  need_fork_fixup (true);
}

fhandler_socket::~fhandler_socket ()
{
  if (prot_info_ptr)
    cfree (prot_info_ptr);
}

char *
fhandler_socket::get_proc_fd_name (char *buf)
{
  __small_sprintf (buf, "socket:[%lu]", get_plain_ino ());
  return buf;
}

int
fhandler_socket::open (int flags, mode_t mode)
{
  set_errno (ENXIO);
  return 0;
}

int
fhandler_socket::set_socket_handle (SOCKET sock, int af, int type, int flags)
{
  DWORD hdl_flags;
  bool lsp_fixup = false;

  /* Usually sockets are inheritable IFS objects.  Unfortunately some virus
     scanners or other network-oriented software replace normal sockets
     with their own kind, which is running through a filter driver called
     "layered service provider" (LSP) which, fortunately, are deprecated.

     LSP sockets are not kernel objects.  They are typically not marked as
     inheritable, nor are they IFS handles.  They are in fact not inheritable
     to child processes, and it does not help to mark them inheritable via
     SetHandleInformation.  Subsequent socket calls in the child process fail
     with error 10038, WSAENOTSOCK.

     There's a neat way to workaround these annoying LSP sockets.  WSAIoctl
     allows to fetch the underlying base socket, which is a normal, inheritable
     IFS handle.  So we fetch the base socket, duplicate it, and close the
     original socket.  Now we have a standard IFS socket which (hopefully)
     works as expected.

     If that doesn't work for some reason, mark the sockets for duplication
     via WSADuplicateSocket/WSASocket.  This requires to start the child
     process in SUSPENDED state so we only do this if really necessary. */
  if (!GetHandleInformation ((HANDLE) sock, &hdl_flags)
      || !(hdl_flags & HANDLE_FLAG_INHERIT))
    {
      int ret;
      SOCKET base_sock;
      DWORD bret;

      lsp_fixup = true;
      debug_printf ("LSP handle: %p", sock);
      ret = WSAIoctl (sock, SIO_BASE_HANDLE, NULL, 0, (void *) &base_sock,
                      sizeof (base_sock), &bret, NULL, NULL);
      if (ret)
        debug_printf ("WSAIoctl: %u", WSAGetLastError ());
      else if (base_sock != sock)
        {
          if (GetHandleInformation ((HANDLE) base_sock, &hdl_flags)
              && (flags & HANDLE_FLAG_INHERIT))
            {
              if (!DuplicateHandle (GetCurrentProcess (), (HANDLE) base_sock,
                                    GetCurrentProcess (), (PHANDLE) &base_sock,
                                    0, TRUE, DUPLICATE_SAME_ACCESS))
                debug_printf ("DuplicateHandle failed, %E");
              else
                {
                  ::closesocket (sock);
                  sock = base_sock;
                  lsp_fixup = false;
                }
            }
        }
    }
  set_addr_family (af);
  set_socket_type (type);
  if (flags & SOCK_NONBLOCK)
    set_nonblocking (true);
  if (flags & SOCK_CLOEXEC)
    set_close_on_exec (true);
  set_io_handle ((HANDLE) sock);
  if (!init_events ())
    return -1;
  if (lsp_fixup)
    init_fixup_before ();
  set_flags (O_RDWR | O_BINARY);
  set_unique_id ();
  if (get_socket_type () == SOCK_DGRAM)
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
      if (WSAIoctl (sock, SIO_UDP_CONNRESET, &cr, sizeof cr, NULL, 0,
		    &blen, NULL, NULL) == SOCKET_ERROR)
	debug_printf ("Reset SIO_UDP_CONNRESET: WinSock error %u",
		      WSAGetLastError ());
    }
#ifdef __x86_64__
  rmem () = 212992;
  wmem () = 212992;
#else
  rmem () = 64512;
  wmem () = 64512;
#endif
  return 0;
}

/* Maximum number of concurrently opened sockets from all Cygwin processes
   per session.  Note that shared sockets (through dup/fork/exec) are
   counted as one socket. */
#define NUM_SOCKS       2048U

#define LOCK_EVENTS	\
  if (wsock_mtx && \
      WaitForSingleObject (wsock_mtx, INFINITE) != WAIT_FAILED) \
    {

#define UNLOCK_EVENTS \
      ReleaseMutex (wsock_mtx); \
    }

static wsa_event wsa_events[NUM_SOCKS] __attribute__((section (".cygwin_dll_common"), shared));

static LONG socket_serial_number __attribute__((section (".cygwin_dll_common"), shared));

static HANDLE wsa_slot_mtx;

static PWCHAR
sock_shared_name (PWCHAR buf, LONG num)
{
  __small_swprintf (buf, L"socket.%d", num);
  return buf;
}

static wsa_event *
search_wsa_event_slot (LONG new_serial_number)
{
  WCHAR name[32], searchname[32];
  UNICODE_STRING uname;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;

  if (!wsa_slot_mtx)
    {
      RtlInitUnicodeString (&uname, sock_shared_name (name, 0));
      InitializeObjectAttributes (&attr, &uname, OBJ_INHERIT | OBJ_OPENIF,
				  get_session_parent_dir (),
				  everyone_sd (CYG_MUTANT_ACCESS));
      status = NtCreateMutant (&wsa_slot_mtx, CYG_MUTANT_ACCESS, &attr, FALSE);
      if (!NT_SUCCESS (status))
	api_fatal ("Couldn't create/open shared socket mutex %S, %y",
		   &uname, status);
    }
  switch (WaitForSingleObject (wsa_slot_mtx, INFINITE))
    {
    case WAIT_OBJECT_0:
    case WAIT_ABANDONED:
      break;
    default:
      api_fatal ("WFSO failed for shared socket mutex, %E");
      break;
    }
  unsigned int slot = new_serial_number % NUM_SOCKS;
  while (wsa_events[slot].serial_number)
    {
      HANDLE searchmtx;
      RtlInitUnicodeString (&uname, sock_shared_name (searchname,
					wsa_events[slot].serial_number));
      InitializeObjectAttributes (&attr, &uname, 0, get_session_parent_dir (),
				  NULL);
      status = NtOpenMutant (&searchmtx, READ_CONTROL, &attr);
      if (!NT_SUCCESS (status))
	break;
      /* Mutex still exists, attached socket is active, try next slot. */
      NtClose (searchmtx);
      slot = (slot + 1) % NUM_SOCKS;
      if (slot == (new_serial_number % NUM_SOCKS))
	{
	  /* Did the whole array once.   Too bad. */
	  debug_printf ("No free socket slot");
	  ReleaseMutex (wsa_slot_mtx);
	  return NULL;
	}
    }
  memset (&wsa_events[slot], 0, sizeof (wsa_event));
  wsa_events[slot].serial_number = new_serial_number;
  ReleaseMutex (wsa_slot_mtx);
  return wsa_events + slot;
}

bool
fhandler_socket::init_events ()
{
  LONG new_serial_number;
  WCHAR name[32];
  UNICODE_STRING uname;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;

  do
    {
      new_serial_number =
	InterlockedIncrement (&socket_serial_number);
      if (!new_serial_number)	/* 0 is reserved for global mutex */
	InterlockedIncrement (&socket_serial_number);
      set_ino (new_serial_number);
      RtlInitUnicodeString (&uname, sock_shared_name (name, new_serial_number));
      InitializeObjectAttributes (&attr, &uname, OBJ_INHERIT | OBJ_OPENIF,
				  get_session_parent_dir (),
				  everyone_sd (CYG_MUTANT_ACCESS));
      status = NtCreateMutant (&wsock_mtx, CYG_MUTANT_ACCESS, &attr, FALSE);
      if (!NT_SUCCESS (status))
	{
	  debug_printf ("NtCreateMutant(%S), %y", &uname, status);
	  set_errno (ENOBUFS);
	  return false;
	}
      if (status == STATUS_OBJECT_NAME_EXISTS)
	NtClose (wsock_mtx);
    }
  while (status == STATUS_OBJECT_NAME_EXISTS);
  if ((wsock_evt = CreateEvent (&sec_all, TRUE, FALSE, NULL))
      == WSA_INVALID_EVENT)
    {
      debug_printf ("CreateEvent, %E");
      set_errno (ENOBUFS);
      NtClose (wsock_mtx);
      return false;
    }
  if (WSAEventSelect (get_socket (), wsock_evt, EVENT_MASK) == SOCKET_ERROR)
    {
      debug_printf ("WSAEventSelect, %E");
      set_winsock_errno ();
      NtClose (wsock_evt);
      NtClose (wsock_mtx);
      return false;
    }
  if (!(wsock_events = search_wsa_event_slot (new_serial_number)))
    {
      set_errno (ENOBUFS);
      NtClose (wsock_evt);
      NtClose (wsock_mtx);
      return false;
    }
  if (get_socket_type () == SOCK_DGRAM)
    wsock_events->events = FD_WRITE;
  return true;
}

int
fhandler_socket::evaluate_events (const long event_mask, long &events,
				  const bool erase)
{
  int ret = 0;
  long events_now = 0;

  WSANETWORKEVENTS evts = { 0 };
  if (!(WSAEnumNetworkEvents (get_socket (), wsock_evt, &evts)))
    {
      if (evts.lNetworkEvents)
	{
	  LOCK_EVENTS;
	  wsock_events->events |= evts.lNetworkEvents;
	  events_now = (wsock_events->events & event_mask);
	  if (evts.lNetworkEvents & FD_CONNECT)
	    {
	      wsock_events->connect_errorcode = evts.iErrorCode[FD_CONNECT_BIT];

	      /* Setting the connect_state and calling the AF_LOCAL handshake 
		 here allows to handle this stuff from a single point.  This
		 is independent of FD_CONNECT being requested.  Consider a
		 server calling connect(2) and then immediately poll(2) with
		 only polling for POLLIN (example: postfix), or select(2) just
		 asking for descriptors ready to read.

		 Something weird occurs in Winsock: If you fork off and call
		 recv/send on the duplicated, already connected socket, another
		 FD_CONNECT event is generated in the child process.  This
		 would trigger a call to af_local_connect which obviously fail. 
		 Avoid this by calling set_connect_state only if connect_state
		 is connect_pending. */
	      if (connect_state () == connect_pending)
		{
		  if (wsock_events->connect_errorcode)
		    connect_state (connect_failed);
		  else if (af_local_connect ())
		    {
		      wsock_events->connect_errorcode = WSAGetLastError ();
		      connect_state (connect_failed);
		    }
		  else
		    connect_state (connected);
		}
	    }
	  UNLOCK_EVENTS;
	  if ((evts.lNetworkEvents & FD_OOB) && wsock_events->owner)
	    kill (wsock_events->owner, SIGURG);
	}
    }

  LOCK_EVENTS;
  if ((events = events_now) != 0
      || (events = (wsock_events->events & event_mask)) != 0)
    {
      if (events & FD_CONNECT)
	{
	  int wsa_err = wsock_events->connect_errorcode;
	  if (wsa_err)
	    {
	      /* CV 2014-04-23: This is really weird.  If you call connect
		 asynchronously on a socket and then select, an error like
		 "Connection refused" is set in the event and in the SO_ERROR
		 socket option.  If you call connect, then dup, then select,
		 the error is set in the event, but not in the SO_ERROR socket
		 option, despite the dup'ed socket handle referring to the same
		 socket.  We're trying to workaround this problem here by
		 taking the connect errorcode from the event and write it back
		 into the SO_ERROR socket option.
	         
		 CV 2014-06-16: Call WSASetLastError *after* setsockopt since,
		 apparently, setsockopt sets the last WSA error code to 0 on
		 success. */
	      ::setsockopt (get_socket (), SOL_SOCKET, SO_ERROR,
			    (const char *) &wsa_err, sizeof wsa_err);
	      WSASetLastError (wsa_err);
	      ret = SOCKET_ERROR;
	    }
	  else
	    wsock_events->events |= FD_WRITE;
	  wsock_events->events &= ~FD_CONNECT;
	  wsock_events->connect_errorcode = 0;
	}
      /* This test makes accept/connect behave as on Linux when accept/connect
         is called on a socket for which shutdown has been called.  The second
	 half of this code is in the shutdown method. */
      if (events & FD_CLOSE)
	{
	  if ((event_mask & FD_ACCEPT) && saw_shutdown_read ())
	    {
	      WSASetLastError (WSAEINVAL);
	      ret = SOCKET_ERROR;
	    }
	  if (event_mask & FD_CONNECT)
	    {
	      WSASetLastError (WSAECONNRESET);
	      ret = SOCKET_ERROR;
	    }
	}
      if (erase)
	wsock_events->events &= ~(events & ~(FD_WRITE | FD_CLOSE));
    }
  UNLOCK_EVENTS;

  return ret;
}

int
fhandler_socket::wait_for_events (const long event_mask, const DWORD flags)
{
  if (async_io ())
    return 0;

  int ret;
  long events = 0;
  DWORD wfmo_timeout = 50;
  DWORD timeout;

  WSAEVENT ev[3] = { wsock_evt, NULL, NULL };
  wait_signal_arrived here (ev[1]);
  DWORD ev_cnt = 2;
  if ((ev[2] = pthread::get_cancel_event ()) != NULL)
    ++ev_cnt;

  if (is_nonblocking () || (flags & MSG_DONTWAIT))
    timeout = 0;
  else if (event_mask & FD_READ)
    timeout = rcvtimeo ();
  else if (event_mask & FD_WRITE)
    timeout = sndtimeo ();
  else
    timeout = INFINITE;

  while (!(ret = evaluate_events (event_mask, events, !(flags & MSG_PEEK)))
	 && !events)
    {
      if (timeout == 0)
	{
	  WSASetLastError (WSAEWOULDBLOCK);
	  return SOCKET_ERROR;
	}

      if (timeout < wfmo_timeout)
	wfmo_timeout = timeout;
      switch (WSAWaitForMultipleEvents (ev_cnt, ev, FALSE, wfmo_timeout, FALSE))
	{
	  case WSA_WAIT_TIMEOUT:
	  case WSA_WAIT_EVENT_0:
	    if (timeout != INFINITE)
	      timeout -= wfmo_timeout;
	    break;

	  case WSA_WAIT_EVENT_0 + 1:
	    if (_my_tls.call_signal_handler ())
	      break;
	    WSASetLastError (WSAEINTR);
	    return SOCKET_ERROR;

	  case WSA_WAIT_EVENT_0 + 2:
	    pthread::static_cancel_self ();
	    break;

	  default:
	    /* wsock_evt can be NULL.  We're generating the same errno values
	       as for sockets on which shutdown has been called. */
	    if (WSAGetLastError () != WSA_INVALID_HANDLE)
	      WSASetLastError (WSAEFAULT);
	    else
	      WSASetLastError ((event_mask & FD_CONNECT) ? WSAECONNRESET
							 : WSAEINVAL);
	    return SOCKET_ERROR;
	}
    }
  return ret;
}

void
fhandler_socket::release_events ()
{
  if (WaitForSingleObject (wsock_mtx, INFINITE) != WAIT_FAILED)
    {
      HANDLE evt = wsock_evt;
      HANDLE mtx = wsock_mtx;

      wsock_evt = wsock_mtx = NULL;
      ReleaseMutex (mtx);
      NtClose (evt);
      NtClose (mtx);
    }
}

/* Called if a freshly created socket is not inheritable.  In that case we
   have to use fixup_before_fork_exec.  See comment in set_socket_handle for
   a description of the problem. */
void
fhandler_socket::init_fixup_before ()
{
  prot_info_ptr = (LPWSAPROTOCOL_INFOW)
		  cmalloc_abort (HEAP_BUF, sizeof (WSAPROTOCOL_INFOW));
  cygheap->fdtab.inc_need_fixup_before ();
}

int
fhandler_socket::fixup_before_fork_exec (DWORD win_pid)
{
  SOCKET ret = WSADuplicateSocketW (get_socket (), win_pid, prot_info_ptr);
  if (ret)
    set_winsock_errno ();
  else
    debug_printf ("WSADuplicateSocket succeeded (%x)", prot_info_ptr->dwProviderReserved);
  return (int) ret;
}

void
fhandler_socket::fixup_after_fork (HANDLE parent)
{
  fork_fixup (parent, wsock_mtx, "wsock_mtx");
  fork_fixup (parent, wsock_evt, "wsock_evt");

  if (!need_fixup_before ())
    {
      fhandler_base::fixup_after_fork (parent);
      return;
    }

  SOCKET new_sock = WSASocketW (FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO,
				FROM_PROTOCOL_INFO, prot_info_ptr, 0,
				WSA_FLAG_OVERLAPPED);
  if (new_sock == INVALID_SOCKET)
    {
      set_winsock_errno ();
      set_io_handle ((HANDLE) INVALID_SOCKET);
    }
  else
    {
      /* Even though the original socket was not inheritable, the duplicated
	 socket is potentially inheritable again. */
      SetHandleInformation ((HANDLE) new_sock, HANDLE_FLAG_INHERIT, 0);
      set_io_handle ((HANDLE) new_sock);
      debug_printf ("WSASocket succeeded (%p)", new_sock);
    }
}

void
fhandler_socket::fixup_after_exec ()
{
  if (need_fixup_before () && !close_on_exec ())
    fixup_after_fork (NULL);
}

int
fhandler_socket::dup (fhandler_base *child, int flags)
{
  debug_printf ("here");
  fhandler_socket *fhs = (fhandler_socket *) child;

  if (!DuplicateHandle (GetCurrentProcess (), wsock_mtx,
			GetCurrentProcess (), &fhs->wsock_mtx,
			0, TRUE, DUPLICATE_SAME_ACCESS))
    {
      __seterrno ();
      return -1;
    }
  if (!DuplicateHandle (GetCurrentProcess (), wsock_evt,
			GetCurrentProcess (), &fhs->wsock_evt,
			0, TRUE, DUPLICATE_SAME_ACCESS))
    {
      __seterrno ();
      NtClose (fhs->wsock_mtx);
      return -1;
    }
  if (!need_fixup_before ())
    {
      int ret = fhandler_base::dup (child, flags);
      if (ret)
	{
	  NtClose (fhs->wsock_evt);
	  NtClose (fhs->wsock_mtx);
	}
      return ret;
    }

  cygheap->user.deimpersonate ();
  fhs->init_fixup_before ();
  fhs->set_io_handle (get_io_handle ());
  int ret = fhs->fixup_before_fork_exec (GetCurrentProcessId ());
  cygheap->user.reimpersonate ();
  if (!ret)
    {
      fhs->fixup_after_fork (GetCurrentProcess ());
      if (fhs->get_io_handle() != (HANDLE) INVALID_SOCKET)
	return 0;
    }
  cygheap->fdtab.dec_need_fixup_before ();
  NtClose (fhs->wsock_evt);
  NtClose (fhs->wsock_mtx);
  return -1;
}

int __reg2
fhandler_socket::fstat (struct stat *buf)
{
  int res;

  res = fhandler_socket::fstat (buf);
  if (!res)
    {
      buf->st_dev = FHDEV (DEV_TCP_MAJOR, 0);
      if (!(buf->st_ino = get_plain_ino ()))
	sscanf (get_name (), "/proc/%*d/fd/socket:[%lld]",
			     (long long *) &buf->st_ino);
      buf->st_uid = uid;
      buf->st_gid = gid;
      buf->st_mode = mode;
      buf->st_size = 0;
    }
  return res;
}

int __reg2
fhandler_socket::fstatvfs (struct statvfs *sfs)
{
  memset (sfs, 0, sizeof (*sfs));
  sfs->f_bsize = sfs->f_frsize = 4096;
  sfs->f_namemax = NAME_MAX;
  return 0;
}

int
fhandler_socket::fchmod (mode_t newmode)
{
  mode = (newmode & ~S_IFMT) | S_IFSOCK;
  return 0;
}

int
fhandler_socket::fchown (uid_t newuid, gid_t newgid)
{
  bool perms = check_token_membership (&well_known_admins_sid);

  /* Admin rulez */
  if (!perms)
    {
      /* Otherwise, new uid == old uid or current uid is fine */
      if (newuid == ILLEGAL_UID || newuid == uid || newuid == myself->uid)
	perms = true;
      /* Otherwise, new gid == old gid or current gid is fine */
      else if (newgid == ILLEGAL_GID || newgid == gid || newgid == myself->gid)
	perms = true;
      else
	{
	  /* Last but not least, newgid in supplementary group list is fine */
	  tmp_pathbuf tp;
	  gid_t *gids = (gid_t *) tp.w_get ();
	  int num = getgroups (65536 / sizeof (*gids), gids);

	  for (int idx = 0; idx < num; ++idx)
	    if (newgid == gids[idx])
	      {
		perms = true;
		break;
	      }
	}
   }

  if (perms)
    {
      if (newuid != ILLEGAL_UID)
	uid = newuid;
      if (newgid != ILLEGAL_GID)
	gid = newgid;
      return 0;
    }
  set_errno (EPERM);
  return -1;
}

int
fhandler_socket::facl (int cmd, int nentries, aclent_t *aclbufp)
{
  set_errno (EOPNOTSUPP);
  return -1;
}

int
fhandler_socket::link (const char *newpath)
{
  return fhandler_base::link (newpath);
}

/* Definitions of old ifreq stuff used prior to Cygwin 1.7.0. */
#define OLD_SIOCGIFFLAGS    _IOW('s', 101, struct __old_ifreq)
#define OLD_SIOCGIFADDR     _IOW('s', 102, struct __old_ifreq)
#define OLD_SIOCGIFBRDADDR  _IOW('s', 103, struct __old_ifreq)
#define OLD_SIOCGIFNETMASK  _IOW('s', 104, struct __old_ifreq)
#define OLD_SIOCGIFHWADDR   _IOW('s', 105, struct __old_ifreq)
#define OLD_SIOCGIFMETRIC   _IOW('s', 106, struct __old_ifreq)
#define OLD_SIOCGIFMTU      _IOW('s', 107, struct __old_ifreq)
#define OLD_SIOCGIFINDEX    _IOW('s', 108, struct __old_ifreq)

#define CONV_OLD_TO_NEW_SIO(old) (((old)&0xff00ffff)|(((long)sizeof(struct ifreq)&IOCPARM_MASK)<<16))

struct __old_ifreq {
#define __OLD_IFNAMSIZ	16
  union {
    char    ifrn_name[__OLD_IFNAMSIZ];   /* if name, e.g. "en0" */
  } ifr_ifrn;

  union {
    struct  sockaddr ifru_addr;
    struct  sockaddr ifru_broadaddr;
    struct  sockaddr ifru_netmask;
    struct  sockaddr ifru_hwaddr;
    short   ifru_flags;
    int     ifru_metric;
    int     ifru_mtu;
    int     ifru_ifindex;
  } ifr_ifru;
};

int
fhandler_socket::ioctl (unsigned int cmd, void *p)
{
  extern int get_ifconf (struct ifconf *ifc, int what); /* net.cc */
  int res;
  struct ifconf ifc, *ifcp;
  struct ifreq *ifrp;

  switch (cmd)
    {
    case SIOCGIFCONF:
      ifcp = (struct ifconf *) p;
      if (!ifcp)
	{
	  set_errno (EINVAL);
	  return -1;
	}
      if (CYGWIN_VERSION_CHECK_FOR_OLD_IFREQ)
	{
	  ifc.ifc_len = ifcp->ifc_len / sizeof (struct __old_ifreq)
			* sizeof (struct ifreq);
	  ifc.ifc_buf = (caddr_t) alloca (ifc.ifc_len);
	}
      else
	{
	  ifc.ifc_len = ifcp->ifc_len;
	  ifc.ifc_buf = ifcp->ifc_buf;
	}
      res = get_ifconf (&ifc, cmd);
      if (res)
	debug_printf ("error in get_ifconf");
      if (CYGWIN_VERSION_CHECK_FOR_OLD_IFREQ)
	{
	  struct __old_ifreq *ifr = (struct __old_ifreq *) ifcp->ifc_buf;
	  for (ifrp = ifc.ifc_req;
	       (caddr_t) ifrp < ifc.ifc_buf + ifc.ifc_len;
	       ++ifrp, ++ifr)
	    {
	      memcpy (&ifr->ifr_ifrn, &ifrp->ifr_ifrn, sizeof ifr->ifr_ifrn);
	      ifr->ifr_name[__OLD_IFNAMSIZ - 1] = '\0';
	      memcpy (&ifr->ifr_ifru, &ifrp->ifr_ifru, sizeof ifr->ifr_ifru);
	    }
	  ifcp->ifc_len = ifc.ifc_len / sizeof (struct ifreq)
			  * sizeof (struct __old_ifreq);
	}
      else
	ifcp->ifc_len = ifc.ifc_len;
      break;
    case OLD_SIOCGIFFLAGS:
    case OLD_SIOCGIFADDR:
    case OLD_SIOCGIFBRDADDR:
    case OLD_SIOCGIFNETMASK:
    case OLD_SIOCGIFHWADDR:
    case OLD_SIOCGIFMETRIC:
    case OLD_SIOCGIFMTU:
    case OLD_SIOCGIFINDEX:
      cmd = CONV_OLD_TO_NEW_SIO (cmd);
      /*FALLTHRU*/
    case SIOCGIFFLAGS:
    case SIOCGIFBRDADDR:
    case SIOCGIFNETMASK:
    case SIOCGIFADDR:
    case SIOCGIFHWADDR:
    case SIOCGIFMETRIC:
    case SIOCGIFMTU:
    case SIOCGIFINDEX:
    case SIOCGIFFRNDLYNAM:
    case SIOCGIFDSTADDR:
      {
	if (!p)
	  {
	    debug_printf ("ifr == NULL");
	    set_errno (EINVAL);
	    return -1;
	  }

	if (cmd > SIOCGIFINDEX && CYGWIN_VERSION_CHECK_FOR_OLD_IFREQ)
	  {
	    debug_printf ("cmd not supported on this platform");
	    set_errno (EINVAL);
	    return -1;
	  }
	ifc.ifc_len = 64 * sizeof (struct ifreq);
	ifc.ifc_buf = (caddr_t) alloca (ifc.ifc_len);
	if (cmd == SIOCGIFFRNDLYNAM)
	  {
	    struct ifreq_frndlyname *iff = (struct ifreq_frndlyname *)
				alloca (64 * sizeof (struct ifreq_frndlyname));
	    for (int i = 0; i < 64; ++i)
	      ifc.ifc_req[i].ifr_frndlyname = &iff[i];
	  }

	res = get_ifconf (&ifc, cmd);
	if (res)
	  {
	    debug_printf ("error in get_ifconf");
	    break;
	  }

	if (CYGWIN_VERSION_CHECK_FOR_OLD_IFREQ)
	  {
	    struct __old_ifreq *ifr = (struct __old_ifreq *) p;
	    debug_printf ("    name: %s", ifr->ifr_name);
	    for (ifrp = ifc.ifc_req;
		 (caddr_t) ifrp < ifc.ifc_buf + ifc.ifc_len;
		 ++ifrp)
	      {
		debug_printf ("testname: %s", ifrp->ifr_name);
		if (! strcmp (ifrp->ifr_name, ifr->ifr_name))
		  {
		    memcpy (&ifr->ifr_ifru, &ifrp->ifr_ifru,
			    sizeof ifr->ifr_ifru);
		    break;
		  }
	      }
	  }
	else
	  {
	    struct ifreq *ifr = (struct ifreq *) p;
	    debug_printf ("    name: %s", ifr->ifr_name);
	    for (ifrp = ifc.ifc_req;
		 (caddr_t) ifrp < ifc.ifc_buf + ifc.ifc_len;
		 ++ifrp)
	      {
		debug_printf ("testname: %s", ifrp->ifr_name);
		if (! strcmp (ifrp->ifr_name, ifr->ifr_name))
		  {
		    if (cmd == SIOCGIFFRNDLYNAM)
		      /* The application has to care for the space. */
		      memcpy (ifr->ifr_frndlyname, ifrp->ifr_frndlyname,
			      sizeof (struct ifreq_frndlyname));
		    else
		      memcpy (&ifr->ifr_ifru, &ifrp->ifr_ifru,
			      sizeof ifr->ifr_ifru);
		    break;
		  }
	      }
	  }
	if ((caddr_t) ifrp >= ifc.ifc_buf + ifc.ifc_len)
	  {
	    set_errno (EINVAL);
	    return -1;
	  }
	break;
      }
    default:
      res = fhandler_base::ioctl (cmd, p);
      break;
    }
  syscall_printf ("%d = ioctl_socket(%x, %p)", res, cmd, p);
  return res;
}

int
fhandler_socket::fcntl (int cmd, intptr_t arg)
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
	int new_flags = arg & O_NONBLOCK_MASK;
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
fhandler_socket::set_close_on_exec (bool val)
{
  set_no_inheritance (wsock_mtx, val);
  set_no_inheritance (wsock_evt, val);
  if (need_fixup_before ())
    {
      close_on_exec (val);
      debug_printf ("set close_on_exec for %s to %d", get_name (), val);
    }
  else
    fhandler_base::set_close_on_exec (val);
}

int
fhandler_socket::getpeereid (pid_t *pid, uid_t *euid, gid_t *egid)
{
  set_errno (EINVAL);
  return -1;
}

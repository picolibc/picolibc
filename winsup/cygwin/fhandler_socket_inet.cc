/* fhandler_socket_inet.cc.

   See fhandler.h for a description of the fhandler classes.

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
#include <w32api/ws2tcpip.h>
#include <w32api/mswsock.h>
#include <unistd.h>
#include <asm/byteorder.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/statvfs.h>
#include <cygwin/acl.h>
#include "cygerrno.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "shared_info.h"
#include "wininfo.h"

#define ASYNC_MASK (FD_READ|FD_WRITE|FD_OOB|FD_ACCEPT|FD_CONNECT)
#define EVENT_MASK (FD_READ|FD_WRITE|FD_OOB|FD_ACCEPT|FD_CONNECT|FD_CLOSE)

#define LOCK_EVENTS	\
  if (wsock_mtx && \
      WaitForSingleObject (wsock_mtx, INFINITE) != WAIT_FAILED) \
    {

#define UNLOCK_EVENTS \
      ReleaseMutex (wsock_mtx); \
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

/* cygwin internal: map sockaddr into internet domain address */
static int
get_inet_addr_inet (const struct sockaddr *in, int inlen,
		    struct sockaddr_storage *out, int *outlen)
{
  switch (in->sa_family)
    {
    case AF_INET:
      memcpy (out, in, inlen);
      *outlen = inlen;
      /* If the peer address given in connect or sendto is the ANY address,
	 Winsock fails with WSAEADDRNOTAVAIL, while Linux converts that into
	 a connection/send attempt to LOOPBACK.  We're doing the same here. */
      if (((struct sockaddr_in *) out)->sin_addr.s_addr == htonl (INADDR_ANY))
	((struct sockaddr_in *) out)->sin_addr.s_addr = htonl (INADDR_LOOPBACK);
      return 0;
    case AF_INET6:
      memcpy (out, in, inlen);
      *outlen = inlen;
      /* See comment in AF_INET case. */
      if (IN6_IS_ADDR_UNSPECIFIED (&((struct sockaddr_in6 *) out)->sin6_addr))
	((struct sockaddr_in6 *) out)->sin6_addr = in6addr_loopback;
      return 0;
    default:
      set_errno (EAFNOSUPPORT);
      return SOCKET_ERROR;
    }
}

/* There's no DLL which exports the symbol WSARecvMsg.  One has to call
   WSAIoctl as below to fetch the function pointer.  Why on earth did the
   MS developers decide not to export a normal symbol for these extension
   functions? */
inline int
get_ext_funcptr (SOCKET sock, void *funcptr)
{
  DWORD bret;
  const GUID guid = WSAID_WSARECVMSG;
  return WSAIoctl (sock, SIO_GET_EXTENSION_FUNCTION_POINTER,
		   (void *) &guid, sizeof (GUID), funcptr, sizeof (void *),
		   &bret, NULL, NULL);
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

fhandler_socket_wsock::fhandler_socket_wsock () :
  fhandler_socket (),
  wsock_events (NULL),
  wsock_mtx (NULL),
  wsock_evt (NULL),
  status (),
  prot_info_ptr (NULL)
{
  need_fork_fixup (true);
}

fhandler_socket_wsock::~fhandler_socket_wsock ()
{
  if (prot_info_ptr)
    cfree (prot_info_ptr);
}

bool
fhandler_socket_wsock::init_events ()
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
fhandler_socket_wsock::evaluate_events (const long event_mask, long &events,
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
fhandler_socket_wsock::wait_for_events (const long event_mask,
					const DWORD flags)
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
fhandler_socket_wsock::release_events ()
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

void
fhandler_socket_wsock::set_close_on_exec (bool val)
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

/* Called if a freshly created socket is not inheritable.  In that case we
   have to use fixup_before_fork_exec.  See comment in set_socket_handle for
   a description of the problem. */
void
fhandler_socket_wsock::init_fixup_before ()
{
  prot_info_ptr = (LPWSAPROTOCOL_INFOW)
		  cmalloc_abort (HEAP_BUF, sizeof (WSAPROTOCOL_INFOW));
  cygheap->fdtab.inc_need_fixup_before ();
}

int
fhandler_socket_wsock::fixup_before_fork_exec (DWORD win_pid)
{
  SOCKET ret = WSADuplicateSocketW (get_socket (), win_pid, prot_info_ptr);
  if (ret)
    set_winsock_errno ();
  else
    debug_printf ("WSADuplicateSocket succeeded (%x)", prot_info_ptr->dwProviderReserved);
  return (int) ret;
}

void
fhandler_socket_wsock::fixup_after_fork (HANDLE parent)
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
fhandler_socket_wsock::fixup_after_exec ()
{
  if (need_fixup_before () && !close_on_exec ())
    fixup_after_fork (NULL);
}

int
fhandler_socket_wsock::dup (fhandler_base *child, int flags)
{
  debug_printf ("here");
  fhandler_socket_wsock *fhs = (fhandler_socket_wsock *) child;

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

int
fhandler_socket_wsock::set_socket_handle (SOCKET sock, int af, int type,
					  int flags)
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

fhandler_socket_inet::fhandler_socket_inet () :
  fhandler_socket_wsock ()
{
}

fhandler_socket_inet::~fhandler_socket_inet ()
{
}

int
fhandler_socket_inet::socket (int af, int type, int protocol, int flags)
{
  SOCKET sock;
  int ret;

  /* This test should be covered by ::socket, but make sure we don't
     accidentally try anything else. */
  if (type != SOCK_STREAM && type != SOCK_DGRAM && type != SOCK_RAW)
        {
          set_errno (EINVAL);
          return -1;
        }
  sock = ::socket (af, type, protocol);
  if (sock == INVALID_SOCKET)
    {
      set_winsock_errno ();
      return -1;
    }
  ret = set_socket_handle (sock, af, type, flags);
  if (ret < 0)
    ::closesocket (sock);
  return ret;
}

int
fhandler_socket_inet::socketpair (int af, int type, int protocol, int flags,
				  fhandler_socket *fh_out)
{
  set_errno (EAFNOSUPPORT);
  return -1;
}

int
fhandler_socket_inet::bind (const struct sockaddr *name, int namelen)
{
  int res = -1;

  if (!saw_reuseaddr ())
    {
      /* If the application didn't explicitely request SO_REUSEADDR,
	 enforce POSIX standard socket binding behaviour by setting the
	 SO_EXCLUSIVEADDRUSE socket option.  See cygwin_setsockopt()
	 for a more detailed description. */
      int on = 1;
      int ret = ::setsockopt (get_socket (), SOL_SOCKET,
			      SO_EXCLUSIVEADDRUSE,
			      (const char *) &on, sizeof on);
      debug_printf ("%d = setsockopt(SO_EXCLUSIVEADDRUSE), %E", ret);
    }
  if (::bind (get_socket (), name, namelen))
    set_winsock_errno ();
  else
    res = 0;

  return res;
}

int
fhandler_socket_inet::connect (const struct sockaddr *name, int namelen)
{
  struct sockaddr_storage sst;

  if (get_inet_addr_inet (name, namelen, &sst, &namelen) == SOCKET_ERROR)
    return SOCKET_ERROR;

  /* Initialize connect state to "connect_pending".  State is ultimately set
     to "connected" or "connect_failed" in wait_for_events when the FD_CONNECT
     event occurs.  Note that the underlying OS sockets are always non-blocking
     and a successfully initiated non-blocking Winsock connect always returns
     WSAEWOULDBLOCK.  Thus it's safe to rely on event handling.

     Check for either unconnected or connect_failed since in both cases it's
     allowed to retry connecting the socket.  It's also ok (albeit ugly) to
     call connect to check if a previous non-blocking connect finished.

     Set connect_state before calling connect, otherwise a race condition with
     an already running select or poll might occur. */
  if (connect_state () == unconnected || connect_state () == connect_failed)
    connect_state (connect_pending);

  int res = ::connect (get_socket (), (struct sockaddr *) &sst, namelen);
  if (!is_nonblocking ()
      && res == SOCKET_ERROR
      && WSAGetLastError () == WSAEWOULDBLOCK)
    res = wait_for_events (FD_CONNECT | FD_CLOSE, 0);

  if (res)
    {
      DWORD err = WSAGetLastError ();

      /* Some applications use the ugly technique to check if a non-blocking
         connect succeeded by calling connect again, until it returns EISCONN.
	 This circumvents the event handling and connect_state is never set.
	 Thus we check for this situation here. */
      if (err == WSAEISCONN)
	connect_state (connected);
      /* Winsock returns WSAEWOULDBLOCK if the non-blocking socket cannot be
         conected immediately.  Convert to POSIX/Linux compliant EINPROGRESS. */
      else if (is_nonblocking () && err == WSAEWOULDBLOCK)
	WSASetLastError (WSAEINPROGRESS);
      /* Winsock returns WSAEINVAL if the socket is already a listener.
	 Convert to POSIX/Linux compliant EISCONN. */
      else if (err == WSAEINVAL && connect_state () == listener)
	WSASetLastError (WSAEISCONN);
      /* Any other error except WSAEALREADY during connect_pending means the
         connect failed. */
      else if (connect_state () == connect_pending && err != WSAEALREADY)
	connect_state (connect_failed);
      set_winsock_errno ();
    }

  return res;
}

int
fhandler_socket_inet::listen (int backlog)
{
  int res = ::listen (get_socket (), backlog);
  if (res && WSAGetLastError () == WSAEINVAL)
    {
      /* It's perfectly valid to call listen on an unbound INET socket.
	 In this case the socket is automatically bound to an unused
	 port number, listening on all interfaces.  On WinSock, listen
	 fails with WSAEINVAL when it's called on an unbound socket.
	 So we have to bind manually here to have POSIX semantics. */
      if (get_addr_family () == AF_INET)
	{
	  struct sockaddr_in sin;
	  sin.sin_family = AF_INET;
	  sin.sin_port = 0;
	  sin.sin_addr.s_addr = INADDR_ANY;
	  if (!::bind (get_socket (), (struct sockaddr *) &sin, sizeof sin))
	    res = ::listen (get_socket (), backlog);
	}
      else if (get_addr_family () == AF_INET6)
	{
	  struct sockaddr_in6 sin6;
	  memset (&sin6, 0, sizeof sin6);
	  sin6.sin6_family = AF_INET6;
	  if (!::bind (get_socket (), (struct sockaddr *) &sin6, sizeof sin6))
	    res = ::listen (get_socket (), backlog);
	}
    }
  if (!res)
    connect_state (listener);	/* gets set to connected on accepted socket. */
  else
    set_winsock_errno ();
  return res;
}

int
fhandler_socket_inet::accept4 (struct sockaddr *peer, int *len, int flags)
{
  int ret = -1;
  /* Allows NULL peer and len parameters. */
  struct sockaddr_storage lpeer;
  int llen = sizeof (struct sockaddr_storage);

  /* Windows event handling does not check for the validity of the desired
     flags so we have to do it here. */
  if (connect_state () != listener)
    {
      WSASetLastError (WSAEINVAL);
      set_winsock_errno ();
      return -1;
    }

  SOCKET res = INVALID_SOCKET;
  while (!(res = wait_for_events (FD_ACCEPT | FD_CLOSE, 0))
	 && (res = ::accept (get_socket (), (struct sockaddr *) &lpeer, &llen))
	    == INVALID_SOCKET
	 && WSAGetLastError () == WSAEWOULDBLOCK)
    ;
  if (res == INVALID_SOCKET)
    set_winsock_errno ();
  else
    {
      cygheap_fdnew fd;

      if (fd >= 0)
	{
	  fhandler_socket_inet *sock = (fhandler_socket_inet *)
				       build_fh_dev (dev ());
	  if (sock && sock->set_socket_handle (res, get_addr_family (),
					       get_socket_type (),
					       get_socket_flags ()) == 0)
	    {
	      sock->async_io (false); /* set_socket_handle disables async. */
	      /* No locking necessary at this point. */
	      sock->wsock_events->events = wsock_events->events | FD_WRITE;
	      sock->wsock_events->owner = wsock_events->owner;
	      sock->connect_state (connected);
	      fd = sock;
	      if (fd <= 2)
		set_std_handle (fd);
	      ret = fd;
	      if (peer)
		{
		  memcpy (peer, &lpeer, MIN (*len, llen));
		  *len = llen;
		}
	    }
	  else
	    fd.release ();
	}
      if (ret == -1)
	::closesocket (res);
    }
  return ret;
}

int
fhandler_socket_inet::getsockname (struct sockaddr *name, int *namelen)
{
  int res = -1;

  /* WinSock just returns WSAEFAULT if the buffer is too small.  Use a
     big enough local buffer and truncate later as necessary, per POSIX. */
  struct sockaddr_storage sock;
  int len = sizeof sock;
  res = ::getsockname (get_socket (), (struct sockaddr *) &sock, &len);
  if (!res)
    {
      memcpy (name, &sock, MIN (*namelen, len));
      *namelen = len;
    }
  else
    {
      if (WSAGetLastError () == WSAEINVAL)
	{
	  /* WinSock returns WSAEINVAL if the socket is locally
	     unbound.  Per SUSv3 this is not an error condition.
	     We're faking a valid return value here by creating the
	     same content in the sockaddr structure as on Linux. */
	  memset (&sock, 0, sizeof sock);
	  sock.ss_family = get_addr_family ();
	  switch (get_addr_family ())
	    {
	    case AF_INET:
	      res = 0;
	      len = (int) sizeof (struct sockaddr_in);
	      break;
	    case AF_INET6:
	      res = 0;
	      len = (int) sizeof (struct sockaddr_in6);
	      break;
	    default:
	      WSASetLastError (WSAEOPNOTSUPP);
	      break;
	    }
	  if (!res)
	    {
	      memcpy (name, &sock, MIN (*namelen, len));
	      *namelen = len;
	    }
	}
      if (res)
	set_winsock_errno ();
    }
  return res;
}

int
fhandler_socket_inet::getpeername (struct sockaddr *name, int *namelen)
{
  /* Always use a local big enough buffer and truncate later as necessary
     per POSIX.  WinSock unfortunately only returns WSAEFAULT if the buffer
     is too small. */
  struct sockaddr_storage sock;
  int len = sizeof sock;
  int res = ::getpeername (get_socket (), (struct sockaddr *) &sock, &len);
  if (res)
    set_winsock_errno ();
  else
    {
      memcpy (name, &sock, MIN (*namelen, len));
      *namelen = len;
    }
  return res;
}

int
fhandler_socket_wsock::shutdown (int how)
{
  int res = ::shutdown (get_socket (), how);

  /* Linux allows to call shutdown for any socket, even if it's not connected.
     This also disables to call accept on this socket, if shutdown has been
     called with the SHUT_RD or SHUT_RDWR parameter.  In contrast, WinSock
     only allows to call shutdown on a connected socket.  The accept function
     is in no way affected.  So, what we do here is to fake success, and to
     change the event settings so that an FD_CLOSE event is triggered for the
     calling Cygwin function.  The evaluate_events method handles the call
     from accept specially to generate a Linux-compatible behaviour. */
  if (res && WSAGetLastError () != WSAENOTCONN)
    set_winsock_errno ();
  else
    {
      res = 0;
      switch (how)
	{
	case SHUT_RD:
	  saw_shutdown_read (true);
	  wsock_events->events |= FD_CLOSE;
	  SetEvent (wsock_evt);
	  break;
	case SHUT_WR:
	  saw_shutdown_write (true);
	  break;
	case SHUT_RDWR:
	  saw_shutdown_read (true);
	  saw_shutdown_write (true);
	  wsock_events->events |= FD_CLOSE;
	  SetEvent (wsock_evt);
	  break;
	}
    }
  return res;
}

int
fhandler_socket_wsock::close ()
{
  int res = 0;

  release_events ();
  while ((res = ::closesocket (get_socket ())) != 0)
    {
      if (WSAGetLastError () != WSAEWOULDBLOCK)
	{
	  set_winsock_errno ();
	  res = -1;
	  break;
	}
      if (cygwait (10) == WAIT_SIGNALED)
	{
	  set_errno (EINTR);
	  res = -1;
	  break;
	}
      WSASetLastError (0);
    }
  return res;
}

ssize_t
fhandler_socket_inet::recv_internal (LPWSAMSG wsamsg, bool use_recvmsg)
{
  ssize_t res = 0;
  DWORD ret = 0, wret;
  int evt_mask = FD_READ | ((wsamsg->dwFlags & MSG_OOB) ? FD_OOB : 0);
  LPWSABUF &wsabuf = wsamsg->lpBuffers;
  ULONG &wsacnt = wsamsg->dwBufferCount;
  static NO_COPY LPFN_WSARECVMSG WSARecvMsg;

  /* CV 2014-10-26: Do not check for the connect_state at this point.  In
     certain scenarios there's no way to check the connect state reliably.
     Example (hexchat): Parent process creates socket, forks, child process
     calls connect, parent process calls read.  Even if the event handling
     allows to check for FD_CONNECT in the parent, there is always yet another
     scenario we can easily break. */

  DWORD wait_flags = wsamsg->dwFlags;
  bool waitall = !!(wait_flags & MSG_WAITALL);
  wsamsg->dwFlags &= (MSG_OOB | MSG_PEEK | MSG_DONTROUTE);
  if (use_recvmsg)
    {
      if (!WSARecvMsg
	  && get_ext_funcptr (get_socket (), &WSARecvMsg) == SOCKET_ERROR)
	{
	  if (wsamsg->Control.len > 0)
	    {
	      set_winsock_errno ();
	      return SOCKET_ERROR;
	    }
	  use_recvmsg = false;
	}
      else /* Only MSG_PEEK is supported by WSARecvMsg. */
	wsamsg->dwFlags &= MSG_PEEK;
    }
  if (waitall)
    {
      if (get_socket_type () != SOCK_STREAM)
	{
	  WSASetLastError (WSAEOPNOTSUPP);
	  set_winsock_errno ();
	  return SOCKET_ERROR;
	}
      if (is_nonblocking () || (wsamsg->dwFlags & (MSG_OOB | MSG_PEEK)))
	waitall = false;
    }

  /* Note: Don't call WSARecvFrom(MSG_PEEK) without actually having data
     waiting in the buffers, otherwise the event handling gets messed up
     for some reason. */
  while (!(res = wait_for_events (evt_mask | FD_CLOSE, wait_flags))
	 || saw_shutdown_read ())
    {
      if (use_recvmsg)
	res = WSARecvMsg (get_socket (), wsamsg, &wret, NULL, NULL);
      /* This is working around a really weird problem in WinSock.

	 Assume you create a socket, fork the process (thus duplicating
	 the socket), connect the socket in the child, then call recv
	 on the original socket handle in the parent process.
	 In this scenario, calls to WinSock's recvfrom and WSARecvFrom
	 in the parent will fail with WSAEINVAL, regardless whether both
	 address parameters, name and namelen, are NULL or point to valid
	 storage.  However, calls to recv and WSARecv succeed as expected.
	 Per MSDN, WSAEINVAL in the context of recv means  "The socket has not
	 been bound".  It is as if the recvfrom functions test if the socket
	 is bound locally, but in the parent process, WinSock doesn't know
	 about that and fails, while the same test is omitted in the recv
	 functions.

	 This also covers another weird case: WinSock returns WSAEFAULT if
	 namelen is a valid pointer while name is NULL.  Both parameters are
	 ignored for TCP sockets, so this only occurs when using UDP socket. */
      else if (!wsamsg->name || get_socket_type () == SOCK_STREAM)
	res = WSARecv (get_socket (), wsabuf, wsacnt, &wret, &wsamsg->dwFlags,
		       NULL, NULL);
      else
	res = WSARecvFrom (get_socket (), wsabuf, wsacnt, &wret,
			   &wsamsg->dwFlags, wsamsg->name, &wsamsg->namelen,
			   NULL, NULL);
      if (!res)
	{
	  ret += wret;
	  if (!waitall)
	    break;
	  while (wret && wsacnt)
	    {
	      if (wsabuf->len > wret)
		{
		  wsabuf->len -= wret;
		  wsabuf->buf += wret;
		  wret = 0;
		}
	      else
		{
		  wret -= wsabuf->len;
		  ++wsabuf;
		  --wsacnt;
		}
	    }
	  if (!wret)
	    break;
	}
      else if (WSAGetLastError () != WSAEWOULDBLOCK)
	break;
    }

  if (res)
    {
      /* According to SUSv3, errno isn't set in that case and no error
	 condition is returned. */
      if (WSAGetLastError () == WSAEMSGSIZE)
	ret += wret;
      else if (!ret)
	{
	  /* ESHUTDOWN isn't defined for recv in SUSv3.  Simply EOF is returned
	     in this case. */
	  if (WSAGetLastError () == WSAESHUTDOWN)
	    ret = 0;
	  else
	    {
	      set_winsock_errno ();
	      return SOCKET_ERROR;
	    }
	}
    }

  return ret;
}

ssize_t
fhandler_socket_wsock::recvfrom (void *in_ptr, size_t len, int flags,
				 struct sockaddr *from, int *fromlen)
{
  char *ptr = (char *) in_ptr;

#ifdef __x86_64__
  /* size_t is 64 bit, but the len member in WSABUF is 32 bit.
     Split buffer if necessary. */
  DWORD bufcnt = len / UINT32_MAX + ((!len || (len % UINT32_MAX)) ? 1 : 0);
  WSABUF wsabuf[bufcnt];
  WSAMSG wsamsg = { from, from && fromlen ? *fromlen : 0,
		    wsabuf, bufcnt,
		    { 0,  NULL },
		    (DWORD) flags };
  /* Don't use len as loop condition, it could be 0. */
  for (WSABUF *wsaptr = wsabuf; bufcnt--; ++wsaptr)
    {
      wsaptr->len = MIN (len, UINT32_MAX);
      wsaptr->buf = ptr;
      len -= wsaptr->len;
      ptr += wsaptr->len;
    }
#else
  WSABUF wsabuf = { len, ptr };
  WSAMSG wsamsg = { from, from && fromlen ? *fromlen : 0,
		    &wsabuf, 1,
		    { 0, NULL},
		    (DWORD) flags };
#endif
  ssize_t ret = recv_internal (&wsamsg, false);
  if (fromlen)
    *fromlen = wsamsg.namelen;
  return ret;
}

ssize_t
fhandler_socket_wsock::recvmsg (struct msghdr *msg, int flags)
{
  /* Disappointing but true:  Even if WSARecvMsg is supported, it's only
     supported for datagram and raw sockets. */
  bool use_recvmsg = true;
  if (get_socket_type () == SOCK_STREAM || get_addr_family () == AF_LOCAL)
    {
      use_recvmsg = false;
      msg->msg_controllen = 0;
    }

  WSABUF wsabuf[msg->msg_iovlen];
  WSABUF *wsaptr = wsabuf + msg->msg_iovlen;
  const struct iovec *iovptr = msg->msg_iov + msg->msg_iovlen;
  while (--wsaptr >= wsabuf)
    {
      wsaptr->len = (--iovptr)->iov_len;
      wsaptr->buf = (char *) iovptr->iov_base;
    }
  WSAMSG wsamsg = { (struct sockaddr *) msg->msg_name, msg->msg_namelen,
		    wsabuf, (DWORD) msg->msg_iovlen,
		    { (DWORD) msg->msg_controllen, (char *) msg->msg_control },
		    (DWORD) flags };
  ssize_t ret = recv_internal (&wsamsg, use_recvmsg);
  if (ret >= 0)
    {
      msg->msg_namelen = wsamsg.namelen;
      msg->msg_controllen = wsamsg.Control.len;
      if (!CYGWIN_VERSION_CHECK_FOR_USING_ANCIENT_MSGHDR)
	msg->msg_flags = wsamsg.dwFlags;
    }
  return ret;
}

void __reg3
fhandler_socket_wsock::read (void *in_ptr, size_t& len)
{
  char *ptr = (char *) in_ptr;

#ifdef __x86_64__
  /* size_t is 64 bit, but the len member in WSABUF is 32 bit.
     Split buffer if necessary. */
  DWORD bufcnt = len / UINT32_MAX + ((!len || (len % UINT32_MAX)) ? 1 : 0);
  WSABUF wsabuf[bufcnt];
  WSAMSG wsamsg = { NULL, 0, wsabuf, bufcnt, { 0,  NULL }, 0 };
  /* Don't use len as loop condition, it could be 0. */
  for (WSABUF *wsaptr = wsabuf; bufcnt--; ++wsaptr)
    {
      wsaptr->len = MIN (len, UINT32_MAX);
      wsaptr->buf = ptr;
      len -= wsaptr->len;
      ptr += wsaptr->len;
    }
#else
  WSABUF wsabuf = { len, ptr };
  WSAMSG wsamsg = { NULL, 0, &wsabuf, 1, { 0,  NULL }, 0 };
#endif

  len = recv_internal (&wsamsg, false);
}

ssize_t
fhandler_socket_wsock::readv (const struct iovec *const iov, const int iovcnt,
			      ssize_t tot)
{
  WSABUF wsabuf[iovcnt];
  WSABUF *wsaptr = wsabuf + iovcnt;
  const struct iovec *iovptr = iov + iovcnt;
  while (--wsaptr >= wsabuf)
    {
      wsaptr->len = (--iovptr)->iov_len;
      wsaptr->buf = (char *) iovptr->iov_base;
    }
  WSAMSG wsamsg = { NULL, 0, wsabuf, (DWORD) iovcnt, { 0,  NULL}, 0 };
  return recv_internal (&wsamsg, false);
}

ssize_t
fhandler_socket_wsock::send_internal (struct _WSAMSG *wsamsg, int flags)
{
  ssize_t res = 0;
  DWORD ret = 0, sum = 0;
  WSABUF out_buf[wsamsg->dwBufferCount];
  bool use_sendmsg = false;
  DWORD wait_flags = flags & MSG_DONTWAIT;
  bool nosignal = !!(flags & MSG_NOSIGNAL);

  flags &= (MSG_OOB | MSG_DONTROUTE);
  if (wsamsg->Control.len > 0)
    use_sendmsg = true;
  /* Workaround for MSDN KB 823764: Split a message into chunks <= SO_SNDBUF.
     in_idx is the index of the current lpBuffers from the input wsamsg buffer.
     in_off is used to keep track of the next byte to write from a wsamsg
     buffer which only gets partially written. */
  for (DWORD in_idx = 0, in_off = 0;
       in_idx < wsamsg->dwBufferCount;
       in_off >= wsamsg->lpBuffers[in_idx].len && (++in_idx, in_off = 0))
    {
      /* Split a message into the least number of pieces to minimize the
	 number of WsaSendTo calls.  Don't split datagram messages (bad idea).
	 out_idx is the index of the next buffer in the out_buf WSABUF,
	 also the number of buffers given to WSASendTo.
	 out_len is the number of bytes in the buffers given to WSASendTo.
	 Don't split datagram messages (very bad idea). */
      DWORD out_idx = 0;
      DWORD out_len = 0;
      if (get_socket_type () == SOCK_STREAM)
	{
	  do
	    {
	      out_buf[out_idx].buf = wsamsg->lpBuffers[in_idx].buf + in_off;
	      out_buf[out_idx].len = wsamsg->lpBuffers[in_idx].len - in_off;
	      out_len += out_buf[out_idx].len;
	      out_idx++;
	    }
	  while (out_len < (unsigned) wmem ()
		 && (in_off = 0, ++in_idx < wsamsg->dwBufferCount));
	  /* Tweak len of the last out_buf buffer so the entire number of bytes
	     is (less than or) equal to wmem ().  Fix out_len as well since it's
	     used in a subsequent test expression. */
	  if (out_len > (unsigned) wmem ())
	    {
	      out_buf[out_idx - 1].len -= out_len - (unsigned) wmem ();
	      out_len = (unsigned) wmem ();
	    }
	  /* Add the bytes written from the current last buffer to in_off,
	     so in_off points to the next byte to be written from that buffer,
	     or beyond which lets the outper loop skip to the next buffer. */
	  in_off += out_buf[out_idx - 1].len;
	}

      do
	{
	  if (use_sendmsg)
	    res = WSASendMsg (get_socket (), wsamsg, flags, &ret, NULL, NULL);
	  else if (get_socket_type () == SOCK_STREAM)
	    res = WSASendTo (get_socket (), out_buf, out_idx, &ret, flags,
			     wsamsg->name, wsamsg->namelen, NULL, NULL);
	  else
	    res = WSASendTo (get_socket (), wsamsg->lpBuffers,
			     wsamsg->dwBufferCount, &ret, flags,
			     wsamsg->name, wsamsg->namelen, NULL, NULL);
	  if (res && (WSAGetLastError () == WSAEWOULDBLOCK))
	    {
	      LOCK_EVENTS;
	      wsock_events->events &= ~FD_WRITE;
	      UNLOCK_EVENTS;
	    }
	}
      while (res && (WSAGetLastError () == WSAEWOULDBLOCK)
	     && !(res = wait_for_events (FD_WRITE | FD_CLOSE, wait_flags)));

      if (!res)
	{
	  sum += ret;
	  /* For streams, return to application if the number of bytes written
	     is less than the number of bytes we intended to write in a single
	     call to WSASendTo.  Otherwise we would have to add code to
	     backtrack in the input buffers, which is questionable.  There was
	     probably a good reason we couldn't write more. */
	  if (get_socket_type () != SOCK_STREAM || ret < out_len)
	    break;
	}
      else if (is_nonblocking () || WSAGetLastError() != WSAEWOULDBLOCK)
	break;
    }

  if (sum)
    res = sum;
  else if (res == SOCKET_ERROR)
    {
      set_winsock_errno ();

      /* Special handling for EPIPE and SIGPIPE.

	 EPIPE is generated if the local end has been shut down on a connection
	 oriented socket.  In this case the process will also receive a SIGPIPE
	 unless MSG_NOSIGNAL is set.  */
      if ((get_errno () == ECONNABORTED || get_errno () == ESHUTDOWN)
	  && get_socket_type () == SOCK_STREAM)
	{
	  set_errno (EPIPE);
	  if (!nosignal)
	    raise (SIGPIPE);
	}
    }

  return res;
}

ssize_t
fhandler_socket_inet::sendto (const void *in_ptr, size_t len, int flags,
			      const struct sockaddr *to, int tolen)
{
  char *ptr = (char *) in_ptr;
  struct sockaddr_storage sst;

  if (to && get_inet_addr_inet (to, tolen, &sst, &tolen) == SOCKET_ERROR)
    return SOCKET_ERROR;

#ifdef __x86_64__
  /* size_t is 64 bit, but the len member in WSABUF is 32 bit.
     Split buffer if necessary. */
  DWORD bufcnt = len / UINT32_MAX + ((!len || (len % UINT32_MAX)) ? 1 : 0);
  WSABUF wsabuf[bufcnt];
  WSAMSG wsamsg = { to ? (struct sockaddr *) &sst : NULL, tolen,
		    wsabuf, bufcnt,
		    { 0,  NULL },
		    0 };
  /* Don't use len as loop condition, it could be 0. */
  for (WSABUF *wsaptr = wsabuf; bufcnt--; ++wsaptr)
    {
      wsaptr->len = MIN (len, UINT32_MAX);
      wsaptr->buf = ptr;
      len -= wsaptr->len;
      ptr += wsaptr->len;
    }
#else
  WSABUF wsabuf = { len, ptr };
  WSAMSG wsamsg = { to ? (struct sockaddr *) &sst : NULL, tolen,
		    &wsabuf, 1,
		    { 0, NULL},
		    0 };
#endif
  return send_internal (&wsamsg, flags);
}

ssize_t
fhandler_socket_inet::sendmsg (const struct msghdr *msg, int flags)
{
  struct sockaddr_storage sst;
  int len = 0;

  if (msg->msg_name
      && get_inet_addr_inet ((struct sockaddr *) msg->msg_name,
			     msg->msg_namelen, &sst, &len) == SOCKET_ERROR)
    return SOCKET_ERROR;

  WSABUF wsabuf[msg->msg_iovlen];
  WSABUF *wsaptr = wsabuf;
  const struct iovec *iovptr = msg->msg_iov;
  for (int i = 0; i < msg->msg_iovlen; ++i)
    {
      wsaptr->len = iovptr->iov_len;
      (wsaptr++)->buf = (char *) (iovptr++)->iov_base;
    }
  /* Disappointing but true:  Even if WSASendMsg is supported, it's only
     supported for datagram and raw sockets. */
  DWORD controllen = (DWORD) ((get_socket_type () == SOCK_STREAM)
			      ? 0 : msg->msg_controllen);
  WSAMSG wsamsg = { msg->msg_name ? (struct sockaddr *) &sst : NULL, len,
		    wsabuf, (DWORD) msg->msg_iovlen,
		    { controllen, (char *) msg->msg_control },
		    0 };
  return send_internal (&wsamsg, flags);
}

ssize_t
fhandler_socket_wsock::write (const void *in_ptr, size_t len)
{
  char *ptr = (char *) in_ptr;

#ifdef __x86_64__
  /* size_t is 64 bit, but the len member in WSABUF is 32 bit.
     Split buffer if necessary. */
  DWORD bufcnt = len / UINT32_MAX + ((!len || (len % UINT32_MAX)) ? 1 : 0);
  WSABUF wsabuf[bufcnt];
  WSAMSG wsamsg = { NULL, 0, wsabuf, bufcnt, { 0,  NULL }, 0 };
  /* Don't use len as loop condition, it could be 0. */
  for (WSABUF *wsaptr = wsabuf; bufcnt--; ++wsaptr)
    {
      wsaptr->len = MIN (len, UINT32_MAX);
      wsaptr->buf = ptr;
      len -= wsaptr->len;
      ptr += wsaptr->len;
    }
#else
  WSABUF wsabuf = { len, ptr };
  WSAMSG wsamsg = { NULL, 0, &wsabuf, 1, { 0, NULL }, 0 };
#endif
  return send_internal (&wsamsg, 0);
}

ssize_t
fhandler_socket_wsock::writev (const struct iovec *const iov, const int iovcnt,
			       ssize_t tot)
{
  WSABUF wsabuf[iovcnt];
  WSABUF *wsaptr = wsabuf;
  const struct iovec *iovptr = iov;
  for (int i = 0; i < iovcnt; ++i)
    {
      wsaptr->len = iovptr->iov_len;
      (wsaptr++)->buf = (char *) (iovptr++)->iov_base;
    }
  WSAMSG wsamsg = { NULL, 0, wsabuf, (DWORD) iovcnt, { 0, NULL}, 0 };
  return send_internal (&wsamsg, 0);
}

int
fhandler_socket_inet::setsockopt (int level, int optname, const void *optval,
				  socklen_t optlen)
{
  bool ignore = false;
  int ret = -1;

  /* Preprocessing setsockopt.  Set ignore to true if setsockopt call should
     get skipped entirely. */
  switch (level)
    {
    case SOL_SOCKET:
      switch (optname)
	{
	case SO_PEERCRED:
	  set_errno (ENOPROTOOPT);
	  return -1;

	case SO_REUSEADDR:
	  /* Per POSIX we must not be able to reuse a complete duplicate of a
	     local TCP address (same IP, same port), even if SO_REUSEADDR has
	     been set.  This behaviour is maintained in WinSock for backward
	     compatibility, while the WinSock standard behaviour of stream
	     socket binding is equivalent to the POSIX behaviour as if
	     SO_REUSEADDR has been set.  The SO_EXCLUSIVEADDRUSE option has
	     been added to allow an application to request POSIX standard
	     behaviour in the non-SO_REUSEADDR case.

	     To emulate POSIX socket binding behaviour, note that SO_REUSEADDR
	     has been set but don't call setsockopt.  Instead
	     fhandler_socket::bind sets SO_EXCLUSIVEADDRUSE if the application
	     did not set SO_REUSEADDR. */
	  if (optlen < (socklen_t) sizeof (int))
	    {
	      set_errno (EINVAL);
	      return ret;
	    }
	  if (get_socket_type () == SOCK_STREAM)
	    ignore = true;
	  break;

	case SO_RCVTIMEO:
	case SO_SNDTIMEO:
	  if (optlen < (socklen_t) sizeof (struct timeval))
	    {
	      set_errno (EINVAL);
	      return ret;
	    }
	  if (timeval_to_ms ((struct timeval *) optval,
			     (optname == SO_RCVTIMEO) ? rcvtimeo ()
						      : sndtimeo ()))
	    ret = 0;
	  else
	    set_errno (EDOM);
	  return ret;

	default:
	  break;
	}
      break;

    case IPPROTO_IP:
      /* Old applications still use the old WinSock1 IPPROTO_IP values. */
      if (CYGWIN_VERSION_CHECK_FOR_USING_WINSOCK1_VALUES)
	optname = convert_ws1_ip_optname (optname);
      switch (optname)
	{
	case IP_TOS:
	  /* Winsock doesn't support setting the IP_TOS field with setsockopt
	     and TOS was never implemented for TCP anyway.  setsockopt returns
	     WinSock error 10022, WSAEINVAL when trying to set the IP_TOS
	     field.  We just return 0 instead. */
	  ignore = true;
	  break;

	default:
	  break;
	}
      break;

    case IPPROTO_IPV6:
      {
      switch (optname)
	{
	case IPV6_TCLASS:
	  /* Unsupported */
	  ignore = true;
	  break;

	default:
	  break;
	}
      }
    default:
      break;
    }

  /* Call Winsock setsockopt (or not) */
  if (ignore)
    ret = 0;
  else
    {
      ret = ::setsockopt (get_socket (), level, optname, (const char *) optval,
			  optlen);
      if (ret == SOCKET_ERROR)
	{
	  set_winsock_errno ();
	  return ret;
	}
    }

  if (optlen == (socklen_t) sizeof (int))
    debug_printf ("setsockopt optval=%x", *(int *) optval);

  /* Postprocessing setsockopt, setting fhandler_socket members, etc. */
  switch (level)
    {
    case SOL_SOCKET:
      switch (optname)
	{
	case SO_REUSEADDR:
	  saw_reuseaddr (*(int *) optval);
	  break;

	case SO_RCVBUF:
	  rmem (*(int *) optval);
	  break;

	case SO_SNDBUF:
	  wmem (*(int *) optval);
	  break;

	default:
	  break;
	}
      break;

    default:
      break;
    }

  return ret;
}

int
fhandler_socket_inet::getsockopt (int level, int optname, const void *optval,
				  socklen_t *optlen)
{
  bool onebyte = false;
  int ret = -1;

  /* Preprocessing getsockopt. */
  switch (level)
    {
    case SOL_SOCKET:
      switch (optname)
	{
	case SO_PEERCRED:
	  set_errno (ENOPROTOOPT);
	  return -1;

	case SO_REUSEADDR:
	  {
	    unsigned int *reuseaddr = (unsigned int *) optval;

	    if (*optlen < (socklen_t) sizeof *reuseaddr)
	      {
		set_errno (EINVAL);
		return -1;
	      }
	    *reuseaddr = saw_reuseaddr();
	    *optlen = (socklen_t) sizeof *reuseaddr;
	    return 0;
	  }

	case SO_RCVTIMEO:
	case SO_SNDTIMEO:
	  {
	    struct timeval *time_out = (struct timeval *) optval;

	    if (*optlen < (socklen_t) sizeof *time_out)
	      {
		set_errno (EINVAL);
		return -1;
	      }
	    DWORD ms = (optname == SO_RCVTIMEO) ? rcvtimeo () : sndtimeo ();
	    if (ms == 0 || ms == INFINITE)
	      {
		time_out->tv_sec = 0;
		time_out->tv_usec = 0;
	      }
	    else
	      {
		time_out->tv_sec = ms / MSPERSEC;
		time_out->tv_usec = ((ms % MSPERSEC) * USPERSEC) / MSPERSEC;
	      }
	    *optlen = (socklen_t) sizeof *time_out;
	    return 0;
	  }

	case SO_TYPE:
	  {
	    unsigned int *type = (unsigned int *) optval;
	    *type = get_socket_type ();
	    *optlen = (socklen_t) sizeof *type;
	    return 0;
	  }

	default:
	  break;
	}
      break;

    case IPPROTO_IP:
      /* Old applications still use the old WinSock1 IPPROTO_IP values. */
      if (CYGWIN_VERSION_CHECK_FOR_USING_WINSOCK1_VALUES)
	optname = convert_ws1_ip_optname (optname);
      break;

    default:
      break;
    }

  /* Call Winsock getsockopt */
  ret = ::getsockopt (get_socket (), level, optname, (char *) optval,
		      (int *) optlen);
  if (ret == SOCKET_ERROR)
    {
      set_winsock_errno ();
      return ret;
    }

  /* Postprocessing getsockopt, setting fhandler_socket members, etc.  Set
     onebyte true for options returning BOOLEAN instead of a boolean DWORD. */
  switch (level)
    {
    case SOL_SOCKET:
      switch (optname)
	{
	case SO_ERROR:
	  {
	    int *e = (int *) optval;
	    debug_printf ("WinSock SO_ERROR = %d", *e);
	    *e = find_winsock_errno (*e);
	  }
	  break;

	case SO_KEEPALIVE:
	case SO_DONTROUTE:
	  onebyte = true;
	  break;

	default:
	  break;
	}
      break;
    case IPPROTO_TCP:
      switch (optname)
	{
	case TCP_NODELAY:
	  onebyte = true;
	  break;

	default:
	  break;
	}
    default:
      break;
    }

  if (onebyte)
    {
      /* Regression in Vista and later: instead of a 4 byte BOOL value, a
	 1 byte BOOLEAN value is returned, in contrast to older systems and
	 the documentation.  Since an int type is expected by the calling
	 application, we convert the result here.  For some reason only three
	 BSD-compatible socket options seem to be affected. */
      BOOLEAN *in = (BOOLEAN *) optval;
      int *out = (int *) optval;
      *out = *in;
      *optlen = 4;
    }

  return ret;
}

int
fhandler_socket_wsock::ioctl (unsigned int cmd, void *p)
{
  int res;

  switch (cmd)
    {
    /* Here we handle only ioctl commands which are understood by Winsock.
       However, we have a problem, which is, the different size of u_long
       in Windows and 64 bit Cygwin.  This affects the definitions of
       FIOASYNC, etc, because they are defined in terms of sizeof(u_long).
       So we have to use case labels which are independent of the sizeof
       u_long.  Since we're redefining u_long at the start of this file to
       matching Winsock's idea of u_long, we can use the real definitions in
       calls to Windows.  In theory we also have to make sure to convert the
       different ideas of u_long between the application and Winsock, but
       fortunately, the parameters defined as u_long pointers are on Linux
       and BSD systems defined as int pointer, so the applications will
       use a type of the expected size.  Hopefully. */
    case FIOASYNC:
#ifdef __x86_64__
    case _IOW('f', 125, u_long):
#endif
      res = WSAAsyncSelect (get_socket (), winmsg, WM_ASYNCIO,
	      *(int *) p ? ASYNC_MASK : 0);
      syscall_printf ("Async I/O on socket %s",
	      *(int *) p ? "started" : "cancelled");
      async_io (*(int *) p != 0);
      /* If async_io is switched off, revert the event handling. */
      if (*(int *) p == 0)
	WSAEventSelect (get_socket (), wsock_evt, EVENT_MASK);
      break;
    case FIONREAD:
#ifdef __x86_64__
    case _IOR('f', 127, u_long):
#endif
      /* Make sure to use the Winsock definition of FIONREAD. */
      res = ::ioctlsocket (get_socket (), _IOR('f', 127, u_long), (u_long *) p);
      if (res == SOCKET_ERROR)
	set_winsock_errno ();
      break;
    case FIONBIO:
    case SIOCATMARK:
      /* Sockets are always non-blocking internally.  So we just note the
	 state here. */
#ifdef __x86_64__
      /* Convert the different idea of u_long in the definition of cmd. */
      if (((cmd >> 16) & IOCPARM_MASK) == sizeof (unsigned long))
	cmd = (cmd & ~(IOCPARM_MASK << 16)) | (sizeof (u_long) << 16);
#endif
      if (cmd == FIONBIO)
	{
	  syscall_printf ("socket is now %sblocking",
			    *(int *) p ? "non" : "");
	  set_nonblocking (*(int *) p);
	  res = 0;
	}
      else
	res = ::ioctlsocket (get_socket (), cmd, (u_long *) p);
      break;
    default:
      res = fhandler_socket::ioctl (cmd, p);
      break;
    }
  syscall_printf ("%d = ioctl_socket(%x, %p)", res, cmd, p);
  return res;
}

int
fhandler_socket_wsock::fcntl (int cmd, intptr_t arg)
{
  int res = 0;

  switch (cmd)
    {
    case F_SETOWN:
      {
	pid_t pid = (pid_t) arg;
	LOCK_EVENTS;
	wsock_events->owner = pid;
	UNLOCK_EVENTS;
	debug_printf ("owner set to %d", pid);
      }
      break;
    case F_GETOWN:
      res = wsock_events->owner;
      break;
    default:
      res = fhandler_socket::fcntl (cmd, arg);
      break;
    }
  return res;
}

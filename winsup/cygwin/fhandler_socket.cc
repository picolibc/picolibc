/* fhandler_socket.cc. See fhandler.h for a description of the fhandler classes.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

/* #define DEBUG_NEST_ON 1 */

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
#include <cygwin/acl.h>
#include "cygtls.h"
#include <sys/un.h>
#include "ntdll.h"
#include "miscfuncs.h"

#define ASYNC_MASK (FD_READ|FD_WRITE|FD_OOB|FD_ACCEPT|FD_CONNECT)
#define EVENT_MASK (FD_READ|FD_WRITE|FD_OOB|FD_ACCEPT|FD_CONNECT|FD_CLOSE)

extern bool fdsock (cygheap_fdmanip& fd, const device *, SOCKET soc);
extern "C" {
int sscanf (const char *, const char *, ...);
} /* End of "C" section */

static inline mode_t
adjust_socket_file_mode (mode_t mode)
{
  /* Kludge: Don't allow to remove read bit on socket files for
     user/group/other, if the accompanying write bit is set.  It would
     be nice to have exact permissions on a socket file, but it's
     necessary that somebody able to access the socket can always read
     the contents of the socket file to avoid spurious "permission
     denied" messages. */
  return mode | ((mode & (S_IWUSR | S_IWGRP | S_IWOTH)) << 1);
}

/* cygwin internal: map sockaddr into internet domain address */
int
get_inet_addr (const struct sockaddr *in, int inlen,
	       struct sockaddr_storage *out, int *outlen,
	       int *type = NULL, int *secret = NULL)
{
  int secret_buf [4];
  int* secret_ptr = (secret ? : secret_buf);

  switch (in->sa_family)
    {
    case AF_LOCAL:
      /* Check for abstract socket. These are generated for AF_LOCAL datagram
	 sockets in recv_internal, to allow a datagram server to use sendto
	 after recvfrom. */
      if (inlen >= (int) sizeof (in->sa_family) + 7
	  && in->sa_data[0] == '\0' && in->sa_data[1] == 'd'
	  && in->sa_data[6] == '\0')
	{
	  struct sockaddr_in addr;
	  addr.sin_family = AF_INET;
	  sscanf (in->sa_data + 2, "%04hx", &addr.sin_port);
	  addr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
	  *outlen = sizeof addr;
	  memcpy (out, &addr, *outlen);
	  return 0;
	}
      break;
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
  /* AF_LOCAL/AF_UNIX only */
  path_conv pc (in->sa_data, PC_SYM_FOLLOW);
  if (pc.error)
    {
      set_errno (pc.error);
      return SOCKET_ERROR;
    }
  if (!pc.exists ())
    {
      set_errno (ENOENT);
      return SOCKET_ERROR;
    }
  /* Do NOT test for the file being a socket file here.  The socket file
     creation is not an atomic operation, so there is a chance that socket
     files which are just in the process of being created are recognized
     as non-socket files.  To work around this problem we now create the
     file with all sharing disabled.  If the below NtOpenFile fails
     with STATUS_SHARING_VIOLATION we know that the file already exists,
     but the creating process isn't finished yet.  So we yield and try
     again, until we can either open the file successfully, or some error
     other than STATUS_SHARING_VIOLATION occurs.
     Since we now don't know if the file is actually a socket file, we
     perform this check here explicitely. */
  NTSTATUS status;
  HANDLE fh;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;

  pc.get_object_attr (attr, sec_none_nih);
  do
    {
      status = NtOpenFile (&fh, GENERIC_READ | SYNCHRONIZE, &attr, &io,
			   FILE_SHARE_VALID_FLAGS,
			   FILE_SYNCHRONOUS_IO_NONALERT
			   | FILE_OPEN_FOR_BACKUP_INTENT
			   | FILE_NON_DIRECTORY_FILE);
      if (status == STATUS_SHARING_VIOLATION)
	{
	  /* While we hope that the sharing violation is only temporary, we
	     also could easily get stuck here, waiting for a file in use by
	     some greedy Win32 application.  Therefore we should never wait
	     endlessly without checking for signals and thread cancel event. */
	  pthread_testcancel ();
	  if (cygwait (NULL, cw_nowait, cw_sig_eintr) == WAIT_SIGNALED
	      && !_my_tls.call_signal_handler ())
	    {
	      set_errno (EINTR);
	      return SOCKET_ERROR;
	    }
	  yield ();
	}
      else if (!NT_SUCCESS (status))
	{
	  __seterrno_from_nt_status (status);
	  return SOCKET_ERROR;
	}
    }
  while (status == STATUS_SHARING_VIOLATION);
  /* Now test for the SYSTEM bit. */
  FILE_BASIC_INFORMATION fbi;
  status = NtQueryInformationFile (fh, &io, &fbi, sizeof fbi,
				   FileBasicInformation);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return SOCKET_ERROR;
    }
  if (!(fbi.FileAttributes & FILE_ATTRIBUTE_SYSTEM))
    {
      NtClose (fh);
      set_errno (EBADF);
      return SOCKET_ERROR;
    }
  /* Eventually check the content and fetch the required information. */
  char buf[128];
  memset (buf, 0, sizeof buf);
  status = NtReadFile (fh, NULL, NULL, NULL, &io, buf, 128, NULL, NULL);
  NtClose (fh);
  if (NT_SUCCESS (status))
    {
      struct sockaddr_in sin;
      char ctype;
      sin.sin_family = AF_INET;
      if (strncmp (buf, SOCKET_COOKIE, strlen (SOCKET_COOKIE)))
	{
	  set_errno (EBADF);
	  return SOCKET_ERROR;
	}
      sscanf (buf + strlen (SOCKET_COOKIE), "%hu %c %08x-%08x-%08x-%08x",
	      &sin.sin_port,
	      &ctype,
	      secret_ptr, secret_ptr + 1, secret_ptr + 2, secret_ptr + 3);
      sin.sin_port = htons (sin.sin_port);
      sin.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
      memcpy (out, &sin, sizeof sin);
      *outlen = sizeof sin;
      if (type)
	*type = (ctype == 's' ? SOCK_STREAM :
		 ctype == 'd' ? SOCK_DGRAM
			      : 0);
      return 0;
    }
  __seterrno_from_nt_status (status);
  return SOCKET_ERROR;
}

/**********************************************************************/
/* fhandler_socket */

fhandler_socket::fhandler_socket () :
  fhandler_base (),
  wsock_events (NULL),
  wsock_mtx (NULL),
  wsock_evt (NULL),
  prot_info_ptr (NULL),
  sun_path (NULL),
  peer_sun_path (NULL),
  status ()
{
  need_fork_fixup (true);
}

fhandler_socket::~fhandler_socket ()
{
  if (prot_info_ptr)
    cfree (prot_info_ptr);
  if (sun_path)
    cfree (sun_path);
  if (peer_sun_path)
    cfree (peer_sun_path);
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

void
fhandler_socket::af_local_set_sockpair_cred ()
{
  sec_pid = sec_peer_pid = getpid ();
  sec_uid = sec_peer_uid = geteuid32 ();
  sec_gid = sec_peer_gid = getegid32 ();
}

void
fhandler_socket::af_local_setblocking (bool &async, bool &nonblocking)
{
  async = async_io ();
  nonblocking = is_nonblocking ();
  if (async)
    {
      WSAAsyncSelect (get_socket (), winmsg, 0, 0);
      WSAEventSelect (get_socket (), wsock_evt, EVENT_MASK);
    }
  set_nonblocking (false);
  async_io (false);
}

void
fhandler_socket::af_local_unsetblocking (bool async, bool nonblocking)
{
  if (nonblocking)
    set_nonblocking (true);
  if (async)
    {
      WSAAsyncSelect (get_socket (), winmsg, WM_ASYNCIO, ASYNC_MASK);
      async_io (true);
    }
}

bool
fhandler_socket::af_local_recv_secret ()
{
  int out[4] = { 0, 0, 0, 0 };
  int rest = sizeof out;
  char *ptr = (char *) out;
  while (rest > 0)
    {
      int ret = recvfrom (ptr, rest, 0, NULL, NULL);
      if (ret <= 0)
	break;
      rest -= ret;
      ptr += ret;
    }
  if (rest == 0)
    {
      debug_printf ("Received af_local secret: %08x-%08x-%08x-%08x",
		    out[0], out[1], out[2], out[3]);
      if (out[0] != connect_secret[0] || out[1] != connect_secret[1]
	  || out[2] != connect_secret[2] || out[3] != connect_secret[3])
	{
	  debug_printf ("Receiving af_local secret mismatch");
	  return false;
	}
    }
  else
    debug_printf ("Receiving af_local secret failed");
  return rest == 0;
}

bool
fhandler_socket::af_local_send_secret ()
{
  int rest = sizeof connect_secret;
  char *ptr = (char *) connect_secret;
  while (rest > 0)
    {
      int ret = sendto (ptr, rest, 0, NULL, 0);
      if (ret <= 0)
	break;
      rest -= ret;
      ptr += ret;
    }
  debug_printf ("Sending af_local secret %s", rest == 0 ? "succeeded"
							: "failed");
  return rest == 0;
}

bool
fhandler_socket::af_local_recv_cred ()
{
  struct ucred out = { (pid_t) 0, (uid_t) -1, (gid_t) -1 };
  int rest = sizeof out;
  char *ptr = (char *) &out;
  while (rest > 0)
    {
      int ret = recvfrom (ptr, rest, 0, NULL, NULL);
      if (ret <= 0)
	break;
      rest -= ret;
      ptr += ret;
    }
  if (rest == 0)
    {
      debug_printf ("Received eid credentials: pid: %d, uid: %d, gid: %d",
		    out.pid, out.uid, out.gid);
      sec_peer_pid = out.pid;
      sec_peer_uid = out.uid;
      sec_peer_gid = out.gid;
    }
  else
    debug_printf ("Receiving eid credentials failed");
  return rest == 0;
}

bool
fhandler_socket::af_local_send_cred ()
{
  struct ucred in = { sec_pid, sec_uid, sec_gid };
  int rest = sizeof in;
  char *ptr = (char *) &in;
  while (rest > 0)
    {
      int ret = sendto (ptr, rest, 0, NULL, 0);
      if (ret <= 0)
	break;
      rest -= ret;
      ptr += ret;
    }
  if (rest == 0)
    debug_printf ("Sending eid credentials succeeded");
  else
    debug_printf ("Sending eid credentials failed");
  return rest == 0;
}

int
fhandler_socket::af_local_connect ()
{
  bool orig_async_io, orig_is_nonblocking;

  if (get_addr_family () != AF_LOCAL || get_socket_type () != SOCK_STREAM)
    return 0;

  debug_printf ("af_local_connect called, no_getpeereid=%d", no_getpeereid ());
  if (no_getpeereid ())
    return 0;

  af_local_setblocking (orig_async_io, orig_is_nonblocking);
  if (!af_local_send_secret () || !af_local_recv_secret ()
      || !af_local_send_cred () || !af_local_recv_cred ())
    {
      debug_printf ("accept from unauthorized server");
      ::shutdown (get_socket (), SD_BOTH);
      WSASetLastError (WSAECONNREFUSED);
      return -1;
    }
  af_local_unsetblocking (orig_async_io, orig_is_nonblocking);
  return 0;
}

int
fhandler_socket::af_local_accept ()
{
  bool orig_async_io, orig_is_nonblocking;

  debug_printf ("af_local_accept called, no_getpeereid=%d", no_getpeereid ());
  if (no_getpeereid ())
    return 0;

  af_local_setblocking (orig_async_io, orig_is_nonblocking);
  if (!af_local_recv_secret () || !af_local_send_secret ()
      || !af_local_recv_cred () || !af_local_send_cred ())
    {
      debug_printf ("connect from unauthorized client");
      ::shutdown (get_socket (), SD_BOTH);
      ::closesocket (get_socket ());
      WSASetLastError (WSAECONNABORTED);
      return -1;
    }
  af_local_unsetblocking (orig_async_io, orig_is_nonblocking);
  return 0;
}

int
fhandler_socket::af_local_set_no_getpeereid ()
{
  if (get_addr_family () != AF_LOCAL || get_socket_type () != SOCK_STREAM)
    {
      set_errno (EINVAL);
      return -1;
    }
  if (connect_state () != unconnected)
    {
      set_errno (EALREADY);
      return -1;
    }

  debug_printf ("no_getpeereid set");
  no_getpeereid (true);
  return 0;
}

void
fhandler_socket::af_local_set_cred ()
{
  sec_pid = getpid ();
  sec_uid = geteuid32 ();
  sec_gid = getegid32 ();
  sec_peer_pid = (pid_t) 0;
  sec_peer_uid = (uid_t) -1;
  sec_peer_gid = (gid_t) -1;
}

void
fhandler_socket::af_local_copy (fhandler_socket *sock)
{
  sock->connect_secret[0] = connect_secret[0];
  sock->connect_secret[1] = connect_secret[1];
  sock->connect_secret[2] = connect_secret[2];
  sock->connect_secret[3] = connect_secret[3];
  sock->sec_pid = sec_pid;
  sock->sec_uid = sec_uid;
  sock->sec_gid = sec_gid;
  sock->sec_peer_pid = sec_peer_pid;
  sock->sec_peer_uid = sec_peer_uid;
  sock->sec_peer_gid = sec_peer_gid;
  sock->no_getpeereid (no_getpeereid ());
}

void
fhandler_socket::af_local_set_secret (char *buf)
{
  if (!RtlGenRandom (connect_secret, sizeof (connect_secret)))
    bzero ((char*) connect_secret, sizeof (connect_secret));
  __small_sprintf (buf, "%08x-%08x-%08x-%08x",
		   connect_secret [0], connect_secret [1],
		   connect_secret [2], connect_secret [3]);
}

/* Maximum number of concurrently opened sockets from all Cygwin processes
   per session.  Note that shared sockets (through dup/fork/exec) are
   counted as one socket. */
#define NUM_SOCKS       (32768 / sizeof (wsa_event))

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
  wsock_events = search_wsa_event_slot (new_serial_number);
  /* sock type not yet set here. */
  if (pc.dev == FH_UDP || pc.dev == FH_DGRAM)
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
	      setsockopt (get_socket (), SOL_SOCKET, SO_ERROR,
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

  WSAEVENT ev[3] = { wsock_evt, NULL, NULL };
  wait_signal_arrived here (ev[1]);
  DWORD ev_cnt = 2;
  if ((ev[2] = pthread::get_cancel_event ()) != NULL)
    ++ev_cnt;

  while (!(ret = evaluate_events (event_mask, events, !(flags & MSG_PEEK)))
	 && !events)
    {
      if (is_nonblocking () || (flags & MSG_DONTWAIT))
	{
	  WSASetLastError (WSAEWOULDBLOCK);
	  return SOCKET_ERROR;
	}

      switch (WSAWaitForMultipleEvents (ev_cnt, ev, FALSE, 50, FALSE))
	{
	  case WSA_WAIT_TIMEOUT:
	  case WSA_WAIT_EVENT_0:
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
  HANDLE evt = wsock_evt;
  HANDLE mtx = wsock_mtx;

  LOCK_EVENTS;
  wsock_evt = wsock_mtx = NULL;
  } ReleaseMutex (mtx);	/* == UNLOCK_EVENTS, but note using local mtx here. */
  NtClose (evt);
  NtClose (mtx);
}

/* Called from net.cc:fdsock() if a freshly created socket is not
   inheritable.  In that case we use fixup_before_fork_exec.  See
   the comment in fdsock() for a description of the problem. */
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
  if (get_addr_family () == AF_LOCAL)
    {
      fhs->set_sun_path (get_sun_path ());
      fhs->set_peer_sun_path (get_peer_sun_path ());
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
  if (get_device () == FH_UNIX)
    {
      res = fhandler_base::fstat_fs (buf);
      if (!res)
	{
	  buf->st_mode = (buf->st_mode & ~S_IFMT) | S_IFSOCK;
	  buf->st_size = 0;
	}
    }
  else
    {
      res = fhandler_base::fstat (buf);
      if (!res)
	{
	  buf->st_dev = FHDEV (DEV_TCP_MAJOR, 0);
	  if (!(buf->st_ino = get_plain_ino ()))
	    sscanf (get_name (), "/proc/%*d/fd/socket:[%lld]",
				 (long long *) &buf->st_ino);
	  buf->st_mode = S_IFSOCK | S_IRWXU | S_IRWXG | S_IRWXO;
	  buf->st_size = 0;
	}
    }
  return res;
}

int __reg2
fhandler_socket::fstatvfs (struct statvfs *sfs)
{
  if (get_device () == FH_UNIX)
    {
      fhandler_disk_file fh (pc);
      fh.get_device () = FH_FS;
      return fh.fstatvfs (sfs);
    }
  set_errno (EBADF);
  return -1;
}

int
fhandler_socket::fchmod (mode_t mode)
{
  if (get_device () == FH_UNIX)
    {
      fhandler_disk_file fh (pc);
      fh.get_device () = FH_FS;
      int ret = fh.fchmod (S_IFSOCK | adjust_socket_file_mode (mode));
      return ret;
    }
  set_errno (EBADF);
  return -1;
}

int
fhandler_socket::fchown (uid_t uid, gid_t gid)
{
  if (get_device () == FH_UNIX)
    {
      fhandler_disk_file fh (pc);
      return fh.fchown (uid, gid);
    }
  set_errno (EBADF);
  return -1;
}

int
fhandler_socket::facl (int cmd, int nentries, aclent_t *aclbufp)
{
  if (get_device () == FH_UNIX)
    {
      fhandler_disk_file fh (pc);
      return fh.facl (cmd, nentries, aclbufp);
    }
  set_errno (EBADF);
  return -1;
}

int
fhandler_socket::link (const char *newpath)
{
  if (get_device () == FH_UNIX)
    {
      fhandler_disk_file fh (pc);
      return fh.link (newpath);
    }
  return fhandler_base::link (newpath);
}

int
fhandler_socket::bind (const struct sockaddr *name, int namelen)
{
  int res = -1;

  if (name->sa_family == AF_LOCAL)
    {
#define un_addr ((struct sockaddr_un *) name)
      struct sockaddr_in sin;
      int len = namelen - offsetof (struct sockaddr_un, sun_path);

      /* Check that name is within bounds.  Don't check if the string is
         NUL-terminated, because there are projects out there which set
	 namelen to a value which doesn't cover the trailing NUL. */
      if (len <= 1 || (len = strnlen (un_addr->sun_path, len)) > UNIX_PATH_MAX)
	{
	  set_errno (len <= 1 ? (len == 1 ? ENOENT : EINVAL) : ENAMETOOLONG);
	  goto out;
	}
      /* Copy over the sun_path string into a buffer big enough to add a
	 trailing NUL. */
      char sun_path[len + 1];
      strncpy (sun_path, un_addr->sun_path, len);
      sun_path[len] = '\0';

      /* This isn't entirely foolproof, but we check first if the file exists
	 so we can return with EADDRINUSE before having bound the socket.
	 This allows an application to call bind again on the same socket using
	 another filename.  If we bind first, the application will not be able
	 to call bind successfully ever again. */
      path_conv pc (sun_path, PC_SYM_FOLLOW);
      if (pc.error)
	{
	  set_errno (pc.error);
	  goto out;
	}
      if (pc.exists ())
	{
	  set_errno (EADDRINUSE);
	  goto out;
	}

      sin.sin_family = AF_INET;
      sin.sin_port = 0;
      sin.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
      if (::bind (get_socket (), (sockaddr *) &sin, len = sizeof sin))
	{
	  syscall_printf ("AF_LOCAL: bind failed");
	  set_winsock_errno ();
	  goto out;
	}
      if (::getsockname (get_socket (), (sockaddr *) &sin, &len))
	{
	  syscall_printf ("AF_LOCAL: getsockname failed");
	  set_winsock_errno ();
	  goto out;
	}

      sin.sin_port = ntohs (sin.sin_port);
      debug_printf ("AF_LOCAL: socket bound to port %u", sin.sin_port);

      mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
      DWORD fattr = FILE_ATTRIBUTE_SYSTEM;
      if (!pc.has_acls ()
	  && !(mode & ~cygheap->umask & (S_IWUSR | S_IWGRP | S_IWOTH)))
	fattr |= FILE_ATTRIBUTE_READONLY;
      SECURITY_ATTRIBUTES sa = sec_none_nih;
      NTSTATUS status;
      HANDLE fh;
      OBJECT_ATTRIBUTES attr;
      IO_STATUS_BLOCK io;
      ULONG access = DELETE | FILE_GENERIC_WRITE;

      /* If the filesystem supports ACLs, we will overwrite the DACL after the
	 call to NtCreateFile.  This requires a handle with READ_CONTROL and
	 WRITE_DAC access, otherwise get_file_sd and set_file_sd both have to
	 open the file again.
	 FIXME: On remote NTFS shares open sometimes fails because even the
	 creator of the file doesn't have the right to change the DACL.
	 I don't know what setting that is or how to recognize such a share,
	 so for now we don't request WRITE_DAC on remote drives. */
      if (pc.has_acls () && !pc.isremote ())
	access |= READ_CONTROL | WRITE_DAC | WRITE_OWNER;

      status = NtCreateFile (&fh, access, pc.get_object_attr (attr, sa), &io,
			     NULL, fattr, 0, FILE_CREATE,
			     FILE_NON_DIRECTORY_FILE
			     | FILE_SYNCHRONOUS_IO_NONALERT
			     | FILE_OPEN_FOR_BACKUP_INTENT,
			     NULL, 0);
      if (!NT_SUCCESS (status))
	{
	  if (io.Information == FILE_EXISTS)
	    set_errno (EADDRINUSE);
	  else
	    __seterrno_from_nt_status (status);
	}
      else
	{
	  if (pc.has_acls ())
	    set_created_file_access (fh, pc, mode);
	  char buf[sizeof (SOCKET_COOKIE) + 80];
	  __small_sprintf (buf, "%s%u %c ", SOCKET_COOKIE, sin.sin_port,
			   get_socket_type () == SOCK_STREAM ? 's'
			   : get_socket_type () == SOCK_DGRAM ? 'd' : '-');
	  af_local_set_secret (strchr (buf, '\0'));
	  DWORD blen = strlen (buf) + 1;
	  status = NtWriteFile (fh, NULL, NULL, NULL, &io, buf, blen, NULL, 0);
	  if (!NT_SUCCESS (status))
	    {
	      __seterrno_from_nt_status (status);
	      FILE_DISPOSITION_INFORMATION fdi = { TRUE };
	      status = NtSetInformationFile (fh, &io, &fdi, sizeof fdi,
					     FileDispositionInformation);
	      if (!NT_SUCCESS (status))
		debug_printf ("Setting delete dispostion failed, status = %y",
			      status);
	    }
	  else
	    {
	      set_sun_path (sun_path);
	      res = 0;
	    }
	  NtClose (fh);
	}
#undef un_addr
    }
  else
    {
      if (!saw_reuseaddr ())
	{
	  /* If the application didn't explicitely request SO_REUSEADDR,
	     enforce POSIX standard socket binding behaviour by setting the
	     SO_EXCLUSIVEADDRUSE socket option.  See cygwin_setsockopt()
	     for a more detailed description. */
	  int on = 1;
	  int ret = ::setsockopt (get_socket (), SOL_SOCKET,
				  ~(SO_REUSEADDR),
				  (const char *) &on, sizeof on);
	  debug_printf ("%d = setsockopt(SO_EXCLUSIVEADDRUSE), %E", ret);
	}
      if (::bind (get_socket (), name, namelen))
	set_winsock_errno ();
      else
	res = 0;
    }

out:
  return res;
}

int
fhandler_socket::connect (const struct sockaddr *name, int namelen)
{
  struct sockaddr_storage sst;
  int type = 0;

  if (get_inet_addr (name, namelen, &sst, &namelen, &type, connect_secret)
      == SOCKET_ERROR)
    return SOCKET_ERROR;

  if (get_addr_family () == AF_LOCAL)
    {
      if (get_socket_type () != type)
	{
	  WSASetLastError (WSAEPROTOTYPE);
	  set_winsock_errno ();
	  return SOCKET_ERROR;
	}

      set_peer_sun_path (name->sa_data);

      /* Don't move af_local_set_cred into af_local_connect which may be called
	 via select, possibly running under another identity.  Call early here,
	 because af_local_connect is called in wait_for_events. */
      if (get_socket_type () == SOCK_STREAM)
	af_local_set_cred ();
    }
  
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
fhandler_socket::listen (int backlog)
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
    {
      if (get_addr_family () == AF_LOCAL && get_socket_type () == SOCK_STREAM)
	af_local_set_cred ();
      connect_state (listener);	/* gets set to connected on accepted socket. */
    }
  else
    set_winsock_errno ();
  return res;
}

int
fhandler_socket::accept4 (struct sockaddr *peer, int *len, int flags)
{
  /* Allows NULL peer and len parameters. */
  struct sockaddr_storage lpeer;
  int llen = sizeof (struct sockaddr_storage);

  int res = (int) INVALID_SOCKET;

  /* Windows event handling does not check for the validity of the desired
     flags so we have to do it here. */
  if (connect_state () != listener)
    {
      WSASetLastError (WSAEINVAL);
      set_winsock_errno ();
      goto out;
    }

  while (!(res = wait_for_events (FD_ACCEPT | FD_CLOSE, 0))
	 && (res = ::accept (get_socket (), (struct sockaddr *) &lpeer, &llen))
	    == SOCKET_ERROR
	 && WSAGetLastError () == WSAEWOULDBLOCK)
    ;
  if (res == (int) INVALID_SOCKET)
    set_winsock_errno ();
  else
    {
      cygheap_fdnew res_fd;
      if (res_fd >= 0 && fdsock (res_fd, &dev (), res))
	{
	  fhandler_socket *sock = (fhandler_socket *) res_fd;
	  sock->set_addr_family (get_addr_family ());
	  sock->set_socket_type (get_socket_type ());
	  sock->async_io (false); /* fdsock switches async mode off. */
	  if (get_addr_family () == AF_LOCAL)
	    {
	      sock->set_sun_path (get_sun_path ());
	      sock->set_peer_sun_path (get_peer_sun_path ());
	      if (get_socket_type () == SOCK_STREAM)
		{
		  /* Don't forget to copy credentials from accepting
		     socket to accepted socket and start transaction
		     on accepted socket! */
		  af_local_copy (sock);
		  res = sock->af_local_accept ();
		  if (res == -1)
		    {
		      res_fd.release ();
		      set_winsock_errno ();
		      goto out;
		    }
		}
	    }
	  sock->set_nonblocking (flags & SOCK_NONBLOCK);
	  if (flags & SOCK_CLOEXEC)
	    sock->set_close_on_exec (true);
	  /* No locking necessary at this point. */
	  sock->wsock_events->events = wsock_events->events | FD_WRITE;
	  sock->wsock_events->owner = wsock_events->owner;
	  sock->connect_state (connected);
	  res = res_fd;
	  if (peer)
	    {
	      if (get_addr_family () == AF_LOCAL)
		{
		  /* FIXME: Right now we have no way to determine the
		     bound socket name of the peer's socket.  For now
		     we just fake an unbound socket on the other side. */
		  static struct sockaddr_un un = { AF_LOCAL, "" };
		  memcpy (peer, &un, MIN (*len, (int) sizeof (un.sun_family)));
		  *len = (int) sizeof (un.sun_family);
		}
	      else
		{
		  memcpy (peer, &lpeer, MIN (*len, llen));
		  *len = llen;
		}
	    }
	}
      else
	{
	  closesocket (res);
	  res = -1;
	}
    }

out:
  debug_printf ("res %d", res);
  return res;
}

int
fhandler_socket::getsockname (struct sockaddr *name, int *namelen)
{
  int res = -1;

  if (get_addr_family () == AF_LOCAL)
    {
      struct sockaddr_un sun;
      sun.sun_family = AF_LOCAL;
      sun.sun_path[0] = '\0';
      if (get_sun_path ())
	strncat (sun.sun_path, get_sun_path (), UNIX_PATH_MAX - 1);
      memcpy (name, &sun, MIN (*namelen, (int) SUN_LEN (&sun) + 1));
      *namelen = (int) SUN_LEN (&sun) + (get_sun_path () ? 1 : 0);
      res = 0;
    }
  else
    {
      /* Always use a local big enough buffer and truncate later as necessary
	 per POSIX.  WinSock unfortunaltey only returns WSAEFAULT if the buffer
	 is too small. */
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
    }

  return res;
}

int
fhandler_socket::getpeername (struct sockaddr *name, int *namelen)
{
  /* Always use a local big enough buffer and truncate later as necessary
     per POSIX.  WinSock unfortunately only returns WSAEFAULT if the buffer
     is too small. */
  struct sockaddr_storage sock;
  int len = sizeof sock;
  int res = ::getpeername (get_socket (), (struct sockaddr *) &sock, &len);
  if (res)
    set_winsock_errno ();
  else if (get_addr_family () == AF_LOCAL)
    {
      struct sockaddr_un sun;
      memset (&sun, 0, sizeof sun);
      sun.sun_family = AF_LOCAL;
      sun.sun_path[0] = '\0';
      if (get_peer_sun_path ())
	strncat (sun.sun_path, get_peer_sun_path (), UNIX_PATH_MAX - 1);
      memcpy (name, &sun, MIN (*namelen, (int) SUN_LEN (&sun) + 1));
      *namelen = (int) SUN_LEN (&sun) + (get_peer_sun_path () ? 1 : 0);
    }
  else
    {
      memcpy (name, &sock, MIN (*namelen, len));
      *namelen = len;
    }

  return res;
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

inline ssize_t
fhandler_socket::recv_internal (LPWSAMSG wsamsg, bool use_recvmsg)
{
  ssize_t res = 0;
  DWORD ret = 0, wret;
  int evt_mask = FD_READ | ((wsamsg->dwFlags & MSG_OOB) ? FD_OOB : 0);
  LPWSABUF &wsabuf = wsamsg->lpBuffers;
  ULONG &wsacnt = wsamsg->dwBufferCount;
  static NO_COPY LPFN_WSARECVMSG WSARecvMsg;
  int orig_namelen = wsamsg->namelen;

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

  if (get_addr_family () == AF_LOCAL && wsamsg->name != NULL
      && orig_namelen >= (int) sizeof (sa_family_t))
    {
      /* WSARecvFrom copied the sockaddr_in block to wsamsg->name.  We have to
	 overwrite it with a sockaddr_un block.  For datagram sockets we
	 generate a sockaddr_un with a filename analogue to abstract socket
	 names under Linux.  See `man 7 unix' under Linux for a description. */
      sockaddr_un *un = (sockaddr_un *) wsamsg->name;
      un->sun_family = AF_LOCAL;
      int len = orig_namelen - offsetof (struct sockaddr_un, sun_path);
      if (len > 0)
	{
	  if (get_socket_type () == SOCK_DGRAM)
	    {
	      if (len >= 7)
		{
		  __small_sprintf (un->sun_path + 1, "d%04x",
			       ((struct sockaddr_in *) wsamsg->name)->sin_port);
		  wsamsg->namelen = offsetof (struct sockaddr_un, sun_path) + 7;
		}
	      else
		wsamsg->namelen = offsetof (struct sockaddr_un, sun_path) + 1;
	      un->sun_path[0] = '\0';
	    }
	  else if (!get_peer_sun_path ())
	    wsamsg->namelen = sizeof (sa_family_t);
	  else
	    {
	      memset (un->sun_path, 0, len);
	      strncpy (un->sun_path, get_peer_sun_path (), len);
	      if (un->sun_path[len - 1] == '\0')
		len = strlen (un->sun_path) + 1;
	      if (len > UNIX_PATH_MAX)
		len = UNIX_PATH_MAX;
	      wsamsg->namelen = offsetof (struct sockaddr_un, sun_path) + len;
	    }
	}
    }

  return ret;
}

void __reg3
fhandler_socket::read (void *in_ptr, size_t& len)
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
fhandler_socket::readv (const struct iovec *const iov, const int iovcnt,
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
fhandler_socket::recvfrom (void *in_ptr, size_t len, int flags,
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
fhandler_socket::recvmsg (struct msghdr *msg, int flags)
{
  /* TODO: Descriptor passing on AF_LOCAL sockets. */

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

inline ssize_t
fhandler_socket::send_internal (struct _WSAMSG *wsamsg, int flags)
{
  ssize_t res = 0;
  DWORD ret = 0, err = 0, sum = 0;
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
	  if (res && (err = WSAGetLastError ()) == WSAEWOULDBLOCK)
	    {
	      LOCK_EVENTS;
	      wsock_events->events &= ~FD_WRITE;
	      UNLOCK_EVENTS;
	    }
	}
      while (res && err == WSAEWOULDBLOCK
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
      else if (is_nonblocking () || err != WSAEWOULDBLOCK)
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
fhandler_socket::write (const void *in_ptr, size_t len)
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
fhandler_socket::writev (const struct iovec *const iov, const int iovcnt,
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

ssize_t
fhandler_socket::sendto (const void *in_ptr, size_t len, int flags,
			 const struct sockaddr *to, int tolen)
{
  char *ptr = (char *) in_ptr;
  struct sockaddr_storage sst;

  if (to && get_inet_addr (to, tolen, &sst, &tolen) == SOCKET_ERROR)
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
fhandler_socket::sendmsg (const struct msghdr *msg, int flags)
{
  /* TODO: Descriptor passing on AF_LOCAL sockets. */

  struct sockaddr_storage sst;
  int len = 0;

  if (msg->msg_name
      && get_inet_addr ((struct sockaddr *) msg->msg_name, msg->msg_namelen,
			&sst, &len) == SOCKET_ERROR)
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
  DWORD controllen = (DWORD) (get_socket_type () == SOCK_STREAM
			      || get_addr_family () == AF_LOCAL
			      ? 0 : msg->msg_controllen);
  WSAMSG wsamsg = { msg->msg_name ? (struct sockaddr *) &sst : NULL, len,
		    wsabuf, (DWORD) msg->msg_iovlen,
		    { controllen, (char *) msg->msg_control },
		    0 };
  return send_internal (&wsamsg, flags);
}

int
fhandler_socket::shutdown (int how)
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
fhandler_socket::close ()
{
  int res = 0;

  release_events ();
  while ((res = closesocket (get_socket ())) != 0)
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

  debug_printf ("%d = fhandler_socket::close()", res);
  return res;
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
    /* From this point on we handle only ioctl commands which are understood by
       Winsock.  However, we have a problem, which is, the different size of
       u_long in Windows and 64 bit Cygwin.  This affects the definitions of
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
      res = ioctlsocket (get_socket (), _IOR('f', 127, u_long), (u_long *) p);
      if (res == SOCKET_ERROR)
	set_winsock_errno ();
      break;
    default:
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
	res = ioctlsocket (get_socket (), cmd, (u_long *) p);
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

void
fhandler_socket::set_sun_path (const char *path)
{
  sun_path = path ? cstrdup (path) : NULL;
}

void
fhandler_socket::set_peer_sun_path (const char *path)
{
  peer_sun_path = path ? cstrdup (path) : NULL;
}

int
fhandler_socket::getpeereid (pid_t *pid, uid_t *euid, gid_t *egid)
{
  if (get_addr_family () != AF_LOCAL || get_socket_type () != SOCK_STREAM)
    {
      set_errno (EINVAL);
      return -1;
    }
  if (no_getpeereid ())
    {
      set_errno (ENOTSUP);
      return -1;
    }
  if (connect_state () != connected)
    {
      set_errno (ENOTCONN);
      return -1;
    }

  __try
    {
      if (pid)
	*pid = sec_peer_pid;
      if (euid)
	*euid = sec_peer_uid;
      if (egid)
	*egid = sec_peer_gid;
      return 0;
    }
  __except (EFAULT) {}
  __endtry
  return -1;
}

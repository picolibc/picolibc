/* fhandler_socket.cc. See fhandler.h for a description of the fhandler classes.

   Copyright 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008,
   2009, 2010 Red Hat, Inc.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

/* #define DEBUG_NEST_ON 1 */

#define  __INSIDE_CYGWIN_NET__

#include "winsup.h"
#include <sys/un.h>
#include <asm/byteorder.h>

#include <stdlib.h>
#define USE_SYS_TYPES_FD_SET
#include <winsock2.h>
#include <mswsock.h>
#include <iphlpapi.h>
#include "cygerrno.h"
#include "security.h"
#include "cygwin/version.h"
#include "perprocess.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "shared_info.h"
#include "sigproc.h"
#include "wininfo.h"
#include <unistd.h>
#include <sys/acl.h>
#include "cygtls.h"
#include "cygwin/in6.h"
#include "ntdll.h"

#define ASYNC_MASK (FD_READ|FD_WRITE|FD_OOB|FD_ACCEPT|FD_CONNECT)
#define EVENT_MASK (FD_READ|FD_WRITE|FD_OOB|FD_ACCEPT|FD_CONNECT|FD_CLOSE)

extern bool fdsock (cygheap_fdmanip& fd, const device *, SOCKET soc);
extern "C" {
int sscanf (const char *, const char *, ...);
} /* End of "C" section */

fhandler_dev_random* entropy_source;

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
static int
get_inet_addr (const struct sockaddr *in, int inlen,
	       struct sockaddr_storage *out, int *outlen,
	       int *type = NULL, int *secret = NULL)
{
  int secret_buf [4];
  int* secret_ptr = (secret ? : secret_buf);

  if (in->sa_family == AF_INET || in->sa_family == AF_INET6)
    {
      memcpy (out, in, inlen);
      *outlen = inlen;
      return 1;
    }
  else if (in->sa_family == AF_LOCAL)
    {
      NTSTATUS status;
      HANDLE fh;
      OBJECT_ATTRIBUTES attr;
      IO_STATUS_BLOCK io;

      path_conv pc (in->sa_data, PC_SYM_FOLLOW);
      if (pc.error)
	{
	  set_errno (pc.error);
	  return 0;
	}
      if (!pc.exists ())
	{
	  set_errno (ENOENT);
	  return 0;
	}
      if (!pc.issocket ())
	{
	  set_errno (EBADF);
	  return 0;
	}
      status = NtOpenFile (&fh, GENERIC_READ | SYNCHRONIZE,
			   pc.get_object_attr (attr, sec_none_nih), &io,
			   FILE_SHARE_VALID_FLAGS,
			   FILE_SYNCHRONOUS_IO_NONALERT
			   | FILE_OPEN_FOR_BACKUP_INTENT);
      if (!NT_SUCCESS (status))
	{
	  __seterrno_from_nt_status (status);
	  return 0;
	}
      int ret = 0;
      char buf[128];
      memset (buf, 0, sizeof buf);
      status = NtReadFile (fh, NULL, NULL, NULL, &io, buf, 128, NULL, NULL);
      NtClose (fh);
      if (NT_SUCCESS (status))
	{
	  struct sockaddr_in sin;
	  char ctype;
	  sin.sin_family = AF_INET;
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
	  ret = 1;
	}
      else
	__seterrno_from_nt_status (status);
      return ret;
    }
  else
    {
      set_errno (EAFNOSUPPORT);
      return 0;
    }
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
  __small_sprintf (buf, "socket:[%d]", get_socket ());
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
  struct ucred out = { (pid_t) 0, (__uid32_t) -1, (__gid32_t) -1 };
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
  /* This keeps the test out of select. */
  if (get_addr_family () != AF_LOCAL || get_socket_type () != SOCK_STREAM)
    return 0;

  debug_printf ("af_local_connect called");
  bool orig_async_io, orig_is_nonblocking;
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
  debug_printf ("af_local_accept called");
  bool orig_async_io, orig_is_nonblocking;
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

void
fhandler_socket::af_local_set_cred ()
{
  sec_pid = getpid ();
  sec_uid = geteuid32 ();
  sec_gid = getegid32 ();
  sec_peer_pid = (pid_t) 0;
  sec_peer_uid = (__uid32_t) -1;
  sec_peer_gid = (__gid32_t) -1;
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
}

void
fhandler_socket::af_local_set_secret (char *buf)
{
  if (!entropy_source)
    {
      void *buf = malloc (sizeof (fhandler_dev_random));
      entropy_source = new (buf) fhandler_dev_random ();
      entropy_source->dev () = *urandom_dev;
    }
  if (entropy_source &&
      !entropy_source->open (O_RDONLY))
    {
      delete entropy_source;
      entropy_source = NULL;
    }
  if (entropy_source)
    {
      size_t len = sizeof (connect_secret);
      entropy_source->read (connect_secret, len);
      if (len != sizeof (connect_secret))
	bzero ((char*) connect_secret, sizeof (connect_secret));
    }
  __small_sprintf (buf, "%08x-%08x-%08x-%08x",
		   connect_secret [0], connect_secret [1],
		   connect_secret [2], connect_secret [3]);
}

/* Maximum number of concurrently opened sockets from all Cygwin processes
   per session.  Note that shared sockets (through dup/fork/exec) are
   counted as one socket. */
#define NUM_SOCKS       (32768 / sizeof (wsa_event))

#define LOCK_EVENTS	WaitForSingleObject (wsock_mtx, INFINITE)
#define UNLOCK_EVENTS	ReleaseMutex (wsock_mtx)

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
	api_fatal ("Couldn't create/open shared socket mutex %S, %p",
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
      RtlInitUnicodeString (&uname, sock_shared_name (name, new_serial_number));
      InitializeObjectAttributes (&attr, &uname, OBJ_INHERIT | OBJ_OPENIF,
				  get_session_parent_dir (),
				  everyone_sd (CYG_MUTANT_ACCESS));
      status = NtCreateMutant (&wsock_mtx, CYG_MUTANT_ACCESS, &attr, FALSE);
      if (!NT_SUCCESS (status))
	{
	  debug_printf ("NtCreateMutant(%S), %p", &uname, status);
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
				  bool erase)
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
	    wsock_events->connect_errorcode = evts.iErrorCode[FD_CONNECT_BIT];
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
	  int wsa_err = 0;
	  if ((wsa_err = wsock_events->connect_errorcode) != 0)
	    {
	      WSASetLastError (wsa_err);
	      ret = SOCKET_ERROR;
	    }
	  else
	    wsock_events->events |= FD_WRITE;
	  wsock_events->events &= ~FD_CONNECT;
	  wsock_events->connect_errorcode = 0;
	}
      if (erase)
	wsock_events->events &= ~(events & ~(FD_WRITE | FD_CLOSE));
    }
  UNLOCK_EVENTS;

  return ret;
}

int
fhandler_socket::wait_for_events (const long event_mask, bool dontwait)
{
  if (async_io ())
    return 0;

  int ret;
  long events;

  while (!(ret = evaluate_events (event_mask, events, true)) && !events)
    {
      if (is_nonblocking () || dontwait)
	{
	  WSASetLastError (WSAEWOULDBLOCK);
	  return SOCKET_ERROR;
	}

      WSAEVENT ev[2] = { wsock_evt, signal_arrived };
      switch (WSAWaitForMultipleEvents (2, ev, FALSE, 50, FALSE))
	{
	  case WSA_WAIT_TIMEOUT:
	  case WSA_WAIT_EVENT_0:
	    break;

	  case WSA_WAIT_EVENT_0 + 1:
	    if (_my_tls.call_signal_handler ())
	      {
		sig_dispatch_pending ();
		break;
	      }
	    WSASetLastError (WSAEINTR);
	    return SOCKET_ERROR;

	  default:
	    WSASetLastError (WSAEFAULT);
	    return SOCKET_ERROR;
	}
    }

  return ret;
}

void
fhandler_socket::release_events ()
{
  NtClose (wsock_evt);
  NtClose (wsock_mtx);
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
    debug_printf ("WSADuplicateSocket succeeded (%lx)", prot_info_ptr->dwProviderReserved);
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
      debug_printf ("WSASocket succeeded (%lx)", new_sock);
    }
}

void
fhandler_socket::fixup_after_exec ()
{
  if (need_fixup_before () && !close_on_exec ())
    fixup_after_fork (NULL);
}

int
fhandler_socket::dup (fhandler_base *child)
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
  fhs->wsock_events = wsock_events;

  fhs->rmem (rmem ());
  fhs->wmem (wmem ());
  fhs->addr_family = addr_family;
  fhs->set_socket_type (get_socket_type ());
  if (get_addr_family () == AF_LOCAL)
    {
      fhs->set_sun_path (get_sun_path ());
      fhs->set_peer_sun_path (get_peer_sun_path ());
      if (get_socket_type () == SOCK_STREAM)
	{
	  fhs->sec_pid = sec_pid;
	  fhs->sec_uid = sec_uid;
	  fhs->sec_gid = sec_gid;
	  fhs->sec_peer_pid = sec_peer_pid;
	  fhs->sec_peer_uid = sec_peer_uid;
	  fhs->sec_peer_gid = sec_peer_gid;
	}
    }
  fhs->connect_state (connect_state ());

  if (!need_fixup_before ())
    {
      int ret = fhandler_base::dup (child);
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
  if (!fhs->fixup_before_fork_exec (GetCurrentProcessId ()))
    {
      cygheap->user.reimpersonate ();
      fhs->fixup_after_fork (GetCurrentProcess ());
      if (fhs->get_io_handle() != (HANDLE) INVALID_SOCKET)
	return 0;
    }
  cygheap->user.reimpersonate ();
  cygheap->fdtab.dec_need_fixup_before ();
  NtClose (fhs->wsock_evt);
  NtClose (fhs->wsock_mtx);
  return -1;
}

int __stdcall
fhandler_socket::fstat (struct __stat64 *buf)
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
	  buf->st_dev = 0;
	  buf->st_ino = (__ino64_t) ((DWORD) get_handle ());
	  buf->st_mode = S_IFSOCK | S_IRWXU | S_IRWXG | S_IRWXO;
	  buf->st_size = 0;
	}
    }
  return res;
}

int __stdcall
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
fhandler_socket::fchown (__uid32_t uid, __gid32_t gid)
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
fhandler_socket::facl (int cmd, int nentries, __aclent32_t *aclbufp)
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

static inline bool
address_in_use (const struct sockaddr *addr)
{
  switch (addr->sa_family)
    {
    case AF_INET:
      {
	PMIB_TCPTABLE tab;
	PMIB_TCPROW entry;
	DWORD size = 0, i;
	struct sockaddr_in *in = (struct sockaddr_in *) addr;

	if (GetTcpTable (NULL, &size, FALSE) == ERROR_INSUFFICIENT_BUFFER)
	  {
	    tab = (PMIB_TCPTABLE) alloca (size += 16 * sizeof (PMIB_TCPROW));
	    if (!GetTcpTable (tab, &size, FALSE))
	      for (i = tab->dwNumEntries, entry = tab->table; i > 0;
		   --i, ++entry)
		if (entry->dwLocalAddr == in->sin_addr.s_addr
		    && entry->dwLocalPort == in->sin_port
		    && entry->dwState >= MIB_TCP_STATE_LISTEN
		    && entry->dwState <= MIB_TCP_STATE_LAST_ACK)
		  return true;
	  }
      }
      break;
    case AF_INET6:
      {
	/* This test works on XP SP2 and above which should cover almost
	   all IPv6 users... */
	PMIB_TCP6TABLE_OWNER_PID tab;
	PMIB_TCP6ROW_OWNER_PID entry;
	DWORD size = 0, i;
	struct sockaddr_in6 *in6 = (struct sockaddr_in6 *) addr;

	if (GetExtendedTcpTable (NULL, &size, FALSE, AF_INET6,
				 TCP_TABLE_OWNER_PID_ALL, 0)
	    == ERROR_INSUFFICIENT_BUFFER)
	  {
	    tab = (PMIB_TCP6TABLE_OWNER_PID)
		  alloca (size += 16 * sizeof (PMIB_TCP6ROW_OWNER_PID));
	    if (!GetExtendedTcpTable (tab, &size, FALSE, AF_INET6,
				      TCP_TABLE_OWNER_PID_ALL, 0))
	      for (i = tab->dwNumEntries, entry = tab->table; i > 0;
		   --i, ++entry)
		if (IN6_ARE_ADDR_EQUAL (entry->ucLocalAddr,
					in6->sin6_addr.s6_addr)
		    /* FIXME: Is testing for the scope required. too?!? */
		    && entry->dwLocalPort == in6->sin6_port
		    && entry->dwState >= MIB_TCP_STATE_LISTEN
		    && entry->dwState <= MIB_TCP_STATE_LAST_ACK)
		  return true;
	  }
      }
      break;
    default:
      break;
    }
  return false;
}

int
fhandler_socket::bind (const struct sockaddr *name, int namelen)
{
  int res = -1;

  if (name->sa_family == AF_LOCAL)
    {
#define un_addr ((struct sockaddr_un *) name)
      struct sockaddr_in sin;
      int len = sizeof sin;

      if (strlen (un_addr->sun_path) >= UNIX_PATH_LEN)
	{
	  set_errno (ENAMETOOLONG);
	  goto out;
	}
      sin.sin_family = AF_INET;
      sin.sin_port = 0;
      sin.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
      if (::bind (get_socket (), (sockaddr *) &sin, len))
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

      path_conv pc (un_addr->sun_path, PC_SYM_FOLLOW);
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
      mode_t mode = adjust_socket_file_mode ((S_IRWXU | S_IRWXG | S_IRWXO)
					     & ~cygheap->umask);
      DWORD fattr = FILE_ATTRIBUTE_SYSTEM;
      if (!(mode & (S_IWUSR | S_IWGRP | S_IWOTH)))
	fattr |= FILE_ATTRIBUTE_READONLY;
      SECURITY_ATTRIBUTES sa = sec_none_nih;
      NTSTATUS status;
      HANDLE fh;
      OBJECT_ATTRIBUTES attr;
      IO_STATUS_BLOCK io;

      status = NtCreateFile (&fh, DELETE | FILE_GENERIC_WRITE,
			     pc.get_object_attr (attr, sa), &io, NULL, fattr,
			     FILE_SHARE_VALID_FLAGS, FILE_CREATE,
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
	    set_file_attribute (fh, pc, ILLEGAL_UID, ILLEGAL_GID,
				S_JUSTCREATED | mode);
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
		debug_printf ("Setting delete dispostion failed, status = %p",
			      status);
	    }
	  else
	    {
	      set_sun_path (un_addr->sun_path);
	      res = 0;
	    }
	  NtClose (fh);
	}
#undef un_addr
    }
  else
    {
      /* If the application didn't explicitely call setsockopt (SO_REUSEADDR),
	 enforce exclusive local address use using the SO_EXCLUSIVEADDRUSE
	 socket option, to emulate POSIX socket behaviour more closely.

	 KB 870562: Note that this option is only available since NT4 SP4.
	 Also note that a bug in Win2K SP1-3 and XP up to SP1 only enables
	 this option for users in the local administrators group. */
      if (wincap.has_exclusiveaddruse ())
	{
	  if (!saw_reuseaddr ())
	    {
	      int on = 1;
	      int ret = ::setsockopt (get_socket (), SOL_SOCKET,
				      ~(SO_REUSEADDR),
				      (const char *) &on, sizeof on);
	      debug_printf ("%d = setsockopt (SO_EXCLUSIVEADDRUSE), %E", ret);
	    }
	  else if (!wincap.has_enhanced_socket_security ())
	    {
	      debug_printf ("SO_REUSEADDR set");
	      /* There's a bug in SO_REUSEADDR handling in WinSock.
		 Per standards, we must not be able to reuse a complete
		 duplicate of a local TCP address (same IP, same port),
		 even if SO_REUSEADDR has been set.  That's unfortunately
		 possible in WinSock.

		 So we're testing here if the local address is already in
		 use and don't bind, if so.  This only works for OSes with
		 IP Helper support and is, of course, still prone to races.

		 However, we don't have to do this on systems supporting
		 "enhanced socket security" (2K3 and later).  On these
		 systems the default binding behaviour is exactly as you'd
		 expect for SO_REUSEADDR, while setting SO_REUSEADDR re-enables
		 the wrong behaviour.  So all we have to do on these newer
		 systems is never to set SO_REUSEADDR but only to note that
		 it has been set for the above SO_EXCLUSIVEADDRUSE setting.
		 See setsockopt() in net.cc. */
	      if (get_socket_type () == SOCK_STREAM
		  && wincap.has_ip_helper_lib ()
		  && address_in_use (name))
		{
		  debug_printf ("Local address in use, don't bind");
		  set_errno (EADDRINUSE);
		  goto out;
		}
	    }
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
  int res = -1;
  bool in_progress = false;
  struct sockaddr_storage sst;
  DWORD err;
  int type;

  if (!get_inet_addr (name, namelen, &sst, &namelen, &type, connect_secret))
    return -1;

  if (get_addr_family () == AF_LOCAL && get_socket_type () != type)
    {
      WSASetLastError (WSAEPROTOTYPE);
      set_winsock_errno ();
      return -1;
    }

  res = ::connect (get_socket (), (struct sockaddr *) &sst, namelen);
  if (!is_nonblocking ()
      && res == SOCKET_ERROR
      && WSAGetLastError () == WSAEWOULDBLOCK)
    res = wait_for_events (FD_CONNECT | FD_CLOSE, false);

  if (!res)
    err = 0;
  else
    {
      err = WSAGetLastError ();
      /* Special handling for connect to return the correct error code
	 when called on a non-blocking socket. */
      if (is_nonblocking ())
	{
	  if (err == WSAEWOULDBLOCK || err == WSAEALREADY)
	    in_progress = true;

	  if (err == WSAEWOULDBLOCK)
	    WSASetLastError (err = WSAEINPROGRESS);
	}
      if (err == WSAEINVAL)
	WSASetLastError (err = WSAEISCONN);
      set_winsock_errno ();
    }

  if (get_addr_family () == AF_LOCAL && (!res || in_progress))
    set_peer_sun_path (name->sa_data);

  if (get_addr_family () == AF_LOCAL && get_socket_type () == SOCK_STREAM)
    {
      af_local_set_cred (); /* Don't move into af_local_connect since
			       af_local_connect is called from select,
			       possibly running under another identity. */
      if (!res && af_local_connect ())
	{
	  set_winsock_errno ();
	  return -1;
	}
    }

  if (err == WSAEINPROGRESS || err == WSAEALREADY)
    connect_state (connect_pending);
  else if (err)
    connect_state (connect_failed);
  else
    connect_state (connected);

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
	 port number, listening on all interfaces.  On Winsock, listen
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
	  struct sockaddr_in6 sin6 =
	    {
	      sin6_family: AF_INET6,
	      sin6_port: 0,
	      sin6_flowinfo: 0,
	      sin6_addr: {{IN6ADDR_ANY_INIT}},
	      sin6_scope_id: 0
	    };
	  if (!::bind (get_socket (), (struct sockaddr *) &sin6, sizeof sin6))
	    res = ::listen (get_socket (), backlog);
	}
    }
  if (!res)
    {
      if (get_addr_family () == AF_LOCAL && get_socket_type () == SOCK_STREAM)
	af_local_set_cred ();
      connect_state (connected);
      listener (true);
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

  int res = 0;
  while (!(res = wait_for_events (FD_ACCEPT | FD_CLOSE, false))
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
	  sock->async_io (async_io ());
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
	  sock->set_nonblocking (flags & SOCK_NONBLOCK
				 ? true : is_nonblocking ());
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
		  memcpy (peer, &un, min (*len, (int) sizeof (un.sun_family)));
		  *len = (int) sizeof (un.sun_family);
		}
	      else
		{
		  memcpy (peer, &lpeer, min (*len, llen));
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
      	strncat (sun.sun_path, get_sun_path (), UNIX_PATH_LEN - 1);
      memcpy (name, &sun, min (*namelen, (int) SUN_LEN (&sun) + 1));
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
	  memcpy (name, &sock, min (*namelen, len));
	  *namelen = len;
	}
      else
	{
	  if (WSAGetLastError () == WSAEINVAL)
	    {
	      /* Winsock returns WSAEINVAL if the socket is locally
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
		  memcpy (name, &sock, min (*namelen, len));
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
      	strncat (sun.sun_path, get_peer_sun_path (), UNIX_PATH_LEN - 1);
      memcpy (name, &sun, min (*namelen, (int) SUN_LEN (&sun) + 1));
      *namelen = (int) SUN_LEN (&sun) + (get_peer_sun_path () ? 1 : 0);
    }
  else
    {
      memcpy (name, &sock, min (*namelen, len));
      *namelen = len;
    }

  return res;
}

int
fhandler_socket::readv (const struct iovec *const iov, const int iovcnt,
			ssize_t tot)
{
  struct msghdr msg =
    {
      msg_name:		NULL,
      msg_namelen:	0,
      msg_iov:		(struct iovec *) iov, // const_cast
      msg_iovlen:	iovcnt,
      msg_control:	NULL,
      msg_controllen:	0,
      msg_flags:	0
    };

  return recvmsg (&msg, 0);
}

extern "C" {
#define WSAID_WSARECVMSG \
	  {0xf689d7c8,0x6f1f,0x436b,{0x8a,0x53,0xe5,0x4f,0xe3,0x51,0xc3,0x22}};
typedef int (WSAAPI *LPFN_WSARECVMSG)(SOCKET,LPWSAMSG,LPDWORD,LPWSAOVERLAPPED,
				      LPWSAOVERLAPPED_COMPLETION_ROUTINE);
int WSAAPI WSASendMsg(SOCKET,LPWSAMSG,DWORD,LPDWORD, LPWSAOVERLAPPED,
		      LPWSAOVERLAPPED_COMPLETION_ROUTINE);
};

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
fhandler_socket::recv_internal (LPWSAMSG wsamsg)
{
  ssize_t res = 0;
  DWORD ret = 0, wret;
  int evt_mask = FD_READ | ((wsamsg->dwFlags & MSG_OOB) ? FD_OOB : 0);
  LPWSABUF &wsabuf = wsamsg->lpBuffers;
  ULONG &wsacnt = wsamsg->dwBufferCount;
  bool use_recvmsg = false;
  static NO_COPY LPFN_WSARECVMSG WSARecvMsg;

  bool waitall = !!(wsamsg->dwFlags & MSG_WAITALL);
  bool dontwait = !!(wsamsg->dwFlags & MSG_DONTWAIT);
  wsamsg->dwFlags &= (MSG_OOB | MSG_PEEK | MSG_DONTROUTE);
  if (wsamsg->Control.len > 0)
    {
      if (!WSARecvMsg
	  && get_ext_funcptr (get_socket (), &WSARecvMsg) == SOCKET_ERROR)
	{
	  set_winsock_errno ();
	  return SOCKET_ERROR;
	}
      use_recvmsg = true;
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
  while (!(res = wait_for_events (evt_mask | FD_CLOSE, dontwait))
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
	 
	 This also covers another weird case: Winsock returns WSAEFAULT if
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
	return ret + wret;

      if (!ret)
	{
	  /* ESHUTDOWN isn't defined for recv in SUSv3.  Simply EOF is returned
	     in this case. */
	  if (WSAGetLastError () == WSAESHUTDOWN)
	    return 0;

	  set_winsock_errno ();
	  return SOCKET_ERROR;
	}
    }

  return ret;
}

ssize_t
fhandler_socket::recvfrom (void *ptr, size_t len, int flags,
			   struct sockaddr *from, int *fromlen)
{
  WSABUF wsabuf = { len, (char *) ptr };
  WSAMSG wsamsg = { from, from && fromlen ? *fromlen : 0,
		    &wsabuf, 1,
		    { 0, NULL},
		    flags };
  ssize_t ret = recv_internal (&wsamsg);
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
  if (!wincap.has_recvmsg () || get_socket_type () == SOCK_STREAM
      || get_addr_family () == AF_LOCAL)
    {
      msg->msg_controllen = 0;
      if (!CYGWIN_VERSION_CHECK_FOR_USING_ANCIENT_MSGHDR)
	msg->msg_flags = 0;
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
		    wsabuf, msg->msg_iovlen,
		    { msg->msg_controllen, (char *) msg->msg_control },
		    flags };
  ssize_t ret = recv_internal (&wsamsg);
  if (ret >= 0)
    {
      msg->msg_namelen = wsamsg.namelen;
      msg->msg_controllen = wsamsg.Control.len;
      if (!CYGWIN_VERSION_CHECK_FOR_USING_ANCIENT_MSGHDR)
	msg->msg_flags = wsamsg.dwFlags;
    }
  return ret;
}

int
fhandler_socket::writev (const struct iovec *const iov, const int iovcnt,
			 ssize_t tot)
{
  struct msghdr msg =
    {
      msg_name:		NULL,
      msg_namelen:	0,
      msg_iov:		(struct iovec *) iov, // const_cast
      msg_iovlen:	iovcnt,
      msg_control:	NULL,
      msg_controllen:	0,
      msg_flags:	0
    };

  return sendmsg (&msg, 0);
}

inline ssize_t
fhandler_socket::send_internal (struct _WSAMSG *wsamsg, int flags)
{
  int res = 0;
  DWORD ret = 0, err = 0, sum = 0, off = 0;
  WSABUF buf;
  bool use_sendmsg = false;
  bool dontwait = !!(flags & MSG_DONTWAIT);
  bool nosignal = !(flags & MSG_NOSIGNAL);

  flags &= (MSG_OOB | MSG_DONTROUTE);
  if (wsamsg->Control.len > 0)
    use_sendmsg = true;
  for (DWORD i = 0; i < wsamsg->dwBufferCount;
       off >= wsamsg->lpBuffers[i].len && (++i, off = 0))
    {
      /* CV 2009-12-02: Don't split datagram messages. */
      /* FIXME: Look for a way to split a message into the least number of
		pieces to minimize the number of WsaSendTo calls. */
      if (get_socket_type () == SOCK_STREAM)
      	{
	  buf.buf = wsamsg->lpBuffers[i].buf + off;
	  buf.len = wsamsg->lpBuffers[i].len - off;
	  /* See net.cc:fdsock() and MSDN KB 823764 */
	  if (buf.len >= (unsigned) wmem ())
	    buf.len = (unsigned) wmem ();
	}

      do
	{
	  if (use_sendmsg)
	    res = WSASendMsg (get_socket (), wsamsg, flags, &ret, NULL, NULL);
	  else if (get_socket_type () == SOCK_STREAM)
	    res = WSASendTo (get_socket (), &buf, 1, &ret, flags,
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
	     && !(res = wait_for_events (FD_WRITE | FD_CLOSE, dontwait)));

      if (!res)
	{
	  off += ret;
	  sum += ret;
	  if (get_socket_type () != SOCK_STREAM)
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
      if (get_errno () == ESHUTDOWN && get_socket_type () == SOCK_STREAM)
	{
	  set_errno (EPIPE);
	  if (!nosignal)
	    raise (SIGPIPE);
	}
    }

  return res;
}

ssize_t
fhandler_socket::sendto (const void *ptr, size_t len, int flags,
			 const struct sockaddr *to, int tolen)
{
  struct sockaddr_storage sst;

  if (to && !get_inet_addr (to, tolen, &sst, &tolen))
    return SOCKET_ERROR;

  WSABUF wsabuf = { len, (char *) ptr };
  WSAMSG wsamsg = { to ? (struct sockaddr *) &sst : NULL, tolen,
		    &wsabuf, 1,
		    { 0, NULL},
		    0 };
  return send_internal (&wsamsg, flags);
}

int
fhandler_socket::sendmsg (const struct msghdr *msg, int flags)
{
  /* TODO: Descriptor passing on AF_LOCAL sockets. */

  WSABUF wsabuf[msg->msg_iovlen];
  WSABUF *wsaptr = wsabuf;
  const struct iovec *iovptr = msg->msg_iov;
  for (int i = 0; i < msg->msg_iovlen; ++i)
    {
      wsaptr->len = iovptr->iov_len;
      (wsaptr++)->buf = (char *) (iovptr++)->iov_base;
    }
  WSAMSG wsamsg = { (struct sockaddr *) msg->msg_name, msg->msg_namelen,
		    wsabuf, msg->msg_iovlen,
		    /* Disappointing but true:  Even if WSASendMsg is
		       supported, it's only supported for datagram and
		       raw sockets. */
		    { !wincap.has_sendmsg ()
		      || get_socket_type () == SOCK_STREAM
		      || get_addr_family () == AF_LOCAL
		      ? 0 : msg->msg_controllen, (char *) msg->msg_control },
		    0 };
  return send_internal (&wsamsg, flags);
}

int
fhandler_socket::shutdown (int how)
{
  int res = ::shutdown (get_socket (), how);

  if (res)
    set_winsock_errno ();
  else
    switch (how)
      {
      case SHUT_RD:
	saw_shutdown_read (true);
	break;
      case SHUT_WR:
	saw_shutdown_write (true);
	break;
      case SHUT_RDWR:
	saw_shutdown_read (true);
	saw_shutdown_write (true);
	break;
      }
  return res;
}

int
fhandler_socket::close ()
{
  int res = 0;
  /* TODO: CV - 2008-04-16.  Lingering disabled.  The original problem
     could be no longer reproduced on NT4, XP, 2K8.  Any return of a
     spurious "Connection reset by peer" *could* be caused by disabling
     the linger code here... */
#if 0
  /* HACK to allow a graceful shutdown even if shutdown() hasn't been
     called by the application. Note that this isn't the ultimate
     solution but it helps in many cases. */
  struct linger linger;
  linger.l_onoff = 1;
  linger.l_linger = 240; /* secs. default 2MSL value according to MSDN. */
  setsockopt (get_socket (), SOL_SOCKET, SO_LINGER,
	      (const char *)&linger, sizeof linger);
#endif
  release_events ();
  while ((res = closesocket (get_socket ())) != 0)
    {
      if (WSAGetLastError () != WSAEWOULDBLOCK)
	{
	  set_winsock_errno ();
	  res = -1;
	  break;
	}
      if (WaitForSingleObject (signal_arrived, 10) == WAIT_OBJECT_0)
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
#define __OLD_IFNAMSIZ        16
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
    case FIOASYNC:
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
      res = ioctlsocket (get_socket (), FIONREAD, (unsigned long *) p);
      if (res == SOCKET_ERROR)
	set_winsock_errno ();
      break;
    default:
      /* Sockets are always non-blocking internally.  So we just note the
	 state here. */
      if (cmd == FIONBIO)
	{
	  syscall_printf ("socket is now %sblocking",
			    *(int *) p ? "non" : "");
	  set_nonblocking (*(int *) p);
	  res = 0;
	}
      else
	res = ioctlsocket (get_socket (), cmd, (unsigned long *) p);
      break;
    }
  syscall_printf ("%d = ioctl_socket (%x, %x)", res, cmd, p);
  return res;
}

int
fhandler_socket::fcntl (int cmd, void *arg)
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
	int new_flags = (int) arg & O_NONBLOCK_MASK;
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
fhandler_socket::getpeereid (pid_t *pid, __uid32_t *euid, __gid32_t *egid)
{
  if (get_addr_family () != AF_LOCAL || get_socket_type () != SOCK_STREAM)
    {
      set_errno (EINVAL);
      return -1;
    }
  if (connect_state () != connected)
    {
      set_errno (ENOTCONN);
      return -1;
    }
  if (sec_peer_pid == (pid_t) 0)
    {
      set_errno (ENOTCONN);	/* Usually when calling getpeereid on
				   accepting (instead of accepted) socket. */
      return -1;
    }

  myfault efault;
  if (efault.faulted (EFAULT))
    return -1;
  if (pid)
    *pid = sec_peer_pid;
  if (euid)
    *euid = sec_peer_uid;
  if (egid)
    *egid = sec_peer_gid;
  return 0;
}

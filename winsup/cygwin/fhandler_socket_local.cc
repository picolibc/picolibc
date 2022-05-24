/* fhandler_socket_local.cc.

   See fhandler.h for a description of the fhandler classes.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#define  __INSIDE_CYGWIN_NET__
#define USE_SYS_TYPES_FD_SET

#include "winsup.h"
/* 2014-04-24: Current Mingw headers define sockaddr_in6 using u_long (8 byte)
   because a redefinition for LP64 systems is missing.  This leads to a wrong
   definition and size of sockaddr_in6 when building with winsock headers.
   This definition is also required to use the right u_long type in subsequent
   function calls. */
#undef u_long
#define u_long __ms_u_long
#include "ntsecapi.h"
#include <w32api/ws2tcpip.h>
#include <w32api/mswsock.h>
#include <unistd.h>
#include <asm/byteorder.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/param.h>
#include <sys/statvfs.h>
#include <cygwin/acl.h>
#include "cygerrno.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "wininfo.h"
#include "ntdll.h"

extern "C" {
  int sscanf (const char *, const char *, ...);
} /* End of "C" section */

#define ASYNC_MASK (FD_READ|FD_WRITE|FD_OOB|FD_ACCEPT|FD_CONNECT)
#define EVENT_MASK (FD_READ|FD_WRITE|FD_OOB|FD_ACCEPT|FD_CONNECT|FD_CLOSE)

#define LOCK_EVENTS	\
  if (wsock_mtx && \
      WaitForSingleObject (wsock_mtx, INFINITE) != WAIT_FAILED) \
    {

#define UNLOCK_EVENTS \
      ReleaseMutex (wsock_mtx); \
    }

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
get_inet_addr_local (const struct sockaddr *in, int inlen,
	       struct sockaddr_storage *out, int *outlen,
	       int *type = NULL, int *secret = NULL)
{
  int secret_buf [4];
  int* secret_ptr = (secret ? : secret_buf);

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

fhandler_socket_local::fhandler_socket_local () :
  fhandler_socket_wsock (),
  sun_path (NULL),
  peer_sun_path (NULL),
  status ()
{
}

fhandler_socket_local::~fhandler_socket_local ()
{
  if (sun_path)
    cfree (sun_path);
  if (peer_sun_path)
    cfree (peer_sun_path);
}

int
fhandler_socket_local::socket (int af, int type, int protocol, int flags)
{
  SOCKET sock;
  int ret;

  if (type != SOCK_STREAM && type != SOCK_DGRAM)
    {
      set_errno (EINVAL);
      return -1;
    }
  if (protocol != 0)
    {
      set_errno (EPROTONOSUPPORT);
      return -1;
    }
  sock = ::socket (AF_INET, type, protocol);
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
fhandler_socket_local::socketpair (int af, int type, int protocol, int flags,
				   fhandler_socket *_fh_out)
{
  SOCKET insock = INVALID_SOCKET;
  SOCKET outsock = INVALID_SOCKET;
  SOCKET sock = INVALID_SOCKET;
  struct sockaddr_in sock_in, sock_out;
  int len;
  fhandler_socket_local *fh_out = reinterpret_cast<fhandler_socket_local *>
				  (_fh_out);

  if (type != SOCK_STREAM && type != SOCK_DGRAM)
        {
          set_errno (EINVAL);
          return -1;
        }
  if (protocol != 0)
    {
      set_errno (EPROTONOSUPPORT);
      return -1;
    }
  /* create listening socket */
  sock = ::socket (AF_INET, type, 0);
  if (sock == INVALID_SOCKET)
    {
      set_winsock_errno ();
      goto err;
    }
  /* bind to unused port */
  sock_in.sin_family = AF_INET;
  sock_in.sin_port = 0;
  sock_in.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
  if (::bind (sock, (struct sockaddr *) &sock_in, sizeof (sock_in)) < 0)
    {
      set_winsock_errno ();
      goto err;
    }
  /* fetch socket name */
  len = sizeof (sock_in);
  if (::getsockname (sock, (struct sockaddr *) &sock_in, &len) < 0)
    {
      set_winsock_errno ();
      goto err;
    }
  /* on stream sockets, create listener */
  if (type == SOCK_STREAM && ::listen (sock, 2) < 0)
    {
      set_winsock_errno ();
      goto err;
    }
  /* create connecting socket */
  outsock = ::socket (AF_INET, type, 0);
  if (outsock == INVALID_SOCKET)
    {
      set_winsock_errno ();
      goto err;
    }
  /* on datagram sockets, bind connecting socket */
  if (type == SOCK_DGRAM)
    {
      sock_out.sin_family = AF_INET;
      sock_out.sin_port = 0;
      sock_out.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
      if (::bind (outsock, (struct sockaddr *) &sock_out,
		  sizeof (sock_out)) < 0)
	{
	  set_winsock_errno ();
	  goto err;
	}
      /* ...and fetch name */
      len = sizeof (sock_out);
      if (::getsockname (outsock, (struct sockaddr *) &sock_out, &len) < 0)
	{
	  set_winsock_errno ();
	  goto err;
	}
    }
  sock_in.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
  if (type == SOCK_DGRAM)
    sock_out.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
  /* connect */
  if (::connect (outsock, (struct sockaddr *) &sock_in, sizeof (sock_in)) < 0)
    {
      set_winsock_errno ();
      goto err;
    }
  if (type == SOCK_STREAM)
    {
      /* on stream sockets, accept connection and close listener */
      len = sizeof (sock_in);
      insock = ::accept (sock, (struct sockaddr *) &sock_in, &len);
      if (insock == INVALID_SOCKET)
	{
	  set_winsock_errno ();
	  goto err;
	}
      ::closesocket (sock);
    }
  else
    {
      /* on datagram sockets, connect vice versa */
      if (::connect (sock, (struct sockaddr *) &sock_out,
		   sizeof (sock_out)) < 0)
	{
	  set_winsock_errno ();
	  goto err;
	}
      insock = sock;
    }
  sock = INVALID_SOCKET;

  /* postprocessing */
  connect_state (connected);
  fh_out->connect_state (connected);
  if (af == AF_LOCAL && type == SOCK_STREAM)
    {
      af_local_set_sockpair_cred ();
      fh_out->af_local_set_sockpair_cred ();
    }
  if (set_socket_handle (insock, af, type, flags) < 0
      || fh_out->set_socket_handle (outsock, af, type, flags) < 0)
    goto err;

  return 0;

err:
  if (sock != INVALID_SOCKET)
    ::closesocket (sock);
  if (insock != INVALID_SOCKET)
    ::closesocket (insock);
  if (outsock != INVALID_SOCKET)
    ::closesocket (outsock);
  return -1;
}

void
fhandler_socket_local::af_local_set_sockpair_cred ()
{
  sec_pid = sec_peer_pid = getpid ();
  sec_uid = sec_peer_uid = geteuid ();
  sec_gid = sec_peer_gid = getegid ();
}

void
fhandler_socket_local::af_local_setblocking (bool &async, bool &nonblocking)
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
fhandler_socket_local::af_local_unsetblocking (bool async, bool nonblocking)
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
fhandler_socket_local::af_local_recv_secret ()
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
fhandler_socket_local::af_local_send_secret ()
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
fhandler_socket_local::af_local_recv_cred ()
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
fhandler_socket_local::af_local_send_cred ()
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
fhandler_socket_local::af_local_connect ()
{
  bool orig_async_io, orig_is_nonblocking;

  if (get_socket_type () != SOCK_STREAM)
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
fhandler_socket_local::af_local_accept ()
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
fhandler_socket_local::af_local_set_no_getpeereid ()
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
fhandler_socket_local::af_local_set_cred ()
{
  sec_pid = getpid ();
  sec_uid = geteuid ();
  sec_gid = getegid ();
  sec_peer_pid = (pid_t) 0;
  sec_peer_uid = (uid_t) -1;
  sec_peer_gid = (gid_t) -1;
}

void
fhandler_socket_local::af_local_copy (fhandler_socket_local *sock)
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
fhandler_socket_local::af_local_set_secret (char *buf)
{
  if (!RtlGenRandom (connect_secret, sizeof (connect_secret)))
    bzero ((char*) connect_secret, sizeof (connect_secret));
  __small_sprintf (buf, "%08x-%08x-%08x-%08x",
		   connect_secret [0], connect_secret [1],
		   connect_secret [2], connect_secret [3]);
}

int
fhandler_socket_local::dup (fhandler_base *child, int flags)
{
  if (get_flags () & O_PATH)
    /* We're viewing the socket as a disk file, but fhandler_base::dup
       suffices here. */
    return fhandler_base::dup (child, flags);

  fhandler_socket_local *fhs = (fhandler_socket_local *) child;
  fhs->set_sun_path (get_sun_path ());
  fhs->set_peer_sun_path (get_peer_sun_path ());
  return fhandler_socket_wsock::dup (child, flags);
}

int
fhandler_socket_local::open (int flags, mode_t mode)
{
  /* We don't support opening sockets unless O_PATH is specified. */
  if (flags & O_PATH)
    return open_fs (flags, mode);

  set_errno (EOPNOTSUPP);
  return 0;
}

int
fhandler_socket_local::close ()
{
  if (get_flags () & O_PATH)
    return fhandler_base::close ();
  else
    return fhandler_socket_wsock::close ();
}

int
fhandler_socket_local::fcntl (int cmd, intptr_t arg)
{
  if (get_flags () & O_PATH)
    /* We're viewing the socket as a disk file, but
       fhandler_base::fcntl suffices here. */
    return fhandler_base::fcntl (cmd, arg);
  else
    return fhandler_socket_wsock::fcntl (cmd, arg);
}

int
fhandler_socket_local::fstat (struct stat *buf)
{
  if (!dev ().isfs ())
    /* fstat called on a socket. */
    return fhandler_socket_wsock::fstat (buf);

  /* stat/lstat on a socket file or fstat on a socket opened w/ O_PATH. */
  int res = fhandler_base::fstat_fs (buf);
  if (!res)
    {
      buf->st_mode = (buf->st_mode & ~S_IFMT) | S_IFSOCK;
      buf->st_size = 0;
    }
  return res;
}

int
fhandler_socket_local::fstatvfs (struct statvfs *sfs)
{
  if (!dev ().isfs ())
    /* fstatvfs called on a socket. */
    return fhandler_socket_wsock::fstatvfs (sfs);

  /* statvfs on a socket file or fstatvfs on a socket opened w/ O_PATH. */
  if (get_flags () & O_PATH)
    /* We already have a handle. */
    {
      HANDLE h = get_handle ();
      if (h)
	return fstatvfs_by_handle (h, sfs);
    }
  fhandler_disk_file fh (pc);
  fh.get_device () = FH_FS;
  return fh.fstatvfs (sfs);
}

int
fhandler_socket_local::fchmod (mode_t newmode)
{
  if (!dev ().isfs ())
    /* fchmod called on a socket. */
    return fhandler_socket_wsock::fchmod (newmode);

  /* chmod on a socket file.  [We won't get here if fchmod is called
     on a socket opened w/ O_PATH.] */
  fhandler_disk_file fh (pc);
  fh.get_device () = FH_FS;
  return fh.fchmod (S_IFSOCK | adjust_socket_file_mode (newmode));
}

int
fhandler_socket_local::fchown (uid_t uid, gid_t gid)
{
  if (!dev ().isfs ())
    /* fchown called on a socket. */
    return fhandler_socket_wsock::fchown (uid, gid);

  /* chown/lchown on a socket file.  [We won't get here if fchown is
     called on a socket opened w/ O_PATH.] */
  fhandler_disk_file fh (pc);
  return fh.fchown (uid, gid);
}

int
fhandler_socket_local::facl (int cmd, int nentries, aclent_t *aclbufp)
{
  if (!dev ().isfs ())
    /* facl called on a socket. */
    return fhandler_socket_wsock::facl (cmd, nentries, aclbufp);

  /* facl on a socket file.  [We won't get here if facl is called on a
     socket opened w/ O_PATH.] */
  fhandler_disk_file fh (pc);
  return fh.facl (cmd, nentries, aclbufp);
}

int
fhandler_socket_local::link (const char *newpath)
{
  if (!dev ().isfs ())
    /* linkat w/ AT_EMPTY_PATH called on a socket not opened w/ O_PATH. */
    return fhandler_socket_wsock::link (newpath);
  /* link on a socket file or linkat w/ AT_EMPTY_PATH called on a
     socket opened w/ O_PATH. */
  fhandler_disk_file fh (pc);
  if (get_flags () & O_PATH)
    fh.set_handle (get_handle ());
  return fh.link (newpath);
}

int
fhandler_socket_local::bind (const struct sockaddr *name, int namelen)
{
  int res = -1;

#define un_addr ((struct sockaddr_un *) name)
  struct sockaddr_in sin;
  int len = namelen - offsetof (struct sockaddr_un, sun_path);

  /* Check that name is within bounds.  Don't check if the string is
     NUL-terminated, because there are projects out there which set
     namelen to a value which doesn't cover the trailing NUL. */
  if (len <= 1 || (len = strnlen (un_addr->sun_path, len)) > UNIX_PATH_MAX)
    {
      set_errno (len <= 1 ? (len == 1 ? ENOENT : EINVAL) : ENAMETOOLONG);
      return -1;
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
      return -1;
    }
  if (pc.exists ())
    {
      set_errno (EADDRINUSE);
      return -1;
    }

  sin.sin_family = AF_INET;
  sin.sin_port = 0;
  sin.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
  if (::bind (get_socket (), (sockaddr *) &sin, len = sizeof sin))
    {
      syscall_printf ("AF_LOCAL: bind failed");
      set_winsock_errno ();
      return -1;
    }
  if (::getsockname (get_socket (), (sockaddr *) &sin, &len))
    {
      syscall_printf ("AF_LOCAL: getsockname failed");
      set_winsock_errno ();
      return -1;
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

  return res;
}

int
fhandler_socket_local::connect (const struct sockaddr *name, int namelen)
{
  struct sockaddr_storage sst;
  int type = 0;
  bool reset = (name->sa_family == AF_UNSPEC
		&& get_socket_type () == SOCK_DGRAM);

  if (reset)
    {
      if (connect_state () == unconnected)
	return 0;
      /* To reset a connected DGRAM socket, call Winsock's connect
	 function with the address member of the sockaddr structure
	 filled with zeroes. */
      memset (&sst, 0, sizeof sst);
      sst.ss_family = get_addr_family ();
    }
  else if (get_inet_addr_local (name, namelen, &sst, &namelen, &type,
				connect_secret) == SOCKET_ERROR)
    return SOCKET_ERROR;

  if (get_socket_type () != type && !reset)
    {
      WSASetLastError (WSAEPROTOTYPE);
      set_winsock_errno ();
      return SOCKET_ERROR;
    }

  if (reset)
    set_peer_sun_path (NULL);
  else
    set_peer_sun_path (name->sa_data);

  /* Don't move af_local_set_cred into af_local_connect which may be called
     via select, possibly running under another identity.  Call early here,
     because af_local_connect is called in wait_for_events. */
  if (get_socket_type () == SOCK_STREAM)
    af_local_set_cred ();

  /* Initialize connect state to "connect_pending".  In the SOCK_STREAM
     case, the state is ultimately set to "connected" or "connect_failed" in
     wait_for_events when the FD_CONNECT event occurs.  Note that the
     underlying OS sockets are always non-blocking in this case and a
     successfully initiated non-blocking Winsock connect always returns
     WSAEWOULDBLOCK.  Thus it's safe to rely on event handling.  For DGRAM
     sockets, however, connect can return immediately.

     Check for either unconnected or connect_failed since in both cases it's
     allowed to retry connecting the socket.  It's also ok (albeit ugly) to
     call connect to check if a previous non-blocking connect finished.

     Set connect_state before calling connect, otherwise a race condition with
     an already running select or poll might occur. */
  if (connect_state () == unconnected || connect_state () == connect_failed)
    connect_state (connect_pending);

  int res = ::connect (get_socket (), (struct sockaddr *) &sst, namelen);
  if (!res)
    {
      if (reset)
	connect_state (unconnected);
      else
	connect_state (connected);
    }
  else if (!is_nonblocking ()
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
      /* Any other error except WSAEALREADY means the connect failed. */
      else if (connect_state () == connect_pending && err != WSAEALREADY)
	connect_state (connect_failed);
      set_winsock_errno ();
    }

  return res;
}

int
fhandler_socket_local::listen (int backlog)
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
fhandler_socket_local::accept4 (struct sockaddr *peer, int *len, int flags)
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
	  fhandler_socket_local *sock = (fhandler_socket_local *)
					build_fh_dev (dev ());
	  if (sock && sock->set_socket_handle (res, get_addr_family (),
					       get_socket_type (),
					       get_socket_flags ()) == 0)
	    {
	      sock->async_io (false); /* set_socket_handle disables async. */
	      sock->set_sun_path (get_sun_path ());
	      sock->set_peer_sun_path (get_peer_sun_path ());
	      if (get_socket_type () == SOCK_STREAM)
		{
		  /* Don't forget to copy credentials from accepting
		     socket to accepted socket and start transaction
		     on accepted socket! */
		  af_local_copy (sock);
		  ret = sock->af_local_accept ();
		  if (ret == -1)
		    {
		      delete sock;
		      set_winsock_errno ();
		      return -1;
		    }
		}
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
		  /* FIXME: Right now we have no way to determine the
		     bound socket name of the peer's socket.  For now
		     we just fake an unbound socket on the other side. */
		  static struct sockaddr_un un = { AF_LOCAL, "" };
		  memcpy (peer, &un, MIN (*len, (int) sizeof (un.sun_family)));
		  *len = (int) sizeof (un.sun_family);
		}
	    }
	  else
	    delete sock;
	}
      if (ret == -1)
	::closesocket (res);
    }
  return ret;
}

int
fhandler_socket_local::getsockname (struct sockaddr *name, int *namelen)
{
  struct sockaddr_un sun;

  sun.sun_family = AF_LOCAL;
  sun.sun_path[0] = '\0';
  if (get_sun_path ())
    strncat (sun.sun_path, get_sun_path (), UNIX_PATH_MAX - 1);
  memcpy (name, &sun, MIN (*namelen, (int) SUN_LEN (&sun) + 1));
  *namelen = (int) SUN_LEN (&sun) + (get_sun_path () ? 1 : 0);
  return 0;
}

int
fhandler_socket_local::getpeername (struct sockaddr *name, int *namelen)
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
      struct sockaddr_un sun;
      memset (&sun, 0, sizeof sun);
      sun.sun_family = AF_LOCAL;
      sun.sun_path[0] = '\0';
      if (get_peer_sun_path ())
	strncat (sun.sun_path, get_peer_sun_path (), UNIX_PATH_MAX - 1);
      memcpy (name, &sun, MIN (*namelen, (int) SUN_LEN (&sun) + 1));
      *namelen = (int) SUN_LEN (&sun) + (get_peer_sun_path () ? 1 : 0);
    }
  return res;
}

ssize_t
fhandler_socket_local::recv_internal (LPWSAMSG wsamsg, bool use_recvmsg)
{
  ssize_t res = 0;
  DWORD ret = 0, wret;
  int evt_mask = FD_READ;
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

  /* Out-of-band data not supported by AF_LOCAL */
  if (wsamsg->dwFlags & MSG_OOB)
    {
      set_errno (EOPNOTSUPP);
      return SOCKET_ERROR;
    }

  wsamsg->dwFlags &= (MSG_PEEK | MSG_DONTROUTE);
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
      if (is_nonblocking () || (wsamsg->dwFlags & MSG_PEEK))
	waitall = false;
    }

  /* Note: Don't call WSARecvFrom(MSG_PEEK) without actually having data
     waiting in the buffers, otherwise the event handling gets messed up
     for some reason. */
  while (!(res = wait_for_events (evt_mask | FD_CLOSE, wait_flags))
	 || saw_shutdown_read ())
    {
      DWORD dwFlags = wsamsg->dwFlags;
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
	res = WSARecv (get_socket (), wsabuf, wsacnt, &wret, &dwFlags,
		       NULL, NULL);
      else
	res = WSARecvFrom (get_socket (), wsabuf, wsacnt, &wret,
			   &dwFlags, wsamsg->name, &wsamsg->namelen,
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
	  if (!wsacnt)
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

  if (wsamsg->name != NULL && orig_namelen >= (int) sizeof (sa_family_t))
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

ssize_t
fhandler_socket_local::sendto (const void *in_ptr, size_t len, int flags,
			       const struct sockaddr *to, int tolen)
{
  char *ptr = (char *) in_ptr;
  struct sockaddr_storage sst;

  /* Out-of-band data not supported by AF_LOCAL */
  if (flags & MSG_OOB)
    {
      set_errno (EOPNOTSUPP);
      return SOCKET_ERROR;
    }

  if (to && get_inet_addr_local (to, tolen, &sst, &tolen) == SOCKET_ERROR)
    return SOCKET_ERROR;

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
  return send_internal (&wsamsg, flags);
}

ssize_t
fhandler_socket_local::sendmsg (const struct msghdr *msg, int flags)
{
  /* TODO: Descriptor passing on AF_LOCAL sockets. */

  struct sockaddr_storage sst;
  int len = 0;

  /* Out-of-band data not supported by AF_LOCAL */
  if (flags & MSG_OOB)
    {
      set_errno (EOPNOTSUPP);
      return SOCKET_ERROR;
    }

  if (msg->msg_name
      && get_inet_addr_local ((struct sockaddr *) msg->msg_name,
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
  DWORD controllen = (DWORD) (get_socket_type () == SOCK_STREAM
			      || get_addr_family () == AF_LOCAL
			      ? 0 : msg->msg_controllen);
  WSAMSG wsamsg = { msg->msg_name ? (struct sockaddr *) &sst : NULL, len,
		    wsabuf, (DWORD) msg->msg_iovlen,
		    { controllen, (char *) msg->msg_control },
		    0 };
  return send_internal (&wsamsg, flags);
}

void
fhandler_socket_local::set_sun_path (const char *path)
{
  sun_path = path ? cstrdup (path) : NULL;
}

void
fhandler_socket_local::set_peer_sun_path (const char *path)
{
  peer_sun_path = path ? cstrdup (path) : NULL;
}

int
fhandler_socket_local::getpeereid (pid_t *pid, uid_t *euid, gid_t *egid)
{
  if (get_socket_type () != SOCK_STREAM)
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

int
fhandler_socket_local::setsockopt (int level, int optname, const void *optval,
				   socklen_t optlen)
{
  int ret = -1;

  /* Preprocessing setsockopt. */
  switch (level)
    {
    case SOL_SOCKET:
      switch (optname)
	{
	case SO_PEERCRED:
	  /* Switch off the AF_LOCAL handshake and thus SO_PEERCRED handling
	     for AF_LOCAL/SOCK_STREAM sockets.  This allows to handle special
	     situations in which connect is called before a listening socket
	     accepts connections.
	     FIXME: In the long run we should find a more generic solution
	     which doesn't require a blocking handshake in accept/connect
	     to exchange SO_PEERCRED credentials. */
	  /* Temporary: Allow SO_PEERCRED to only be zeroed. Two ways to
	     accomplish this: pass NULL,0 for optval,optlen; or pass the
	     address,length of an '(int) 0' set up by the caller. */
	  if ((!optval && !optlen) ||
		(optlen == (socklen_t) sizeof (int) && !*(int *) optval))
	    ret = af_local_set_no_getpeereid ();
	  else
	    set_errno (EINVAL);
	  return ret;

	case SO_REUSEADDR:
	  saw_reuseaddr (*(int *) optval);
	  return 0;

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
	    return 0;
	  set_errno (EDOM);
	  return -1;

	case SO_DEBUG:
	case SO_RCVBUF:
	case SO_RCVLOWAT:
	case SO_SNDBUF:
	case SO_SNDLOWAT:
	  break;

	default:
	  /* AF_LOCAL sockets simply ignore all other SOL_SOCKET options. */
	  return 0;
	}
      break;

    default:
      set_errno (ENOPROTOOPT);
      return -1;
    }

  /* Call Winsock setsockopt */
  ret = ::setsockopt (get_socket (), level, optname, (const char *) optval,
		      optlen);
  if (ret == SOCKET_ERROR)
    {
      set_winsock_errno ();
      return ret;
    }

  if (optlen == (socklen_t) sizeof (int))
    debug_printf ("setsockopt optval=%x", *(int *) optval);

  /* Postprocessing setsockopt, setting fhandler_socket members, etc. */
  switch (level)
    {
    case SOL_SOCKET:
      switch (optname)
	{
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
fhandler_socket_local::getsockopt (int level, int optname, const void *optval,
				   socklen_t *optlen)
{
  int ret = -1;

  /* Preprocessing getsockopt.*/
  switch (level)
    {
    case SOL_SOCKET:
      switch (optname)
	{
	case SO_PEERCRED:
	  {
	    struct ucred *cred = (struct ucred *) optval;

	    if (*optlen < (socklen_t) sizeof *cred)
	      {
		set_errno (EINVAL);
		return ret;
	      }
	    ret = getpeereid (&cred->pid, &cred->uid, &cred->gid);
	    if (!ret)
	      *optlen = (socklen_t) sizeof *cred;
	    return ret;
	  }

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
		return ret;
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

	case SO_ACCEPTCONN:
	case SO_DEBUG:
	case SO_ERROR:
	case SO_RCVBUF:
	case SO_RCVLOWAT:
	case SO_SNDBUF:
	case SO_SNDLOWAT:
	  break;

	/* AF_LOCAL sockets simply ignore all other SOL_SOCKET options. */

	case SO_LINGER:
	  {
	    struct linger *linger = (struct linger *) optval;
	    memset (linger, 0, sizeof *linger);
	    *optlen = (socklen_t) sizeof *linger;
	    return 0;
	  }

	default:
	  {
	    unsigned int *val = (unsigned int *) optval;
	    *val = 0;
	    *optlen = (socklen_t) sizeof *val;
	    return 0;
	  }
	}
      break;

    default:
      set_errno (ENOPROTOOPT);
      return -1;
    }

  /* Call Winsock getsockopt */
  ret = ::getsockopt (get_socket (), level, optname, (char *) optval,
		      (int *) optlen);
  if (ret == SOCKET_ERROR)
    {
      set_winsock_errno ();
      return ret;
    }

  /* Postprocessing getsockopt, setting fhandler_socket members, etc. */
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

	default:
	  break;
	}
      break;
    default:
      break;
    }

  return ret;
}

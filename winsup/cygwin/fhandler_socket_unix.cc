/* fhandler_socket_unix.cc.

   See fhandler.h for a description of the fhandler classes.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include "winsup.h"
#include <w32api/winioctl.h>
#include <asm/byteorder.h>
#include <unistd.h>
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
#include "hires.h"
#include "shared_info.h"
#include "ntdll.h"
#include "miscfuncs.h"
#include "tls_pbuf.h"

/*
   Abstract socket:

     An abstract socket is represented by a symlink in the native
     NT namespace, within the Cygin subdir in BasedNamedObjects.
     So it's globally available but only exists as long as at least on
     descriptor on the socket is open, as desired.

     The name of the symlink is: "af-unix-<sun_path>"

     <sun_path> is the transposed sun_path string, including the leading
     NUL.  The transposition is simplified in that it uses every byte
     in the valid sun_path name as is, no extra multibyte conversion.
     The content of the symlink is the name of the underlying pipe.

  Named socket:

    A named socket is represented by a reparse point with a Cygwin-specific
    tag and GUID.  The GenericReparseBuffer content is the name of the
    underlying pipe.

  Pipe:

    The pipe is named \\.\pipe\cygwin-<installation_key>-unix-[sd]-<uniq_id>

    - <installation_key> is the 8 byte hex Cygwin installation key
    - [sd] is s for SOCK_STREAM, d for SOCK_DGRAM
    - <uniq_id> is an 8 byte hex unique number

   Note: We use MAX_PATH here for convenience where sufficient.  It's
   big enough to hold sun_path's as well as pipe names so we don't have
   to use tmp_pathbuf as often.
*/

GUID __cygwin_socket_guid = {
  .Data1 = 0xefc1714d,
  .Data2 = 0x7b19,
  .Data3 = 0x4407,
  .Data4 = { 0xba, 0xb3, 0xc5, 0xb1, 0xf9, 0x2c, 0xb8, 0x8c }
};

HANDLE
fhandler_socket_unix::create_abstract_link (const sun_name_t *sun,
					    PUNICODE_STRING pipe_name)
{
  WCHAR name[MAX_PATH];
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;
  UNICODE_STRING uname;
  HANDLE fh = NULL;

  PWCHAR p = wcpcpy (name, L"af-unix-");
  /* NUL bytes have no special meaning in an abstract socket name, so
     we assume iso-8859-1 for simplicity and transpose the string.
     transform_chars_af_unix is doing just that. */
  transform_chars_af_unix (p, sun->un.sun_path, sun->un_len);
  RtlInitUnicodeString (&uname, name);
  InitializeObjectAttributes (&attr, &uname, OBJ_CASE_INSENSITIVE,
			      get_shared_parent_dir (), NULL);
  /* Fill symlink with name of pipe */
  status = NtCreateSymbolicLinkObject (&fh, SYMBOLIC_LINK_ALL_ACCESS,
				       &attr, pipe_name);
  if (!NT_SUCCESS (status))
    {
      if (status == STATUS_OBJECT_NAME_EXISTS
	  || status == STATUS_OBJECT_NAME_COLLISION)
	set_errno (EADDRINUSE);
      else
	__seterrno_from_nt_status (status);
    }
  return fh;
}

struct rep_pipe_name_t
{
  USHORT Length;
  WCHAR  PipeName[1];
};

HANDLE
fhandler_socket_unix::create_reparse_point (const sun_name_t *sun,
					    PUNICODE_STRING pipe_name)
{
  ULONG access;
  HANDLE old_trans = NULL, trans = NULL;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  NTSTATUS status;
  HANDLE fh = NULL;
  PREPARSE_GUID_DATA_BUFFER rp;
  rep_pipe_name_t *rep_pipe_name;

  const DWORD data_len = sizeof (*rep_pipe_name) + pipe_name->Length;

  path_conv pc (sun->un.sun_path, PC_SYM_FOLLOW);
  if (pc.error)
    {
      set_errno (pc.error);
      return NULL;
    }
  if (pc.exists ())
    {
      set_errno (EADDRINUSE);
      return NULL;
    }
 /* We will overwrite the DACL after the call to NtCreateFile.  This
    requires READ_CONTROL and WRITE_DAC access, otherwise get_file_sd
    and set_file_sd both have to open the file again.
    FIXME: On remote NTFS shares open sometimes fails because even the
    creator of the file doesn't have the right to change the DACL.
    I don't know what setting that is or how to recognize such a share,
    so for now we don't request WRITE_DAC on remote drives. */
  access = DELETE | FILE_GENERIC_WRITE;
  if (!pc.isremote ())
    access |= READ_CONTROL | WRITE_DAC | WRITE_OWNER;
  if ((pc.fs_flags () & FILE_SUPPORTS_TRANSACTIONS))
    start_transaction (old_trans, trans);

retry_after_transaction_error:
  status = NtCreateFile (&fh, DELETE | FILE_GENERIC_WRITE,
			 pc.get_object_attr (attr, sec_none_nih), &io,
			 NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE,
			 FILE_SYNCHRONOUS_IO_NONALERT
			 | FILE_NON_DIRECTORY_FILE
			 | FILE_OPEN_FOR_BACKUP_INTENT
			 | FILE_OPEN_REPARSE_POINT,
			 NULL, 0);
  if (NT_TRANSACTIONAL_ERROR (status) && trans)
    {
      stop_transaction (status, old_trans, trans);
      goto retry_after_transaction_error;
    }

  if (!NT_SUCCESS (status))
    {
      if (io.Information == FILE_EXISTS)
	set_errno (EADDRINUSE);
      else
	__seterrno_from_nt_status (status);
      goto out;
    }
  rp = (PREPARSE_GUID_DATA_BUFFER)
       alloca (REPARSE_GUID_DATA_BUFFER_HEADER_SIZE + data_len);
  rp->ReparseTag = IO_REPARSE_TAG_CYGUNIX;
  rp->ReparseDataLength = data_len;
  rp->Reserved = 0;
  memcpy (&rp->ReparseGuid, CYGWIN_SOCKET_GUID, sizeof (GUID));
  rep_pipe_name = (rep_pipe_name_t *) rp->GenericReparseBuffer.DataBuffer;
  rep_pipe_name->Length = pipe_name->Length;
  memcpy (rep_pipe_name->PipeName, pipe_name->Buffer, pipe_name->Length);
  rep_pipe_name->PipeName[pipe_name->Length / sizeof (WCHAR)] = L'\0';
  status = NtFsControlFile (fh, NULL, NULL, NULL, &io,
			    FSCTL_SET_REPARSE_POINT, rp,
			    REPARSE_GUID_DATA_BUFFER_HEADER_SIZE
			    + rp->ReparseDataLength, NULL, 0);
  if (NT_SUCCESS (status))
    {
      set_created_file_access (fh, pc, S_IRUSR | S_IWUSR
				       | S_IRGRP | S_IWGRP
				       | S_IROTH | S_IWOTH);
      NtClose (fh);
      /* We don't have to keep the file open, but the caller needs to
         get a value != NULL to know the file creation went fine. */
      fh = INVALID_HANDLE_VALUE;
    }
  else if (!trans)
    {
      FILE_DISPOSITION_INFORMATION fdi = { TRUE };

      __seterrno_from_nt_status (status);
      status = NtSetInformationFile (fh, &io, &fdi, sizeof fdi,
				     FileDispositionInformation);
      if (!NT_SUCCESS (status))
	debug_printf ("Setting delete dispostion failed, status = %y",
		      status);
      NtClose (fh);
      fh = NULL;
    }

out:
  if (trans)
    stop_transaction (status, old_trans, trans);
  return fh;
}

HANDLE
fhandler_socket_unix::create_file (const sun_name_t *sun)
{
  if (sun->un_len <= (socklen_t) sizeof (sa_family_t)
      || (sun->un_len == 3 && sun->un.sun_path[0] == '\0')
      || sun->un_len > (socklen_t) sizeof sun->un)
    {
      set_errno (EINVAL);
      return NULL;
    }
  if (sun->un.sun_path[0] == '\0')
    return create_abstract_link (sun, pc.get_nt_native_path ());
  return create_reparse_point (sun, pc.get_nt_native_path ());
}

int
fhandler_socket_unix::open_abstract_link (sun_name_t *sun,
					  PUNICODE_STRING pipe_name)
{
  WCHAR name[MAX_PATH];
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;
  UNICODE_STRING uname;
  HANDLE fh;

  PWCHAR p = wcpcpy (name, L"af-unix-");
  p = transform_chars_af_unix (p, sun->un.sun_path, sun->un_len);
  *p = L'\0';
  RtlInitUnicodeString (&uname, name);
  InitializeObjectAttributes (&attr, &uname, OBJ_CASE_INSENSITIVE,
			      get_shared_parent_dir (), NULL);
  status = NtOpenSymbolicLinkObject (&fh, SYMBOLIC_LINK_QUERY, &attr);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return -1;
    }
  if (pipe_name)
    status = NtQuerySymbolicLinkObject (fh, pipe_name, NULL);
  NtClose (fh);
  if (pipe_name)
    {
      if (!NT_SUCCESS (status))
	{
	  __seterrno_from_nt_status (status);
	  return -1;
	}
      /* Enforce NUL-terminated pipe name. */
      pipe_name->Buffer[pipe_name->Length / sizeof (WCHAR)] = L'\0';
    }
  return 0;
}

int
fhandler_socket_unix::open_reparse_point (sun_name_t *sun,
					  PUNICODE_STRING pipe_name)
{
  /* TODO: Open reparse point and fetch type and pipe name */
  NTSTATUS status;
  HANDLE fh;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  PREPARSE_GUID_DATA_BUFFER rp;
  tmp_pathbuf tp;

  path_conv pc (sun->un.sun_path, PC_SYM_FOLLOW);
  if (pc.error)
    {
      set_errno (pc.error);
      return -1;
    }
  if (!pc.exists ())
    {
      set_errno (ENOENT);
      return -1;
    }
  pc.get_object_attr (attr, sec_none_nih);
  do
    {
      status = NtOpenFile (&fh, GENERIC_READ | SYNCHRONIZE, &attr, &io,
			   FILE_SHARE_VALID_FLAGS,
			   FILE_SYNCHRONOUS_IO_NONALERT
			   | FILE_NON_DIRECTORY_FILE
			   | FILE_OPEN_FOR_BACKUP_INTENT
			   | FILE_OPEN_REPARSE_POINT);
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
              return -1;
            }
          yield ();
        }
      else if (!NT_SUCCESS (status))
        {
          __seterrno_from_nt_status (status);
          return -1;
        }
    }
  while (status == STATUS_SHARING_VIOLATION);
  rp = (PREPARSE_GUID_DATA_BUFFER) tp.c_get ();
  status = NtFsControlFile (fh, NULL, NULL, NULL, &io, FSCTL_GET_REPARSE_POINT,
			    NULL, 0, rp, MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
  NtClose (fh);
  if (rp->ReparseTag == IO_REPARSE_TAG_CYGUNIX
      && memcmp (CYGWIN_SOCKET_GUID, &rp->ReparseGuid, sizeof (GUID)) == 0)
    {
      if (pipe_name)
	{
	  rep_pipe_name_t *rep_pipe_name = (rep_pipe_name_t *)
					   rp->GenericReparseBuffer.DataBuffer;
	  pipe_name->Length = rep_pipe_name->Length;
	  /* pipe name in reparse point is NUL-terminated */
	  memcpy (pipe_name->Buffer, rep_pipe_name->PipeName,
		  rep_pipe_name->Length + sizeof (WCHAR));
	}
      return 0;
    }
  return -1;
}

int
fhandler_socket_unix::open_file (sun_name_t *sun, int &type,
				 PUNICODE_STRING pipe_name)
{
  int ret = -1;

  if (sun->un_len <= (socklen_t) sizeof (sa_family_t)
      || (sun->un_len == 3 && sun->un.sun_path[0] == '\0')
      || sun->un_len > (socklen_t) sizeof sun->un)
    set_errno (EINVAL);
  else if (sun->un.sun_path[0] == '\0')
    ret = open_abstract_link (sun, pipe_name);
  else
    ret = open_reparse_point (sun, pipe_name);
  if (!ret)
    switch (pipe_name->Buffer[38])
      {
      case 'd':
	type = SOCK_DGRAM;
	break;
      case 's':
	type = SOCK_STREAM;
	break;
      default:
	set_errno (EINVAL);
	ret = -1;
	break;
      }
  return ret;
}

HANDLE
fhandler_socket_unix::autobind (sun_name_t* sun)
{
  uint32_t id;
  HANDLE fh;

  do
    {
      /* Use only 5 hex digits (up to 2^20 sockets) for Linux compat */
      set_unique_id ();
      id = get_unique_id () & 0xfffff;
      sun->un.sun_path[0] = '\0';
      sun->un_len = sizeof (sa_family_t)
		    + 1 /* leading NUL */
		    + __small_sprintf (sun->un.sun_path + 1, "%5X", id);
    }
  while ((fh = create_abstract_link (sun, pc.get_nt_native_path ())) == NULL);
  return fh;
}

wchar_t
fhandler_socket_unix::get_type_char ()
{
  switch (get_socket_type ())
    {
    case SOCK_STREAM:
      return 's';
    case SOCK_DGRAM:
      return 'd';
    default:
      return '?';
    }
}

void
fhandler_socket_unix::gen_pipe_name ()
{
  WCHAR pipe_name_buf[MAX_PATH];
  UNICODE_STRING pipe_name;

  __small_swprintf (pipe_name_buf,
		    L"\\Device\\NamedPipe\\cygwin-%S-unix-%C-%016_X",
		    &cygheap->installation_key,
		    get_type_char (),
		    get_plain_ino ());
  RtlInitUnicodeString (&pipe_name, pipe_name_buf);
  pc.set_nt_native_path (&pipe_name);
}

void
fhandler_socket_unix::set_wait_state (DWORD wait_state)
{
  if (get_handle ())
    {
      NTSTATUS status;
      IO_STATUS_BLOCK io;
      FILE_PIPE_INFORMATION fpi;

      fpi.ReadMode = FILE_PIPE_MESSAGE_MODE;
      fpi.CompletionMode = wait_state;
      status = NtSetInformationFile (get_handle (), &io, &fpi, sizeof fpi,
				     FilePipeInformation);
      if (!NT_SUCCESS (status))
	system_printf ("NtSetInformationFile(FilePipeInformation): %y", status);
    }
}

HANDLE
fhandler_socket_unix::create_pipe ()
{
  NTSTATUS status;
  HANDLE ph;
  ACCESS_MASK access;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  ULONG sharing;
  ULONG nonblocking;
  ULONG max_instances;
  LARGE_INTEGER timeout;

  access = GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE;
  sharing = FILE_SHARE_READ | FILE_SHARE_WRITE;
  InitializeObjectAttributes (&attr, pc.get_nt_native_path (), OBJ_INHERIT,
			      NULL, NULL);
  nonblocking = is_nonblocking () ? FILE_PIPE_COMPLETE_OPERATION
				  : FILE_PIPE_QUEUE_OPERATION;
  max_instances = (get_socket_type () == SOCK_DGRAM) ? 1 : -1;
  timeout.QuadPart = -500000;
  status = NtCreateNamedPipeFile (&ph, access, &attr, &io, sharing,
				  FILE_CREATE, FILE_SYNCHRONOUS_IO_NONALERT,
				  FILE_PIPE_MESSAGE_TYPE,
				  FILE_PIPE_MESSAGE_MODE,
				  nonblocking, max_instances,
				  PREFERRED_IO_BLKSIZE, PREFERRED_IO_BLKSIZE,
				  &timeout);
  if (!NT_SUCCESS (status))
    system_printf ("NtCreateNamedPipeFile: %y", status);
  return ph;
}

HANDLE
fhandler_socket_unix::create_pipe_instance ()
{
  NTSTATUS status;
  HANDLE ph;
  ACCESS_MASK access;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  ULONG sharing;
  ULONG nonblocking;
  ULONG max_instances;
  LARGE_INTEGER timeout;

  access = GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE;
  sharing = FILE_SHARE_READ | FILE_SHARE_WRITE;
  InitializeObjectAttributes (&attr, pc.get_nt_native_path (), OBJ_INHERIT,
			      NULL, NULL);
  nonblocking = is_nonblocking () ? FILE_PIPE_COMPLETE_OPERATION
				  : FILE_PIPE_QUEUE_OPERATION;
  max_instances = (get_socket_type () == SOCK_DGRAM) ? 1 : -1;
  timeout.QuadPart = -500000;
  status = NtCreateNamedPipeFile (&ph, access, &attr, &io, sharing,
				  FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT,
				  FILE_PIPE_MESSAGE_TYPE,
				  FILE_PIPE_MESSAGE_MODE,
				  nonblocking, max_instances,
				  PREFERRED_IO_BLKSIZE, PREFERRED_IO_BLKSIZE,
				  &timeout);
  if (!NT_SUCCESS (status))
    system_printf ("NtCreateNamedPipeFile: %y", status);
  return ph;
}

fhandler_socket_unix::fhandler_socket_unix () :
  sun_path (NULL),
  peer_sun_path (NULL)
{
  set_cred ();
}

fhandler_socket_unix::~fhandler_socket_unix ()
{
  if (sun_path)
    delete sun_path;
  if (peer_sun_path)
    delete peer_sun_path;
}

void
fhandler_socket_unix::set_sun_path (struct sockaddr_un *un, socklen_t unlen)
{
  if (!un)
    sun_path = NULL;
  sun_path = new sun_name_t ((const struct sockaddr *) un, unlen);
}

void
fhandler_socket_unix::set_peer_sun_path (struct sockaddr_un *un,
					 socklen_t unlen)
{
  if (!un)
    peer_sun_path = NULL;
  peer_sun_path = new sun_name_t ((const struct sockaddr *) un, unlen);
}

void
fhandler_socket_unix::set_cred ()
{
  peer_cred.pid = (pid_t) 0;
  peer_cred.uid = (uid_t) -1;
  peer_cred.gid = (gid_t) -1;
}

int
fhandler_socket_unix::dup (fhandler_base *child, int flags)
{
  fhandler_socket_unix *fhs = (fhandler_socket_unix *) child;
  fhs->set_sun_path (get_sun_path ());
  fhs->set_peer_sun_path (get_peer_sun_path ());
  return fhandler_socket::dup (child, flags);
}

int
fhandler_socket_unix::socket (int af, int type, int protocol, int flags)
{
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
  set_addr_family (af);
  set_socket_type (type);
  if (flags & SOCK_NONBLOCK)
    set_nonblocking (true);
  if (flags & SOCK_CLOEXEC)
    set_close_on_exec (true);
  set_io_handle (NULL);
  set_unique_id ();
  set_ino (get_unique_id ());
  return 0;
}

int
fhandler_socket_unix::socketpair (int af, int type, int protocol, int flags,
				  fhandler_socket *fh_out)
{
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
  set_errno (EAFNOSUPPORT);
  return -1;
}

int
fhandler_socket_unix::bind (const struct sockaddr *name, int namelen)
{
  __try
    {
      sun_name_t sun (name, namelen);
      bool unnamed = (sun.un_len == sizeof sun.un.sun_family);
      HANDLE pipe = NULL;

      if (get_handle ())
	{
	  set_errno (EINVAL);
	  __leave;
	}
      gen_pipe_name ();
      pipe = create_pipe ();
      if (pipe)
	{
	  file = unnamed ? autobind (&sun) : create_file (&sun);
	  if (!file)
	    {
	      NtClose (pipe);
	      __leave;
	    }
	  set_io_handle (pipe);
	  set_sun_path (&sun);
	  return 0;
	}
    }
  __except (EFAULT) {}
  __endtry
  return -1;
}

int
fhandler_socket_unix::listen (int backlog)
{
  set_errno (EAFNOSUPPORT);
  return -1;
}

int
fhandler_socket_unix::accept4 (struct sockaddr *peer, int *len, int flags)
{
  set_errno (EAFNOSUPPORT);
  return -1;
}

int
fhandler_socket_unix::connect (const struct sockaddr *name, int namelen)
{
  set_errno (EAFNOSUPPORT);
  return -1;
}

int
fhandler_socket_unix::getsockname (struct sockaddr *name, int *namelen)
{
  sun_name_t sun;

  if (get_sun_path ())
    memcpy (&sun, &get_sun_path ()->un, get_sun_path ()->un_len);
  else
    {
      sun.un_len = sizeof (sa_family_t);
      sun.un.sun_family = AF_UNIX;
      sun.un.sun_path[0] = '\0';
    }
  memcpy (name, &sun, MIN (*namelen, sun.un_len));
  return 0;
}

int
fhandler_socket_unix::getpeername (struct sockaddr *name, int *namelen)
{
  sun_name_t sun;

  if (get_peer_sun_path ())
    memcpy (&sun, &get_peer_sun_path ()->un, get_peer_sun_path ()->un_len);
  else
    {
      sun.un_len = sizeof (sa_family_t);
      sun.un.sun_family = AF_UNIX;
      sun.un.sun_path[0] = '\0';
    }
  memcpy (name, &sun, MIN (*namelen, sun.un_len));
  return 0;
}

int
fhandler_socket_unix::shutdown (int how)
{
  set_errno (EAFNOSUPPORT);
  return -1;
}

int
fhandler_socket_unix::close ()
{
  if (get_handle ())
    NtClose (get_handle ());
  if (file && file != INVALID_HANDLE_VALUE)
    NtClose (file);
  return 0;
}

int
fhandler_socket_unix::getpeereid (pid_t *pid, uid_t *euid, gid_t *egid)
{
  if (get_socket_type () != SOCK_STREAM)
    {
      set_errno (EINVAL);
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
	*pid = peer_cred.pid;
      if (euid)
	*euid = peer_cred.uid;
      if (egid)
	*egid = peer_cred.gid;
      return 0;
    }
  __except (EFAULT) {}
  __endtry
  return -1;
}

ssize_t
fhandler_socket_unix::recvfrom (void *ptr, size_t len, int flags,
				struct sockaddr *from, int *fromlen)
{
  set_errno (EAFNOSUPPORT);
  return -1;
}

ssize_t
fhandler_socket_unix::recvmsg (struct msghdr *msg, int flags)
{
  set_errno (EAFNOSUPPORT);
  return -1;
}

void __reg3
fhandler_socket_unix::read (void *ptr, size_t& len)
{
  set_errno (EAFNOSUPPORT);
  len = 0;
}

ssize_t __stdcall
fhandler_socket_unix::readv (const struct iovec *, int iovcnt, ssize_t tot)
{
  set_errno (EAFNOSUPPORT);
  return -1;
}

ssize_t
fhandler_socket_unix::sendto (const void *in_ptr, size_t len, int flags,
			       const struct sockaddr *to, int tolen)
{
  set_errno (EAFNOSUPPORT);
  return -1;
}

ssize_t
fhandler_socket_unix::sendmsg (const struct msghdr *msg, int flags)
{
  set_errno (EAFNOSUPPORT);
  return -1;
}

ssize_t __stdcall
fhandler_socket_unix::write (const void *ptr, size_t len)
{
  set_errno (EAFNOSUPPORT);
  return -1;
}

ssize_t __stdcall
fhandler_socket_unix::writev (const struct iovec *, int iovcnt, ssize_t tot)
{
  set_errno (EAFNOSUPPORT);
  return -1;
}

int
fhandler_socket_unix::setsockopt (int level, int optname, const void *optval,
				   socklen_t optlen)
{
  /* Preprocessing setsockopt. */
  switch (level)
    {
    case SOL_SOCKET:
      switch (optname)
	{
	case SO_PASSCRED:
	  break;

	case SO_REUSEADDR:
	  saw_reuseaddr (*(int *) optval);
	  break;

	case SO_RCVBUF:
	  rmem (*(int *) optval);
	  break;

	case SO_SNDBUF:
	  wmem (*(int *) optval);
	  break;

	case SO_RCVTIMEO:
	case SO_SNDTIMEO:
	  if (optlen < (socklen_t) sizeof (struct timeval))
	    {
	      set_errno (EINVAL);
	      return -1;
	    }
	  if (!timeval_to_ms ((struct timeval *) optval,
			      (optname == SO_RCVTIMEO) ? rcvtimeo ()
						       : sndtimeo ()))
	  {
	    set_errno (EDOM);
	    return -1;
	  }
	  break;

	default:
	  /* AF_UNIX sockets simply ignore all other SOL_SOCKET options. */
	  break;
	}
      break;

    default:
      set_errno (ENOPROTOOPT);
      return -1;
    }

  return 0;
}

int
fhandler_socket_unix::getsockopt (int level, int optname, const void *optval,
				   socklen_t *optlen)
{
  /* Preprocessing getsockopt.*/
  switch (level)
    {
    case SOL_SOCKET:
      switch (optname)
	{
	case SO_ERROR:
	  {
	    int *e = (int *) optval;
	    *e = 0;
	    break;
	  }

	case SO_PASSCRED:
	  break;

	case SO_PEERCRED:
	  {
	    struct ucred *cred = (struct ucred *) optval;

	    if (*optlen < (socklen_t) sizeof *cred)
	      {
		set_errno (EINVAL);
		return -1;
	      }
	    int ret = getpeereid (&cred->pid, &cred->uid, &cred->gid);
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
	    break;
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
	    break;
	  }

	case SO_TYPE:
	  {
	    unsigned int *type = (unsigned int *) optval;
	    *type = get_socket_type ();
	    *optlen = (socklen_t) sizeof *type;
	    break;
	  }

	/* AF_UNIX sockets simply ignore all other SOL_SOCKET options. */

	case SO_LINGER:
	  {
	    struct linger *linger = (struct linger *) optval;
	    memset (linger, 0, sizeof *linger);
	    *optlen = (socklen_t) sizeof *linger;
	    break;
	  }

	default:
	  {
	    unsigned int *val = (unsigned int *) optval;
	    *val = 0;
	    *optlen = (socklen_t) sizeof *val;
	    break;
	  }
	}
      break;

    default:
      set_errno (ENOPROTOOPT);
      return -1;
    }

  return 0;
}

int
fhandler_socket_unix::ioctl (unsigned int cmd, void *p)
{
  int ret;

  switch (cmd)
    {
    case FIOASYNC:
#ifdef __x86_64__
    case _IOW('f', 125, int):
#endif
      break;
    case FIONREAD:
#ifdef __x86_64__
    case _IOR('f', 127, int):
#endif
    case FIONBIO:
    case SIOCATMARK:
      break;
    default:
      ret = fhandler_socket::ioctl (cmd, p);
      break;
    }
  return ret;
}

int
fhandler_socket_unix::fcntl (int cmd, intptr_t arg)
{
  int ret = 0;

  switch (cmd)
    {
    case F_SETOWN:
      break;
    case F_GETOWN:
      break;
    default:
      ret = fhandler_socket::fcntl (cmd, arg);
      break;
    }
  return ret;
}

int __reg2
fhandler_socket_unix::fstat (struct stat *buf)
{
  int ret = 0;

  if (!get_sun_path ()
      || get_sun_path ()->un_len <= (socklen_t) sizeof (sa_family_t)
      || get_sun_path ()->un.sun_path[0] == '\0')
    return fhandler_socket::fstat (buf);
  ret = fhandler_base::fstat_fs (buf);
  if (!ret)
    {
      buf->st_mode = (buf->st_mode & ~S_IFMT) | S_IFSOCK;
      buf->st_size = 0;
    }
  return ret;
}

int __reg2
fhandler_socket_unix::fstatvfs (struct statvfs *sfs)
{
  if (!get_sun_path ()
      || get_sun_path ()->un_len <= (socklen_t) sizeof (sa_family_t)
      || get_sun_path ()->un.sun_path[0] == '\0')
    return fhandler_socket::fstatvfs (sfs);
  fhandler_disk_file fh (pc);
  fh.get_device () = FH_FS;
  return fh.fstatvfs (sfs);
}

int
fhandler_socket_unix::fchmod (mode_t newmode)
{
  if (!get_sun_path ()
      || get_sun_path ()->un_len <= (socklen_t) sizeof (sa_family_t)
      || get_sun_path ()->un.sun_path[0] == '\0')
    return fhandler_socket::fchmod (newmode);
  fhandler_disk_file fh (pc);
  fh.get_device () = FH_FS;
  /* Kludge: Don't allow to remove read bit on socket files for
     user/group/other, if the accompanying write bit is set.  It would
     be nice to have exact permissions on a socket file, but it's
     necessary that somebody able to access the socket can always read
     the contents of the socket file to avoid spurious "permission
     denied" messages. */
  newmode |= (newmode & (S_IWUSR | S_IWGRP | S_IWOTH)) << 1;
  return fh.fchmod (S_IFSOCK | newmode);
}

int
fhandler_socket_unix::fchown (uid_t uid, gid_t gid)
{
  if (!get_sun_path ()
      || get_sun_path ()->un_len <= (socklen_t) sizeof (sa_family_t)
      || get_sun_path ()->un.sun_path[0] == '\0')
    return fhandler_socket::fchown (uid, gid);
  fhandler_disk_file fh (pc);
  return fh.fchown (uid, gid);
}

int
fhandler_socket_unix::facl (int cmd, int nentries, aclent_t *aclbufp)
{
  if (!get_sun_path ()
      || get_sun_path ()->un_len <= (socklen_t) sizeof (sa_family_t)
      || get_sun_path ()->un.sun_path[0] == '\0')
    return fhandler_socket::facl (cmd, nentries, aclbufp);
  fhandler_disk_file fh (pc);
  return fh.facl (cmd, nentries, aclbufp);
}

int
fhandler_socket_unix::link (const char *newpath)
{
  if (!get_sun_path ()
      || get_sun_path ()->un_len <= (socklen_t) sizeof (sa_family_t)
      || get_sun_path ()->un.sun_path[0] == '\0')
    return fhandler_socket::link (newpath);
  fhandler_disk_file fh (pc);
  return fh.link (newpath);
}

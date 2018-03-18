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
     The content of the symlink is the basename of the underlying pipe.

  Named socket:

    A named socket is represented by a reparse point with a Cygwin-specific
    tag and GUID.  The GenericReparseBuffer content is the basename of the
    underlying pipe.

  Pipe:

    The pipe is named \\.\pipe\cygwin-<installation_key>-unix-[sd]-<uniq_id>

    - <installation_key> is the 8 byte hex Cygwin installation key
    - [sd] is s for SOCK_STREAM, d for SOCK_DGRAM
    - <uniq_id> is an 8 byte hex unique number

   Note: We use MAX_PATH below for convenience where sufficient.  It's
   big enough to hold sun_paths as well as pipe names as well as packet
   headers etc., so we don't have to use tmp_pathbuf as often.

   Every packet sent to a peer is a combination of the socket name of the
   local socket, the ancillary data, and the actual user data.  The data
   is always sent in this order.  The header contains length information
   for the entire packet, as well as for all three data blocks.  The
   combined maximum size of a packet is 64K, including the header.

   A connecting, bound STREAM socket sends it's local sun_path once after
   a successful connect.  An already connected socket also sends its local
   sun_path after a successful bind (border case, but still...).  These
   packages don't contain any other data (cmsg_len == 0, data_len == 0).

   A bound DGRAM socket sends its sun_path with each sendmsg/sendto.
*/
class af_unix_pkt_hdr_t
{
 public:
  uint16_t	pckt_len;	/* size of packet including header	*/
  bool		admin_pkg : 1;	/* admin packets are marked as such	*/
  shut_state	shut_info : 2;	/* _SHUT_RECV /_SHUT_SEND.		*/
  uint8_t	name_len;	/* size of name, a sockaddr_un		*/
  uint16_t	cmsg_len;	/* size of ancillary data block		*/
  uint16_t	data_len;	/* size of user data			*/

  af_unix_pkt_hdr_t (bool a, shut_state s, uint8_t n, uint16_t c, uint16_t d)
    { init (a, s, n, c, d); }
  void init (bool a, shut_state s, uint8_t n, uint16_t c, uint16_t d)
    {
      admin_pkg = a;
      shut_info = s;
      name_len = n;
      cmsg_len = c;
      data_len = d;
      pckt_len = sizeof (*this) + name_len + cmsg_len + data_len;
    }
};

#define AF_UNIX_PKT_OFFSETOF_NAME(phdr)	\
	(sizeof (af_unix_pkt_hdr_t))
#define AF_UNIX_PKT_OFFSETOF_CMSG(phdr)	\
	(sizeof (af_unix_pkt_hdr_t) + (phdr)->name_len)
#define AF_UNIX_PKT_OFFSETOF_DATA(phdr)	\
	({ \
	   af_unix_pkt_hdr_t *_p = phdr; \
	   sizeof (af_unix_pkt_hdr_t) + (_p)->name_len + (_p)->cmsg_len; \
	})
#define AF_UNIX_PKT_NAME(phdr) \
	({ \
	   af_unix_pkt_hdr_t *_p = phdr; \
	   (struct sockaddr_un *)(((PBYTE)(_p)) \
				  + AF_UNIX_PKT_OFFSETOF_NAME (_p)); \
	})
#define AF_UNIX_PKT_CMSG(phdr) \
	({ \
	   af_unix_pkt_hdr_t *_p = phdr; \
	   (struct cmsghdr *)(((PBYTE)(_p)) + AF_UNIX_PKT_OFFSETOF_CMSG (_p)); \
	})
#define AF_UNIX_PKT_DATA(phdr) \
	({ \
	   af_unix_pkt_hdr_t _p = phdr; \
	   (void *)(((PBYTE)(_p)) + AF_UNIX_PKT_OFFSETOF_DATA (_p)); \
	})

GUID __cygwin_socket_guid = {
  .Data1 = 0xefc1714d,
  .Data2 = 0x7b19,
  .Data3 = 0x4407,
  .Data4 = { 0xba, 0xb3, 0xc5, 0xb1, 0xf9, 0x2c, 0xb8, 0x8c }
};

/* Some error conditions on pipes have multiple status codes, unfortunately. */
#define STATUS_PIPE_NO_INSTANCE_AVAILABLE(status)	\
		({ NTSTATUS _s = (status); \
		   _s == STATUS_INSTANCE_NOT_AVAILABLE \
		   || _s == STATUS_PIPE_NOT_AVAILABLE \
		   || _s == STATUS_PIPE_BUSY; })

#define STATUS_PIPE_IS_CLOSING(status)	\
		({ NTSTATUS _s = (status); \
		   _s == STATUS_PIPE_CLOSING \
		   || _s == STATUS_PIPE_EMPTY; })

#define STATUS_PIPE_INVALID(status) \
		({ NTSTATUS _s = (status); \
		   _s == STATUS_INVALID_INFO_CLASS \
		   || _s == STATUS_INVALID_PIPE_STATE \
		   || _s == STATUS_INVALID_READ_MODE; })

#define STATUS_PIPE_MORE_DATA(status) \
		({ NTSTATUS _s = (status); \
		   _s == STATUS_BUFFER_OVERFLOW \
		   || _s == STATUS_MORE_PROCESSING_REQUIRED; })

/* Default timeout value of connect: 20 secs, as on Linux. */
#define AF_UNIX_CONNECT_TIMEOUT (-20 * NS100PERSEC)

void
sun_name_t::set (const struct sockaddr_un *name, socklen_t namelen)
{
  if (namelen < 0)
    namelen = 0;
  un_len = namelen < (__socklen_t) sizeof un ? namelen : sizeof un;
  un.sun_family = AF_UNIX;
  if (name && un_len)
    memcpy (&un, name, un_len);
  _nul[un_len] = '\0';
}

static HANDLE
create_event ()
{
  NTSTATUS status;
  OBJECT_ATTRIBUTES attr;
  HANDLE evt = NULL;

  InitializeObjectAttributes (&attr, NULL, 0, NULL, NULL);
  status = NtCreateEvent (&evt, EVENT_ALL_ACCESS, &attr,
			  NotificationEvent, FALSE);
  if (!NT_SUCCESS (status))
    __seterrno_from_nt_status (status);
  return evt;
}

/* Called from socket, socketpair, accept4 */
int
fhandler_socket_unix::create_shmem ()
{
  HANDLE sect;
  OBJECT_ATTRIBUTES attr;
  NTSTATUS status;
  LARGE_INTEGER size = { .QuadPart = sizeof (af_unix_shmem_t) };
  SIZE_T viewsize = sizeof (af_unix_shmem_t);
  PVOID addr = NULL;

  InitializeObjectAttributes (&attr, NULL, OBJ_INHERIT, NULL, NULL);
  status = NtCreateSection (&sect, STANDARD_RIGHTS_REQUIRED | SECTION_QUERY
				   | SECTION_MAP_READ | SECTION_MAP_WRITE,
			    &attr, &size, PAGE_READWRITE, SEC_COMMIT, NULL);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return -1;
    }
  status = NtMapViewOfSection (sect, NtCurrentProcess (), &addr, 0, viewsize,
			       NULL, &viewsize, ViewShare, 0, PAGE_READWRITE);
  if (!NT_SUCCESS (status))
    {
      NtClose (sect);
      __seterrno_from_nt_status (status);
      return -1;
    }
  shmem_handle = sect;
  shmem = (af_unix_shmem_t *) addr;
  return 0;
}

/* Called from dup, fixup_after_fork.  Expects shmem_handle to be
   valid. */
int
fhandler_socket_unix::reopen_shmem ()
{
  NTSTATUS status;
  SIZE_T viewsize = PAGESIZE;
  PVOID addr = NULL;

  status = NtMapViewOfSection (shmem_handle, NtCurrentProcess (), &addr, 0,
			       PAGESIZE, NULL, &viewsize, ViewShare, 0,
			       PAGE_READWRITE);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return -1;
    }
  shmem = (af_unix_shmem_t *) addr;
  return 0;
}

/* Character length of pipe name, excluding trailing NUL. */
#define CYGWIN_PIPE_SOCKET_NAME_LEN     47

/* Character position encoding the socket type in a pipe name. */
#define CYGWIN_PIPE_SOCKET_TYPE_POS	29

void
fhandler_socket_unix::gen_pipe_name ()
{
  WCHAR pipe_name_buf[CYGWIN_PIPE_SOCKET_NAME_LEN + 1];
  UNICODE_STRING pipe_name;

  __small_swprintf (pipe_name_buf, L"cygwin-%S-unix-%C-%016_X",
		    &cygheap->installation_key,
		    get_type_char (),
		    get_unique_id ());
  RtlInitUnicodeString (&pipe_name, pipe_name_buf);
  pc.set_nt_native_path (&pipe_name);
}

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
  p = transform_chars_af_unix (p, sun->un.sun_path, sun->un_len);
  *p = L'\0';
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

  const DWORD data_len = offsetof (rep_pipe_name_t, PipeName)
			 + pipe_name->Length + sizeof (WCHAR);

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
      mode_t perms = (S_IRWXU | S_IRWXG | S_IRWXO) & ~cygheap->umask;
      set_created_file_access (fh, pc, perms);
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
      || (sun->un_len == 3 && sun->un.sun_path[0] == '\0'))
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
      status = NtOpenFile (&fh, FILE_GENERIC_READ, &attr, &io,
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
      || (sun->un_len == 3 && sun->un.sun_path[0] == '\0'))
    set_errno (EINVAL);
  else if (sun->un.sun_path[0] == '\0')
    ret = open_abstract_link (sun, pipe_name);
  else
    ret = open_reparse_point (sun, pipe_name);
  if (!ret)
    switch (pipe_name->Buffer[CYGWIN_PIPE_SOCKET_TYPE_POS])
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

/* This also sets the pipe to message mode unconditionally. */
void
fhandler_socket_unix::set_pipe_non_blocking (bool nonblocking)
{
  if (get_handle ())
    {
      NTSTATUS status;
      IO_STATUS_BLOCK io;
      FILE_PIPE_INFORMATION fpi;

      fpi.ReadMode = FILE_PIPE_MESSAGE_MODE;
      fpi.CompletionMode = nonblocking ? FILE_PIPE_COMPLETE_OPERATION
				       : FILE_PIPE_QUEUE_OPERATION;
      status = NtSetInformationFile (get_handle (), &io, &fpi, sizeof fpi,
				     FilePipeInformation);
      if (!NT_SUCCESS (status))
	debug_printf ("NtSetInformationFile(FilePipeInformation): %y", status);
    }
}

/* Apart from being called from bind(), from_bind indicates that the caller
   already locked state_lock, so send_sock_info doesn't lock, only unlocks
   state_lock. */
int
fhandler_socket_unix::send_sock_info (bool from_bind)
{
  sun_name_t *sun;
  size_t plen;
  size_t clen = 0;
  af_unix_pkt_hdr_t *packet;
  NTSTATUS status;
  IO_STATUS_BLOCK io;

  if (!from_bind)
    {
      state_lock ();
      /* When called from connect, initialize credentials.  accept4 already
	 did it (copied from listening socket). */
      if (sock_cred ()->pid == 0)
	set_cred ();
    }
  sun = sun_path ();
  plen = sizeof *packet + sun->un_len;
  /* When called from connect/accept4, send SCM_CREDENTIALS, too. */
  if (!from_bind)
    {
      clen = CMSG_SPACE (sizeof (struct ucred));
      plen += clen;
    }
  packet = (af_unix_pkt_hdr_t *) alloca (plen);
  packet->init (true, _SHUT_NONE, sun->un_len, clen, 0);
  if (sun)
    memcpy (AF_UNIX_PKT_NAME (packet), &sun->un, sun->un_len);
  if (!from_bind)
    {
      struct cmsghdr *cmsg = AF_UNIX_PKT_CMSG (packet);
      cmsg->cmsg_level = SOL_SOCKET;
      cmsg->cmsg_type = SCM_CREDENTIALS;
      cmsg->cmsg_len = CMSG_LEN (sizeof (struct ucred));
      memcpy (CMSG_DATA(cmsg), sock_cred (), sizeof (struct ucred));
    }

  state_unlock ();

  /* The theory: Fire and forget. */
  io_lock ();
  set_pipe_non_blocking (true);
  status = NtWriteFile (get_handle (), NULL, NULL, NULL, &io, packet,
			packet->pckt_len, NULL, NULL);
  set_pipe_non_blocking (is_nonblocking ());
  io_unlock ();
  if (!NT_SUCCESS (status))
    {
      debug_printf ("Couldn't send my name: NtWriteFile: %y", status);
      return -1;
    }
  return 0;
}

/* Checks if the next packet in the pipe is an administrative packet.
   If so, it reads it from the pipe, handles it.  Returns an error code. */
int
fhandler_socket_unix::grab_admin_pkg ()
{
  HANDLE evt;
  NTSTATUS status;
  IO_STATUS_BLOCK io;
  /* MAX_PATH is more than sufficient for admin packets. */
  PFILE_PIPE_PEEK_BUFFER pbuf = (PFILE_PIPE_PEEK_BUFFER) alloca (MAX_PATH);
  if (!(evt = create_event ()))
    return 0;
  io_lock ();
  ULONG ret_len = peek_pipe (pbuf, MAX_PATH, evt);
  if (pbuf->NumberOfMessages == 0 || ret_len < sizeof (af_unix_pkt_hdr_t))
    {
      io_unlock ();
      NtClose (evt);
      return 0;
    }
  af_unix_pkt_hdr_t *packet = (af_unix_pkt_hdr_t *) pbuf->Data;
  if (!packet->admin_pkg)
    io_unlock ();
  else
    {
      packet = (af_unix_pkt_hdr_t *) pbuf;
      status = NtReadFile (get_handle (), evt, NULL, NULL, &io, packet,
			   MAX_PATH, NULL, NULL);
      if (status == STATUS_PENDING)
	{
	  /* Very short-lived */
	  status = NtWaitForSingleObject (evt, FALSE, NULL);
	  if (NT_SUCCESS (status))
	    status = io.Status;
	}
      io_unlock ();
      if (NT_SUCCESS (status))
	{
	  state_lock ();
	  if (packet->shut_info)
	    {
	      /* Peer's shutdown sends the SHUT flags as used by the peer.
		 They have to be reversed for our side. */
	      int shut_info = saw_shutdown ();
	      if (packet->shut_info & _SHUT_RECV)
		shut_info |= _SHUT_SEND;
	      if (packet->shut_info & _SHUT_SEND)
		shut_info |= _SHUT_RECV;
	      saw_shutdown (shut_info);
	      /* FIXME: anything else here? */
	    }
	  if (packet->name_len > 0)
	    peer_sun_path (AF_UNIX_PKT_NAME (packet), packet->name_len);
	  if (packet->cmsg_len > 0)
	    {
	      struct cmsghdr *cmsg = (struct cmsghdr *)
				     alloca (packet->cmsg_len);
	      memcpy (cmsg, AF_UNIX_PKT_CMSG (packet), packet->cmsg_len);
	      if (cmsg->cmsg_level == SOL_SOCKET
		  && cmsg->cmsg_type == SCM_CREDENTIALS)
		peer_cred ((struct ucred *) CMSG_DATA(cmsg));
	    }
	  state_unlock ();
	}
    }
  NtClose (evt);
  return 0;
}

/* Returns an error code.  Locking is not required when called from accept4,
   user space doesn't know about this socket yet. */
int
fhandler_socket_unix::recv_peer_info ()
{
  HANDLE evt;
  NTSTATUS status;
  IO_STATUS_BLOCK io;
  af_unix_pkt_hdr_t *packet;
  struct sockaddr_un *un;
  ULONG len;
  int ret = 0;

  if (!(evt = create_event ()))
    return ENOBUFS;
  len = sizeof *packet + sizeof *un + CMSG_SPACE (sizeof (struct ucred));
  packet = (af_unix_pkt_hdr_t *) alloca (len);
  set_pipe_non_blocking (false);
  status = NtReadFile (get_handle (), evt, NULL, NULL, &io, packet, len,
		       NULL, NULL);
  if (status == STATUS_PENDING)
    {
      DWORD ret;
      LARGE_INTEGER timeout;

      timeout.QuadPart = AF_UNIX_CONNECT_TIMEOUT;
      ret = cygwait (evt, &timeout, cw_sig_eintr);
      switch (ret)
	{
	case WAIT_OBJECT_0:
	  status = io.Status;
	  break;
	case WAIT_TIMEOUT:
	  ret = ECONNABORTED;
	  break;
	case WAIT_SIGNALED:
	  ret = EINTR;
	  break;
	default:
	  ret = EPROTO;
	  break;
	}
    }
  set_pipe_non_blocking (is_nonblocking ());
  NtClose (evt);
  if (!NT_SUCCESS (status) && ret == 0)
    ret = geterrno_from_nt_status (status);
  if (ret == 0)
    {
      if (packet->name_len > 0)
	peer_sun_path (AF_UNIX_PKT_NAME (packet), packet->name_len);
      if (packet->cmsg_len > 0)
	{
	  struct cmsghdr *cmsg = (struct cmsghdr *) alloca (packet->cmsg_len);
	  memcpy (cmsg, AF_UNIX_PKT_CMSG (packet), packet->cmsg_len);
	  if (cmsg->cmsg_level == SOL_SOCKET
	      && cmsg->cmsg_type == SCM_CREDENTIALS)
	    peer_cred ((struct ucred *) CMSG_DATA(cmsg));
	}
    }
  return ret;
}

NTSTATUS
fhandler_socket_unix::npfs_handle (HANDLE &nph)
{
  static NO_COPY SRWLOCK npfs_lock;
  static NO_COPY HANDLE npfs_dirh;

  NTSTATUS status = STATUS_SUCCESS;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;

  /* Lockless after first call. */
  if (npfs_dirh)
    {
      nph = npfs_dirh;
      return STATUS_SUCCESS;
    }
  AcquireSRWLockExclusive (&npfs_lock);
  if (!npfs_dirh)
    {
      InitializeObjectAttributes (&attr, &ro_u_npfs, 0, NULL, NULL);
      status = NtOpenFile (&npfs_dirh, FILE_READ_ATTRIBUTES | SYNCHRONIZE,
			   &attr, &io, FILE_SHARE_READ | FILE_SHARE_WRITE,
			   0);
    }
  ReleaseSRWLockExclusive (&npfs_lock);
  if (NT_SUCCESS (status))
    nph = npfs_dirh;
  return status;
}

HANDLE
fhandler_socket_unix::create_pipe (bool single_instance)
{
  NTSTATUS status;
  HANDLE npfsh;
  HANDLE ph;
  ACCESS_MASK access;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  ULONG sharing;
  ULONG nonblocking;
  ULONG max_instances;
  LARGE_INTEGER timeout;

  status = npfs_handle (npfsh);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return NULL;
    }
  access = GENERIC_READ | FILE_READ_ATTRIBUTES
	   | GENERIC_WRITE |  FILE_WRITE_ATTRIBUTES
	   | SYNCHRONIZE;
  sharing = FILE_SHARE_READ | FILE_SHARE_WRITE;
  InitializeObjectAttributes (&attr, pc.get_nt_native_path (),
			      OBJ_INHERIT | OBJ_CASE_INSENSITIVE,
			      npfsh, NULL);
  nonblocking = is_nonblocking () ? FILE_PIPE_COMPLETE_OPERATION
				  : FILE_PIPE_QUEUE_OPERATION;
  max_instances = single_instance ? 1 : -1;
  timeout.QuadPart = -500000;
  status = NtCreateNamedPipeFile (&ph, access, &attr, &io, sharing,
				  FILE_CREATE, 0,
				  FILE_PIPE_MESSAGE_TYPE,
				  FILE_PIPE_MESSAGE_MODE,
				  nonblocking, max_instances,
				  rmem (), wmem (), &timeout);
  if (!NT_SUCCESS (status))
    __seterrno_from_nt_status (status);
  return ph;
}

HANDLE
fhandler_socket_unix::create_pipe_instance ()
{
  NTSTATUS status;
  HANDLE npfsh;
  HANDLE ph;
  ACCESS_MASK access;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  ULONG sharing;
  ULONG nonblocking;
  ULONG max_instances;
  LARGE_INTEGER timeout;

  status = npfs_handle (npfsh);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return NULL;
    }
  access = GENERIC_READ | FILE_READ_ATTRIBUTES
	   | GENERIC_WRITE |  FILE_WRITE_ATTRIBUTES
	   | SYNCHRONIZE;
  sharing = FILE_SHARE_READ | FILE_SHARE_WRITE;
  /* NPFS doesn't understand reopening by handle, unfortunately. */
  InitializeObjectAttributes (&attr, pc.get_nt_native_path (), OBJ_INHERIT,
			      npfsh, NULL);
  nonblocking = is_nonblocking () ? FILE_PIPE_COMPLETE_OPERATION
				  : FILE_PIPE_QUEUE_OPERATION;
  max_instances = (get_socket_type () == SOCK_DGRAM) ? 1 : -1;
  timeout.QuadPart = -500000;
  status = NtCreateNamedPipeFile (&ph, access, &attr, &io, sharing,
				  FILE_OPEN, 0,
				  FILE_PIPE_MESSAGE_TYPE,
				  FILE_PIPE_MESSAGE_MODE,
				  nonblocking, max_instances,
				  rmem (), wmem (), &timeout);
  if (!NT_SUCCESS (status))
    __seterrno_from_nt_status (status);
  return ph;
}

NTSTATUS
fhandler_socket_unix::open_pipe (PUNICODE_STRING pipe_name, bool xchg_sock_info)
{
  NTSTATUS status;
  HANDLE npfsh;
  ACCESS_MASK access;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  ULONG sharing;
  HANDLE ph = NULL;

  status = npfs_handle (npfsh);
  if (!NT_SUCCESS (status))
    return status;
  access = GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE;
  InitializeObjectAttributes (&attr, pipe_name, OBJ_INHERIT, npfsh, NULL);
  sharing = FILE_SHARE_READ | FILE_SHARE_WRITE;
  status = NtOpenFile (&ph, access, &attr, &io, sharing, 0);
  if (NT_SUCCESS (status))
    {
      set_io_handle (ph);
      if (xchg_sock_info)
	send_sock_info (false);
    }
  return status;
}

struct conn_wait_info_t
{
  fhandler_socket_unix *fh;
  UNICODE_STRING pipe_name;
  WCHAR pipe_name_buf[CYGWIN_PIPE_SOCKET_NAME_LEN + 1];
};

/* Just hop to the wait_pipe_thread method. */
DWORD WINAPI
connect_wait_func (LPVOID param)
{
  conn_wait_info_t *wait_info = (conn_wait_info_t *) param;
  return wait_info->fh->wait_pipe_thread (&wait_info->pipe_name);
}

/* Start a waiter thread to wait for a pipe instance to become available.
   in blocking mode, wait for the thread to finish.  In nonblocking mode
   just return with errno set to EINPROGRESS. */
int
fhandler_socket_unix::wait_pipe (PUNICODE_STRING pipe_name)
{
  conn_wait_info_t *wait_info;
  DWORD waitret, err;
  int ret = -1;
  HANDLE thr, evt;
  PVOID param;

  if (!(cwt_termination_evt = create_event ()))
    return -1;
  wait_info = (conn_wait_info_t *) cmalloc (HEAP_3_FHANDLER, sizeof *wait_info);
  if (!wait_info)
    return -1;
  wait_info->fh = this;
  RtlInitEmptyUnicodeString (&wait_info->pipe_name, wait_info->pipe_name_buf,
			     sizeof wait_info->pipe_name_buf);
  RtlCopyUnicodeString (&wait_info->pipe_name, pipe_name);

  cwt_param = (PVOID) wait_info;
  connect_wait_thr = CreateThread (NULL, PREFERRED_IO_BLKSIZE,
				   connect_wait_func, cwt_param, 0, NULL);
  if (!connect_wait_thr)
    {
      cfree (wait_info);
      __seterrno ();
      goto out;
    }
  if (is_nonblocking ())
    {
      set_errno (EINPROGRESS);
      goto out;
    }

  waitret = cygwait (connect_wait_thr, cw_infinite, cw_cancel | cw_sig_eintr);
  if (waitret == WAIT_OBJECT_0)
    GetExitCodeThread (connect_wait_thr, &err);
  else
    {
      SetEvent (cwt_termination_evt);
      NtWaitForSingleObject (connect_wait_thr, FALSE, NULL);
      GetExitCodeThread (connect_wait_thr, &err);
      waitret = WAIT_SIGNALED;
    }
  thr = InterlockedExchangePointer (&connect_wait_thr, NULL);
  if (thr)
    NtClose (thr);
  param = InterlockedExchangePointer (&cwt_param, NULL);
  if (param)
    cfree (param);
  switch (waitret)
    {
    case WAIT_CANCELED:
      pthread::static_cancel_self ();
      /*NOTREACHED*/
    case WAIT_SIGNALED:
      set_errno (EINTR);
      break;
    default:
      so_error (err);
      if (err)
	set_errno (err);
      else
	ret = 0;
      break;
    }
out:
  evt = InterlockedExchangePointer (&cwt_termination_evt, NULL);
  if (evt)
    NtClose (evt);
  return ret;
}

int
fhandler_socket_unix::connect_pipe (PUNICODE_STRING pipe_name)
{
  NTSTATUS status;

  /* Try connecting first.  If it doesn't work, wait for the pipe
     to become available. */
  status = open_pipe (pipe_name, get_socket_type () != SOCK_DGRAM);
  if (STATUS_PIPE_NO_INSTANCE_AVAILABLE (status))
    return wait_pipe (pipe_name);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      so_error (get_errno ());
      return -1;
    }
  so_error (0);
  return 0;
}

int
fhandler_socket_unix::listen_pipe ()
{
  NTSTATUS status;
  IO_STATUS_BLOCK io;
  HANDLE evt = NULL;
  DWORD waitret = WAIT_OBJECT_0;

  io.Status = STATUS_PENDING;
  if (!is_nonblocking () && !(evt = create_event ()))
    return -1;
  status = NtFsControlFile (get_handle (), evt, NULL, NULL, &io,
			    FSCTL_PIPE_LISTEN, NULL, 0, NULL, 0);
  if (status == STATUS_PENDING)
    {
      waitret = cygwait (evt ?: get_handle (), cw_infinite,
			 cw_cancel | cw_sig_eintr);
      if (waitret == WAIT_OBJECT_0)
	status = io.Status;
    }
  if (evt)
    NtClose (evt);
  if (waitret == WAIT_CANCELED)
    pthread::static_cancel_self ();
  else if (waitret == WAIT_SIGNALED)
    set_errno (EINTR);
  else if (status == STATUS_PIPE_LISTENING)
    set_errno (EAGAIN);
  else if (status != STATUS_PIPE_CONNECTED)
    __seterrno_from_nt_status (status);
  return (status == STATUS_PIPE_CONNECTED) ? 0 : -1;
}

ULONG
fhandler_socket_unix::peek_pipe (PFILE_PIPE_PEEK_BUFFER pbuf, ULONG psize,
				 HANDLE evt)
{
  NTSTATUS status;
  IO_STATUS_BLOCK io;

  status = NtFsControlFile (get_handle (), evt, NULL, NULL, &io,
			    FSCTL_PIPE_PEEK, NULL, 0, pbuf, psize);
  if (status == STATUS_PENDING)
    {
      /* Very short-lived */
      status = NtWaitForSingleObject (evt ?: get_handle (), FALSE, NULL);
      if (NT_SUCCESS (status))
	status = io.Status;
    }
  return NT_SUCCESS (status) ? (io.Information
				- offsetof (FILE_PIPE_PEEK_BUFFER, Data))
			     : 0;
}

int
fhandler_socket_unix::disconnect_pipe (HANDLE ph)
{
  NTSTATUS status;
  IO_STATUS_BLOCK io;

  status = NtFsControlFile (ph, NULL, NULL, NULL, &io, FSCTL_PIPE_DISCONNECT,
			    NULL, 0, NULL, 0);
  /* Short-lived.  Don't use cygwait.  We don't want to be interrupted. */
  if (status == STATUS_PENDING
      && NtWaitForSingleObject (ph, FALSE, NULL) == WAIT_OBJECT_0)
    status = io.Status;
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return -1;
    }
  return 0;
}

void
fhandler_socket_unix::init_cred ()
{
  struct ucred *scred = shmem->sock_cred ();
  struct ucred *pcred = shmem->peer_cred ();
  scred->pid = pcred->pid = (pid_t) 0;
  scred->uid = pcred->uid = (uid_t) -1;
  scred->gid = pcred->gid = (gid_t) -1;
}

void
fhandler_socket_unix::set_cred ()
{
  struct ucred *scred = shmem->sock_cred ();
  scred->pid = myself->pid;
  scred->uid = myself->uid;
  scred->gid = myself->gid;
}

/* ========================== public methods ========================= */

void
fhandler_socket_unix::fixup_after_fork (HANDLE parent)
{
  fhandler_socket::fixup_after_fork (parent);
  if (backing_file_handle && backing_file_handle != INVALID_HANDLE_VALUE)
    fork_fixup (parent, backing_file_handle, "backing_file_handle");
  if (shmem_handle)
    {
      fork_fixup (parent, shmem_handle, "shmem_handle");
      reopen_shmem ();
    }
  connect_wait_thr = NULL;
  cwt_termination_evt = NULL;
  cwt_param = NULL;
}

void
fhandler_socket_unix::fixup_after_exec ()
{
  if (!close_on_exec ())
    fixup_after_fork (NULL);
}

void
fhandler_socket_unix::set_close_on_exec (bool val)
{
  fhandler_base::set_close_on_exec (val);
  if (backing_file_handle && backing_file_handle != INVALID_HANDLE_VALUE)
    set_no_inheritance (backing_file_handle, val);
  if (shmem_handle)
    set_no_inheritance (shmem_handle, val);
}

fhandler_socket_unix::fhandler_socket_unix ()
{
}

fhandler_socket_unix::~fhandler_socket_unix ()
{
}

int
fhandler_socket_unix::dup (fhandler_base *child, int flags)
{
  if (fhandler_socket::dup (child, flags))
    {
      __seterrno ();
      return -1;
    }
  fhandler_socket_unix *fhs = (fhandler_socket_unix *) child;
  if (reopen_shmem () < 0)
    {
      __seterrno ();
      fhs->close ();
      return -1;
    }
  if (backing_file_handle && backing_file_handle != INVALID_HANDLE_VALUE
      && !DuplicateHandle (GetCurrentProcess (), backing_file_handle,
			    GetCurrentProcess (), &fhs->backing_file_handle,
			    0, TRUE, DUPLICATE_SAME_ACCESS))
    {
      __seterrno ();
      fhs->close ();
      return -1;
    }
  if (!DuplicateHandle (GetCurrentProcess (), shmem_handle,
			GetCurrentProcess (), &fhs->shmem_handle,
			0, TRUE, DUPLICATE_SAME_ACCESS))
    {
      __seterrno ();
      fhs->close ();
      return -1;
    }
  fhs->sun_path (sun_path ());
  fhs->peer_sun_path (peer_sun_path ());
  fhs->connect_wait_thr = NULL;
  fhs->cwt_termination_evt = NULL;
  fhs->cwt_param = NULL;
  return 0;
}

/* Waiter thread method.  Here we wait for a pipe instance to become
   available and connect to it, if so.  This function is running
   asynchronously if called on a non-blocking pipe.  The important
   things to do:

   - Set the peer pipe handle if successful
   - Send own sun_path to peer if successful
   - Set connect_state
   - Set so_error for later call to select
*/
DWORD
fhandler_socket_unix::wait_pipe_thread (PUNICODE_STRING pipe_name)
{
  HANDLE npfsh;
  HANDLE evt;
  LONG error = 0;
  NTSTATUS status;
  IO_STATUS_BLOCK io;
  ULONG pwbuf_size;
  PFILE_PIPE_WAIT_FOR_BUFFER pwbuf;
  LONGLONG stamp;

  status = npfs_handle (npfsh);
  if (!NT_SUCCESS (status))
    {
      error = geterrno_from_nt_status (status);
      goto out;
    }
  if (!(evt = create_event ()))
    goto out;
  pwbuf_size = offsetof (FILE_PIPE_WAIT_FOR_BUFFER, Name) + pipe_name->Length;
  pwbuf = (PFILE_PIPE_WAIT_FOR_BUFFER) alloca (pwbuf_size);
  pwbuf->Timeout.QuadPart = AF_UNIX_CONNECT_TIMEOUT;
  pwbuf->NameLength = pipe_name->Length;
  pwbuf->TimeoutSpecified = TRUE;
  memcpy (pwbuf->Name, pipe_name->Buffer, pipe_name->Length);
  stamp = ntod.nsecs ();
  do
    {
      status = NtFsControlFile (npfsh, evt, NULL, NULL, &io, FSCTL_PIPE_WAIT,
				pwbuf, pwbuf_size, NULL, 0);
      if (status == STATUS_PENDING)
	{
	  HANDLE w[2] = { evt, cwt_termination_evt };
	  switch (WaitForMultipleObjects (2, w, FALSE, INFINITE))
	    {
	    case WAIT_OBJECT_0:
	      status = io.Status;
	      break;
	    case WAIT_OBJECT_0 + 1:
	    default:
	      status = STATUS_THREAD_IS_TERMINATING;
	      break;
	    }
	}
      switch (status)
	{
	  case STATUS_SUCCESS:
	    {
	      status = open_pipe (pipe_name, get_socket_type () != SOCK_DGRAM);
	      if (STATUS_PIPE_NO_INSTANCE_AVAILABLE (status))
		{
		  /* Another concurrent connect grabbed the pipe instance
		     under our nose.  Fix the timeout value and go waiting
		     again, unless the timeout has passed. */
		  pwbuf->Timeout.QuadPart -= (stamp - ntod.nsecs ()) / 100LL;
		  if (pwbuf->Timeout.QuadPart >= 0)
		    {
		      status = STATUS_IO_TIMEOUT;
		      error = ETIMEDOUT;
		    }
		}
	      else if (!NT_SUCCESS (status))
		error = geterrno_from_nt_status (status);
	    }
	    break;
	  case STATUS_OBJECT_NAME_NOT_FOUND:
	    error = EADDRNOTAVAIL;
	    break;
	  case STATUS_IO_TIMEOUT:
	    error = ETIMEDOUT;
	    break;
	  case STATUS_INSUFFICIENT_RESOURCES:
	    error = ENOBUFS;
	    break;
	  case STATUS_THREAD_IS_TERMINATING:
	    error = EINTR;
	    break;
	  case STATUS_INVALID_DEVICE_REQUEST:
	  default:
	    error = EIO;
	    break;
	}
    }
  while (STATUS_PIPE_NO_INSTANCE_AVAILABLE (status));
out:
  PVOID param = InterlockedExchangePointer (&cwt_param, NULL);
  if (param)
    cfree (param);
  conn_lock ();
  state_lock ();
  so_error (error);
  connect_state (error ? connect_failed : connected);
  state_unlock ();
  conn_unlock ();
  return error;
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
  if (create_shmem () < 0)
    return -1;
  rmem (262144);
  wmem (262144);
  set_addr_family (AF_UNIX);
  set_socket_type (type);
  if (flags & SOCK_NONBLOCK)
    set_nonblocking (true);
  if (flags & SOCK_CLOEXEC)
    set_close_on_exec (true);
  init_cred ();
  set_io_handle (NULL);
  set_unique_id ();
  set_ino (get_unique_id ());
  return 0;
}

int
fhandler_socket_unix::socketpair (int af, int type, int protocol, int flags,
				  fhandler_socket *fh_out)
{
  HANDLE pipe;
  sun_name_t sun;
  fhandler_socket_unix *fh = (fhandler_socket_unix *) fh_out;

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

  if (create_shmem () < 0)
    return -1;
  if (fh->create_shmem () < 0)
    goto fh_shmem_failed;
  /* socket() on both sockets */
  rmem (262144);
  fh->rmem (262144);
  wmem (262144);
  fh->wmem (262144);
  set_addr_family (AF_UNIX);
  fh->set_addr_family (AF_UNIX);
  set_socket_type (type);
  fh->set_socket_type (type);
  set_cred ();
  fh->set_cred ();
  set_unique_id ();
  set_ino (get_unique_id ());
  /* bind/listen 1st socket */
  gen_pipe_name ();
  pipe = create_pipe (true);
  if (!pipe)
    goto create_pipe_failed;
  set_io_handle (pipe);
  sun_path (&sun);
  fh->peer_sun_path (&sun);
  connect_state (listener);
  /* connect 2nd socket, even for DGRAM.  There's no difference as far
     as socketpairs are concerned. */
  if (fh->open_pipe (pc.get_nt_native_path (), false) < 0)
    goto fh_open_pipe_failed;
  fh->connect_state (connected);
  if (flags & SOCK_NONBLOCK)
    {
      set_nonblocking (true);
      fh->set_nonblocking (true);
    }
  if (flags & SOCK_CLOEXEC)
    {
      set_close_on_exec (true);
      fh->set_close_on_exec (true);
    }
  return 0;

fh_open_pipe_failed:
  NtClose (pipe);
create_pipe_failed:
  NtUnmapViewOfSection (GetCurrentProcess (), fh->shmem);
  NtClose (fh->shmem_handle);
fh_shmem_failed:
  NtUnmapViewOfSection (GetCurrentProcess (), shmem);
  NtClose (shmem_handle);
  return -1;
}

/* Bind creates the backing file, generates the pipe name and sets
   bind_state.  On DGRAM sockets it also creates the pipe.  On STREAM
   sockets either listen or connect will do that. */
int
fhandler_socket_unix::bind (const struct sockaddr *name, int namelen)
{
  sun_name_t sun (name, namelen);
  bool unnamed = (sun.un_len == sizeof sun.un.sun_family);
  HANDLE pipe = NULL;

  if (sun.un.sun_family != AF_UNIX)
    {
      set_errno (EINVAL);
      return -1;
    }
  bind_lock ();
  if (binding_state () == bind_pending)
    {
      set_errno (EALREADY);
      bind_unlock ();
      return -1;
    }
  if (binding_state () == bound)
    {
      set_errno (EINVAL);
      bind_unlock ();
      return -1;
    }
  binding_state (bind_pending);
  bind_unlock ();
  gen_pipe_name ();
  if (get_socket_type () == SOCK_DGRAM)
    {
      pipe = create_pipe (true);
      if (!pipe)
	{
	  binding_state (unbound);
	  return -1;
	}
      set_io_handle (pipe);
    }
  backing_file_handle = unnamed ? autobind (&sun) : create_file (&sun);
  if (!backing_file_handle)
    {
      set_io_handle (NULL);
      if (pipe)
	NtClose (pipe);
      binding_state (unbound);
      return -1;
    }
  state_lock ();
  sun_path (&sun);
  /* If we're already connected, send socket info to peer.  In this case
     send_sock_info calls state_unlock */
  if (connect_state () == connected)
    send_sock_info (true);
  else
    state_unlock ();
  binding_state (bound);
  return 0;
}

/* Create pipe on non-DGRAM sockets and set conn_state to listener. */
int
fhandler_socket_unix::listen (int backlog)
{
  if (get_socket_type () == SOCK_DGRAM)
    {
      set_errno (EOPNOTSUPP);
      return -1;
    }
  bind_lock ();
  while (binding_state () == bind_pending)
    yield ();
  if (binding_state () == unbound)
    {
      set_errno (EDESTADDRREQ);
      bind_unlock ();
      return -1;
    }
  bind_unlock ();
  conn_lock ();
  if (connect_state () != unconnected && connect_state () != connect_failed)
    {
      set_errno (connect_state () == listener ? EADDRINUSE : EINVAL);
      conn_unlock ();
      return -1;
    }
  HANDLE pipe = create_pipe (false);
  if (!pipe)
    {
      connect_state (unconnected);
      return -1;
    }
  set_io_handle (pipe);
  state_lock ();
  set_cred ();
  state_unlock ();
  connect_state (listener);
  conn_unlock ();
  return 0;
}

int
fhandler_socket_unix::accept4 (struct sockaddr *peer, int *len, int flags)
{
  if (get_socket_type () != SOCK_STREAM)
    {
      set_errno (EOPNOTSUPP);
      return -1;
    }
  if (connect_state () != listener
      || (peer && (!len || *len < (int) sizeof (sa_family_t))))
    {
      set_errno (EINVAL);
      return -1;
    }
  if (listen_pipe () == 0)
    {
      /* Our handle is now connected with a client.  This handle is used
         for the accepted socket.  Our handle has to be replaced with a
	 new instance handle for the next accept. */
      io_lock ();
      HANDLE accepted = get_handle ();
      HANDLE new_inst = create_pipe_instance ();
      int error = ENOBUFS;
      if (!new_inst)
	io_unlock ();
      else
	{
	  /* Set new io handle. */
	  set_io_handle (new_inst);
	  io_unlock ();
	  /* Prepare new file descriptor. */
	  cygheap_fdnew fd;

	  if (fd >= 0)
	    {
	      fhandler_socket_unix *sock = (fhandler_socket_unix *)
					   build_fh_dev (dev ());
	      if (sock)
		{
		  if (sock->create_shmem () < 0)
		    goto create_shmem_failed;

		  sock->set_addr_family (AF_UNIX);
		  sock->set_socket_type (get_socket_type ());
		  if (flags & SOCK_NONBLOCK)
		    sock->set_nonblocking (true);
		  if (flags & SOCK_CLOEXEC)
		    sock->set_close_on_exec (true);
		  sock->set_unique_id ();
		  sock->set_ino (sock->get_unique_id ());
		  sock->pc.set_nt_native_path (pc.get_nt_native_path ());
		  sock->connect_state (connected);
		  sock->binding_state (binding_state ());
		  sock->set_io_handle (accepted);

		  sock->sun_path (sun_path ());
		  sock->sock_cred (sock_cred ());
		  /* Send this socket info to connecting socket. */
		  sock->send_sock_info (false);
		  /* Fetch the packet sent by send_sock_info called by
		     connecting peer. */
		  error = sock->recv_peer_info ();
		  if (error == 0)
		    {
		      __try
			{
			  if (peer)
			    {
			      sun_name_t *sun = sock->peer_sun_path ();
			      if (sun)
				{
				  memcpy (peer, &sun->un,
					  MIN (*len, sun->un_len));
				  *len = sun->un_len;
				}
			      else if (len)
				*len = 0;
			    }
			  fd = sock;
			  if (fd <= 2)
			    set_std_handle (fd);
			  return fd;
			}
		      __except (NO_ERROR)
			{
			  error = EFAULT;
			}
		      __endtry
		    }
create_shmem_failed:
		  delete sock;
		}
	      fd.release ();
	    }
	}
      /* Ouch!  We can't handle the client if we couldn't
	 create a new instance to accept more connections.*/
      disconnect_pipe (accepted);
      set_errno (error);
    }
  return -1;
}

int
fhandler_socket_unix::connect (const struct sockaddr *name, int namelen)
{
  sun_name_t sun (name, namelen);
  int peer_type;
  WCHAR pipe_name_buf[CYGWIN_PIPE_SOCKET_NAME_LEN + 1];
  UNICODE_STRING pipe_name;

  /* Test and set connection state. */
  conn_lock ();
  if (connect_state () == connect_pending)
    {
      set_errno (EALREADY);
      conn_unlock ();
      return -1;
    }
  if (connect_state () == listener)
    {
      set_errno (EADDRINUSE);
      conn_unlock ();
      return -1;
    }
  if (connect_state () == connected && get_socket_type () != SOCK_DGRAM)
    {
      set_errno (EISCONN);
      conn_unlock ();
      return -1;
    }
  connect_state (connect_pending);
  conn_unlock ();
  /* Check validity of name */
  if (sun.un_len <= (int) sizeof (sa_family_t))
    {
      set_errno (EINVAL);
      connect_state (unconnected);
      return -1;
    }
  if (sun.un.sun_family != AF_UNIX)
    {
      set_errno (EAFNOSUPPORT);
      connect_state (unconnected);
      return -1;
    }
  if (sun.un_len == 3 && sun.un.sun_path[0] == '\0')
    {
      set_errno (EINVAL);
      connect_state (unconnected);
      return -1;
    }
  /* Check if peer address exists. */
  RtlInitEmptyUnicodeString (&pipe_name, pipe_name_buf, sizeof pipe_name_buf);
  if (open_file (&sun, peer_type, &pipe_name) < 0)
    {
      connect_state (unconnected);
      return -1;
    }
  if (peer_type != get_socket_type ())
    {
      set_errno (EINVAL);
      connect_state (unconnected);
      return -1;
    }
  peer_sun_path (&sun);
  if (get_socket_type () != SOCK_DGRAM)
    {
      if (connect_pipe (&pipe_name) < 0)
	{
	  if (get_errno () != EINPROGRESS)
	    {
	      peer_sun_path (NULL);
	      connect_state (connect_failed);
	    }
	  return -1;
	}
    }
  connect_state (connected);
  return 0;
}

int
fhandler_socket_unix::getsockname (struct sockaddr *name, int *namelen)
{
  sun_name_t *sun = sun_path ();

  memcpy (name, sun, MIN (*namelen, sun->un_len));
  *namelen = sun->un_len;
  return 0;
}

int
fhandler_socket_unix::getpeername (struct sockaddr *name, int *namelen)
{
  sun_name_t *sun = peer_sun_path ();
  memcpy (name, sun, MIN (*namelen, sun->un_len));
  *namelen = sun->un_len;
  return 0;
}

int
fhandler_socket_unix::shutdown (int how)
{
  NTSTATUS status = STATUS_SUCCESS;
  IO_STATUS_BLOCK io;

  if (how < SHUT_RD || how > SHUT_RDWR)
    {
      set_errno (EINVAL);
      return -1;
    }
  /* Convert SHUT_RD/SHUT_WR/SHUT_RDWR to _SHUT_RECV/_SHUT_SEND bits. */
  ++how;
  state_lock ();
  int old_shutdown_mask = saw_shutdown ();
  int new_shutdown_mask = old_shutdown_mask | how;
  if (new_shutdown_mask != old_shutdown_mask)
    saw_shutdown (new_shutdown_mask);
  state_unlock ();
  if (new_shutdown_mask != old_shutdown_mask)
    {
      /* Send shutdown info to peer.  Note that it's not necessarily fatal
	 if the info isn't sent here.  The info will be reproduced by any
	 followup package sent to the peer. */
      af_unix_pkt_hdr_t packet (true, (shut_state) new_shutdown_mask, 0, 0, 0);
      io_lock ();
      set_pipe_non_blocking (true);
      status = NtWriteFile (get_handle (), NULL, NULL, NULL, &io, &packet,
			    packet.pckt_len, NULL, NULL);
      set_pipe_non_blocking (is_nonblocking ());
      io_unlock ();
    }
  if (!NT_SUCCESS (status))
    {
      debug_printf ("Couldn't send shutdown info: NtWriteFile: %y", status);
      return -1;
    }
  return 0;
}

int
fhandler_socket_unix::close ()
{
  HANDLE evt = InterlockedExchangePointer (&cwt_termination_evt, NULL);
  HANDLE thr = InterlockedExchangePointer (&connect_wait_thr, NULL);
  if (thr)
    {
      if (evt)
	SetEvent (evt);
      NtWaitForSingleObject (thr, FALSE, NULL);
      NtClose (thr);
    }
  if (evt)
    NtClose (evt);
  PVOID param = InterlockedExchangePointer (&cwt_param, NULL);
  if (param)
    cfree (param);
  HANDLE hdl = InterlockedExchangePointer (&get_handle (), NULL);
  if (hdl)
    NtClose (hdl);
  if (backing_file_handle && backing_file_handle != INVALID_HANDLE_VALUE)
    NtClose (backing_file_handle);
  HANDLE shm = InterlockedExchangePointer (&shmem_handle, NULL);
  if (shm)
    NtClose (shm);
  param = InterlockedExchangePointer ((PVOID *) &shmem, NULL);
  if (param)
    NtUnmapViewOfSection (GetCurrentProcess (), param);
  return 0;
}

int
fhandler_socket_unix::getpeereid (pid_t *pid, uid_t *euid, gid_t *egid)
{
  int ret = -1;

  if (get_socket_type () != SOCK_STREAM)
    {
      set_errno (EINVAL);
      return -1;
    }
  if (connect_state () != connected)
    set_errno (ENOTCONN);
  else
    {
      __try
	{
	  state_lock ();
	  struct ucred *pcred = peer_cred ();
	  if (pid)
	    *pid = pcred->pid;
	  if (euid)
	    *euid = pcred->uid;
	  if (egid)
	    *egid = pcred->gid;
	  state_unlock ();
	  ret = 0;
	}
      __except (EFAULT) {}
      __endtry
    }
  return ret;
}

ssize_t
fhandler_socket_unix::recvmsg (struct msghdr *msg, int flags)
{
  set_errno (EAFNOSUPPORT);
  return -1;
}

ssize_t
fhandler_socket_unix::recvfrom (void *ptr, size_t len, int flags,
				struct sockaddr *from, int *fromlen)
{
  struct iovec iov;
  struct msghdr msg;
  ssize_t ret;

  iov.iov_base = ptr;
  iov.iov_len = len;
  msg.msg_name = from;
  msg.msg_namelen = from && fromlen ? *fromlen : 0;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = NULL;
  msg.msg_controllen = 0;
  msg.msg_flags = 0;
  ret = recvmsg (&msg, flags);
  if (ret >= 0 && from && fromlen)
    *fromlen = msg.msg_namelen;
  return ret;
}

void __reg3
fhandler_socket_unix::read (void *ptr, size_t& len)
{
  set_errno (EAFNOSUPPORT);
  len = 0;
  struct iovec iov;
  struct msghdr msg;

  iov.iov_base = ptr;
  iov.iov_len = len;
  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = NULL;
  msg.msg_controllen = 0;
  msg.msg_flags = 0;
  len = recvmsg (&msg, 0);
}

ssize_t __stdcall
fhandler_socket_unix::readv (const struct iovec *const iov, int iovcnt,
			     ssize_t tot)
{
  struct msghdr msg;

  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov = (struct iovec *) iov;
  msg.msg_iovlen = iovcnt;
  msg.msg_control = NULL;
  msg.msg_controllen = 0;
  msg.msg_flags = 0;
  return recvmsg (&msg, 0);
}

ssize_t
fhandler_socket_unix::sendmsg (const struct msghdr *msg, int flags)
{
  set_errno (EAFNOSUPPORT);
  return -1;
}

ssize_t
fhandler_socket_unix::sendto (const void *in_ptr, size_t len, int flags,
			       const struct sockaddr *to, int tolen)
{
  struct iovec iov;
  struct msghdr msg;

  iov.iov_base = (void *) in_ptr;
  iov.iov_len = len;
  msg.msg_name = (void *) to;
  msg.msg_namelen = to ? tolen : 0;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = NULL;
  msg.msg_controllen = 0;
  msg.msg_flags = 0;
  return sendmsg (&msg, flags);
}

ssize_t __stdcall
fhandler_socket_unix::write (const void *ptr, size_t len)
{
  struct iovec iov;
  struct msghdr msg;

  iov.iov_base = (void *) ptr;
  iov.iov_len = len;
  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = NULL;
  msg.msg_controllen = 0;
  msg.msg_flags = 0;
  return sendmsg (&msg, 0);
}

ssize_t __stdcall
fhandler_socket_unix::writev (const struct iovec *const iov, int iovcnt,
			      ssize_t tot)
{
  struct msghdr msg;

  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov = (struct iovec *) iov;
  msg.msg_iovlen = iovcnt;
  msg.msg_control = NULL;
  msg.msg_controllen = 0;
  msg.msg_flags = 0;
  return sendmsg (&msg, 0);
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
	  if (optlen < (socklen_t) sizeof (int))
	    {
	      set_errno (EINVAL);
	      return -1;
	    }

	  bool val;
	  val = !!*(int *) optval;
	  /* Using bind_lock here to make sure the autobind below is
	     covered.  This is the only place to set so_passcred anyway. */
	  bind_lock ();
	  if (val && binding_state () == unbound)
	    {
	      sun_name_t sun;

	      binding_state (bind_pending);
	      backing_file_handle = autobind (&sun);
	      if (!backing_file_handle)
		{
		  binding_state (unbound);
		  bind_unlock ();
		  return -1;
		}
	      sun_path (&sun);
	      binding_state (bound);
	    }
	  so_passcred (val);
	  bind_unlock ();
	  break;

	case SO_REUSEADDR:
	  if (optlen < (socklen_t) sizeof (int))
	    {
	      set_errno (EINVAL);
	      return -1;
	    }
	  reuseaddr (!!*(int *) optval);
	  break;

	case SO_RCVBUF:
	  if (optlen < (socklen_t) sizeof (int))
	    {
	      set_errno (EINVAL);
	      return -1;
	    }
	  rmem (*(int *) optval);
	  break;

	case SO_SNDBUF:
	  if (optlen < (socklen_t) sizeof (int))
	    {
	      set_errno (EINVAL);
	      return -1;
	    }
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
	    if (*optlen < (socklen_t) sizeof (int))
	      {
		set_errno (EINVAL);
		return -1;
	      }

	    int *e = (int *) optval;
	    LONG err;

	    err = so_error (0);
	    *e = err;
	    break;
	  }

	case SO_PASSCRED:
	  {
	    if (*optlen < (socklen_t) sizeof (int))
	      {
		set_errno (EINVAL);
		return -1;
	      }

	    int *e = (int *) optval;
	    *e = so_passcred ();
	    break;
	  }

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
	    unsigned int *reuse = (unsigned int *) optval;

	    if (*optlen < (socklen_t) sizeof *reuse)
	      {
		set_errno (EINVAL);
		return -1;
	      }
	    *reuse = reuseaddr ();
	    *optlen = (socklen_t) sizeof *reuse;
	    break;
	  }

	case SO_RCVBUF:
	case SO_SNDBUF:
	  if (*optlen < (socklen_t) sizeof (int))
	    {
	      set_errno (EINVAL);
	      return -1;
	    }
	  *(int *) optval = (optname == SO_RCVBUF) ? rmem () : wmem ();
	  break;

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
	    if (*optlen < (socklen_t) sizeof (int))
	      {
		set_errno (EINVAL);
		return -1;
	      }
	    unsigned int *type = (unsigned int *) optval;
	    *type = get_socket_type ();
	    *optlen = (socklen_t) sizeof *type;
	    break;
	  }

	/* AF_UNIX sockets simply ignore all other SOL_SOCKET options. */

	case SO_LINGER:
	  {
	    if (*optlen < (socklen_t) sizeof (struct linger))
	      {
		set_errno (EINVAL);
		return -1;
	      }
	    struct linger *linger = (struct linger *) optval;
	    memset (linger, 0, sizeof *linger);
	    *optlen = (socklen_t) sizeof *linger;
	    break;
	  }

	default:
	  {
	    if (*optlen < (socklen_t) sizeof (int))
	      {
		set_errno (EINVAL);
		return -1;
	      }
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
  int ret = -1;

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
      {
	const bool was_nonblocking = is_nonblocking ();
	set_nonblocking (*(int *) p);
	const bool now_nonblocking = is_nonblocking ();
	if (was_nonblocking != now_nonblocking)
	  set_pipe_non_blocking (now_nonblocking);
	ret = 0;
	break;
      }
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
  int ret = -1;

  switch (cmd)
    {
    case F_SETOWN:
      break;
    case F_GETOWN:
      break;
    case F_SETFL:
      {
	const bool was_nonblocking = is_nonblocking ();
	const int allowed_flags = O_APPEND | O_NONBLOCK_MASK;
	int new_flags = arg & allowed_flags;
	if ((new_flags & OLD_O_NDELAY) && (new_flags & O_NONBLOCK))
	  new_flags &= ~OLD_O_NDELAY;
	set_flags ((get_flags () & ~allowed_flags) | new_flags);
	const bool now_nonblocking = is_nonblocking ();
	if (was_nonblocking != now_nonblocking)
	  set_pipe_non_blocking (now_nonblocking);
	ret = 0;
	break;
      }
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

  if (sun_path ()
      && (sun_path ()->un_len <= (socklen_t) sizeof (sa_family_t)
	  || sun_path ()->un.sun_path[0] == '\0'))
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
  if (sun_path ()
      && (sun_path ()->un_len <= (socklen_t) sizeof (sa_family_t)
	  || sun_path ()->un.sun_path[0] == '\0'))
    return fhandler_socket::fstatvfs (sfs);
  fhandler_disk_file fh (pc);
  fh.get_device () = FH_FS;
  return fh.fstatvfs (sfs);
}

int
fhandler_socket_unix::fchmod (mode_t newmode)
{
  if (sun_path ()
      && (sun_path ()->un_len <= (socklen_t) sizeof (sa_family_t)
	  || sun_path ()->un.sun_path[0] == '\0'))
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
  if (sun_path ()
      && (sun_path ()->un_len <= (socklen_t) sizeof (sa_family_t)
	  || sun_path ()->un.sun_path[0] == '\0'))
    return fhandler_socket::fchown (uid, gid);
  fhandler_disk_file fh (pc);
  return fh.fchown (uid, gid);
}

int
fhandler_socket_unix::facl (int cmd, int nentries, aclent_t *aclbufp)
{
  if (sun_path ()
      && (sun_path ()->un_len <= (socklen_t) sizeof (sa_family_t)
	  || sun_path ()->un.sun_path[0] == '\0'))
    return fhandler_socket::facl (cmd, nentries, aclbufp);
  fhandler_disk_file fh (pc);
  return fh.facl (cmd, nentries, aclbufp);
}

int
fhandler_socket_unix::link (const char *newpath)
{
  if (sun_path ()
      && (sun_path ()->un_len <= (socklen_t) sizeof (sa_family_t)
	  || sun_path ()->un.sun_path[0] == '\0'))
    return fhandler_socket::link (newpath);
  fhandler_disk_file fh (pc);
  return fh.link (newpath);
}

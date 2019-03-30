/* fhandler.cc.  See console.cc for fhandler_console functions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <cygwin/acl.h>
#include <sys/param.h>
#include "cygerrno.h"
#include "perprocess.h"
#include "security.h"
#include "cygwin/version.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "pinfo.h"
#include <assert.h>
#include <winioctl.h>
#include "ntdll.h"
#include "cygtls.h"
#include "sigproc.h"
#include "shared_info.h"
#include <asm/socket.h>
#include "cygwait.h"

#define MAX_OVERLAPPED_WRITE_LEN (64 * 1024 * 1024)
#define MIN_OVERLAPPED_WRITE_LEN (1 * 1024 * 1024)

static const int CHUNK_SIZE = 1024; /* Used for crlf conversions */

struct __cygwin_perfile *perfile_table;

HANDLE NO_COPY fhandler_base_overlapped::asio_done;
LONG NO_COPY fhandler_base_overlapped::asio_close_counter;

void
fhandler_base::reset (const fhandler_base *from)
{
  pc << from->pc;
  rabuf = NULL;
  ralen = 0;
  raixget = 0;
  raixput = 0;
  rabuflen = 0;
  _refcnt = 0;
}

int
fhandler_base::puts_readahead (const char *s, size_t len)
{
  int success = 1;
  while ((len == (size_t) -1 ? *s : len--)
	 && (success = put_readahead (*s++) > 0))
    continue;
  return success;
}

int
fhandler_base::put_readahead (char value)
{
  char *newrabuf;
  if (raixput < rabuflen)
    /* Nothing to do */;
  else if ((newrabuf = (char *) realloc (rabuf, rabuflen += 32)))
    rabuf = newrabuf;
  else
    return 0;

  rabuf[raixput++] = value;
  ralen++;
  return 1;
}

int
fhandler_base::get_readahead ()
{
  int chret = -1;
  if (raixget < ralen)
    chret = ((unsigned char) rabuf[raixget++]) & 0xff;
  /* FIXME - not thread safe */
  if (raixget >= ralen)
    raixget = raixput = ralen = 0;
  return chret;
}

int
fhandler_base::peek_readahead (int queryput)
{
  int chret = -1;
  if (!queryput && raixget < ralen)
    chret = ((unsigned char) rabuf[raixget]) & 0xff;
  else if (queryput && raixput > 0)
    chret = ((unsigned char) rabuf[raixput - 1]) & 0xff;
  return chret;
}

void
fhandler_base::set_readahead_valid (int val, int ch)
{
  if (!val)
    ralen = raixget = raixput = 0;
  if (ch != -1)
    put_readahead (ch);
}

int
fhandler_base::get_readahead_into_buffer (char *buf, size_t buflen)
{
  int ch;
  int copied_chars = 0;

  while (buflen)
    if ((ch = get_readahead ()) < 0)
      break;
    else
      {
	buf[copied_chars++] = (unsigned char)(ch & 0xff);
	buflen--;
      }

  return copied_chars;
}

/* Record the file name. and name hash */
void
fhandler_base::set_name (path_conv &in_pc)
{
  pc << in_pc;
}

char *fhandler_base::get_proc_fd_name (char *buf)
{
  IO_STATUS_BLOCK io;
  FILE_STANDARD_INFORMATION fsi;

  /* If the file had been opened with O_TMPFILE, don't expose the filename. */
  if ((get_flags () & O_TMPFILE)
      || (get_device () == FH_FS
	  && NT_SUCCESS (NtQueryInformationFile (get_handle (), &io,
						 &fsi, sizeof fsi,
						 FileStandardInformation))
	  && fsi.DeletePending))
    {
      stpcpy (stpcpy (buf, get_name ()), " (deleted)");
      return buf;
    }
  if (get_name ())
    return strcpy (buf, get_name ());
  if (dev ().name ())
    return strcpy (buf, dev ().name ());
  return strcpy (buf, "");
}

/* Detect if we are sitting at EOF for conditions where Windows
   returns an error but UNIX doesn't.  */
int __stdcall
is_at_eof (HANDLE h)
{
  IO_STATUS_BLOCK io;
  FILE_POSITION_INFORMATION fpi;
  FILE_STANDARD_INFORMATION fsi;

  if (NT_SUCCESS (NtQueryInformationFile (h, &io, &fsi, sizeof fsi,
					  FileStandardInformation))
      && NT_SUCCESS (NtQueryInformationFile (h, &io, &fpi, sizeof fpi,
					     FilePositionInformation))
      && fsi.EndOfFile.QuadPart == fpi.CurrentByteOffset.QuadPart)
    return 1;
  return 0;
}

void
fhandler_base::set_flags (int flags, int supplied_bin)
{
  int bin;
  int fmode;
  debug_printf ("flags %y, supplied_bin %y", flags, supplied_bin);
  if ((bin = flags & (O_BINARY | O_TEXT)))
    debug_printf ("O_TEXT/O_BINARY set in flags %y", bin);
  else if (rbinset () && wbinset ())
    bin = rbinary () ? O_BINARY : O_TEXT;	// FIXME: Not quite right
  else if ((fmode = get_default_fmode (flags)) & O_BINARY)
    bin = O_BINARY;
  else if (fmode & O_TEXT)
    bin = O_TEXT;
  else if (supplied_bin)
    bin = supplied_bin;
  else
    bin = wbinary () || rbinary () ? O_BINARY : O_TEXT;

  openflags = flags | bin;
  if (openflags & O_NONBLOCK)
    was_nonblocking (true);

  bin &= O_BINARY;
  rbinary (bin ? true : false);
  wbinary (bin ? true : false);
  syscall_printf ("filemode set to %s", bin ? "binary" : "text");
}

/* Normal file i/o handlers.  */

/* Cover function to ReadFile to achieve (as much as possible) Posix style
   semantics and use of errno.  */
void __reg3
fhandler_base::raw_read (void *ptr, size_t& len)
{
  NTSTATUS status;
  IO_STATUS_BLOCK io;
  int try_noreserve = 1;

retry:
  status = NtReadFile (get_handle (), NULL, NULL, NULL, &io, ptr, len,
		       NULL, NULL);
  if (NT_SUCCESS (status))
    len = io.Information;
  else
    {
      /* Some errors are not really errors.  Detect such cases here.  */
      switch (status)
	{
	case STATUS_END_OF_FILE:
	case STATUS_PIPE_BROKEN:
	  /* This is really EOF.  */
	  len = 0;
	  break;
	case STATUS_MORE_ENTRIES:
	case STATUS_BUFFER_OVERFLOW:
	  /* `io.Information' is supposedly valid.  */
	  len = io.Information;
	  break;
	case STATUS_ACCESS_VIOLATION:
	  if (is_at_eof (get_handle ()))
	    {
	      len = 0;
	      break;
	    }
	  if (try_noreserve)
	    {
	      try_noreserve = 0;
	      switch (mmap_is_attached_or_noreserve (ptr, len))
		{
		case MMAP_NORESERVE_COMMITED:
		  goto retry;
		case MMAP_RAISE_SIGBUS:
		  raise(SIGBUS);
		case MMAP_NONE:
		  break;
		}
	    }
	  /*FALLTHRU*/
	case STATUS_INVALID_DEVICE_REQUEST:
	case STATUS_INVALID_PARAMETER:
	case STATUS_INVALID_HANDLE:
	  if (pc.isdir ())
	    {
	      set_errno (EISDIR);
	      len = (size_t) -1;
	      break;
	    }
	default:
	  __seterrno_from_nt_status (status);
	  len = (size_t) -1;
	  break;
	}
    }
}

/* Cover function to WriteFile to provide Posix interface and semantics
   (as much as possible).  */
ssize_t __reg3
fhandler_base::raw_write (const void *ptr, size_t len)
{
  NTSTATUS status;
  IO_STATUS_BLOCK io;
  static _RDATA LARGE_INTEGER off_current =
			  { QuadPart:FILE_USE_FILE_POINTER_POSITION };
  static _RDATA LARGE_INTEGER off_append =
			  { QuadPart:FILE_WRITE_TO_END_OF_FILE };

  status = NtWriteFile (get_output_handle (), NULL, NULL, NULL, &io,
			(PVOID) ptr, len,
			(get_flags () & O_APPEND) ? &off_append : &off_current,
			NULL);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      if (get_errno () == EPIPE)
	raise (SIGPIPE);
      return -1;
    }
  return io.Information;
}

int
fhandler_base::get_default_fmode (int flags)
{
  int fmode = __fmode;
  if (perfile_table)
    {
      size_t nlen = strlen (get_name ());
      unsigned accflags = (flags & O_ACCMODE);
      for (__cygwin_perfile *pf = perfile_table; pf->name; pf++)
	if (!*pf->name && (pf->flags & O_ACCMODE) == accflags)
	  {
	    fmode = pf->flags & ~O_ACCMODE;
	    break;
	  }
	else
	  {
	    size_t pflen = strlen (pf->name);
	    const char *stem = get_name () + nlen - pflen;
	    if (pflen > nlen || (stem != get_name () && !isdirsep (stem[-1])))
	      continue;
	    else if ((pf->flags & O_ACCMODE) == accflags
		     && pathmatch (stem, pf->name, !!pc.objcaseinsensitive ()))
	      {
		fmode = pf->flags & ~O_ACCMODE;
		break;
	      }
	  }
    }
  return fmode;
}

bool
fhandler_base::device_access_denied (int flags)
{
  int mode = 0;

  if (flags & O_RDWR)
    mode |= R_OK | W_OK;
  if (flags & (O_WRONLY | O_APPEND))
    mode |= W_OK;
  if (!mode)
    mode |= R_OK;

  return fhaccess (mode, true);
}

int
fhandler_base::fhaccess (int flags, bool effective)
{
  int res = -1;
  if (error ())
    {
      set_errno (error ());
      goto done;
    }

  if (!exists ())
    {
      set_errno (ENOENT);
      goto done;
    }

  if (!(flags & (R_OK | W_OK | X_OK)))
    return 0;

  if (is_fs_special ())
    /* short circuit */;
  else if (has_attribute (FILE_ATTRIBUTE_READONLY) && (flags & W_OK)
	   && !pc.isdir ())
    goto eaccess_done;
  else if (has_acls ())
    {
      res = check_file_access (pc, flags, effective);
      goto done;
    }
  else if (get_device () == FH_REGISTRY && open (O_RDONLY, 0) && get_handle ())
    {
      res = check_registry_access (get_handle (), flags, effective);
      close ();
      return res;
    }

  struct stat st;
  if (fstat (&st))
    goto done;

  if (flags & R_OK)
    {
      if (st.st_uid == (effective ? myself->uid : cygheap->user.real_uid))
	{
	  if (!(st.st_mode & S_IRUSR))
	    goto eaccess_done;
	}
      else if (st.st_gid == (effective ? myself->gid : cygheap->user.real_gid))
	{
	  if (!(st.st_mode & S_IRGRP))
	    goto eaccess_done;
	}
      else if (!(st.st_mode & S_IROTH))
	goto eaccess_done;
    }

  if (flags & W_OK)
    {
      if (st.st_uid == (effective ? myself->uid : cygheap->user.real_uid))
	{
	  if (!(st.st_mode & S_IWUSR))
	    goto eaccess_done;
	}
      else if (st.st_gid == (effective ? myself->gid : cygheap->user.real_gid))
	{
	  if (!(st.st_mode & S_IWGRP))
	    goto eaccess_done;
	}
      else if (!(st.st_mode & S_IWOTH))
	goto eaccess_done;
    }

  if (flags & X_OK)
    {
      if (st.st_uid == (effective ? myself->uid : cygheap->user.real_uid))
	{
	  if (!(st.st_mode & S_IXUSR))
	    goto eaccess_done;
	}
      else if (st.st_gid == (effective ? myself->gid : cygheap->user.real_gid))
	{
	  if (!(st.st_mode & S_IXGRP))
	    goto eaccess_done;
	}
      else if (!(st.st_mode & S_IXOTH))
	goto eaccess_done;
    }

  res = 0;
  goto done;

eaccess_done:
  set_errno (EACCES);
done:
  if (!res && (flags & W_OK) && get_device () == FH_FS
      && (pc.fs_flags () & FILE_READ_ONLY_VOLUME))
    {
      set_errno (EROFS);
      res = -1;
    }
  debug_printf ("returning %d", res);
  return res;
}

int
fhandler_base::open_with_arch (int flags, mode_t mode)
{
  int res;
  if (!(res = (archetype && archetype->io_handle)
	|| open (flags, mode & 07777)))
    {
      if (archetype)
	delete archetype;
    }
  else if (archetype)
    {
      if (!archetype->get_handle ())
	{
	  copyto (archetype);
	  archetype_usecount (1);
	  archetype->archetype = NULL;
	  usecount = 0;
	}
      else
	{
	  char *name;
	  /* Preserve any name (like /dev/tty) derived from build_fh_pc. */
	  if (!get_name ())
	    name = NULL;
	  else
	    {
	      name = (char *) alloca (strlen (get_name ()) + 1);
	      strcpy (name, get_name ());
	    }
	  fhandler_base *arch = archetype;
	  archetype->copyto (this);
	  if (name)
	    set_name (name);
	  archetype = arch;
	  archetype_usecount (1);
	  usecount = 0;
	}
      open_setup (flags);
    }

  close_on_exec (flags & O_CLOEXEC);
  /* A unique ID is necessary to recognize fhandler entries which are
     duplicated by dup(2) or fork(2).  This is used in BSD flock calls
     to identify the descriptor.  Skip nohandle fhandlers since advisory
     locking is unusable for those anyway. */
  if (!nohandle ())
    set_unique_id ();
  return res;
}

/* Open a fake handle to \\Device\\Null.  This is a helper function for
   fhandlers which just need some handle to keep track of BSD flock locks. */
int
fhandler_base::open_null (int flags)
{
  int res = 0;
  HANDLE fh;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  NTSTATUS status;

  InitializeObjectAttributes (&attr, &ro_u_null, OBJ_CASE_INSENSITIVE |
			      ((flags & O_CLOEXEC) ? 0 : OBJ_INHERIT),
			      NULL, NULL);
  status = NtCreateFile (&fh, GENERIC_READ | SYNCHRONIZE, &attr, &io, NULL, 0,
			 FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN,
			 FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      goto done;
    }
  set_handle (fh);
  set_flags (flags, pc.binmode ());
  res = 1;
  set_open_status ();
done:
  debug_printf ("%y = NtCreateFile (%p, ... %S ...)", status, fh, &ro_u_null);
  syscall_printf ("%d = fhandler_base::open_null (%y)", res, flags);
  return res;
}

/* Open system call handler function. */
int
fhandler_base::open (int flags, mode_t mode)
{
  int res = 0;
  HANDLE fh;
  ULONG file_attributes = 0;
  ULONG shared = (get_major () == DEV_TAPE_MAJOR ? 0 : FILE_SHARE_VALID_FLAGS);
  ULONG create_disposition;
  OBJECT_ATTRIBUTES attr;
  IO_STATUS_BLOCK io;
  NTSTATUS status;
  PFILE_FULL_EA_INFORMATION p = NULL;
  ULONG plen = 0;

  syscall_printf ("(%S, %y)%s", pc.get_nt_native_path (), flags,
				get_handle () ? " by handle" : "");

  if (flags & O_PATH)
    query_open (query_read_attributes);

  /* Allow to reopen from handle.  This is utilized by
     open ("/proc/PID/fd/DESCRIPTOR", ...); */
  if (get_handle ())
    {
      pc.init_reopen_attr (attr, get_handle ());
      if (!(flags & O_CLOEXEC))
	attr.Attributes |= OBJ_INHERIT;
      if (pc.has_buggy_reopen ())
	debug_printf ("Reopen by handle requested but FS doesn't support it");
    }
  else
    pc.get_object_attr (attr, *sec_none_cloexec (flags));

  options = FILE_OPEN_FOR_BACKUP_INTENT;
  switch (query_open ())
    {
    case query_read_control:
      access = READ_CONTROL;
      break;
    case query_read_attributes:
      access = READ_CONTROL | FILE_READ_ATTRIBUTES;
      break;
    case query_write_control:
      access = READ_CONTROL | WRITE_OWNER | WRITE_DAC | FILE_WRITE_ATTRIBUTES;
      break;
    case query_write_dac:
      access = READ_CONTROL | WRITE_DAC | FILE_WRITE_ATTRIBUTES;
      break;
    case query_write_attributes:
      access = READ_CONTROL | FILE_WRITE_ATTRIBUTES;
      break;
    default:
      switch (flags & O_ACCMODE)
	{
	case O_RDONLY:
	  access = GENERIC_READ;
	  break;
	case O_WRONLY:
	  access = GENERIC_WRITE | READ_CONTROL | FILE_READ_ATTRIBUTES;
	  break;
	default:
	  access = GENERIC_READ | GENERIC_WRITE;
	  break;
	}
      if (flags & O_SYNC)
	options |= FILE_WRITE_THROUGH;
      if (flags & O_DIRECT)
	options |= FILE_NO_INTERMEDIATE_BUFFERING;
      if (get_major () != DEV_SERIAL_MAJOR && get_major () != DEV_TAPE_MAJOR)
	{
	  options |= FILE_SYNCHRONOUS_IO_NONALERT;
	  access |= SYNCHRONIZE;
	}
      break;
    }

  /* Don't use the FILE_OVERWRITE{_IF} flags here.  See below for an
     explanation, why that's not such a good idea. */
  if (((flags & O_EXCL) && (flags & O_CREAT)) || (flags & O_TMPFILE))
    create_disposition = FILE_CREATE;
  else
    create_disposition = (flags & O_CREAT) ? FILE_OPEN_IF : FILE_OPEN;

  if (get_device () == FH_FS)
    {
      /* Add the reparse point flag to known repares points, otherwise we
	 open the target, not the reparse point.  This would break lstat. */
      if (pc.is_known_reparse_point ())
	options |= FILE_OPEN_REPARSE_POINT;

      /* O_TMPFILE files are created with delete-on-close semantics, as well
	 as with FILE_ATTRIBUTE_TEMPORARY.  The latter speeds up file access,
	 because the OS tries to keep the file in memory as much as possible.
	 In conjunction with FILE_DELETE_ON_CLOSE, ideally the OS never has
	 to write to the disk at all.
	 Note that O_TMPFILE_FILE_ATTRS also sets the DOS HIDDEN attribute
	 to help telling Cygwin O_TMPFILE files apart from other files
	 accidentally setting FILE_ATTRIBUTE_TEMPORARY. */
      if (flags & O_TMPFILE)
	{
	  access |= DELETE;
	  file_attributes |= O_TMPFILE_FILE_ATTRS;
	  options |= FILE_DELETE_ON_CLOSE;
	}

      if (pc.fs_is_nfs ())
	{
	  /* Make sure we can read EAs of files on an NFS share.  Also make
	     sure that we're going to act on the file itself, even if it's a
	     a symlink. */
	  access |= FILE_READ_EA;
	  if (query_open ())
	    {
	      if (query_open () >= query_write_control)
		access |=  FILE_WRITE_EA;
	      plen = sizeof nfs_aol_ffei;
	      p = (PFILE_FULL_EA_INFORMATION) &nfs_aol_ffei;
	    }
	}

      if (flags & (O_CREAT | O_TMPFILE))
	{
	  file_attributes |= FILE_ATTRIBUTE_NORMAL;

	  if (pc.fs_is_nfs ())
	    {
	      /* When creating a file on an NFS share, we have to set the
		 file mode by writing a NFS fattr3 structure with the
		 correct mode bits set. */
	      access |= FILE_WRITE_EA;
	      plen = sizeof (FILE_FULL_EA_INFORMATION) + sizeof (NFS_V3_ATTR)
		     + sizeof (fattr3);
	      p = (PFILE_FULL_EA_INFORMATION) alloca (plen);
	      p->NextEntryOffset = 0;
	      p->Flags = 0;
	      p->EaNameLength = sizeof (NFS_V3_ATTR) - 1;
	      p->EaValueLength = sizeof (fattr3);
	      strcpy (p->EaName, NFS_V3_ATTR);
	      fattr3 *nfs_attr = (fattr3 *) (p->EaName
					     + p->EaNameLength + 1);
	      memset (nfs_attr, 0, sizeof (fattr3));
	      nfs_attr->type = NF3REG;
	      nfs_attr->mode = (mode & 07777) & ~cygheap->umask;
	    }
	  else if (!has_acls ()
		   && !(mode & ~cygheap->umask & (S_IWUSR | S_IWGRP | S_IWOTH)))
	    /* If mode has no write bits set, and ACLs are not used, we set
	       the DOS R/O attribute. */
	    file_attributes |= FILE_ATTRIBUTE_READONLY;
	  /* The file attributes are needed for later use in, e.g. fchmod. */
	  pc.file_attributes (file_attributes);
	  /* Never set the WRITE_DAC flag here.  Calls to fstat may return
	     wrong st_ctime information after calls to fchmod, fchown, etc
	     because Windows only guarantees the update of metadata when
	     the handle is closed or flushed.  However, flushing the file
	     on every fstat to enforce POSIXy stat behaviour is excessivly
	     slow, compared to an extra open/close to change the file's
	     security descriptor. */
	}
    }

  status = NtCreateFile (&fh, access, &attr, &io, NULL, file_attributes, shared,
			 create_disposition, options, p, plen);
  /* Pre-W10, we can't reopen a file by handle with delete disposition
     set, so we have to lie our ass off. */
  if (get_handle () && status == STATUS_DELETE_PENDING)
    {
      BOOL ret = DuplicateHandle (GetCurrentProcess (), get_handle (),
				  GetCurrentProcess (), &fh,
				  access, !(flags & O_CLOEXEC), 0);
      if (!ret)
	ret = DuplicateHandle (GetCurrentProcess (), get_handle (),
			       GetCurrentProcess (), &fh,
			       0, !(flags & O_CLOEXEC),
			       DUPLICATE_SAME_ACCESS);
      if (!ret)
	debug_printf ("DuplicateHandle after STATUS_DELETE_PENDING, %E");
      else
	status = STATUS_SUCCESS;
    }
  if (!NT_SUCCESS (status))
    {
      /* Trying to create a directory should return EISDIR, not ENOENT. */
      PUNICODE_STRING upath = pc.get_nt_native_path ();
      if (status == STATUS_OBJECT_NAME_INVALID && (flags & O_CREAT)
	  && upath->Buffer[upath->Length / sizeof (WCHAR) - 1] == '\\')
	set_errno (EISDIR);
      else
	__seterrno_from_nt_status (status);
      if (!nohandle ())
	goto done;
   }

  /* Always create files using a NULL SD.  Create correct permission bits
     afterwards, maintaining the owner and group information just like chmod.

     This is done for two reasons.

     On Windows filesystems we need to create the file with default
     permissions to allow inheriting ACEs.  When providing an explicit DACL
     in calls to [Nt]CreateFile, the created file will not inherit default
     permissions from the parent object.  This breaks not only Windows
     inheritance, but also POSIX ACL inheritance.

     Another reason to do this are remote shares.  Files on a remote share
     are created as the user used for authentication.  In a domain that's
     usually the user you're logged in as.  Outside of a domain you're
     authenticating using a local user account on the sharing machine.
     If the SIDs of the client machine are used, that's entirely
     unexpected behaviour.  Doing it like we do here creates the expected SD
     in a domain as well as on standalone servers.
     This is the result of a discussion on the samba-technical list, starting at
     http://lists.samba.org/archive/samba-technical/2008-July/060247.html */
  if (io.Information == FILE_CREATED && has_acls ())
    set_created_file_access (fh, pc, mode);

  /* If you O_TRUNC a file on Linux, the data is truncated, but the EAs are
     preserved.  If you open a file on Windows with FILE_OVERWRITE{_IF} or
     FILE_SUPERSEDE, all streams are truncated, including the EAs.  So we don't
     use the FILE_OVERWRITE{_IF} flags, but instead just open the file and set
     the size of the data stream explicitely to 0.  Apart from being more Linux
     compatible, this implementation has the pleasant side-effect to be more
     than 5% faster than using FILE_OVERWRITE{_IF} (tested on W7 32 bit). */
  if ((flags & O_TRUNC)
      && (flags & O_ACCMODE) != O_RDONLY
      && io.Information != FILE_CREATED
      && get_device () == FH_FS)
    {
      FILE_END_OF_FILE_INFORMATION feofi = { EndOfFile:{ QuadPart:0 } };
      status = NtSetInformationFile (fh, &io, &feofi, sizeof feofi,
				     FileEndOfFileInformation);
      /* In theory, truncating the file should never fail, since the opened
	 handle has FILE_WRITE_DATA permissions, which is all you need to
	 be allowed to truncate a file.  Better safe than sorry. */
      if (!NT_SUCCESS (status))
	{
	  __seterrno_from_nt_status (status);
	  NtClose (fh);
	  goto done;
	}
    }

  set_handle (fh);
  set_flags (flags, pc.binmode ());

  res = 1;
  set_open_status ();
done:
  debug_printf ("%y = NtCreateFile "
		"(%p, %y, %S, io, NULL, %y, %y, %y, %y, NULL, 0)",
		status, fh, access, pc.get_nt_native_path (), file_attributes,
		shared, create_disposition, options);

  syscall_printf ("%d = fhandler_base::open(%S, %y)",
		  res, pc.get_nt_native_path (), flags);
  return res;
}

fhandler_base *
fhandler_base::fd_reopen (int, mode_t)
{
  /* This is implemented in fhandler_process only. */
  return NULL;
}

void
fhandler_base::open_setup (int)
{
}

/* states:
   open buffer in binary mode?  Just do the read.

   open buffer in text mode?  Scan buffer for control zs and handle
   the first one found.  Then scan buffer, converting every \r\n into
   an \n.  If last char is an \r, look ahead one more char, if \n then
   modify \r, if not, remember char.
*/
void __reg3
fhandler_base::read (void *in_ptr, size_t& len)
{
  char *ptr = (char *) in_ptr;
  ssize_t copied_chars = get_readahead_into_buffer (ptr, len);

  if (copied_chars || !len)
    {
      len = (size_t) copied_chars;
      goto out;
    }

  raw_read (ptr, len);

  if (rbinary () || (ssize_t) len <= 0)
    goto out;

  /* Scan buffer and turn \r\n into \n */
  char *src, *dst, *end;
  src = (char *) ptr;
  dst = (char *) ptr;
  end = src + len - 1;

  /* Read up to the last but one char - the last char needs special handling */
  while (src < end)
    {
      if (*src == '\r' && src[1] == '\n')
	src++;
      *dst++ = *src++;
    }

  /* If not beyond end and last char is a '\r' then read one more
     to see if we should translate this one too */
  if (src > end)
    /* nothing */;
  else if (*src != '\r')
    *dst++ = *src;
  else
    {
      char c1;
      size_t c1len = 1;
      raw_read (&c1, c1len);
      if (c1len <= 0)
	/* nothing */;
      else if (c1 == '\n')
	*dst++ = '\n';
      else
	{
	  set_readahead_valid (1, c1);
	  *dst++ = *src;
	}
    }

  len = dst - (char *) ptr;

out:
  debug_printf ("returning %d, %s mode", len, rbinary () ? "binary" : "text");
}

ssize_t __stdcall
fhandler_base::write (const void *ptr, size_t len)
{
  ssize_t res;

  if (did_lseek ())
    {
      IO_STATUS_BLOCK io;
      FILE_POSITION_INFORMATION fpi;
      FILE_STANDARD_INFORMATION fsi;

      did_lseek (false); /* don't do it again */

      if (!(get_flags () & O_APPEND)
	  && !has_attribute (FILE_ATTRIBUTE_SPARSE_FILE)
	  && NT_SUCCESS (NtQueryInformationFile (get_output_handle (),
						 &io, &fsi, sizeof fsi,
						 FileStandardInformation))
	  && NT_SUCCESS (NtQueryInformationFile (get_output_handle (),
						 &io, &fpi, sizeof fpi,
						 FilePositionInformation))
	  && fpi.CurrentByteOffset.QuadPart
	     >= fsi.EndOfFile.QuadPart + (128 * 1024))
	{
	  /* If the file system supports sparse files and the application
	     is writing after a long seek beyond EOF, convert the file to
	     a sparse file. */
	  NTSTATUS status;
	  status = NtFsControlFile (get_output_handle (), NULL, NULL, NULL,
				    &io, FSCTL_SET_SPARSE, NULL, 0, NULL, 0);
	  if (NT_SUCCESS (status))
	    pc.file_attributes (pc.file_attributes ()
				| FILE_ATTRIBUTE_SPARSE_FILE);
	  debug_printf ("%y = NtFsControlFile(%S, FSCTL_SET_SPARSE)",
			status, pc.get_nt_native_path ());
	}
    }

  if (wbinary ())
    res = raw_write (ptr, len);
  else
    {
      debug_printf ("text write");
      /* This is the Microsoft/DJGPP way.  Still not ideal, but it's
	 compatible.
	 Modified slightly by CGF 2000-10-07 */

      int left_in_data = len;
      char *data = (char *)ptr;
      res = 0;

      while (left_in_data > 0)
	{
	  char buf[CHUNK_SIZE + 1], *buf_ptr = buf;
	  int left_in_buf = CHUNK_SIZE;

	  while (left_in_buf > 0 && left_in_data > 0)
	    {
	      char ch = *data++;
	      if (ch == '\n')
		{
		  *buf_ptr++ = '\r';
		  left_in_buf--;
		}
	      *buf_ptr++ = ch;
	      left_in_buf--;
	      left_in_data--;
	      if (left_in_data > 0 && ch == '\r' && *data == '\n')
		{
		  *buf_ptr++ = *data++;
		  left_in_buf--;
		  left_in_data--;
		}
	    }

	  /* We've got a buffer-full, or we're out of data.  Write it out */
	  int nbytes;
	  int want = buf_ptr - buf;
	  if ((nbytes = raw_write (buf, want)) == want)
	    {
	      /* Keep track of how much written not counting additional \r's */
	      res = data - (char *)ptr;
	      continue;
	    }

	  if (nbytes == -1)
	    res = -1;		/* Error */
	  else
	    res += nbytes;	/* Partial write.  Return total bytes written. */
	  break;		/* All done */
	}
    }

  return res;
}

ssize_t __stdcall
fhandler_base::readv (const struct iovec *const iov, const int iovcnt,
		      ssize_t tot)
{
  assert (iov);
  assert (iovcnt >= 1);

  size_t len = tot;
  if (iovcnt == 1)
    {
      len = iov->iov_len;
      read (iov->iov_base, len);
      return len;
    }

  if (tot == -1)		// i.e. if not pre-calculated by the caller.
    {
      len = 0;
      const struct iovec *iovptr = iov + iovcnt;
      do
	{
	  iovptr -= 1;
	  len += iovptr->iov_len;
	}
      while (iovptr != iov);
    }

  if (!len)
    return 0;

  char *buf = (char *) malloc (len);

  if (!buf)
    {
      set_errno (ENOMEM);
      return -1;
    }

  read (buf, len);
  ssize_t nbytes = (ssize_t) len;

  const struct iovec *iovptr = iov;

  char *p = buf;
  while (nbytes > 0)
    {
      const int frag = MIN (nbytes, (ssize_t) iovptr->iov_len);
      memcpy (iovptr->iov_base, p, frag);
      p += frag;
      iovptr += 1;
      nbytes -= frag;
    }

  free (buf);
  return len;
}

ssize_t __stdcall
fhandler_base::writev (const struct iovec *const iov, const int iovcnt,
		       ssize_t tot)
{
  assert (iov);
  assert (iovcnt >= 1);

  if (iovcnt == 1)
    return write (iov->iov_base, iov->iov_len);

  if (tot == -1)		// i.e. if not pre-calculated by the caller.
    {
      tot = 0;
      const struct iovec *iovptr = iov + iovcnt;
      do
	{
	  iovptr -= 1;
	  tot += iovptr->iov_len;
	}
      while (iovptr != iov);
    }

  assert (tot >= 0);

  if (tot == 0)
    return 0;

  char *const buf = (char *) malloc (tot);

  if (!buf)
    {
      set_errno (ENOMEM);
      return -1;
    }

  char *bufptr = buf;
  const struct iovec *iovptr = iov;
  int nbytes = tot;

  while (nbytes != 0)
    {
      const int frag = MIN (nbytes, (ssize_t) iovptr->iov_len);
      memcpy (bufptr, iovptr->iov_base, frag);
      bufptr += frag;
      iovptr += 1;
      nbytes -= frag;
    }
  ssize_t ret = write (buf, tot);
  free (buf);
  return ret;
}

off_t
fhandler_base::lseek (off_t offset, int whence)
{
  NTSTATUS status;
  IO_STATUS_BLOCK io;
  FILE_POSITION_INFORMATION fpi;
  FILE_STANDARD_INFORMATION fsi;

  /* Seeks on text files is tough, we rewind and read till we get to the
     right place.  */

  if (whence != SEEK_CUR || offset != 0)
    {
      if (whence == SEEK_CUR)
	offset -= ralen - raixget;
      set_readahead_valid (0);
    }

  switch (whence)
    {
    case SEEK_SET:
      fpi.CurrentByteOffset.QuadPart = offset;
      break;
    case SEEK_CUR:
      status = NtQueryInformationFile (get_handle (), &io, &fpi, sizeof fpi,
				       FilePositionInformation);
      if (!NT_SUCCESS (status))
	{
	  __seterrno_from_nt_status (status);
	  return -1;
	}
      fpi.CurrentByteOffset.QuadPart += offset;
      break;
    default: /* SEEK_END */
      status = NtQueryInformationFile (get_handle (), &io, &fsi, sizeof fsi,
				       FileStandardInformation);
      if (!NT_SUCCESS (status))
	{
	  __seterrno_from_nt_status (status);
	  return -1;
	}
      fpi.CurrentByteOffset.QuadPart = fsi.EndOfFile.QuadPart + offset;
      break;
    }

  debug_printf ("setting file pointer to %U", fpi.CurrentByteOffset.QuadPart);
  status = NtSetInformationFile (get_handle (), &io, &fpi, sizeof fpi,
				 FilePositionInformation);
  if (!NT_SUCCESS (status))
    {
      __seterrno_from_nt_status (status);
      return -1;
    }
  off_t res = fpi.CurrentByteOffset.QuadPart;

  /* When next we write(), we will check to see if *this* seek went beyond
     the end of the file and if so, potentially sparsify the file. */
  if (pc.support_sparse ())
    did_lseek (true);

  /* If this was a SEEK_CUR with offset 0, we still might have
     readahead that we have to take into account when calculating
     the actual position for the application.  */
  if (whence == SEEK_CUR)
    res -= ralen - raixget;

  return res;
}

ssize_t __reg3
fhandler_base::pread (void *, size_t, off_t, void *)
{
  set_errno (ESPIPE);
  return -1;
}

ssize_t __reg3
fhandler_base::pwrite (void *, size_t, off_t, void *)
{
  set_errno (ESPIPE);
  return -1;
}

int
fhandler_base::close_with_arch ()
{
  int res;
  fhandler_base *fh;
  if (usecount)
    {
      /* This was the archetype itself. */
      if (--usecount)
	{
	  debug_printf ("not closing passed in archetype %p, usecount %d", archetype, usecount);
	  return 0;
	}
      debug_printf ("closing passed in archetype %p, usecount %d", archetype, usecount);
      /* Set archetype temporarily so that it will eventually be deleted. */
      archetype = fh = this;
    }
  else if (!archetype)
    fh = this;
  else if (archetype_usecount (-1) == 0)
    {
      debug_printf ("closing archetype");
      fh = archetype;
    }
  else
    {
      debug_printf ("not closing archetype");
      return 0;
    }

  cleanup ();
  res = fh->close ();
  if (archetype)
    {
      cygheap->fdtab.delete_archetype (archetype);
      archetype = NULL;
    }
  return res;
}

void
fhandler_base::cleanup ()
{
  /* Delete all POSIX locks on the file.  Delete all flock locks on the
     file if this is the last reference to this file. */
  if (unique_id)
    del_my_locks (on_close);
}

int
fhandler_base::close ()
{
  int res = -1;

  syscall_printf ("closing '%s' handle %p", get_name (), get_handle ());
  if (nohandle () || CloseHandle (get_handle ()))
    res = 0;
  else
    {
      paranoid_printf ("CloseHandle failed, %E");
      __seterrno ();
    }
  return res;
}

DWORD WINAPI
flush_async_io (void *arg)
{
  fhandler_base_overlapped *fh = (fhandler_base_overlapped *) arg;
  debug_only_printf ("waiting for write I/O for %s", fh->get_name ());
  DWORD nbytes;
  bool res = GetOverlappedResult (fh->get_output_handle (),
				  fh->get_overlapped (), &nbytes, true);
  debug_printf ("finished waiting for I/O from %s, res %d", fh->get_name (),
		res);
  fh->close ();
  delete fh;

  InterlockedDecrement (&fhandler_base_overlapped::asio_close_counter);
  SetEvent (fhandler_base_overlapped::asio_done);

  _my_tls._ctinfo->auto_release ();
  return 0;
}

void
fhandler_base_overlapped::flush_all_async_io ()
{
  while (asio_close_counter > 0)
    if (WaitForSingleObject (asio_done, INFINITE) != WAIT_OBJECT_0)
      {
	system_printf ("WaitForSingleObject failed, possible data loss in pipe, %E");
	break;
      }
  asio_close_counter = 0;
  if (asio_done)
    CloseHandle (asio_done);
}

/* Start a thread to handle closing of overlapped asynchronous I/O since
   Windows amazingly does not seem to always flush I/O on close.  */
void
fhandler_base_overlapped::check_later ()
{
  set_close_on_exec (true);
  char buf[MAX_PATH];
  if (!asio_done
      && !(asio_done = CreateEvent (&sec_none_nih, false, false,
				    shared_name (buf, "asio",
						 GetCurrentProcessId ()))))
    api_fatal ("CreateEvent failed, %E");

  InterlockedIncrement (&asio_close_counter);
  if (!new cygthread(flush_async_io, this, "flasio"))
    api_fatal ("couldn't create a thread to track async I/O, %E");
  debug_printf ("started thread to handle asynchronous closing for %s", get_name ());
}

int
fhandler_base_overlapped::close ()
{
  int res;
  int writer = (get_access () & GENERIC_WRITE);
  /* Need to treat non-blocking I/O specially because Windows appears to
     be brain-dead.  We're checking here if the descriptor was ever set
     to nonblocking, rather than checking if it's nonblocking at close time.
     The reason is that applications may switch back to blocking (for the
     sake of some other application accessing this descriptor) without
     performaing any further I/O.  These applications would suffer data
     loss, which this workaround is trying to fix. */
  if (writer && was_nonblocking () && has_ongoing_io ())
    {
      clone (HEAP_3_FHANDLER)->check_later ();
      res = 0;
    }
  else
    {
     /* Cancelling seems to be necessary for cases where a reader is
	 still executing when a signal handler performs a close.  */
      if (!writer)
	CancelIo (get_handle ());
      destroy_overlapped ();
      res = fhandler_base::close ();
    }
  return res;
}

int
fhandler_base::ioctl (unsigned int cmd, void *buf)
{
  int res;

  switch (cmd)
    {
    case FIONBIO:
      set_nonblocking (*(int *) buf);
      res = 0;
      break;
    case FIONREAD:
    case TIOCSCTTY:
      set_errno (ENOTTY);
      res = -1;
      break;
    default:
      set_errno (EINVAL);
      res = -1;
      break;
    }

  syscall_printf ("%d = ioctl(%x, %p)", res, cmd, buf);
  return res;
}

int __reg2
fhandler_base::fstat (struct stat *buf)
{
  if (is_fs_special ())
    return fstat_fs (buf);

  switch (get_device ())
    {
    case FH_PIPE:
      buf->st_mode = S_IFIFO | S_IRUSR | S_IWUSR;
      break;
    case FH_PIPEW:
      buf->st_mode = S_IFIFO | S_IWUSR;
      break;
    case FH_PIPER:
      buf->st_mode = S_IFIFO | S_IRUSR;
      break;
    default:
      buf->st_mode = S_IFCHR | STD_RBITS | STD_WBITS | S_IWGRP | S_IWOTH;
      break;
    }

  buf->st_uid = geteuid32 ();
  buf->st_gid = getegid32 ();
  buf->st_nlink = 1;
  buf->st_blksize = PREFERRED_IO_BLKSIZE;

  buf->st_ctim.tv_sec = 1164931200L;	/* Arbitrary value: 2006-12-01 */
  buf->st_ctim.tv_nsec = 0L;
  buf->st_birthtim = buf->st_ctim;
  buf->st_mtim.tv_sec = time (NULL);	/* Arbitrary value: current time,
					   like Linux */
  buf->st_mtim.tv_nsec = 0L;
  buf->st_atim = buf->st_mtim;

  return 0;
}

int __reg2
fhandler_base::fstatvfs (struct statvfs *sfs)
{
  /* If we hit this base implementation, it's some device in /dev.
     Just call statvfs on /dev for simplicity. */
  path_conv pc ("/dev", PC_KEEP_HANDLE);
  fhandler_disk_file fh (pc);
  return fh.fstatvfs (sfs);
}

int
fhandler_base::init (HANDLE f, DWORD a, mode_t bin)
{
  set_handle (f);
  access = a;
  a &= GENERIC_READ | GENERIC_WRITE;
  int flags = 0;
  if (a == GENERIC_READ)
    flags = O_RDONLY;
  else if (a == GENERIC_WRITE)
    flags = O_WRONLY;
  else if (a == (GENERIC_READ | GENERIC_WRITE))
    flags = O_RDWR;
  set_flags (flags | bin);
  set_open_status ();
  debug_printf ("created new fhandler_base for handle %p, bin %d", f, rbinary ());
  return 1;
}

int
fhandler_base::dup (fhandler_base *child, int)
{
  debug_printf ("in fhandler_base dup");

  HANDLE nh;
  if (!nohandle () && !archetype)
    {
      if (!DuplicateHandle (GetCurrentProcess (), get_handle (),
			    GetCurrentProcess (), &nh,
			    0, TRUE, DUPLICATE_SAME_ACCESS))
	{
	  debug_printf ("dup(%s) failed, handle %p, %E",
			get_name (), get_handle ());
	  __seterrno ();
	  return -1;
	}

      VerifyHandle (nh);
      child->set_handle (nh);
    }
  return 0;
}

int
fhandler_base_overlapped::dup (fhandler_base *child, int flags)
{
  int res = fhandler_base::dup (child, flags) ||
	    ((fhandler_base_overlapped *) child)->setup_overlapped ();
  return res;
}

int fhandler_base::fcntl (int cmd, intptr_t arg)
{
  int res;

  switch (cmd)
    {
    case F_GETFD:
      res = close_on_exec () ? FD_CLOEXEC : 0;
      break;
    case F_SETFD:
      set_close_on_exec ((arg & FD_CLOEXEC) ? 1 : 0);
      res = 0;
      break;
    case F_GETFL:
      res = get_flags ();
      debug_printf ("GETFL: %y", res);
      break;
    case F_SETFL:
      {
	/* Only O_APPEND, O_ASYNC and O_NONBLOCK/O_NDELAY are allowed.
	   Each other flag will be ignored.
	   Since O_ASYNC isn't defined in fcntl.h it's currently
	   ignored as well.  */
	const int allowed_flags = O_APPEND | O_NONBLOCK_MASK;
	int new_flags = arg & allowed_flags;
	/* Carefully test for the O_NONBLOCK or deprecated OLD_O_NDELAY flag.
	   Set only the flag that has been passed in.  If both are set, just
	   record O_NONBLOCK.   */
	if ((new_flags & OLD_O_NDELAY) && (new_flags & O_NONBLOCK))
	  new_flags &= ~OLD_O_NDELAY;
	set_flags ((get_flags () & ~allowed_flags) | new_flags);
      }
      res = 0;
      break;
    case F_GETLK:
    case F_SETLK:
    case F_SETLKW:
	{
	  struct flock *fl = (struct flock *) arg;
	  fl->l_type &= F_RDLCK | F_WRLCK | F_UNLCK;
	  res = mandatory_locking () ? mand_lock (cmd, fl) : lock (cmd, fl);
	}
      break;
    default:
      set_errno (EINVAL);
      res = -1;
      break;
    }
  return res;
}

/* Base terminal handlers.  These just return errors.  */

int
fhandler_base::tcflush (int)
{
  set_errno (ENOTTY);
  return -1;
}

int
fhandler_base::tcsendbreak (int)
{
  set_errno (ENOTTY);
  return -1;
}

int
fhandler_base::tcdrain ()
{
  set_errno (ENOTTY);
  return -1;
}

int
fhandler_base::tcflow (int)
{
  set_errno (ENOTTY);
  return -1;
}

int
fhandler_base::tcsetattr (int, const struct termios *)
{
  set_errno (ENOTTY);
  return -1;
}

int
fhandler_base::tcgetattr (struct termios *)
{
  set_errno (ENOTTY);
  return -1;
}

int
fhandler_base::tcsetpgrp (const pid_t)
{
  set_errno (ENOTTY);
  return -1;
}

int
fhandler_base::tcgetpgrp ()
{
  set_errno (ENOTTY);
  return -1;
}

pid_t
fhandler_base::tcgetsid ()
{
  set_errno (ENOTTY);
  return -1;
}

int
fhandler_base::ptsname_r (char *, size_t)
{
  set_errno (ENOTTY);
  return ENOTTY;
}

/* Normal I/O constructor */
fhandler_base::fhandler_base () :
  status (),
  open_status (),
  access (0),
  io_handle (NULL),
  ino (0),
  _refcnt (0),
  openflags (0),
  rabuf (NULL),
  ralen (0),
  raixget (0),
  raixput (0),
  rabuflen (0),
  unique_id (0),
  archetype (NULL),
  usecount (0)
{
  isclosed (false);
}

/* Normal I/O destructor */
fhandler_base::~fhandler_base ()
{
  if (rabuf)
    free (rabuf);
}

/**********************************************************************/
/* /dev/null */

fhandler_dev_null::fhandler_dev_null () :
	fhandler_base ()
{
}

ssize_t __stdcall
fhandler_dev_null::write (const void *ptr, size_t len)
{
  /* Shortcut.  This also fixes a problem with the NUL device on 64 bit:
     If you write > 4 GB in a single attempt, the bytes written returned
     from by is numBytes & 0xffffffff. */
  return len;
}

void
fhandler_base::set_no_inheritance (HANDLE &h, bool not_inheriting)
{
  if (!SetHandleInformation (h, HANDLE_FLAG_INHERIT,
			     not_inheriting ? 0 : HANDLE_FLAG_INHERIT))
    debug_printf ("SetHandleInformation failed, %E");
#ifdef DEBUGGING_AND_FDS_PROTECTED
  if (h)
    setclexec (oh, h, not_inheriting);
#endif
}

bool
fhandler_base::fork_fixup (HANDLE parent, HANDLE &h, const char *name)
{
  HANDLE oh = h;
  bool res = false;
  if (!close_on_exec ())
    debug_printf ("handle %p already opened", h);
  else if (!DuplicateHandle (parent, h, GetCurrentProcess (), &h,
			     0, !close_on_exec (), DUPLICATE_SAME_ACCESS))
    system_printf ("%s - %E, handle %s<%p>", get_name (), name, h);
  else
    {
      if (oh != h)
	VerifyHandle (h);
      res = true;
    }
  return res;
}

void
fhandler_base::set_close_on_exec (bool val)
{
  if (!nohandle ())
    set_no_inheritance (io_handle, val);
  close_on_exec (val);
  debug_printf ("set close_on_exec for %s to %d", get_name (), val);
}

void
fhandler_base::fixup_after_fork (HANDLE parent)
{
  debug_printf ("inheriting '%s' from parent", get_name ());
  if (!nohandle ())
    fork_fixup (parent, io_handle, "io_handle");
  /* POSIX locks are not inherited across fork. */
  if (unique_id)
    del_my_locks (after_fork);
}

void
fhandler_base_overlapped::fixup_after_fork (HANDLE parent)
{
  setup_overlapped ();
  fhandler_base::fixup_after_fork (parent);
}

void
fhandler_base::fixup_after_exec ()
{
  debug_printf ("here for '%s'", get_name ());
  if (unique_id && close_on_exec ())
    del_my_locks (after_exec);
  mandatory_locking (false);
}
void
fhandler_base_overlapped::fixup_after_exec ()
{
  setup_overlapped ();
  fhandler_base::fixup_after_exec ();
}

bool
fhandler_base::is_nonblocking ()
{
  return (openflags & O_NONBLOCK_MASK) != 0;
}

void
fhandler_base::set_nonblocking (int yes)
{
  int current = openflags & O_NONBLOCK_MASK;
  int new_flags = yes ? (!current ? O_NONBLOCK : current) : 0;
  openflags = (openflags & ~O_NONBLOCK_MASK) | new_flags;
  if (new_flags)
    was_nonblocking (true);
}

int
fhandler_base::mkdir (mode_t)
{
  if (exists ())
    set_errno (EEXIST);
  else
    set_errno (EROFS);
  return -1;
}

int
fhandler_base::rmdir ()
{
  if (!exists ())
    set_errno (ENOENT);
  else if (!pc.isdir ())
    set_errno (ENOTDIR);
  else
    set_errno (EROFS);
  return -1;
}

DIR *
fhandler_base::opendir (int fd)
{
  set_errno (ENOTDIR);
  return NULL;
}

int
fhandler_base::readdir (DIR *, dirent *)
{
  return ENOTDIR;
}

long
fhandler_base::telldir (DIR *)
{
  set_errno (ENOTDIR);
  return -1;
}

void
fhandler_base::seekdir (DIR *, long)
{
  set_errno (ENOTDIR);
}

void
fhandler_base::rewinddir (DIR *)
{
  set_errno (ENOTDIR);
}

int
fhandler_base::closedir (DIR *)
{
  set_errno (ENOTDIR);
  return -1;
}

int
fhandler_base::fchmod (mode_t mode)
{
  extern int chmod_device (path_conv& pc, mode_t mode);
  if (pc.is_fs_special ())
    return chmod_device (pc, mode);
  /* By default, just succeeds. */
  return 0;
}

int
fhandler_base::fchown (uid_t uid, gid_t gid)
{
  if (pc.is_fs_special ())
    return ((fhandler_disk_file *) this)->fhandler_disk_file::fchown (uid, gid);
  /* By default, just succeeds. */
  return 0;
}

int
fhandler_base::facl (int cmd, int nentries, aclent_t *aclbufp)
{
  int res = -1;
  switch (cmd)
    {
      case SETACL:
	/* By default, just succeeds. */
	res = 0;
	break;
      case GETACL:
	if (!aclbufp)
	  set_errno(EFAULT);
	else if (nentries < MIN_ACL_ENTRIES)
	  set_errno (ENOSPC);
	else
	  {
	    aclbufp[0].a_type = USER_OBJ;
	    aclbufp[0].a_id = myself->uid;
	    aclbufp[0].a_perm = (S_IRUSR | S_IWUSR) >> 6;
	    aclbufp[1].a_type = GROUP_OBJ;
	    aclbufp[1].a_id = myself->gid;
	    aclbufp[1].a_perm = (S_IRGRP | S_IWGRP) >> 3;
	    aclbufp[2].a_type = OTHER_OBJ;
	    aclbufp[2].a_id = ILLEGAL_GID;
	    aclbufp[2].a_perm = S_IROTH | S_IWOTH;
	    res = MIN_ACL_ENTRIES;
	  }
	break;
      case GETACLCNT:
	res = MIN_ACL_ENTRIES;
	break;
      default:
	set_errno (EINVAL);
	break;
    }
  return res;
}

ssize_t
fhandler_base::fgetxattr (const char *name, void *value, size_t size)
{
  set_errno (ENOTSUP);
  return -1;
}

int
fhandler_base::fsetxattr (const char *name, const void *value, size_t size,
			  int flags)
{
  set_errno (ENOTSUP);
  return -1;
}

int
fhandler_base::fadvise (off_t offset, off_t length, int advice)
{
  set_errno (EINVAL);
  return -1;
}

int
fhandler_base::ftruncate (off_t length, bool allow_truncate)
{
  return EINVAL;
}

int
fhandler_base::link (const char *newpath)
{
  set_errno (EPERM);
  return -1;
}

int
fhandler_base::utimens (const struct timespec *tvp)
{
  if (is_fs_special ())
    return utimens_fs (tvp);

  set_errno (EINVAL);
  return -1;
}

int
fhandler_base::fsync ()
{
  if (!get_handle () || nohandle ())
    {
      set_errno (EINVAL);
      return -1;
    }
  if (pc.isdir ()) /* Just succeed. */
    return 0;
  if (FlushFileBuffers (get_handle ()))
    return 0;

  /* Ignore ERROR_INVALID_FUNCTION because FlushFileBuffers() always fails
     with this code on raw devices which are unbuffered by default.  */
  DWORD errcode = GetLastError();
  if (errcode == ERROR_INVALID_FUNCTION)
    return 0;

  __seterrno_from_win_error (errcode);
  return -1;
}

int
fhandler_base::fpathconf (int v)
{
  int ret;

  switch (v)
    {
    case _PC_LINK_MAX:
      return pc.fs_is_ntfs () || pc.fs_is_samba () || pc.fs_is_nfs ()
	     ? LINK_MAX : 1;
    case _PC_MAX_CANON:
      if (is_tty ())
	return MAX_CANON;
      set_errno (EINVAL);
      break;
    case _PC_MAX_INPUT:
      if (is_tty ())
	return MAX_INPUT;
      set_errno (EINVAL);
      break;
    case _PC_NAME_MAX:
      /* NAME_MAX is without trailing \0 */
      if (!pc.isdir ())
	return NAME_MAX;
      ret = NT_MAX_PATH - strlen (get_name ()) - 2;
      return ret < 0 ? 0 : ret > NAME_MAX ? NAME_MAX : ret;
    case _PC_PATH_MAX:
      /* PATH_MAX is with trailing \0 */
      if (!pc.isdir ())
	return PATH_MAX;
      ret = NT_MAX_PATH - strlen (get_name ()) - 1;
      return ret < 0 ? 0 : ret > PATH_MAX ? PATH_MAX : ret;
    case _PC_PIPE_BUF:
      if (pc.isdir ()
	  || get_device () == FH_FIFO || get_device () == FH_PIPE
	  || get_device () == FH_PIPER || get_device () == FH_PIPEW)
	return PIPE_BUF;
      set_errno (EINVAL);
      break;
    case _PC_CHOWN_RESTRICTED:
      return 1;
    case _PC_NO_TRUNC:
      return 1;
    case _PC_VDISABLE:
      if (is_tty ())
	return _POSIX_VDISABLE;
      set_errno (EINVAL);
      break;
    case _PC_ASYNC_IO:
      return 1;
    case _PC_PRIO_IO:
      break;
    case _PC_SYNC_IO:
      return 1;
    case _PC_FILESIZEBITS:
      return FILESIZEBITS;
    case _PC_2_SYMLINKS:
      return 1;
    case _PC_SYMLINK_MAX:
      return SYMLINK_MAX;
    case _PC_POSIX_PERMISSIONS:
    case _PC_POSIX_SECURITY:
      if (get_device () == FH_FS)
	return pc.has_acls () || pc.fs_is_nfs ();
      set_errno (EINVAL);
      break;
    case _PC_CASE_INSENSITIVE:
      return !!pc.objcaseinsensitive ();
    default:
      set_errno (EINVAL);
      break;
    }
  return -1;
}

/* Overlapped I/O */

int __reg1
fhandler_base_overlapped::setup_overlapped ()
{
  OVERLAPPED *ov = get_overlapped_buffer ();
  memset (ov, 0, sizeof (*ov));
  set_overlapped (ov);
  ov->hEvent = CreateEvent (&sec_none_nih, true, true, NULL);
  io_pending = false;
  return ov->hEvent ? 0 : -1;
}

void __reg1
fhandler_base_overlapped::destroy_overlapped ()
{
  OVERLAPPED *ov = get_overlapped ();
  if (ov && ov->hEvent)
    {
      SetEvent (ov->hEvent);
      CloseHandle (ov->hEvent);
      ov->hEvent = NULL;
    }
  io_pending = false;
  get_overlapped () = NULL;
}

bool __reg1
fhandler_base_overlapped::has_ongoing_io ()
{
  if (!io_pending)
    return false;

  if (!IsEventSignalled (get_overlapped ()->hEvent))
    return true;
  io_pending = false;
  DWORD nbytes;
  GetOverlappedResult (get_output_handle (), get_overlapped (), &nbytes, false);
  return false;
}

fhandler_base_overlapped::wait_return __reg3
fhandler_base_overlapped::wait_overlapped (bool inres, bool writing, DWORD *bytes, bool nonblocking, DWORD len)
{
  if (!get_overlapped ())
    return inres ? overlapped_success : overlapped_error;

  wait_return res = overlapped_unknown;
  DWORD err;
  if (inres)
    /* handle below */;
  else if ((err = GetLastError ()) != ERROR_IO_PENDING)
    res = overlapped_error;
  else if (!nonblocking)
    /* handle below */;
  else if (!writing)
    SetEvent (get_overlapped ()->hEvent);	/* Force immediate WFMO return */
  else
    {
      *bytes = len;				/* Assume that this worked */
      io_pending = true;			/*  but don't allow subsequent */
      res = overlapped_success;			/*  writes until completed */
    }
  if (res == overlapped_unknown)
    {
      DWORD wfres = cygwait (get_overlapped ()->hEvent);
      HANDLE h = writing ? get_output_handle () : get_handle ();
      BOOL wores;
      if (isclosed ())
	{
	  switch (wfres)
	    {
	    case WAIT_OBJECT_0:
	      err = ERROR_INVALID_HANDLE;
	      break;
	    case WAIT_SIGNALED:
	      err = ERROR_INVALID_AT_INTERRUPT_TIME;
	      break;
	    default:
	      err = GetLastError ();
	      break;
	    }
	  res = overlapped_error;
	}
      else
	{
	  /* Cancelling here to prevent races.  It's possible that the I/O has
	     completed already, in which case this is a no-op.  Otherwise,
	     WFMO returned because 1) This is a non-blocking call, 2) a signal
	     arrived, or 3) the operation was cancelled.  These cases may be
	     overridden by the return of GetOverlappedResult which could detect
	     that I/O completion occurred.  */
	  CancelIo (h);
	  wores = GetOverlappedResult (h, get_overlapped (), bytes, false);
	  err = GetLastError ();
	  ResetEvent (get_overlapped ()->hEvent);	/* Probably not needed but CYA */
	  debug_printf ("wfres %u, wores %d, bytes %u", wfres, wores, *bytes);
	  if (wores)
	    res = overlapped_success;	/* operation succeeded */
	  else if (wfres == WAIT_OBJECT_0 + 1)
	    {
	      err = ERROR_INVALID_AT_INTERRUPT_TIME; /* forces an EINTR below */
	      debug_printf ("signal");
	      res = overlapped_error;
	    }
	  else if (wfres == WAIT_OBJECT_0 + 2)
	    pthread::static_cancel_self ();		/* never returns */
	  else if (nonblocking)
	    res = overlapped_nonblocking_no_data;	/* more handling below */
	  else
	    {
	      debug_printf ("GetOverlappedResult failed, h %p, bytes %u, %E", h, *bytes);
	      res = overlapped_error;
	    }
	}
    }

  if (res == overlapped_success)
    {
      debug_printf ("normal %s, %u bytes ispipe() %d", writing ? "write" : "read", *bytes, ispipe ());
      if (*bytes == 0 && !writing && ispipe ())
	res = overlapped_nullread;
    }
  else if (res == overlapped_nonblocking_no_data)
    {
      *bytes = (DWORD) -1;
      set_errno (EAGAIN);
      debug_printf ("no data to read for nonblocking I/O");
    }
  else if (err == ERROR_HANDLE_EOF || err == ERROR_BROKEN_PIPE)
    {
      debug_printf ("EOF, %E");
      *bytes = 0;
      res = overlapped_success;
      if (writing && err == ERROR_BROKEN_PIPE)
	raise (SIGPIPE);
    }
  else
    {
      debug_printf ("res %u, Win32 Error %u", (unsigned) res, err);
      *bytes = (DWORD) -1;
      __seterrno_from_win_error (err);
      if (writing && err == ERROR_NO_DATA)
	raise (SIGPIPE);
    }

  return res;
}

void __reg3
fhandler_base_overlapped::raw_read (void *ptr, size_t& len)
{
  DWORD nbytes;
  bool keep_looping;
  do
    {
      bool res = ReadFile (get_handle (), ptr, len, &nbytes,
			   get_overlapped ());
      switch (wait_overlapped (res, false, &nbytes, is_nonblocking ()))
	{
	case overlapped_nullread:
	  keep_looping = true;
	  break;
	default:	/* Added to quiet gcc */
	case overlapped_success:
	case overlapped_error:
	  keep_looping = false;
	  break;
	}
    }
  while (keep_looping);
  len = (nbytes == (DWORD) -1) ? (size_t) -1 : (size_t) nbytes;
}

ssize_t __reg3
fhandler_base_overlapped::raw_write (const void *ptr, size_t len)
{
  size_t nbytes;
  if (has_ongoing_io ())
    {
      set_errno (EAGAIN);
      nbytes = (size_t) -1;
    }
  else
    {
      size_t chunk;
      if (!max_atomic_write || len < max_atomic_write)
	chunk = MIN (len, INT_MAX);
      else if (is_nonblocking ())
	chunk = len = max_atomic_write;
      else
	chunk = max_atomic_write;

      /* MSDN "WriteFile" contains the following note: "Accessing the output
         buffer while a write operation is using the buffer may lead to
	 corruption of the data written from that buffer.  [...]  This can
	 be particularly problematic when using an asynchronous file handle.
	 (https://msdn.microsoft.com/en-us/library/windows/desktop/aa365747)

	 MSDN "Synchronous and Asynchronous I/O" contains the following note:
	 "Do not deallocate or modify [...] the data buffer until all
	 asynchronous I/O operations to the file object have been completed."
	 (https://msdn.microsoft.com/en-us/library/windows/desktop/aa365683)

	 This problem is a non-issue for blocking I/O, but it can lead to
	 problems when using nonblocking I/O.  Consider:
	 - The application uses a static buffer in repeated calls to
	   non-blocking write.
	 - The previous write returned with success, but the overlapped I/O
	   operation is ongoing.
	 - The application copies the next set of data to the static buffer,
	   thus overwriting data still accessed by the previous write call.
	 --> potential data corruption.

	 What we do here is to allocate a per-fhandler buffer big enough
	 to perform the maximum atomic operation from, copy the user space
	 data over to this buffer and then call NtWriteFile on this buffer.
	 This decouples the write operation from the user buffer and the
	 user buffer can be reused without data corruption issues.

	 Since no further write can occur while we're still having ongoing
	 I/O, this should be reasanably safe.

	 Note: We only have proof that this problem actually occurs on Wine
	 yet.  However, the MSDN language indicates that this may be a real
	 problem on real Windows as well. */
      if (is_nonblocking ())
	{
	  if (!atomic_write_buf)
	    atomic_write_buf = cmalloc_abort (HEAP_BUF, max_atomic_write);
	  ptr = memcpy (atomic_write_buf, ptr, chunk);
	}

      nbytes = 0;
      DWORD nbytes_now = 0;
      /* Write to fd in smaller chunks, accumulating a total.
	 If there's an error, just return the accumulated total
	 unless the first write fails, in which case return value
	 from wait_overlapped(). */
      while (nbytes < len)
	{
	  size_t left = len - nbytes;
	  size_t len1;
	  if (left > chunk)
	    len1 = chunk;
	  else
	    len1 = left;
	  bool res = WriteFile (get_output_handle (), ptr, len1, &nbytes_now,
				get_overlapped ());
	  switch (wait_overlapped (res, true, &nbytes_now,
				   is_nonblocking (), len1))
	    {
	    case overlapped_success:
	      ptr = ((char *) ptr) + chunk;
	      nbytes += nbytes_now;
	      break;
	    case overlapped_error:
	      len = 0;		/* terminate loop */
	    case overlapped_unknown:
	    case overlapped_nullread:
	    case overlapped_nonblocking_no_data:
	      break;
	    }
	}
      if (!nbytes)
	nbytes = (nbytes_now == (DWORD) -1) ? (size_t) -1 : nbytes_now;
    }
  return nbytes;
}

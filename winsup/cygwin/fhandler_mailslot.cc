/* fhandler_mailslot.cc.  See fhandler.h for a description of the fhandler classes.

   Copyright 2005, 2007 Red Hat, Inc.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include "winsup.h"

#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "ntdll.h"

/**********************************************************************/
/* fhandler_mailslot */

fhandler_mailslot::fhandler_mailslot ()
  : fhandler_base ()
{
}

int __stdcall
fhandler_mailslot::fstat (struct __stat64 *buf)
{
  debug_printf ("here");

  fhandler_base::fstat (buf);
  if (is_auto_device ())
    {
      buf->st_mode = S_IFCHR | S_IRUSR | S_IWUSR;
      buf->st_uid = geteuid32 ();
      buf->st_gid = getegid32 ();
      buf->st_nlink = 1;
      buf->st_blksize = PREFERRED_IO_BLKSIZE;
      time_as_timestruc_t (&buf->st_ctim);
      buf->st_atim = buf->st_mtim = buf->st_birthtim = buf->st_ctim;
    }
  return 0;
}

int
fhandler_mailslot::open (int flags, mode_t mode)
{
  int res = 0;
  NTSTATUS status;
  IO_STATUS_BLOCK io;
  OBJECT_ATTRIBUTES attr;
  HANDLE x;
  LARGE_INTEGER timeout;

  switch (flags & O_ACCMODE)
    {
    case O_RDONLY:	/* Server */
      timeout.QuadPart = (flags & O_NONBLOCK) ? 0LL : 0x8000000000000000LL;
      status = NtCreateMailslotFile (&x, GENERIC_READ | SYNCHRONIZE,
				     pc.get_object_attr (attr, sec_none),
				     &io, FILE_SYNCHRONOUS_IO_NONALERT,
				     0, 0, &timeout);
      if (!NT_SUCCESS (status))
	{
	  /* FIXME: It's not possible to open the read side of an existing
	     mailslot again.  You'll get a handle, but using it in ReadFile
	     returns ERROR_INVALID_PARAMETER.  On the other hand,
	     NtCreateMailslotFile returns with STATUS_OBJECT_NAME_EXISTS if
	     the mailslot has been created already.
	     So this is an exclusive open for now.  *Duplicating* read side
	     handles works, though, so it might be an option to duplicate
	     the handle from the first process to the current process for
	     opening the mailslot. */
#if 0
	  if (status != STATUS_OBJECT_NAME_COLLISION)
	    {
	      __seterrno_from_nt_status (status);
	      break;
	    }
	  status = NtOpenFile (&x, GENERIC_READ | SYNCHRONIZE,
			       pc.get_object_attr (attr, sec_none), &io,
			       FILE_SHARE_VALID_FLAGS,
			       FILE_SYNCHRONOUS_IO_NONALERT);
#endif
	  if (!NT_SUCCESS (status))
	    {
	      __seterrno_from_nt_status (status);
	      break;
	    }
	}
      set_io_handle (x);
      set_flags (flags, O_BINARY);
      res = 1;
      set_open_status ();
      break;
    case O_WRONLY:	/* Client */
      /* The client is the DLL exclusively.  Don't allow opening from
	 application code. */
      extern fhandler_mailslot *dev_kmsg;
      if (this != dev_kmsg)
	{
	  set_errno (EPERM);	/* As on Linux. */
	  break;
	}
      status = NtOpenFile (&x, GENERIC_WRITE | SYNCHRONIZE,
			   pc.get_object_attr (attr, sec_none), &io,
			   FILE_SHARE_VALID_FLAGS,
			   FILE_SYNCHRONOUS_IO_NONALERT);
      if (!NT_SUCCESS (status))
	{
	  __seterrno_from_nt_status (status);
	  break;
	}
      set_io_handle (x);
      set_flags (flags, O_BINARY);
      res = 1;
      set_open_status ();
      break;
    default:
      set_errno (EINVAL);
      break;
    }
  return res;
}

int
fhandler_mailslot::write (const void *ptr, size_t len)
{
  /* Check for 425/426 byte weirdness */
  if (len == 425 || len == 426)
    {
      char buf[427];
      buf[425] = buf[426] = '\0';
      memcpy (buf, ptr, len);
      return raw_write (buf, 427);
    }
  return raw_write (ptr, len);
}

int
fhandler_mailslot::ioctl (unsigned int cmd, void *buf)
{
  int res = -1;
  NTSTATUS status;
  IO_STATUS_BLOCK io;

  switch (cmd)
    {
    case FIONBIO:
      {
	FILE_MAILSLOT_SET_INFORMATION fmsi;
	fmsi.ReadTimeout.QuadPart = buf ? 0LL : 0x8000000000000000LL;
	status = NtSetInformationFile (get_handle (), &io, &fmsi, sizeof fmsi,
				       FileMailslotSetInformation);
	if (!NT_SUCCESS (status))
	  {
	    debug_printf ("NtSetInformationFile (%X): %08x",
			  fmsi.ReadTimeout.QuadPart, status);
	    __seterrno_from_nt_status (status);
	    break;
	  }
      }
      /*FALLTHRU*/
    default:
      res =  fhandler_base::ioctl (cmd, buf);
      break;
    }
  return res;
}

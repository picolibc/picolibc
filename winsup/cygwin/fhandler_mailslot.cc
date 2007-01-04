/* fhandler_mailslot.cc.  See fhandler.h for a description of the fhandler classes.

   Copyright 2005, 2007 Red Hat, Inc.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include "winsup.h"
#include <unistd.h>
#include <sys/termios.h>

#include <ntdef.h>
#include "cygerrno.h"
#include "perprocess.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
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
      buf->st_atim = buf->st_mtim = buf->st_ctim;
    }
  return 0;
}

int
fhandler_mailslot::open (int flags, mode_t mode)
{
  int res = 0;
  HANDLE x;

  switch (flags & O_ACCMODE)
    {
    case O_RDONLY:	/* Server */
      x = CreateMailslot (get_win32_name (),
			  0, /* Any message size */
			  (flags & O_NONBLOCK) ? 0 : MAILSLOT_WAIT_FOREVER,
			  &sec_none);
      if (x == INVALID_HANDLE_VALUE)
	{
	  /* FIXME: It's not possible to open the read side of an existing
	     mailslot using CreateFile.  You'll get a handle, but using it
	     in ReadFile returns ERROR_INVALID_PARAMETER.  On the other
	     hand, CreateMailslot returns with ERROR_ALREADY_EXISTS if the
	     mailslot has been created already.
	     So this is an exclusive open for now.  *Duplicating* read side
	     handles works, though, so it might be an option to duplicate
	     the handle from the first process to the current process for
	     opening the mailslot. */
#if 0
	  if (GetLastError () != ERROR_ALREADY_EXISTS)
	    {
	      __seterrno ();
	      break;
	    }
	  x = CreateFile (get_win32_name (), GENERIC_READ, wincap.shared (),
			  &sec_none, OPEN_EXISTING, 0, 0);
#endif
	  if (x == INVALID_HANDLE_VALUE)
	    {
	      __seterrno ();
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
      x = CreateFile (get_win32_name (), GENERIC_WRITE, wincap.shared (),
		      &sec_none, OPEN_EXISTING, 0, 0);
      if (x == INVALID_HANDLE_VALUE)
	{
	  __seterrno ();
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

  switch (cmd)
    {
    case FIONBIO:
      {
	DWORD timeout = buf ? 0 : MAILSLOT_WAIT_FOREVER;
	if (!SetMailslotInfo (get_handle (), timeout))
	  {
	    debug_printf ("SetMailslotInfo (%u): %E", timeout);
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

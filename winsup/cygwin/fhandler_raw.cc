/* fhandler_raw.cc.  See fhandler.h for a description of the fhandler classes.

   Copyright 1999, 2000, 2001 Red Hat, Inc.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include "winsup.h"
#include <sys/termios.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <cygwin/rdevio.h>
#include <sys/mtio.h>
#include <ntdef.h>
#include "cygerrno.h"
#include "perprocess.h"
#include "security.h"
#include "fhandler.h"
#include "path.h"
#include "dtable.h"
#include "cygheap.h"
#include "ntdll.h"

/* static wrapper functions to hide the effect of media changes and
   bus resets which occurs after a new media is inserted. This is
   also related to the used tape device.  */

static BOOL write_file (HANDLE fh, const void *buf, DWORD to_write,
			DWORD *written, int *err)
{
  BOOL ret;

  *err = 0;
  if (!(ret = WriteFile (fh, buf, to_write, written, 0)))
    {
      if ((*err = GetLastError ()) == ERROR_MEDIA_CHANGED
	  || *err == ERROR_BUS_RESET)
	{
	  *err = 0;
	  if (!(ret = WriteFile (fh, buf, to_write, written, 0)))
	    *err = GetLastError ();
	}
    }
  syscall_printf ("%d (err %d) = WriteFile (%d, %d, write %d, written %d, 0)",
		  ret, *err, fh, buf, to_write, *written);
  return ret;
}

static BOOL read_file (HANDLE fh, void *buf, DWORD to_read,
		       DWORD *read, int *err)
{
  BOOL ret;

  *err = 0;
  if (!(ret = ReadFile(fh, buf, to_read, read, 0)))
    {
      if ((*err = GetLastError ()) == ERROR_MEDIA_CHANGED
	  || *err == ERROR_BUS_RESET)
	{
	  *err = 0;
	  if (!(ret = ReadFile (fh, buf, to_read, read, 0)))
	    *err = GetLastError ();
	}
    }
  syscall_printf ("%d (err %d) = ReadFile (%d, %d, to_read %d, read %d, 0)",
		  ret, *err, fh, buf, to_read, *read);
  return ret;
}

/**********************************************************************/
/* fhandler_dev_raw */

void
fhandler_dev_raw::clear (void)
{
  devbuf = NULL;
  devbufsiz = 0;
  devbufstart = 0;
  devbufend = 0;
  eom_detected = 0;
  eof_detected = 0;
  lastblk_to_read = 0;
  varblkop = 0;
}

int
fhandler_dev_raw::writebuf (void)
{
  DWORD written;
  int ret = 0;

  if (is_writing && devbuf && devbufend)
    {
      DWORD to_write;
      int ret = 0;

      memset (devbuf + devbufend, 0, devbufsiz - devbufend);
      if (get_device () != FH_TAPE)
	to_write = ((devbufend - 1) / 512 + 1) * 512;
      else if (varblkop)
	to_write = devbufend;
      else
	to_write = devbufsiz;
      if (!write_file (get_handle (), devbuf, to_write, &written, &ret)
	  && is_eom (ret))
	eom_detected = 1;
      if (written)
	has_written = 1;
      devbufstart = devbufend = 0;
    }
  is_writing = 0;
  return ret;
}

fhandler_dev_raw::fhandler_dev_raw (DWORD devtype, int nunit)
  : fhandler_base (devtype), unit (nunit)
{
  clear ();
  set_need_fork_fixup ();
}

fhandler_dev_raw::~fhandler_dev_raw (void)
{
  if (devbufsiz > 1L)
    delete [] devbuf;
  clear ();
}

int
fhandler_dev_raw::open (path_conv *real_path, int flags, mode_t)
{
  if (!wincap.has_raw_devices ())
    {
      set_errno (ENOENT);
      debug_printf("%s is accessible under NT/W2K only",real_path->get_win32());
      return 0;
    }

  /* Check for illegal flags. */
  if (flags & (O_APPEND | O_EXCL))
    {
      set_errno (EINVAL);
      return 0;
    }

  /* Always open a raw device existing and binary. */
  flags &= ~(O_CREAT | O_TRUNC);
  flags |= O_BINARY;

  DWORD access = GENERIC_READ | SYNCHRONIZE;
  if (get_device () == FH_TAPE
      || (flags & (O_RDONLY | O_WRONLY | O_RDWR)) == O_WRONLY
      || (flags & (O_RDONLY | O_WRONLY | O_RDWR)) == O_RDWR)
    access |= GENERIC_WRITE;

  extern void str2buf2uni (UNICODE_STRING &, WCHAR *, const char *);
  UNICODE_STRING dev;
  WCHAR devname[MAX_PATH + 1];
  str2buf2uni (dev, devname, real_path->get_win32 ());
  OBJECT_ATTRIBUTES attr;
  InitializeObjectAttributes (&attr, &dev, OBJ_CASE_INSENSITIVE, NULL, NULL);

  HANDLE h;
  IO_STATUS_BLOCK io;
  NTSTATUS status = NtOpenFile (&h, access, &attr, &io, wincap.shared (),
				FILE_SYNCHRONOUS_IO_NONALERT);
  if (!NT_SUCCESS (status))
    {
      set_errno (RtlNtStatusToDosError (status));
      debug_printf ("NtOpenFile: NTSTATUS: %d, Win32: %E", status);
      return 0;
    }

  set_io_handle (h);
  set_flags (flags);
  set_r_binary (O_BINARY);
  set_w_binary (O_BINARY);

  if (devbufsiz > 1L)
    devbuf = new char [devbufsiz];

  return 1;
}

int
fhandler_dev_raw::close (void)
{
  return fhandler_base::close ();
}

int
fhandler_dev_raw::raw_read (void *ptr, size_t ulen)
{
  DWORD bytes_read = 0;
  DWORD read2;
  DWORD bytes_to_read;
  int ret;
  size_t len = ulen;
  char *tgt;

  /* In mode O_RDWR the buffer has to be written to device first */
  ret = writebuf ();
  if (ret)
    {
      set_errno (is_eom (ret) ? ENOSPC : EACCES);
      return -1;
    }

  /* Checking a previous end of file */
  if (eof_detected && !lastblk_to_read)
    {
      eof_detected = 0;
      return 0;
    }

  /* Checking a previous end of media */
  if (eom_detected && !lastblk_to_read)
    {
      set_errno (ENOSPC);
      return -1;
    }

  if (devbuf)
    {
      while (len > 0)
	{
	  if (devbufstart < devbufend)
	    {
	      bytes_to_read = min (len, devbufend - devbufstart);
	      debug_printf ("read %d bytes from buffer (rest %d)",
			    bytes_to_read, devbufstart - devbufend);
	      memcpy (ptr, devbuf + devbufstart, bytes_to_read);
	      len -= bytes_to_read;
	      ptr = (void *) ((char *) ptr + bytes_to_read);
	      bytes_read += bytes_to_read;
	      devbufstart += bytes_to_read;

	      if (lastblk_to_read)
		{
		  lastblk_to_read = 0;
		  break;
		}
	    }
	  if (len > 0)
	    {
	      if (!varblkop && len >= devbufsiz)
		{
		  if (get_device () == FH_TAPE)
		    bytes_to_read = (len / devbufsiz) * devbufsiz;
		  else
		    bytes_to_read = (len / 512) * 512;
		  tgt = (char *) ptr;
		  debug_printf ("read %d bytes direct from file",bytes_to_read);
		}
	      else
		{
		  bytes_to_read = devbufsiz;
		  tgt = devbuf;
		  if (varblkop)
		    debug_printf ("read variable bytes from file into buffer");
		  else
		    debug_printf ("read %d bytes from file into buffer",
				  bytes_to_read);
		}
	      if (!read_file (get_handle (), tgt, bytes_to_read, &read2, &ret))
		{
		  if (!is_eof (ret) && !is_eom (ret))
		    {
		      debug_printf ("return -1, set errno to EACCES");
		      set_errno (EACCES);
		      return -1;
		    }

		  if (is_eof (ret))
		    eof_detected = 1;
		  else
		    eom_detected = 1;

		  if (!read2)
		    {
		      if (!bytes_read && is_eom (ret))
			{
			  debug_printf ("return -1, set errno to ENOSPC");
			  set_errno (ENOSPC);
			  return -1;
			}
		      break;
		    }
		  lastblk_to_read = 1;
		}
	      if (! read2)
	       break;
	      if (tgt == devbuf)
		{
		  devbufstart = 0;
		  devbufend = read2;
		}
	      else
		{
		  len -= bytes_to_read;
		  ptr = (void *) ((char *) ptr + bytes_to_read);
		  bytes_read += bytes_to_read;
		}
	    }
	}
    }
  else if (!read_file (get_handle (), ptr, len, &bytes_read, &ret))
    {
      if (!is_eof (ret) && !is_eom (ret))
	{
	  debug_printf ("return -1, set errno to EACCES");
	  set_errno (EACCES);
	  return -1;
	}
      if (bytes_read)
	{
	  if (is_eof (ret))
	    eof_detected = 1;
	  else
	    eom_detected = 1;
	}
      else if (is_eom (ret))
	{
	  debug_printf ("return -1, set errno to ENOSPC");
	  set_errno (ENOSPC);
	  return -1;
	}
    }

  return bytes_read;
}

int
fhandler_dev_raw::raw_write (const void *ptr, size_t len)
{
  DWORD bytes_written = 0;
  DWORD bytes_to_write;
  DWORD written;
  char *p = (char *) ptr;
  char *tgt;
  int ret;

  /* Checking a previous end of media on tape */
  if (eom_detected)
    {
      set_errno (ENOSPC);
      return -1;
    }

  if (!is_writing)
    {
      devbufend = devbufstart;
      devbufstart = 0;
    }
  is_writing = 1;

  if (devbuf)
    {
      while (len > 0)
	{
	  if (!varblkop &&
	      (len < devbufsiz || devbufend > 0) && devbufend < devbufsiz)
	    {
	      bytes_to_write = min (len, devbufsiz - devbufend);
	      memcpy (devbuf + devbufend, p, bytes_to_write);
	      bytes_written += bytes_to_write;
	      devbufend += bytes_to_write;
	      p += bytes_to_write;
	      len -= bytes_to_write;
	    }
	  else
	    {
	      if (varblkop)
		{
		  bytes_to_write = len;
		  tgt = p;
		}
	      else if (devbufend == devbufsiz)
		{
		  bytes_to_write = devbufsiz;
		  tgt = devbuf;
		}
	      else
		{
		  bytes_to_write = (len / devbufsiz) * devbufsiz;
		  tgt = p;
		}

	      ret = 0;
	      write_file (get_handle (), tgt, bytes_to_write, &written, &ret);
	      if (written)
		has_written = 1;

	      if (ret)
		{
		  if (!is_eom (ret))
		    {
		      __seterrno ();
		      return -1;
		    }

		  eom_detected = 1;

		  if (!written && !bytes_written)
		    {
		      set_errno (ENOSPC);
		      return -1;
		    }

		  if (tgt == p)
		    bytes_written += written;

		  break;	// from while (len > 0)
		}

	      if (tgt == devbuf)
		{
		  if (written != devbufsiz)
		    memmove (devbuf, devbuf + written, devbufsiz - written);
		  devbufend = devbufsiz - written;
		}
	      else
		{
		  len -= written;
		  p += written;
		  bytes_written += written;
		}
	    }
	}
    }
  else if (len > 0)
    {
      if (!write_file (get_handle (), ptr, len, &bytes_written, &ret))
	{
	  if (bytes_written)
	    has_written = 1;
	  if (!is_eom (ret))
	    {
	      set_errno (EACCES);
	      return -1;
	    }
	  eom_detected = 1;
	  if (!bytes_written)
	    {
	      set_errno (ENOSPC);
	      return -1;
	    }
	}
      has_written = 1;
    }
  return bytes_written;
}

int
fhandler_dev_raw::dup (fhandler_base *child)
{
  int ret = fhandler_base::dup (child);

  if (! ret)
    {
      fhandler_dev_raw *fhc = (fhandler_dev_raw *) child;

      fhc->devbufsiz = devbufsiz;
      if (devbufsiz > 1L)
	fhc->devbuf = new char [devbufsiz];
      fhc->devbufstart = 0;
      fhc->devbufend = 0;
      fhc->eom_detected = eom_detected;
      fhc->eof_detected = eof_detected;
      fhc->lastblk_to_read = 0;
      fhc->varblkop = varblkop;
      fhc->unit = unit;
    }
  return ret;
}

void
fhandler_dev_raw::fixup_after_fork (HANDLE)
{
  devbufstart = 0;
  devbufend = 0;
  lastblk_to_read = 0;
}

void
fhandler_dev_raw::fixup_after_exec (HANDLE)
{
  if (devbufsiz > 1L)
    devbuf = new char [devbufsiz];
  devbufstart = 0;
  devbufend = 0;
  lastblk_to_read = 0;
}

int
fhandler_dev_raw::ioctl (unsigned int cmd, void *buf)
{
  int ret = NO_ERROR;

  if (cmd == RDIOCDOP)
    {
      struct rdop *op = (struct rdop *) buf;

      if (!op)
	ret = ERROR_INVALID_PARAMETER;
      else
	switch (op->rd_op)
	  {
	  case RDSETBLK:
	    if (get_device () == FH_TAPE)
	      {
		struct mtop mop;

		mop.mt_op = MTSETBLK;
		mop.mt_count = op->rd_parm;
		ret = ioctl (MTIOCTOP, &mop);
	      }
	    else if (op->rd_parm % 512)
	      ret = ERROR_INVALID_PARAMETER;
	    else if (devbuf && op->rd_parm < devbufend - devbufstart)
	      ret = ERROR_INVALID_PARAMETER;
	    else if (!devbuf || op->rd_parm != devbufsiz)
	      {
		char *buf = new char [op->rd_parm];
		if (devbufsiz > 1L)
		  {
		    memcpy (buf, devbuf + devbufstart, devbufend - devbufstart);
		    devbufend -= devbufstart;
		    delete [] devbuf;
		  }
		else
		  devbufend = 0;

		devbufstart = 0;
		devbuf = buf;
		devbufsiz = op->rd_parm;
	      }
	    break;
	  default:
	    break;
	  }
    }
  else if (cmd == RDIOCGET)
    {
      struct rdget *get = (struct rdget *) buf;

      if (!get)
	ret = ERROR_INVALID_PARAMETER;
      else
	get->bufsiz = devbufsiz ? devbufsiz : 1L;
    }
  else
    return fhandler_base::ioctl (cmd, buf);

  if (ret != NO_ERROR)
    {
      SetLastError (ret);
      __seterrno ();
      return -1;
    }
  return 0;
}

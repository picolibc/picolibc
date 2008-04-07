/* fhandler_raw.cc.  See fhandler.h for a description of the fhandler classes.

   Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2007 Red Hat, Inc.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include "winsup.h"

#include <cygwin/rdevio.h>
#include <sys/mtio.h>
#include "cygerrno.h"
#include "security.h"
#include "path.h"
#include "fhandler.h"

/**********************************************************************/
/* fhandler_dev_raw */

fhandler_dev_raw::fhandler_dev_raw ()
  : fhandler_base (), status ()
{
  need_fork_fixup (true);
}

fhandler_dev_raw::~fhandler_dev_raw ()
{
  if (devbufsiz > 1L)
    delete [] devbuf;
}

int __stdcall
fhandler_dev_raw::fstat (struct __stat64 *buf)
{
  debug_printf ("here");

  fhandler_base::fstat (buf);
  if (is_auto_device ())
    {
      if (get_major () == DEV_TAPE_MAJOR)
	buf->st_mode = S_IFCHR | STD_RBITS | STD_WBITS | S_IWGRP | S_IWOTH;
      else
	buf->st_mode = S_IFBLK | STD_RBITS | STD_WBITS | S_IWGRP | S_IWOTH;

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
fhandler_dev_raw::open (int flags, mode_t)
{
  /* Check for illegal flags. */
  if (get_major () != DEV_TAPE_MAJOR && (flags & (O_APPEND | O_EXCL)))
    {
      set_errno (EINVAL);
      return 0;
    }

  /* Always open a raw device existing and binary. */
  flags &= ~(O_CREAT | O_TRUNC);
  flags |= O_BINARY;

  /* Write-only doesn't work well with raw devices */
  if ((flags & O_ACCMODE) == O_WRONLY)
    flags = ((flags & ~O_WRONLY) | O_RDWR);

  int res = fhandler_base::open (flags, 0);
  if (res && devbufsiz > 1L)
    devbuf = new char [devbufsiz];

  return res;
}

int
fhandler_dev_raw::dup (fhandler_base *child)
{
  int ret = fhandler_base::dup (child);

  if (!ret)
    {
      fhandler_dev_raw *fhc = (fhandler_dev_raw *) child;

      fhc->devbufsiz = devbufsiz;
      if (devbufsiz > 1L)
	fhc->devbuf = new char [devbufsiz];
      fhc->devbufstart = 0;
      fhc->devbufend = 0;
      fhc->lastblk_to_read (false);
    }
  return ret;
}

void
fhandler_dev_raw::fixup_after_fork (HANDLE)
{
  devbufstart = 0;
  devbufend = 0;
  lastblk_to_read (false);
}

void
fhandler_dev_raw::fixup_after_exec ()
{
  if (!close_on_exec ())
    {
      if (devbufsiz > 1L)
	devbuf = new char [devbufsiz];
      devbufstart = 0;
      devbufend = 0;
      lastblk_to_read (false);
    }
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
	    if (get_major () == DEV_TAPE_MAJOR)
	      {
		struct mtop mop;

		mop.mt_op = MTSETBLK;
		mop.mt_count = op->rd_parm;
		ret = ioctl (MTIOCTOP, &mop);
	      }
	    else if ((devbuf && ((op->rd_parm <= 1 && (devbufend - devbufstart))
				 || op->rd_parm < devbufend - devbufstart))
		     || (op->rd_parm > 1 && (op->rd_parm % 512))
		     || (get_flags () & O_DIRECT))
	      /* The conditions for a *valid* parameter are these:
	         - If there's still data in the current buffer, it must
		   fit in the new buffer.
		 - The new size is either 0 or 1, both indicating unbufferd
		   I/O, or the new buffersize must be a multiple of 512.
		 - In the O_DIRECT case, the whole request is invalid. */
	      ret = ERROR_INVALID_PARAMETER;
	    else if (!devbuf || op->rd_parm != devbufsiz)
	      {
		char *buf = NULL;
		if (op->rd_parm > 1L)
		  buf = new char [op->rd_parm];
		if (buf && devbufsiz > 1L)
		  {
		    memcpy (buf, devbuf + devbufstart, devbufend - devbufstart);
		    devbufend -= devbufstart;
		  }
		else
		  devbufend = 0;

		if (devbufsiz > 1L)
		  delete [] devbuf;

		devbufstart = 0;
		devbuf = buf;
		devbufsiz = op->rd_parm ?: 1L;
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
	get->bufsiz = devbufsiz;
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

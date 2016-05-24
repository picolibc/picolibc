/* fhandler_raw.cc.  See fhandler.h for a description of the fhandler classes.

   This file is part of Cygwin.

   This software is a copyrighted work licensed under the terms of the
   Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
   details. */

#include "winsup.h"

#include <unistd.h>
#include <cygwin/rdevio.h>
#include <sys/mtio.h>
#include <sys/param.h>
#include "cygerrno.h"
#include "path.h"
#include "fhandler.h"

/**********************************************************************/
/* fhandler_dev_raw */

fhandler_dev_raw::fhandler_dev_raw ()
  : fhandler_base (),
    status ()
{
  need_fork_fixup (true);
}

fhandler_dev_raw::~fhandler_dev_raw ()
{
  if (devbufsiz > 1L)
    delete [] devbufalloc;
}

int __reg2
fhandler_dev_raw::fstat (struct stat *buf)
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
  if (get_major () != DEV_TAPE_MAJOR && (flags & O_APPEND))
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

  return res;
}

int
fhandler_dev_raw::dup (fhandler_base *child, int flags)
{
  int ret = fhandler_base::dup (child, flags);

  if (!ret)
    {
      fhandler_dev_raw *fhc = (fhandler_dev_raw *) child;

      if (devbufsiz > 1L)
	{
	  /* Create sector-aligned buffer */
	  fhc->devbufalloc = new char [devbufsiz + devbufalign];
	  fhc->devbuf = (char *) roundup2 ((uintptr_t) fhc->devbufalloc,
					   (uintptr_t) devbufalign);
	}
      fhc->devbufstart = 0;
      fhc->devbufend = 0;
      fhc->lastblk_to_read (false);
    }
  return ret;
}

void
fhandler_dev_raw::fixup_after_fork (HANDLE parent)
{
  fhandler_base::fixup_after_fork (parent);
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
	{
	  /* Create sector-aligned buffer */
	  devbufalloc = new char [devbufsiz + devbufalign];
	  devbuf = (char *) roundup2 ((uintptr_t) devbufalloc,
				      (uintptr_t) devbufalign);
	}
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
	    else if ((op->rd_parm <= 1 && get_major () != DEV_TAPE_MAJOR)
		     || (op->rd_parm > 1 && (op->rd_parm % devbufalign))
		     || (get_flags () & O_DIRECT))
	      /* The conditions for a valid parameter are:
		 - The new size is either 0 or 1, both indicating unbuffered
		   I/O, and the device is a tape device.
		 - Or, the new buffersize must be a multiple of the
		   required buffer alignment.
		 - In the O_DIRECT case, the whole request is invalid. */
	      ret = ERROR_INVALID_PARAMETER;
	    else if (!devbuf || op->rd_parm != devbufsiz)
	      {
		char *buf = NULL;
		off_t curpos = lseek (0, SEEK_CUR);

		if (op->rd_parm > 1L)
		  buf = new char [op->rd_parm + devbufalign];

		if (devbufsiz > 1L)
		  delete [] devbufalloc;

		devbufalloc = buf;
		devbuf = (char *) roundup2 ((uintptr_t) buf,
					    (uintptr_t) devbufalign);
		devbufsiz = op->rd_parm ?: 1L;
		devbufstart = devbufend = 0;
		lseek (curpos, SEEK_SET);
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

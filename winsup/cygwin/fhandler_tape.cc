/* fhandler_tape.cc.  See fhandler.h for a description of the fhandler
   classes.

   Copyright 1999, 2000 Cygnus Solutions.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include <sys/termios.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <sys/mtio.h>
#include "cygheap.h"
#include "cygerrno.h"
#include "fhandler.h"
#include "path.h"

/**********************************************************************/
/* fhandler_dev_tape */

void
fhandler_dev_tape::clear (void)
{
  norewind = 0;
  lasterr = 0;
  fhandler_dev_raw::clear ();
}

int
fhandler_dev_tape::is_eom (int win_error)
{
  int ret = ((win_error == ERROR_END_OF_MEDIA)
	  || (win_error == ERROR_EOM_OVERFLOW)
	  || (win_error == ERROR_NO_DATA_DETECTED));
  if (ret)
    debug_printf ("end of medium");
  return ret;
}

int
fhandler_dev_tape::is_eof (int win_error)
{
  int ret = ((win_error == ERROR_FILEMARK_DETECTED)
	  || (win_error == ERROR_SETMARK_DETECTED));
  if (ret)
    debug_printf ("end of file");
  return ret;
}

fhandler_dev_tape::fhandler_dev_tape (const char *name, int unit) : fhandler_dev_raw (FH_TAPE, name, unit)
{
  set_cb (sizeof *this);
}

int
fhandler_dev_tape::open (const char *path, int flags, mode_t)
{
  int ret;
  int minor;

  if (get_device_number (path, minor) != FH_TAPE)
    {
      set_errno (EINVAL);
      return -1;
    }

  norewind = (minor >= 128);
  devbufsiz = 1L;

  ret = fhandler_dev_raw::open (path, flags);
  if (ret)
    {
      struct mtget get;
      struct mtop op;
      struct mtpos pos;

      if (! ioctl (MTIOCGET, &get))
	/* Tape drive supports and is set to variable block size. */
	if (get.mt_dsreg == 0)
	  devbufsiz = get.mt_maxblksize;
	else
	  devbufsiz = get.mt_dsreg;
	varblkop = get.mt_dsreg == 0;

      if (devbufsiz > 1L)
	devbuf = new char [devbufsiz];

      /* The following rewind in position 0 solves a problem which appears
       * in case of multi volume archives: The last ReadFile on first medium
       * returns ERROR_NO_DATA_DETECTED. After media change, all subsequent
       * ReadFile calls return ERROR_NO_DATA_DETECTED, too.
       * The call to tape_set_pos seems to reset some internal flags. */
      if ((! ioctl (MTIOCPOS, &pos)) && (! pos.mt_blkno))
	{
	  op.mt_op = MTREW;
	  ioctl (MTIOCTOP, &op);
	}

      if (flags & O_APPEND)
	{
	  /* In append mode, seek to beginning of next filemark */
	  op.mt_op = MTFSFM;
	  op.mt_count = 1;
	  ioctl (MTIOCTOP, &op);
	}
    }

  return ret;
}

int
fhandler_dev_tape::close (void)
{
  struct mtop op;
  int ret = 0;

  if (is_writing)
    {
      ret = writebuf ();
      if ((has_written) && (! eom_detected))
	{
	  /* if last operation was writing, write a filemark */
	  debug_printf ("writing filemark\n");
	  op.mt_op = MTWEOF;
	  op.mt_count = 1;
	  ioctl (MTIOCTOP, &op);
	}
    }

  // To protected reads on signaling (e.g. Ctrl-C)
  eof_detected = 1;

  if (! norewind)
    {
      debug_printf ("rewinding\n");
      op.mt_op = MTREW;
      ioctl (MTIOCTOP, &op);
    }

  if (ret)
    {
      fhandler_dev_raw::close ();
      return ret;
    }

  return fhandler_dev_raw::close ();
}

int
fhandler_dev_tape::fstat (struct stat *buf)
{
  int ret;

  if (! (ret = fhandler_dev_raw::fstat (buf)))
    {
      struct mtget get;

      if (! ioctl (MTIOCGET, &get))
	{
	  buf->st_blocks = get.mt_capacity / buf->st_blksize;
	}
    }

  return ret;
}

off_t
fhandler_dev_tape::lseek (off_t offset, int whence)
{
  struct mtop op;
  struct mtpos pos;

  debug_printf ("lseek (%s, %d, %d)\n", get_name (), offset, whence);

  writebuf ();
  eom_detected = eof_detected = 0;
  lastblk_to_read = 0;
  devbufstart = devbufend = 0;

  if (ioctl (MTIOCPOS, &pos))
    {
      return (off_t) -1;
    }

  switch (whence)
    {
      case SEEK_END:
	op.mt_op = MTFSF;
	op.mt_count = 1;
	if (ioctl (MTIOCTOP, &op))
	  return -1;
	break;
      case SEEK_SET:
	if (whence == SEEK_SET && offset < 0)
	  {
	    set_errno (EINVAL);
	    return -1;
	  }
	break;
      case SEEK_CUR:
	break;
      default:
	set_errno (EINVAL);
	return -1;
    }

  op.mt_op = MTFSR;
  op.mt_count = offset / devbufsiz
		- (whence == SEEK_SET ? pos.mt_blkno : 0);

  if (op.mt_count < 0)
    {
      op.mt_op = MTBSR;
      op.mt_count = -op.mt_count;
    }

  if (ioctl (MTIOCTOP, &op) || ioctl (MTIOCPOS, &pos))
    return -1;

  return (pos.mt_blkno * devbufsiz);
}

int
fhandler_dev_tape::dup (fhandler_base *child)
{
  fhandler_dev_tape *fhc = (fhandler_dev_tape *) child;

  fhc->norewind = norewind;
  fhc->lasterr = lasterr;
  return fhandler_dev_raw::dup (child);
}

int
fhandler_dev_tape::ioctl (unsigned int cmd, void *buf)
{
  int ret = NO_ERROR;
  unsigned long block;

  if (cmd == MTIOCTOP)
    {
      struct mtop *op = (struct mtop *) buf;

      if (! op)
	ret = ERROR_INVALID_PARAMETER;
      else
	switch (op->mt_op)
	  {
	  case MTRESET:
	    break;
	  case MTFSF:
	    ret = tape_set_pos (TAPE_SPACE_FILEMARKS, op->mt_count);
	    break;
	  case MTBSF:
	    ret = tape_set_pos (TAPE_SPACE_FILEMARKS, -op->mt_count);
	    break;
	  case MTFSR:
	    ret = tape_set_pos (TAPE_SPACE_RELATIVE_BLOCKS, op->mt_count);
	    break;
	  case MTBSR:
	    ret = tape_set_pos (TAPE_SPACE_RELATIVE_BLOCKS, -op->mt_count);
	    break;
	  case MTWEOF:
	    if (tape_get_feature (TAPE_DRIVE_WRITE_FILEMARKS))
	      ret = tape_write_marks (TAPE_FILEMARKS, op->mt_count);
	    else if (tape_get_feature (TAPE_DRIVE_WRITE_LONG_FMKS))
	      ret = tape_write_marks (TAPE_LONG_FILEMARKS, op->mt_count);
	    else
	      ret = tape_write_marks (TAPE_SHORT_FILEMARKS, op->mt_count);
	    break;
	  case MTREW:
	    ret = tape_set_pos (TAPE_REWIND, 0);
	    break;
	  case MTOFFL:
	    ret = tape_prepare (TAPE_UNLOAD);
	    break;
	  case MTNOP:
	    break;
	  case MTRETEN:
	    if (! tape_get_feature (TAPE_DRIVE_END_OF_DATA))
	      ret = ERROR_INVALID_PARAMETER;
	    else if (! (ret = tape_set_pos (TAPE_REWIND, 0, FALSE)))
	      ret = tape_prepare (TAPE_TENSION);
	    break;
	  case MTBSFM:
	    ret = tape_set_pos (TAPE_SPACE_FILEMARKS, -op->mt_count, TRUE);
	    break;
	  case MTFSFM:
	    ret = tape_set_pos (TAPE_SPACE_FILEMARKS, op->mt_count, TRUE);
	    break;
	  case MTEOM:
	    if (tape_get_feature (TAPE_DRIVE_END_OF_DATA))
	      ret = tape_set_pos (TAPE_SPACE_END_OF_DATA, 0);
	    else
	      ret = tape_set_pos (TAPE_SPACE_FILEMARKS, 32767);
	    break;
	  case MTERASE:
	    ret = tape_erase (TAPE_ERASE_SHORT);
	    break;
	  case MTRAS1:
	  case MTRAS2:
	  case MTRAS3:
	    ret = ERROR_INVALID_PARAMETER;
	    break;
	  case MTSETBLK:
	    {
	      long min, max;

	      if (! tape_get_feature (TAPE_DRIVE_SET_BLOCK_SIZE))
		{
		  ret = ERROR_INVALID_PARAMETER;
		  break;
		}
	      ret = tape_get_blocksize (&min, NULL, &max, NULL);
	      if (ret)
		  break;
	      if (devbuf && (size_t) op->mt_count == devbufsiz && !varblkop)
		{
		  ret = 0;
		  break;
		}
	      if ((op->mt_count == 0
		   && !tape_get_feature (TAPE_DRIVE_VARIABLE_BLOCK))
		  || (op->mt_count > 0
		      && (op->mt_count < min || op->mt_count > max)))
		{
		  ret = ERROR_INVALID_PARAMETER;
		  break;
		}
	      if (devbuf && op->mt_count > 0
		  && (size_t) op->mt_count < devbufend - devbufstart)
		{
		  ret = ERROR_MORE_DATA;
		  break;
		}
	      if (! (ret = tape_set_blocksize (op->mt_count)))
		{
		  size_t size = 0;
		  if (op->mt_count == 0)
		    {
		      struct mtget get;
		      if ((ret = tape_status (&get)) != NO_ERROR)
			break;
		      size = get.mt_maxblksize;
		      ret = NO_ERROR;
		    }
		  char *buf = NULL;
		  if (size > 1L && !(buf = new char [size]))
		    {
		      ret = ERROR_OUTOFMEMORY;
		      break;
		    }
		  if (devbufsiz > 1L && size > 1L)
		    {
		      memcpy(buf, devbuf + devbufstart,
			     devbufend - devbufstart);
		      devbufend -= devbufstart;
		    }
		  else
		    devbufend = 0;
		  if (devbufsiz > 1L)
		    delete [] devbuf;
		  devbufstart = 0;
		  devbuf = buf;
		  devbufsiz = size;
		  varblkop = op->mt_count == 0;
		}
	    }
	    break;
	  case MTSETDENSITY:
	    ret = ERROR_INVALID_PARAMETER;
	    break;
	  case MTSEEK:
	    if (tape_get_feature (TAPE_DRIVE_ABSOLUTE_BLK))
	      {
		ret = tape_set_pos (TAPE_ABSOLUTE_BLOCK, op->mt_count);
		break;
	      }
	    if (! (ret = tape_get_pos (&block)))
	      {
		ret = tape_set_pos (TAPE_SPACE_RELATIVE_BLOCKS,
				 op->mt_count - block);
	      }
	    break;
	  case MTTELL:
	    if (! (ret = tape_get_pos (&block)))
	      op->mt_count = block;
	    break;
	  case MTSETDRVBUFFER:
	    ret = ERROR_INVALID_PARAMETER;
	    break;
	  case MTFSS:
	    ret = tape_set_pos (TAPE_SPACE_SETMARKS, op->mt_count);
	    break;
	  case MTBSS:
	    ret = tape_set_pos (TAPE_SPACE_SETMARKS, -op->mt_count);
	    break;
	  case MTWSM:
	    ret = tape_write_marks (TAPE_SETMARKS, op->mt_count);
	    break;
	  case MTLOCK:
	    ret = tape_prepare (TAPE_LOCK);
	    break;
	  case MTUNLOCK:
	    ret = tape_prepare (TAPE_UNLOCK);
	    break;
	  case MTLOAD:
	    ret = tape_prepare (TAPE_LOAD);
	    break;
	  case MTUNLOAD:
	    ret = tape_prepare (TAPE_UNLOAD);
	    break;
	  case MTCOMPRESSION:
	    ret = tape_compression (op->mt_count);
	    break;
	  case MTSETPART:
	  case MTMKPART:
	  default:
	    ret = ERROR_INVALID_PARAMETER;
	    break;
	  }
    }
  else if (cmd == MTIOCGET)
    ret = tape_status ((struct mtget *) buf);
  else if (cmd == MTIOCPOS)
    {
      ret = ERROR_INVALID_PARAMETER;
      if (buf && (ret = tape_get_pos (&block)))
	((struct mtpos *) buf)->mt_blkno = block;
    }
  else
    return fhandler_dev_raw::ioctl (cmd, buf);

  if (ret != NO_ERROR)
    {
      SetLastError (ret);
      __seterrno ();
      return -1;
    }

  return 0;
}

/* ------------------------------------------------------------------ */
/*                  Private functions used by `ioctl'                 */
/* ------------------------------------------------------------------ */

static int
tape_error (DWORD lasterr, const char *txt)
{
  if (lasterr)
    debug_printf ("%s: error: %d\n", txt, lasterr);

  return lasterr;
}

int
fhandler_dev_tape::tape_write_marks (int marktype, DWORD len)
{
  syscall_printf ("write_tapemark\n");
  while (((lasterr = WriteTapemark (get_handle (),
				    marktype,
				    len,
				    FALSE)) == ERROR_MEDIA_CHANGED)
	 || (lasterr == ERROR_BUS_RESET))
    ;

  return tape_error (lasterr, "tape_write_marks");
}

int
fhandler_dev_tape::tape_get_pos (unsigned long *ret)
{
  DWORD part, low, high;

  while (((lasterr = GetTapePosition (get_handle (),
				      TAPE_ABSOLUTE_POSITION,
				      &part,
				      &low,
				      &high)) == ERROR_MEDIA_CHANGED)
	 || (lasterr == ERROR_BUS_RESET))
    ;
  if (! tape_error (lasterr, "tape_get_pos") && ret)
    *ret = low;

  return lasterr;
}

static int _tape_set_pos (HANDLE hTape, int mode, long count)
{
  int err;

  while (((err = SetTapePosition (hTape,
				  mode,
				  1,
				  count,
				  count < 0 ? -1 : 0,
				  FALSE)) == ERROR_MEDIA_CHANGED)
	 || (err == ERROR_BUS_RESET))
    ;

  return err;
}

int
fhandler_dev_tape::tape_set_pos (int mode, long count, BOOLEAN sfm_func)
{
  unsigned long pos, tgtpos;

  switch (mode)
    {
      case TAPE_SPACE_RELATIVE_BLOCKS:
	lasterr = tape_get_pos (&pos);

	if (lasterr)
	  return lasterr;

	tgtpos = pos + count;

	while (((lasterr = _tape_set_pos (get_handle (),
					  mode,
					  count)) == ERROR_FILEMARK_DETECTED)
	       || (lasterr == ERROR_SETMARK_DETECTED))
	  {
	    lasterr = tape_get_pos (&pos);
	    if (lasterr)
	      return lasterr;
	    count = tgtpos - pos;
	  }

	if (lasterr == ERROR_BEGINNING_OF_MEDIA && ! tgtpos)
	  lasterr = NO_ERROR;

	break;
      case TAPE_SPACE_FILEMARKS:
	if (count < 0)
	  {
	    if (pos > 0)
	      {
		if ((! _tape_set_pos (get_handle (),
				  TAPE_SPACE_RELATIVE_BLOCKS,
				  -1))
		    || (sfm_func))
		  ++count;
		_tape_set_pos (get_handle (), TAPE_SPACE_RELATIVE_BLOCKS, 1);
	      }

	    while (! (lasterr = _tape_set_pos (get_handle (), mode, -1))
		   && count++ < 0)
	      ;

	    if (lasterr == ERROR_BEGINNING_OF_MEDIA)
	      {
		if (! count)
		  lasterr = NO_ERROR;
	      }
	    else if (! sfm_func)
	      lasterr = _tape_set_pos (get_handle (), mode, 1);
	  }
	else
	  {
	    if (sfm_func)
	      {
		if (_tape_set_pos (get_handle (),
				   TAPE_SPACE_RELATIVE_BLOCKS,
				   1) == ERROR_FILEMARK_DETECTED)
		  ++count;
		_tape_set_pos (get_handle (), TAPE_SPACE_RELATIVE_BLOCKS, -1);
	      }

	    if (! (lasterr = _tape_set_pos (get_handle (), mode, count))
		&& sfm_func)
	      lasterr = _tape_set_pos (get_handle (), mode, -1);
	  }
	break;
      case TAPE_SPACE_SETMARKS:
      case TAPE_ABSOLUTE_BLOCK:
      case TAPE_SPACE_END_OF_DATA:
      case TAPE_REWIND:
	lasterr = _tape_set_pos (get_handle (), mode, count);
	break;
    }

  return tape_error (lasterr, "tape_set_pos");
}

int
fhandler_dev_tape::tape_erase (int mode)
{
  DWORD varlen;
  TAPE_GET_DRIVE_PARAMETERS dp;

  while (((lasterr = GetTapeParameters (get_handle (),
					GET_TAPE_DRIVE_INFORMATION,
					(varlen = sizeof dp, &varlen),
					&dp)) == ERROR_MEDIA_CHANGED)
	 || (lasterr == ERROR_BUS_RESET))
    ;

  switch (mode)
    {
      case TAPE_ERASE_SHORT:
	if (! lasterr && ! (dp.FeaturesLow & TAPE_DRIVE_ERASE_SHORT))
	  mode = TAPE_ERASE_LONG;
	break;
      case TAPE_ERASE_LONG:
	if (! lasterr && ! (dp.FeaturesLow & TAPE_DRIVE_ERASE_LONG))
	  mode = TAPE_ERASE_SHORT;
	break;
    }

  return tape_error (EraseTape (get_handle (), mode, FALSE), "tape_erase");
}

int
fhandler_dev_tape::tape_prepare (int action)
{
  while (((lasterr = PrepareTape (get_handle (),
				  action,
				  FALSE)) == ERROR_MEDIA_CHANGED)
	 || (lasterr == ERROR_BUS_RESET))
    ;
  return tape_error (lasterr, "tape_prepare");
}

BOOLEAN
fhandler_dev_tape::tape_get_feature (DWORD parm)
{
  DWORD varlen;
  TAPE_GET_DRIVE_PARAMETERS dp;

  while (((lasterr = GetTapeParameters (get_handle (),
					GET_TAPE_DRIVE_INFORMATION,
					(varlen = sizeof dp, &varlen),
					&dp)) == ERROR_MEDIA_CHANGED)
	 || (lasterr == ERROR_BUS_RESET))
    ;

  if (lasterr)
    return FALSE;

  return ((parm & TAPE_DRIVE_HIGH_FEATURES)
	  ? ((dp.FeaturesHigh & parm) != 0)
	  : ((dp.FeaturesLow & parm) != 0));
}

int
fhandler_dev_tape::tape_get_blocksize (long *min, long *def, long *max, long *cur)
{
  DWORD varlen;
  TAPE_GET_DRIVE_PARAMETERS dp;
  TAPE_GET_MEDIA_PARAMETERS mp;

  while (((lasterr = GetTapeParameters (get_handle (),
					GET_TAPE_DRIVE_INFORMATION,
					(varlen = sizeof dp, &varlen),
					&dp)) == ERROR_MEDIA_CHANGED)
	 || (lasterr == ERROR_BUS_RESET))
    ;

  if (lasterr)
    return tape_error (lasterr, "tape_get_blocksize");

  while (((lasterr = GetTapeParameters (get_handle (),
					GET_TAPE_MEDIA_INFORMATION,
					(varlen = sizeof mp, &varlen),
					&mp)) == ERROR_MEDIA_CHANGED)
	 || (lasterr == ERROR_BUS_RESET))
    ;

  if (lasterr)
    return tape_error (lasterr, "tape_get_blocksize");

  if (min)
    *min = (long) dp.MinimumBlockSize;
  if (def)
    *def = (long) dp.DefaultBlockSize;
  if (max)
    *max = (long) dp.MaximumBlockSize;
  if (cur)
    *cur = (long) mp.BlockSize;

  return tape_error (lasterr, "tape_get_blocksize");
}

int
fhandler_dev_tape::tape_set_blocksize (long count)
{
  long min, max;
  TAPE_SET_MEDIA_PARAMETERS mp;

  lasterr = tape_get_blocksize (&min, NULL, &max, NULL);

  if (lasterr)
    return lasterr;

  if (count < min || count > max)
    return tape_error (ERROR_INVALID_PARAMETER, "tape_set_blocksize");

  mp.BlockSize = count;

  return tape_error (SetTapeParameters (get_handle (),
					SET_TAPE_MEDIA_INFORMATION,
					&mp),
		     "tape_set_blocksize");
}

static long long
get_ll (PLARGE_INTEGER i)
{
  long long l = 0;

  l = i->HighPart;
  l <<= 32;
  l |= i->LowPart;
  return l;
}

int
fhandler_dev_tape::tape_status (struct mtget *get)
{
  DWORD varlen;
  TAPE_GET_DRIVE_PARAMETERS dp;
  TAPE_GET_MEDIA_PARAMETERS mp;
  int notape = 0;

  if (! get)
    return ERROR_INVALID_PARAMETER;

  while (((lasterr = GetTapeParameters (get_handle (),
					GET_TAPE_DRIVE_INFORMATION,
					(varlen = sizeof dp, &varlen),
					&dp)) == ERROR_MEDIA_CHANGED)
	 || (lasterr == ERROR_BUS_RESET))
    ;

  if ((lasterr) || (lasterr = GetTapeParameters (get_handle (),
						 GET_TAPE_MEDIA_INFORMATION,
						 (varlen = sizeof mp, &varlen),
						 &mp)))
    notape = 1;

  memset (get, 0, sizeof *get);

  get->mt_type = MT_ISUNKNOWN;

  if (! notape && (dp.FeaturesLow & TAPE_DRIVE_TAPE_REMAINING))
    {
      get->mt_remaining = get_ll (&mp.Remaining);
      get->mt_resid = get->mt_remaining >> 10;
    }

  if ((dp.FeaturesHigh & TAPE_DRIVE_SET_BLOCK_SIZE) && ! notape)
    get->mt_dsreg = mp.BlockSize;
  else
    get->mt_dsreg = dp.DefaultBlockSize;

  if (notape)
    get->mt_gstat |= GMT_DR_OPEN (-1);

  if (! notape)
    {
      if (dp.FeaturesLow & TAPE_DRIVE_GET_ABSOLUTE_BLK)
	tape_get_pos ((unsigned long *) &get->mt_blkno);

      if (! get->mt_blkno)
	get->mt_gstat |= GMT_BOT (-1);

      get->mt_gstat |= GMT_ONLINE (-1);

      if ((dp.FeaturesLow & TAPE_DRIVE_WRITE_PROTECT) && mp.WriteProtected)
	get->mt_gstat |= GMT_WR_PROT (-1);

      if (dp.FeaturesLow & TAPE_DRIVE_TAPE_CAPACITY)
	get->mt_capacity = get_ll (&mp.Capacity);
    }

  if ((dp.FeaturesLow & TAPE_DRIVE_COMPRESSION) && dp.Compression)
    get->mt_gstat |= GMT_HW_COMP (-1);

  if ((dp.FeaturesLow & TAPE_DRIVE_ECC) && dp.ECC)
    get->mt_gstat |= GMT_HW_ECC (-1);

  if ((dp.FeaturesLow & TAPE_DRIVE_PADDING) && dp.DataPadding)
    get->mt_gstat |= GMT_PADDING (-1);

  if ((dp.FeaturesLow & TAPE_DRIVE_REPORT_SMKS) && dp.ReportSetmarks)
    get->mt_gstat |= GMT_IM_REP_EN (-1);

  get->mt_erreg = lasterr;

  get->mt_minblksize = dp.MinimumBlockSize;
  get->mt_maxblksize = dp.MaximumBlockSize;
  get->mt_defblksize = dp.DefaultBlockSize;
  get->mt_featureslow = dp.FeaturesLow;
  get->mt_featureshigh = dp.FeaturesHigh;

  return 0;
}

int
fhandler_dev_tape::tape_compression (long count)
{
  DWORD varlen;
  TAPE_GET_DRIVE_PARAMETERS dpg;
  TAPE_SET_DRIVE_PARAMETERS dps;

  while (((lasterr = GetTapeParameters (get_handle (),
					GET_TAPE_DRIVE_INFORMATION,
					(varlen = sizeof dpg, &varlen),
					&dpg)) == ERROR_MEDIA_CHANGED)
	 || (lasterr == ERROR_BUS_RESET))
    ;

  if (lasterr)
    return tape_error (lasterr, "tape_compression");

  if (! (dpg.FeaturesLow & TAPE_DRIVE_COMPRESSION))
    return ERROR_INVALID_PARAMETER;

  if (count)
    {
      dps.ECC = dpg.ECC;
      dps.Compression = count ? TRUE : FALSE;
      dps.DataPadding = dpg.DataPadding;
      dps.ReportSetmarks = dpg.ReportSetmarks;
      dps.EOTWarningZoneSize = dpg.EOTWarningZoneSize;
      lasterr = SetTapeParameters (get_handle (),
				   SET_TAPE_DRIVE_INFORMATION,
				   &dps);

      if (lasterr)
	return tape_error (lasterr, "tape_compression");

      dpg.Compression = dps.Compression;
    }

  return 0;
}


/* fhandler_tape.cc.  See fhandler.h for a description of the fhandler
   classes.

   Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009,
   2010, 2011, 2012, 2013 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include "winsup.h"
#include "cygtls.h"
#include <stdlib.h>
#include <sys/mtio.h>
#include <sys/param.h>
#include <devioctl.h>
#include <ntddstor.h>
#include "security.h"
#include "path.h"
#include "fhandler.h"
#include "dtable.h"
#include "cygheap.h"
#include "shared_info.h"
#include "sigproc.h"
#include "child_info.h"

/* Media changes and bus resets are sometimes reported and the function
   hasn't been executed.  We repeat all functions which return with one
   of these error codes. */
#define TAPE_FUNC(func) while ((lasterr = (func)) == ERROR_MEDIA_CHANGED) \
			  { \
			    initialize (drive, false); \
			    part (partition)->initialize (0); \
			  }

#define IS_BOT(err) ((err) == ERROR_BEGINNING_OF_MEDIA)

#define IS_EOF(err) ((err) == ERROR_FILEMARK_DETECTED \
		     || (err) == ERROR_SETMARK_DETECTED)

#define IS_SM(err)  ((err) == ERROR_SETMARK_DETECTED)

#define IS_EOD(err) ((err) == ERROR_END_OF_MEDIA \
		     || (err) == ERROR_EOM_OVERFLOW \
		     || (err) == ERROR_NO_DATA_DETECTED)

#define IS_EOM(err) ((err) == ERROR_END_OF_MEDIA \
		     || (err) == ERROR_EOM_OVERFLOW)

/**********************************************************************/
/* mtinfo_part */

void
mtinfo_part::initialize (int64_t nblock)
{
  block = nblock;
  if (block == 0)
    file = fblock = 0;
  else
    file = fblock = -1;
  smark = false;
  emark = no_eof;
}

/**********************************************************************/
/* mtinfo_drive */

void
mtinfo_drive::initialize (int num, bool first_time)
{
  drive = num;
  partition = 0;
  block = -1;
  lock = unlocked;
  if (first_time)
    {
      buffer_writes (true);
      async_writes (false);
      two_fm (false);
      fast_eom (false);
      auto_lock (false);
      sysv (false);
      nowait (false);
    }
  for (int i = 0; i < MAX_PARTITION_NUM; ++i)
    part (i)->initialize ();
}

int
mtinfo_drive::get_dp (HANDLE mt)
{
  DWORD len = sizeof _dp;
  TAPE_FUNC (GetTapeParameters (mt, GET_TAPE_DRIVE_INFORMATION, &len, &_dp));
  return error ("get_dp");
}

int
mtinfo_drive::get_mp (HANDLE mt)
{
  DWORD len = sizeof _mp;
  TAPE_FUNC (GetTapeParameters (mt, GET_TAPE_MEDIA_INFORMATION, &len, &_mp));
  return error ("get_mp");
}

int
mtinfo_drive::open (HANDLE mt)
{
  /* First access after opening the device can return BUS RESET, but we
     need the drive parameters, so just try again. */
  while (get_dp (mt) == ERROR_BUS_RESET)
    ;
  get_mp (mt);
  get_pos (mt);
  if (partition < MAX_PARTITION_NUM && part (partition)->block != block)
    part (partition)->initialize (block);
  /* The following rewind in position 0 solves a problem which appears
   * in case of multi volume archives (at least on NT4): The last ReadFile
   * on the previous medium returns ERROR_NO_DATA_DETECTED.  After media
   * change, all subsequent ReadFile calls return ERROR_NO_DATA_DETECTED,
   * too.  The call to set_pos apparently reset some internal flags.
   * FIXME:  Is that really true or based on a misinterpretation? */
  if (!block)
    {
      debug_printf ("rewind in position 0");
      set_pos (mt, TAPE_REWIND, 0, false);
    }
  return error ("open");
}

int
mtinfo_drive::close (HANDLE mt, bool rewind)
{
  lasterr = 0;
  if (GetTapeStatus (mt) == ERROR_NO_MEDIA_IN_DRIVE)
    dirty = clean;
  if (dirty >= has_written)
    {
      /* If an async write is still pending, wait for completion. */
      if (dirty == async_write_pending)
	lasterr = async_wait (mt, NULL);
      if (!lasterr)
	{
	  /* if last operation was writing, write a filemark */
	  debug_printf ("writing filemark");
	  write_marks (mt, TAPE_FILEMARKS, two_fm () ? 2 : 1);
	  if (two_fm () && !lasterr && !rewind) /* Backspace over 2nd fmark. */
	    {
	      set_pos (mt, TAPE_SPACE_FILEMARKS, -1, false);
	      if (!lasterr)
		part (partition)->fblock = 0; /* That's obvious, isn't it? */
	    }
	}
    }
  else if (dirty == has_read && !rewind)
    {
      if (sysv ())
	{
	  /* Under SYSV semantics, the tape is moved past the next file mark
	     after read. */
	  if (part (partition)->emark == no_eof)
	    set_pos (mt, TAPE_SPACE_FILEMARKS, 1, false);
	  else if (part (partition)->emark == eof_hit)
	    part (partition)->emark = eof;
	}
      else
	{
	  /* Under BSD semantics, we must check if the filemark has been
	     inadvertendly crossed.  If so cross the filemark backwards
	     and position the tape right before EOF. */
	  if (part (partition)->emark == eof_hit)
	    set_pos (mt, TAPE_SPACE_FILEMARKS, -1, false);
	}
    }
  if (rewind)
    {
      debug_printf ("rewinding");
      set_pos (mt, TAPE_REWIND, 0, false);
    }
  if (auto_lock () && lock == auto_locked)
    prepare (mt, TAPE_UNLOCK);
  dirty = clean;
  return error ("close");
}

int
mtinfo_drive::read (HANDLE mt, LPOVERLAPPED pov, void *ptr, size_t &ulen)
{
  BOOL ret;
  DWORD bytes_read = 0;

  if (GetTapeStatus (mt) == ERROR_NO_MEDIA_IN_DRIVE)
    return lasterr = ERROR_NO_MEDIA_IN_DRIVE;
  if (lasterr == ERROR_BUS_RESET)
    {
      ulen = 0;
      goto out;
    }
  /* If an async write is still pending, wait for completion. */
  if (dirty == async_write_pending)
    lasterr = async_wait (mt, NULL);
  dirty = clean;
  if (part (partition)->emark == eof_hit)
    {
      part (partition)->emark = eof;
      lasterr = ulen = 0;
      goto out;
    }
  else if (part (partition)->emark == eod_hit)
    {
      part (partition)->emark = eod;
      lasterr = ulen = 0;
      goto out;
    }
  else if (part (partition)->emark == eod)
    {
      lasterr = ERROR_NO_DATA_DETECTED;
      ulen = (size_t) -1;
      goto out;
    }
  else if (part (partition)->emark == eom_hit)
    {
      part (partition)->emark = eom;
      lasterr = ulen = 0;
      goto out;
    }
  else if (part (partition)->emark == eom)
    {
      lasterr = ERROR_END_OF_MEDIA;
      ulen = (size_t) -1;
      goto out;
    }
  part (partition)->smark = false;
  if (auto_lock () && lock < auto_locked)
    prepare (mt, TAPE_LOCK, true);
  ov = pov;
  ov->Offset = ov->OffsetHigh = 0;
  ret = ReadFile (mt, ptr, ulen, &bytes_read, ov);
  lasterr = ret ? 0 : GetLastError ();
  if (lasterr == ERROR_IO_PENDING)
    lasterr = async_wait (mt, &bytes_read);
  ulen = (size_t) bytes_read;
  if (bytes_read > 0)
    {
      int32_t blocks_read = mp ()->BlockSize == 0
			    ? 1 : howmany (bytes_read, mp ()->BlockSize);
      block += blocks_read;
      part (partition)->block += blocks_read;
      if (part (partition)->fblock >= 0)
	part (partition)->fblock += blocks_read;
    }
  if (IS_EOF (lasterr))
    {
      block++;
      part (partition)->block++;
      if (part (partition)->file >= 0)
	part (partition)->file++;
      part (partition)->fblock = 0;
      part (partition)->smark = IS_SM (lasterr);
      part (partition)->emark = bytes_read > 0 ? eof_hit : eof;
      lasterr = 0;
    }
  else if (IS_EOD (lasterr))
    {
      if (part (partition)->emark == eof)
	part (partition)->emark = IS_EOM (lasterr) ? eom : eod;
      else
	{
	  part (partition)->emark = IS_EOM (lasterr) ? eom_hit : eod_hit;
	  lasterr = 0;
	}
    }
  else
    {
      part (partition)->emark = no_eof;
      /* This happens if the buffer is too small when in variable block
	 size mode.  Linux returns ENOMEM here.  We're doing the same. */
      if (lasterr == ERROR_MORE_DATA)
	lasterr = ERROR_NOT_ENOUGH_MEMORY;
    }
  if (!lasterr)
    dirty = has_read;
out:
  return error ("read");
}

int
mtinfo_drive::async_wait (HANDLE mt, DWORD *bytes_written)
{
  DWORD written;

  bool ret = GetOverlappedResult (mt, ov, &written, TRUE);
  if (bytes_written)
    *bytes_written = written;
  return ret ? 0 : GetLastError ();
}

int
mtinfo_drive::write (HANDLE mt, LPOVERLAPPED pov, const void *ptr, size_t &len)
{
  BOOL ret;
  DWORD bytes_written = 0;
  int async_err = 0;

  if (GetTapeStatus (mt) == ERROR_NO_MEDIA_IN_DRIVE)
    return lasterr = ERROR_NO_MEDIA_IN_DRIVE;
  if (lasterr == ERROR_BUS_RESET)
    {
      len = 0;
      return error ("write");
    }
  if (dirty == async_write_pending)
    async_err = async_wait (mt, &bytes_written);
  dirty = clean;
  part (partition)->smark = false;
  if (auto_lock () && lock < auto_locked)
    prepare (mt, TAPE_LOCK, true);
  ov = pov;
  ov->Offset = ov->OffsetHigh = 0;
  ret = WriteFile (mt, ptr, len, &bytes_written, ov);
  lasterr = ret ? 0: GetLastError ();
  if (lasterr == ERROR_IO_PENDING)
    {
      if (async_writes () && mp ()->BlockSize == 0)
	dirty = async_write_pending;
      else
	/* Wait for completion if a non-async write. */
	lasterr = async_wait (mt, &bytes_written);
    }
  len = (size_t) bytes_written;
  if (bytes_written > 0)
    {
      int32_t blocks_written = mp ()->BlockSize == 0
			       ? 1 : howmany (bytes_written, mp ()->BlockSize);
      block += blocks_written;
      part (partition)->block += blocks_written;
      if (part (partition)->fblock >= 0)
	part (partition)->fblock += blocks_written;
    }
  if (!lasterr && async_err)
    lasterr = async_err;
  if (lasterr == ERROR_EOM_OVERFLOW)
    part (partition)->emark = eom;
  else if (lasterr == ERROR_END_OF_MEDIA)
    ; // FIXME?: part (partition)->emark = eom_hit;
  else
    {
      part (partition)->emark = no_eof;
      if (!lasterr)
	dirty = has_written;
      else if (lasterr == ERROR_IO_PENDING)
	dirty = async_write_pending;
    }
  return error ("write");
}

int
mtinfo_drive::get_pos (HANDLE mt, int32_t *ppartition, int64_t *pblock)
{
  DWORD p;
  ULARGE_INTEGER b;

  TAPE_FUNC (GetTapePosition (mt, TAPE_LOGICAL_POSITION, &p,
			      &b.LowPart, &b.HighPart));
  if (lasterr == ERROR_INVALID_FUNCTION)
    TAPE_FUNC (GetTapePosition (mt, TAPE_ABSOLUTE_POSITION, &p,
				&b.LowPart, &b.HighPart));
  if (!lasterr)
    {
      if (p > 0)
	partition = (int32_t) p - 1;
      block = (int64_t) b.QuadPart;
      if (ppartition)
	*ppartition= partition;
      if (pblock)
	*pblock = block;
    }
  else
    {
      partition = 0;
      block = -1;
    }
  return error ("get_pos");
}

int
mtinfo_drive::_set_pos (HANDLE mt, int mode, int64_t count, int partition,
			BOOL dont_wait)
{
  /* If an async write is still pending, wait for completion. */
  if (dirty == async_write_pending)
    lasterr = async_wait (mt, NULL);
  dirty = clean;
  LARGE_INTEGER c = { QuadPart:count };
  TAPE_FUNC (SetTapePosition (mt, mode, partition, c.LowPart, c.HighPart,
			      dont_wait));
  return lasterr;
}

int
mtinfo_drive::set_pos (HANDLE mt, int mode, int64_t count, bool sfm_func)
{
  int err = 0;
  int64_t undone = count;
  BOOL dont_wait = FALSE;

  switch (mode)
    {
      case TAPE_SPACE_RELATIVE_BLOCKS:
      case TAPE_SPACE_FILEMARKS:
      case TAPE_SPACE_SETMARKS:
	if (!count)
	  {
	    lasterr = 0;
	    goto out;
	  }
	break;
      case TAPE_ABSOLUTE_BLOCK:
      case TAPE_LOGICAL_BLOCK:
      case TAPE_REWIND:
	dont_wait = nowait () ? TRUE : FALSE;
	break;
    }
  if (mode == TAPE_SPACE_FILEMARKS)
    {
      while (!err && undone > 0)
	if (!(err = _set_pos (mt, mode, 1, 0, FALSE)) || IS_SM (err))
	  --undone;
      while (!err && undone < 0)
	if (!(err = _set_pos (mt, mode, -1, 0, FALSE)) || IS_SM (err))
	  ++undone;
    }
  else
    err = _set_pos (mt, mode, count, 0, dont_wait);
  switch (mode)
    {
      case TAPE_ABSOLUTE_BLOCK:
      case TAPE_LOGICAL_BLOCK:
	get_pos (mt);
	part (partition)->initialize (block);
	break;
      case TAPE_REWIND:
	if (!err)
	  {
	    block = 0;
	    part (partition)->initialize (0);
	  }
	else
	  {
	    get_pos (mt);
	    part (partition)->initialize (block);
	  }
	break;
      case TAPE_SPACE_END_OF_DATA:
	get_pos (mt);
	part (partition)->initialize (block);
	part (partition)->emark = IS_EOM (err) ? eom : eod;
	break;
      case TAPE_SPACE_FILEMARKS:
	if (!err || IS_SM (err))
	  {
	    get_pos (mt);
	    part (partition)->block = block;
	    if (count > 0)
	      {
		if (part (partition)->file >= 0)
		  part (partition)->file += count - undone;
		part (partition)->fblock = 0;
		part (partition)->smark = IS_SM (err);
	      }
	    else
	      {
		if (part (partition)->file >= 0)
		  part (partition)->file += count - undone;
		part (partition)->fblock = -1;
		part (partition)->smark = false;
	      }
	    if (sfm_func)
	      err = set_pos (mt, mode, count > 0 ? -1 : 1, false);
	    else
	      part (partition)->emark = count > 0 ? eof : no_eof;
	  }
	else if (IS_EOD (err))
	  {
	    get_pos (mt);
	    part (partition)->block = block;
	    if (part (partition)->file >= 0)
	      part (partition)->file += count - undone;
	    part (partition)->fblock = -1;
	    part (partition)->smark = false;
	    part (partition)->emark = IS_EOM (err) ? eom : eod;
	  }
	else if (IS_BOT (err))
	  {
	    block = 0;
	    part (partition)->initialize (0);
	  }
	else
	  {
	    get_pos (mt);
	    part (partition)->initialize (block);
	  }
	break;
      case TAPE_SPACE_RELATIVE_BLOCKS:
	if (!err)
	  {
	    block += count;
	    part (partition)->block += count;
	    if (part (partition)->fblock >= 0)
	      part (partition)->fblock += count;
	    part (partition)->smark = false;
	    part (partition)->emark = no_eof;
	  }
	else if (IS_EOF (err))
	  {
	    get_pos (mt);
	    part (partition)->block = block;
	    if (part (partition)->file >= 0)
	      part (partition)->file += count > 0 ? 1 : -1;
	    part (partition)->fblock = count > 0 ? 0 : -1;
	    part (partition)->smark = (count > 0 && IS_SM (err));
	    part (partition)->emark = count > 0 ? eof : no_eof;
	  }
	else if (IS_EOD (err))
	  {
	    get_pos (mt);
	    part (partition)->fblock = block - part (partition)->block;
	    part (partition)->block = block;
	    part (partition)->smark = false;
	    part (partition)->emark = IS_EOM (err) ? eom : eod;
	  }
	else if (IS_BOT (err))
	  {
	    block = 0;
	    part (partition)->initialize (0);
	  }
	break;
      case TAPE_SPACE_SETMARKS:
	get_pos (mt);
	part (partition)->block = block;
	if (!err)
	  {
	    part (partition)->file = -1;
	    part (partition)->fblock = -1;
	    part (partition)->smark = true;
	  }
	break;
    }
  lasterr = err;
out:
  return error ("set_pos");
}

int
mtinfo_drive::create_partitions (HANDLE mt, int32_t count)
{
  if (dp ()->MaximumPartitionCount <= 1)
    return ERROR_INVALID_PARAMETER;
  if (set_pos (mt, TAPE_REWIND, 0, false))
    goto out;
  partition = 0;
  part (partition)->initialize (0);
  debug_printf ("Format tape with %s partition(s)", count <= 0 ? "one" : "two");
  if (get_feature (TAPE_DRIVE_INITIATOR))
    {
      TAPE_FUNC (CreateTapePartition (mt, TAPE_INITIATOR_PARTITIONS,
				      count <= 0 ? 0 : 2, (DWORD) count));
    }
  else if (get_feature (TAPE_DRIVE_SELECT))
    {
      TAPE_FUNC (CreateTapePartition (mt, TAPE_SELECT_PARTITIONS,
				      count <= 0 ? 0 : 2, 0));
    }
  else if (get_feature (TAPE_DRIVE_FIXED))
    {
      /* This is supposed to work for Tandberg SLR drivers up to version
	 1.6 which missed to set the TAPE_DRIVE_INITIATOR flag.  According
	 to Tandberg, CreateTapePartition(TAPE_FIXED_PARTITIONS) apparently
	 does not ignore the dwCount parameter.  Go figure! */
      TAPE_FUNC (CreateTapePartition (mt, TAPE_FIXED_PARTITIONS,
				      count <= 0 ? 0 : 2, (DWORD) count));
    }
  else
    lasterr = ERROR_INVALID_PARAMETER;
out:
  return error ("partition");
}

int
mtinfo_drive::set_partition (HANDLE mt, int32_t count)
{
  if (count < 0 || (uint32_t) count >= MAX_PARTITION_NUM)
    lasterr = ERROR_INVALID_PARAMETER;
  else if ((DWORD) count >= dp ()->MaximumPartitionCount)
    lasterr = ERROR_IO_DEVICE;
  else
    {
      uint64_t part_block = part (count)->block >= 0 ? part (count)->block : 0;
      int err = _set_pos (mt, TAPE_LOGICAL_BLOCK, part_block, count + 1, FALSE);
      if (err)
	{
	  int64_t sav_block = block;
	  int32_t sav_partition = partition;
	  get_pos (mt);
	  if (sav_partition != partition)
	    {
	      if (partition < MAX_PARTITION_NUM
		  && part (partition)->block != block)
		part (partition)->initialize (block);
	    }
	  else if (sav_block != block && partition < MAX_PARTITION_NUM)
	    part (partition)->initialize (block);
	  lasterr = err;
	}
      else
	{
	  partition = count;
	  if (part (partition)->block == -1)
	    part (partition)->initialize (0);
	}
    }
  return error ("set_partition");
}

int
mtinfo_drive::write_marks (HANDLE mt, int marktype, DWORD count)
{
  /* If an async write is still pending, wait for completion. */
  if (dirty == async_write_pending)
    {
      lasterr = async_wait (mt, NULL);
      dirty = has_written;
    }
  if (marktype != TAPE_SETMARKS)
    dirty = clean;
  if (marktype == TAPE_FILEMARKS
      && !get_feature (TAPE_DRIVE_WRITE_FILEMARKS))
    {
      if (get_feature (TAPE_DRIVE_WRITE_LONG_FMKS))
	marktype = TAPE_LONG_FILEMARKS;
      else
	marktype = TAPE_SHORT_FILEMARKS;
    }
  TAPE_FUNC (WriteTapemark (mt, marktype, count, FALSE));
  int err = lasterr;
  if (!err)
    {
      block += count;
      part (partition)->block += count;
      if (part (partition)->file >= 0)
	part (partition)->file += count;
      part (partition)->fblock = 0;
      part (partition)->emark = eof;
      part (partition)->smark = (marktype == TAPE_SETMARKS);
    }
  else
    {
      int64_t sav_block = block;
      int32_t sav_partition = partition;
      get_pos (mt);
      if (sav_partition != partition)
	{
	  if (partition < MAX_PARTITION_NUM
	      && part (partition)->block != block)
	    part (partition)->initialize (block);
	}
      else if (sav_block != block && partition < MAX_PARTITION_NUM)
	part (partition)->initialize (block);
      lasterr = err;
    }
  return error ("write_marks");
}

int
mtinfo_drive::erase (HANDLE mt, int mode)
{
  switch (mode)
    {
      case TAPE_ERASE_SHORT:
	if (!get_feature (TAPE_DRIVE_ERASE_SHORT))
	  mode = TAPE_ERASE_LONG;
	break;
      case TAPE_ERASE_LONG:
	if (!get_feature (TAPE_DRIVE_ERASE_LONG))
	  mode = TAPE_ERASE_SHORT;
	break;
    }
  TAPE_FUNC (EraseTape (mt, mode, nowait () ? TRUE : FALSE));
  part (partition)->initialize (0);
  return error ("erase");
}

int
mtinfo_drive::prepare (HANDLE mt, int action, bool is_auto)
{
  BOOL dont_wait = FALSE;

  /* If an async write is still pending, wait for completion. */
  if (dirty == async_write_pending)
    lasterr = async_wait (mt, NULL);
  dirty = clean;
  if (action == TAPE_UNLOAD || action == TAPE_LOAD || action == TAPE_TENSION)
    dont_wait = nowait () ? TRUE : FALSE;
  TAPE_FUNC (PrepareTape (mt, action, dont_wait));
  /* Reset buffer after all successful preparations but lock and unlock. */
  switch (action)
    {
      case TAPE_FORMAT:
      case TAPE_UNLOAD:
      case TAPE_LOAD:
	initialize (drive, false);
	break;
      case TAPE_TENSION:
	part (partition)->initialize (0);
	break;
      case TAPE_LOCK:
	lock = lasterr ? lock_error : is_auto ? auto_locked : locked;
	break;
      case TAPE_UNLOCK:
	lock = lasterr ? lock_error : unlocked;
	break;
    }
  return error ("prepare");
}

int
mtinfo_drive::set_compression (HANDLE mt, int32_t count)
{
  if (!get_feature (TAPE_DRIVE_SET_COMPRESSION))
    return ERROR_INVALID_PARAMETER;
  TAPE_SET_DRIVE_PARAMETERS sdp =
    {
      dp ()->ECC,
      (BOOLEAN) (count ? TRUE : FALSE),
      dp ()->DataPadding,
      dp ()->ReportSetmarks,
      dp ()->EOTWarningZoneSize
    };
  TAPE_FUNC (SetTapeParameters (mt, SET_TAPE_DRIVE_INFORMATION, &sdp));
  int err = lasterr;
  if (!err)
    dp ()->Compression = sdp.Compression;
  else
    get_dp (mt);
  lasterr = err;
  return error ("set_compression");
}

int
mtinfo_drive::set_blocksize (HANDLE mt, DWORD count)
{
  TAPE_SET_MEDIA_PARAMETERS smp = {count};
  TAPE_FUNC (SetTapeParameters (mt, SET_TAPE_MEDIA_INFORMATION, &smp));
  /* Make sure to update blocksize info! */
  return lasterr ? error ("set_blocksize") : get_mp (mt);
}

int
mtinfo_drive::get_status (HANDLE mt, struct mtget *get)
{
  int notape = 0;
  DWORD tstat;

  if (!get)
    return ERROR_INVALID_PARAMETER;

  if ((tstat = GetTapeStatus (mt)) == ERROR_NO_MEDIA_IN_DRIVE
      || get_mp (mt) == ERROR_NO_MEDIA_IN_DRIVE)
    notape = 1;

  memset (get, 0, sizeof *get);

  get->mt_type = MT_ISUNKNOWN;

  if (!notape && get_feature (TAPE_DRIVE_SET_BLOCK_SIZE))
    get->mt_dsreg = (mp ()->BlockSize << MT_ST_BLKSIZE_SHIFT)
		    & MT_ST_BLKSIZE_MASK;
  else
    get->mt_dsreg = (dp ()->DefaultBlockSize << MT_ST_BLKSIZE_SHIFT)
		    & MT_ST_BLKSIZE_MASK;

  DWORD size = sizeof (GET_MEDIA_TYPES) + 10 * sizeof (DEVICE_MEDIA_INFO);
  void *buf = alloca (size);
  if (DeviceIoControl (mt, IOCTL_STORAGE_GET_MEDIA_TYPES_EX,
		       NULL, 0, buf, size, &size, NULL)
      || GetLastError () == ERROR_MORE_DATA)
    {
      PGET_MEDIA_TYPES gmt = (PGET_MEDIA_TYPES) buf;
      for (DWORD i = 0; i < gmt->MediaInfoCount; ++i)
	{
	  PDEVICE_MEDIA_INFO dmi = &gmt->MediaInfo[i];
	  get->mt_type = dmi->DeviceSpecific.TapeInfo.MediaType;
#define TINFO DeviceSpecific.TapeInfo
	  if (dmi->TINFO.MediaCharacteristics & MEDIA_CURRENTLY_MOUNTED)
	    {
	      get->mt_type = dmi->DeviceSpecific.TapeInfo.MediaType;
	      if (dmi->TINFO.BusType == BusTypeScsi)
		get->mt_dsreg |=
		  (dmi->TINFO.BusSpecificData.ScsiInformation.DensityCode
		   << MT_ST_DENSITY_SHIFT)
		  & MT_ST_DENSITY_MASK;
	      break;
	    }
#undef TINFO
	}
    }

  if (!notape)
    {
      get->mt_resid = (partition & 0xffff)
		      | ((mp ()->PartitionCount & 0xffff) << 16);
      get->mt_fileno = part (partition)->file;
      get->mt_blkno = part (partition)->fblock;

      if (get->mt_blkno != 0)
	/* nothing to do */;
      else if (get->mt_fileno == 0)
	get->mt_gstat |= GMT_BOT (-1);
      else
	get->mt_gstat |= GMT_EOF (-1);
      if (part (partition)->emark >= eod_hit)
	get->mt_gstat |= GMT_EOD (-1);
      if (part (partition)->emark >= eom_hit)
	get->mt_gstat |= GMT_EOT (-1);

      if (part (partition)->smark)
	get->mt_gstat |= GMT_SM (-1);

      get->mt_gstat |= GMT_ONLINE (-1);

      if (mp ()->WriteProtected)
	get->mt_gstat |= GMT_WR_PROT (-1);

      get->mt_capacity = mp ()->Capacity.QuadPart;
      get->mt_remaining = mp ()->Remaining.QuadPart;
  }

  if (notape)
    get->mt_gstat |= GMT_DR_OPEN (-1);

  if (buffer_writes ())
    get->mt_gstat |= GMT_IM_REP_EN (-1);	/* TODO: Async writes */

  if (tstat == ERROR_DEVICE_REQUIRES_CLEANING)
    get->mt_gstat |= GMT_CLN (-1);

  /* Cygwin specials: */
  if (dp ()->ReportSetmarks)
    get->mt_gstat |= GMT_REP_SM (-1);
  if (dp ()->DataPadding)
    get->mt_gstat |= GMT_PADDING (-1);
  if (dp ()->ECC)
    get->mt_gstat |= GMT_HW_ECC (-1);
  if (dp ()->Compression)
    get->mt_gstat |= GMT_HW_COMP (-1);
  if (two_fm ())
    get->mt_gstat |= GMT_TWO_FM (-1);
  if (fast_eom ())
    get->mt_gstat |= GMT_FAST_MTEOM (-1);
  if (auto_lock ())
    get->mt_gstat |= GMT_AUTO_LOCK (-1);
  if (sysv ())
    get->mt_gstat |= GMT_SYSV (-1);
  if (nowait ())
    get->mt_gstat |= GMT_NOWAIT (-1);
  if (async_writes ())
    get->mt_gstat |= GMT_ASYNC (-1);

  get->mt_erreg = 0;				/* FIXME: No softerr counting */

  get->mt_minblksize = dp ()->MinimumBlockSize;
  get->mt_maxblksize = dp ()->MaximumBlockSize;
  get->mt_defblksize = dp ()->DefaultBlockSize;
  get->mt_featureslow = dp ()->FeaturesLow;
  get->mt_featureshigh = dp ()->FeaturesHigh;
  get->mt_eotwarningzonesize = dp ()->EOTWarningZoneSize;

  return 0;
}

int
mtinfo_drive::set_options (HANDLE mt, int32_t options)
{
  int32_t what = (options & MT_ST_OPTIONS);
  bool call_setparams = false;
  bool set;
  TAPE_SET_DRIVE_PARAMETERS sdp =
    {
      dp ()->ECC,
      dp ()->Compression,
      dp ()->DataPadding,
      dp ()->ReportSetmarks,
      dp ()->EOTWarningZoneSize
    };

  lasterr = 0;
  switch (what)
    {
      case 0:
	if (options == 0 || options == 1)
	  {
	    buffer_writes ((options == 1));
	  }
	break;
      case MT_ST_BOOLEANS:
	buffer_writes (!!(options & MT_ST_BUFFER_WRITES));
	async_writes (!!(options & MT_ST_ASYNC_WRITES));
	two_fm (!!(options & MT_ST_TWO_FM));
	fast_eom (!!(options & MT_ST_FAST_MTEOM));
	auto_lock (!!(options & MT_ST_AUTO_LOCK));
	sysv (!!(options & MT_ST_SYSV));
	nowait (!!(options & MT_ST_NOWAIT));
	if (get_feature (TAPE_DRIVE_SET_ECC))
	  sdp.ECC = !!(options & MT_ST_ECC);
	if (get_feature (TAPE_DRIVE_SET_PADDING))
	  sdp.DataPadding = !!(options & MT_ST_PADDING);
	if (get_feature (TAPE_DRIVE_SET_REPORT_SMKS))
	  sdp.ReportSetmarks = !!(options & MT_ST_REPORT_SM);
	if (sdp.ECC != dp ()->ECC || sdp.DataPadding != dp ()->DataPadding
	    || sdp.ReportSetmarks != dp ()->ReportSetmarks)
	  call_setparams = true;
	break;
      case MT_ST_SETBOOLEANS:
      case MT_ST_CLEARBOOLEANS:
	set = (what == MT_ST_SETBOOLEANS);
	if (options & MT_ST_BUFFER_WRITES)
	  buffer_writes (set);
	if (options & MT_ST_ASYNC_WRITES)
	  async_writes (set);
	if (options & MT_ST_TWO_FM)
	  two_fm (set);
	if (options & MT_ST_FAST_MTEOM)
	  fast_eom (set);
	if (options & MT_ST_AUTO_LOCK)
	  auto_lock (set);
	if (options & MT_ST_SYSV)
	  sysv (set);
	if (options & MT_ST_NOWAIT)
	  nowait (set);
	if (options & MT_ST_ECC)
	  sdp.ECC = set;
	if (options & MT_ST_PADDING)
	  sdp.DataPadding = set;
	if (options & MT_ST_REPORT_SM)
	  sdp.ReportSetmarks = set;
	if (sdp.ECC != dp ()->ECC || sdp.DataPadding != dp ()->DataPadding
	    || sdp.ReportSetmarks != dp ()->ReportSetmarks)
	  call_setparams = true;
	break;
      case MT_ST_EOT_WZ_SIZE:
	if (get_feature (TAPE_DRIVE_SET_EOT_WZ_SIZE))
	  {
	    sdp.EOTWarningZoneSize = (options & ~MT_ST_OPTIONS);
	    if (sdp.EOTWarningZoneSize != dp ()->EOTWarningZoneSize)
	      call_setparams = true;
	  }
	break;
    }
  if (call_setparams)
    {
      TAPE_FUNC (SetTapeParameters (mt, SET_TAPE_DRIVE_INFORMATION, &sdp));
      int err = lasterr;
      if (!err)
	{
	  dp ()->ECC = sdp.ECC;
	  dp ()->DataPadding = sdp.DataPadding;
	  dp ()->ReportSetmarks = sdp.ReportSetmarks;
	}
      else
	get_dp (mt);
      lasterr = err;
    }
  return error ("set_options");
}

int
mtinfo_drive::ioctl (HANDLE mt, unsigned int cmd, void *buf)
{
  myfault efault;
  if (efault.faulted ())
    return ERROR_NOACCESS;
  if (cmd == MTIOCTOP)
    {
      struct mtop *op = (struct mtop *) buf;
      if (lasterr == ERROR_BUS_RESET)
	{
	  /* If a bus reset occurs, block further access to this device
	     until the user rewinds, unloads or in any other way tries
	     to maintain a well-known tape position. */
	  if (op->mt_op != MTREW && op->mt_op != MTOFFL
	      && op->mt_op != MTRETEN && op->mt_op != MTERASE
	      && op->mt_op != MTSEEK && op->mt_op != MTEOM)
	    return ERROR_BUS_RESET;
	  /* Try to maintain last lock state after bus reset. */
	  if (lock >= auto_locked && PrepareTape (mt, TAPE_LOCK, FALSE))
	    {
	      debug_printf ("Couldn't relock drive after bus reset.");
	      lock = unlocked;
	    }
	}
      switch (op->mt_op)
	{
	  case MTRESET:
	    break;
	  case MTFSF:
	    set_pos (mt, TAPE_SPACE_FILEMARKS, op->mt_count, false);
	    break;
	  case MTBSF:
	    set_pos (mt, TAPE_SPACE_FILEMARKS, -op->mt_count, false);
	    break;
	  case MTFSR:
	    set_pos (mt, TAPE_SPACE_RELATIVE_BLOCKS, op->mt_count, false);
	    break;
	  case MTBSR:
	    set_pos (mt, TAPE_SPACE_RELATIVE_BLOCKS, -op->mt_count, false);
	    break;
	  case MTWEOF:
	    write_marks (mt, TAPE_FILEMARKS, op->mt_count);
	    break;
	  case MTREW:
	    set_pos (mt, TAPE_REWIND, 0, false);
	    break;
	  case MTOFFL:
	  case MTUNLOAD:
	    prepare (mt, TAPE_UNLOAD);
	    break;
	  case MTNOP:
	    lasterr = 0;
	    break;
	  case MTRETEN:
	    if (!get_feature (TAPE_DRIVE_TENSION))
	      lasterr = ERROR_INVALID_PARAMETER;
	    else if (!set_pos (mt, TAPE_REWIND, 0, false))
	      prepare (mt, TAPE_TENSION);
	    break;
	  case MTBSFM:
	    set_pos (mt, TAPE_SPACE_FILEMARKS, -op->mt_count, true);
	    break;
	  case MTFSFM:
	    set_pos (mt, TAPE_SPACE_FILEMARKS, op->mt_count, true);
	    break;
	  case MTEOM:
	    if (fast_eom () && get_feature (TAPE_DRIVE_END_OF_DATA))
	      set_pos (mt, TAPE_SPACE_END_OF_DATA, 0, false);
	    else
	      set_pos (mt, TAPE_SPACE_FILEMARKS, 32767, false);
	    break;
	  case MTERASE:
	    erase (mt, TAPE_ERASE_LONG);
	    break;
	  case MTRAS1:
	  case MTRAS2:
	  case MTRAS3:
	    lasterr = ERROR_INVALID_PARAMETER;
	    break;
	  case MTSETBLK:
	    if (!get_feature (TAPE_DRIVE_SET_BLOCK_SIZE))
	      {
		lasterr = ERROR_INVALID_PARAMETER;
		break;
	      }
	    if ((DWORD) op->mt_count == mp ()->BlockSize)
	      {
		/* Nothing has changed. */
		lasterr = 0;
		break;
	      }
	    if ((op->mt_count == 0 && !get_feature (TAPE_DRIVE_VARIABLE_BLOCK))
		|| (op->mt_count > 0
		    && ((DWORD) op->mt_count < dp ()->MinimumBlockSize
			|| (DWORD) op->mt_count > dp ()->MaximumBlockSize)))
	      {
		lasterr = ERROR_INVALID_PARAMETER;
		break;
	      }
	    if (set_blocksize (mt, op->mt_count)
		&& lasterr == ERROR_INVALID_FUNCTION)
	      lasterr = ERROR_INVALID_BLOCK_LENGTH;
	    break;
	  case MTSEEK:
	    if (get_feature (TAPE_DRIVE_LOGICAL_BLK))
	      set_pos (mt, TAPE_LOGICAL_BLOCK, op->mt_count, false);
	    else if (!get_pos (mt))
	      set_pos (mt, TAPE_SPACE_RELATIVE_BLOCKS,
		       op->mt_count - block, false);
	    break;
	  case MTTELL:
	    if (!get_pos (mt))
	      op->mt_count = (int) block;
	    break;
	  case MTFSS:
	    set_pos (mt, TAPE_SPACE_SETMARKS, op->mt_count, false);
	    break;
	  case MTBSS:
	    set_pos (mt, TAPE_SPACE_SETMARKS, -op->mt_count, false);
	    break;
	  case MTWSM:
	    write_marks (mt, TAPE_SETMARKS, op->mt_count);
	    break;
	  case MTLOCK:
	    prepare (mt, TAPE_LOCK);
	    break;
	  case MTUNLOCK:
	    prepare (mt, TAPE_UNLOCK);
	    break;
	  case MTLOAD:
	    prepare (mt, TAPE_LOAD);
	    break;
	  case MTCOMPRESSION:
	    set_compression (mt, op->mt_count);
	    break;
	  case MTSETPART:
	    set_partition (mt, op->mt_count);
	    break;
	  case MTMKPART:
	    create_partitions (mt, op->mt_count);
	    break;
	  case MTSETDRVBUFFER:
	    set_options (mt, op->mt_count);
	    break;
	  case MTSETDENSITY:
	  default:
	    lasterr = ERROR_INVALID_PARAMETER;
	    break;
	}
    }
  else if (cmd == MTIOCGET)
    get_status (mt, (struct mtget *) buf);
  else if (cmd == MTIOCPOS && !get_pos (mt))
    ((struct mtpos *) buf)->mt_blkno = (long) block;

  return lasterr;
}

/**********************************************************************/
/* mtinfo */

void
mtinfo::initialize ()
{
  for (unsigned i = 0; i < MAX_DRIVE_NUM; ++i)
    drive (i)->initialize (i, true);
}

/**********************************************************************/
/* fhandler_dev_tape */

#define mt	(cygwin_shared->mt)

#define lock(err_ret_val) if (!_lock (false)) return (err_ret_val);

inline bool
fhandler_dev_tape::_lock (bool cancelable)
{
  /* O_NONBLOCK is only valid in a read or write call.  Only those are
     cancelable. */
  DWORD timeout = cancelable && is_nonblocking () ? 0 : INFINITE;
  switch (cygwait (mt_mtx, timeout, cw_sig | cw_cancel | cw_cancel_self))
    {
    case WAIT_OBJECT_0:
      return true;
    case WAIT_TIMEOUT:
      set_errno (EAGAIN);
      return false;
    default:
      __seterrno ();
      return false;
    }
}

inline int
fhandler_dev_tape::unlock (int ret)
{
  ReleaseMutex (mt_mtx);
  return ret;
}

fhandler_dev_tape::fhandler_dev_tape ()
  : fhandler_dev_raw ()
{
  debug_printf ("unit: %d", dev ().get_minor ());
}

int
fhandler_dev_tape::open (int flags, mode_t)
{
  int ret;

  if (driveno () >= MAX_DRIVE_NUM)
    {
      set_errno (ENOENT);
      return 0;
    }
  if (!(mt_mtx = CreateMutex (&sec_all, !!(flags & O_CLOEXEC), NULL)))
    {
      __seterrno ();
      return 0;
    }

  /* The O_SYNC flag is not supported by the tape driver.  Use the
     MT_ST_BUFFER_WRITES and MT_ST_ASYNC_WRITES flags in the drive
     settings instead.  In turn, the MT_ST_BUFFER_WRITES is translated
     into O_SYNC, which controls the FILE_WRITE_THROUGH flag in the
     NtCreateFile call in fhandler_base::open. */
  flags &= ~O_SYNC;
  if (!mt.drive (driveno ())->buffer_writes ())
    flags |= O_SYNC;

  ret = fhandler_dev_raw::open (flags);
  if (ret)
    {
      mt.drive (driveno ())->open (get_handle ());

      /* In append mode, seek to beginning of next filemark */
      if (flags & O_APPEND)
	mt.drive (driveno ())->set_pos (get_handle (),
					 TAPE_SPACE_FILEMARKS, 1, true);

      if (!(flags & O_DIRECT))
	{
	  devbufsiz = mt.drive (driveno ())->dp ()->MaximumBlockSize;
	  devbufalign = 1;
	  devbufalloc = devbuf = new char [devbufsiz];
	}
    }
  else
    ReleaseMutex (mt_mtx);
  return ret;
}

int
fhandler_dev_tape::close ()
{
  int ret = 0;
  int cret = 0;

  if (!have_execed)
    {
      lock (-1);
      ret = mt.drive (driveno ())->close (get_handle (), is_rewind_device ());
      if (ret)
	__seterrno_from_win_error (ret);
      cret = fhandler_dev_raw::close ();
      unlock (0);
    }
  if (ov.hEvent)
    CloseHandle (ov.hEvent);
  CloseHandle (mt_mtx);
  return ret ? -1 : cret;
}

void __reg3
fhandler_dev_tape::raw_read (void *ptr, size_t &ulen)
{
  char *buf = (char *) ptr;
  size_t len = ulen;
  size_t block_size;
  size_t bytes_to_read;
  size_t bytes_read = 0;
  int ret = 0;

  if (lastblk_to_read ())
    {
      lastblk_to_read (false);
      ulen = 0;
      return;
    }
  if (!_lock (true))
    {
      ulen = (size_t) -1;
      return;
    }
  block_size = mt.drive (driveno ())->mp ()->BlockSize;
  if (devbuf)
    {
      if (devbufend > devbufstart)
	{
	  bytes_to_read = MIN (len, devbufend - devbufstart);
	  debug_printf ("read %lu bytes from buffer (rest %lu)",
			bytes_to_read, devbufend - devbufstart - bytes_to_read);
	  memcpy (buf, devbuf + devbufstart, bytes_to_read);
	  len -= bytes_to_read;
	  bytes_read += bytes_to_read;
	  buf += bytes_to_read;
	  devbufstart += bytes_to_read;
	  if (devbufstart == devbufend)
	    devbufstart = devbufend = 0;
	  /* If a switch to variable block_size occured, just return the buffer
	     remains until the buffer is empty, then proceed with usual variable
	     block size handling (one block per read call). */
	  if (!block_size)
	    len = 0;
	}
      if (len > 0)
	{
	  if (!ov.hEvent
	      && !(ov.hEvent = CreateEvent (&sec_none, TRUE, FALSE, NULL)))
	    debug_printf ("Creating event failed, %E");
	  size_t block_fit = !block_size ? len : rounddown(len,  block_size);
	  if (block_fit)
	    {
	      debug_printf ("read %lu bytes from tape (rest %lu)",
			    block_fit, len - block_fit);
	      ret = mt.drive (driveno ())->read (get_handle (), &ov, buf,
						 block_fit);
	      if (ret)
		__seterrno_from_win_error (ret);
	      else if (block_fit)
		{
		  len -= block_fit;
		  bytes_read += block_fit;
		  buf += block_fit;
		  /* Only one block in each read call, please. */
		  if (!block_size)
		    len = 0;
		}
	      else {
		len = 0;
		if (bytes_read)
		  lastblk_to_read (true);
	      }
	    }
	  if (!ret && len > 0)
	    {
	      debug_printf ("read %lu bytes from tape (one block)", block_size);
	      ret = mt.drive (driveno ())->read (get_handle (), &ov, devbuf,
						 block_size);
	      if (ret)
		__seterrno_from_win_error (ret);
	      else if (block_size)
		{
		  devbufstart = len;
		  devbufend = block_size;
		  bytes_read += len;
		  memcpy (buf, devbuf, len);
		}
	      else if (bytes_read)
		lastblk_to_read (true);
	    }
	}
    }
  else
    {
      if (!ov.hEvent
	  && !(ov.hEvent = CreateEvent (&sec_none, TRUE, FALSE, NULL)))
	debug_printf ("Creating event failed, %E");
      bytes_read = ulen;
      ret = mt.drive (driveno ())->read (get_handle (), &ov, ptr, bytes_read);
    }
  ulen = (ret ? (size_t) -1 : bytes_read);
  unlock ();
}

ssize_t __reg3
fhandler_dev_tape::raw_write (const void *ptr, size_t len)
{
  if (!_lock (true))
    return -1;
  if (!ov.hEvent && !(ov.hEvent = CreateEvent (&sec_none, TRUE, FALSE, NULL)))
    debug_printf ("Creating event failed, %E");
  int ret = mt.drive (driveno ())->write (get_handle (), &ov, ptr, len);
  if (ret)
    __seterrno_from_win_error (ret);
  return unlock (ret ? -1 : (int) len);
}

off_t
fhandler_dev_tape::lseek (off_t offset, int whence)
{
#if 1
  /* On Linux lseek on tapes is a no-op.  For now, let's keep the old code
     intact but commented out, should incompatibilities arise. */
  return 0;
#else
  struct mtop op;
  struct mtpos pos;
  DWORD block_size;
  off_t ret = ILLEGAL_SEEK;

  lock (ILLEGAL_SEEK);

  debug_printf ("lseek (%s, %D, %d)", get_name (), offset, whence);

  block_size = mt.drive (driveno ())->mp ()->BlockSize;
  if (block_size == 0)
    {
      set_errno (EIO);
      goto out;
    }

  if (ioctl (MTIOCPOS, &pos))
    goto out;

  switch (whence)
    {
      case SEEK_END:
	op.mt_op = MTFSF;
	op.mt_count = 1;
	if (ioctl (MTIOCTOP, &op))
	  goto out;
	break;
      case SEEK_SET:
	if (whence == SEEK_SET && offset < 0)
	  {
	    set_errno (EINVAL);
	    goto out;
	  }
	break;
      case SEEK_CUR:
	break;
      default:
	set_errno (EINVAL);
	goto out;
    }

  op.mt_op = MTFSR;
  op.mt_count = offset / block_size
		- (whence == SEEK_SET ? pos.mt_blkno : 0);

  if (op.mt_count < 0)
    {
      op.mt_op = MTBSR;
      op.mt_count = -op.mt_count;
    }

  if (ioctl (MTIOCTOP, &op) || ioctl (MTIOCPOS, &pos))
    goto out;

  ret = pos.mt_blkno * block_size;

out:
  return unlock (ret);
#endif
}

int __reg2
fhandler_dev_tape::fstat (struct stat *buf)
{
  int ret;

  if (driveno () >= MAX_DRIVE_NUM)
    {
      set_errno (ENOENT);
      return -1;
    }
  if (!(ret = fhandler_base::fstat (buf)))
    buf->st_blocks = 0;
  return ret;
}

int
fhandler_dev_tape::dup (fhandler_base *child, int flags)
{
  lock (-1);
  fhandler_dev_tape *fh = (fhandler_dev_tape *) child;
  if (!DuplicateHandle (GetCurrentProcess (), mt_mtx,
			GetCurrentProcess (), &fh->mt_mtx,
			0, TRUE, DUPLICATE_SAME_ACCESS))
    {
      debug_printf ("dup(%s) failed, mutex handle %p, %E",
		    get_name (), mt_mtx);
      __seterrno ();
      return unlock (-1);
    }
  fh->ov.hEvent = NULL;
  if (ov.hEvent &&
      !DuplicateHandle (GetCurrentProcess (), ov.hEvent,
			GetCurrentProcess (), &fh->ov.hEvent,
			0, TRUE, DUPLICATE_SAME_ACCESS))
    {
      debug_printf ("dup(%s) failed, event handle %p, %E",
		    get_name (), ov.hEvent);
      __seterrno ();
      return unlock (-1);
    }
  return unlock (fhandler_dev_raw::dup (child, flags));
}

void
fhandler_dev_tape::fixup_after_fork (HANDLE parent)
{
  fhandler_dev_raw::fixup_after_fork (parent);
  fork_fixup (parent, mt_mtx, "mt_mtx");
  if (ov.hEvent)
    fork_fixup (parent, ov.hEvent, "ov.hEvent");
}

void
fhandler_dev_tape::set_close_on_exec (bool val)
{
  fhandler_dev_raw::set_close_on_exec (val);
  set_no_inheritance (mt_mtx, val);
  if (ov.hEvent)
    set_no_inheritance (ov.hEvent, val);
}

int
fhandler_dev_tape::ioctl (unsigned int cmd, void *buf)
{
  int ret = 0;
  lock (-1);
  if (cmd == MTIOCTOP || cmd == MTIOCGET || cmd == MTIOCPOS)
    {
      ret = mt.drive (driveno ())->ioctl (get_handle (), cmd, buf);
      if (ret)
	__seterrno_from_win_error (ret);
      return unlock (ret ? -1 : 0);
    }
  return unlock (fhandler_dev_raw::ioctl (cmd, buf));
}

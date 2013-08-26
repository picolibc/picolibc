/* mtinfo.h: Defininitions for the Cygwin tape driver class.

   Copyright 2004, 2005, 2006, 2008, 2012, 2013 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

/* Maximum number of supported partitions per drive. */
#define MAX_PARTITION_NUM	64
/* Maximum number of supported drives. */
#define MAX_DRIVE_NUM		8

/* Values for bookkeeping of the tape position relative to filemarks
   and eod/eom.  */
enum eom_val
{
  no_eof,
  eof_hit,
  eof,
  eod_hit,
  eod,
  eom_hit,
  eom
};

enum dirty_state
{
  clean,
  has_read,
  has_written,
  async_write_pending
};

enum lock_state
{
  unlocked,
  lock_error,
  auto_locked,
  locked
};

/* Partition specific information */
class mtinfo_part
{
public:
  int64_t block;	/* logical block no */
  int64_t fblock;	/* relative block no */
  int32_t file;		/* current file no */
  bool smark;		/* At setmark? */
  eom_val emark;	/* "end-of"-mark */

  void initialize (int64_t nblock = -1);
};

class mtinfo_drive
{
  int drive;
  int lasterr;
  int32_t partition;
  int64_t block;
  dirty_state dirty;
  lock_state lock;
  TAPE_GET_DRIVE_PARAMETERS _dp;
  TAPE_GET_MEDIA_PARAMETERS _mp;
  /* sizeof(OVERLAPPED) == 20 on 32 bit, 32 on 64 bit.  A drive is always
     opened exclusively by a single process, though, so instead of the
     OVERLAPPED structure, we just keep track of the pointer to the
     OVERLAPPED structure in the application's fhandler. */
  LPOVERLAPPED ov;
  struct status_flags
  {
    unsigned buffer_writes : 1;
    unsigned async_writes  : 1;
    unsigned two_fm        : 1;
    unsigned fast_eom      : 1;
    unsigned auto_lock     : 1;
    unsigned sysv          : 1;
    unsigned nowait        : 1;
  } status;
  mtinfo_part _part[MAX_PARTITION_NUM];

  inline int error (const char *str)
    {
      if (lasterr)
	debug_printf ("%s: Win32 error %d", str, lasterr);
      return lasterr;
    }
  inline bool get_feature (DWORD parm)
    {
      return ((parm & TAPE_DRIVE_HIGH_FEATURES)
	      ? ((_dp.FeaturesHigh & parm) != 0)
	      : ((_dp.FeaturesLow & parm) != 0));
    }
  int get_pos (HANDLE mt, int32_t *ppartition = NULL, int64_t *pblock = NULL);
  int _set_pos (HANDLE mt, int mode, int64_t count, int partition, BOOL dont_wait);
  int create_partitions (HANDLE mt, int32_t count);
  int set_partition (HANDLE mt, int32_t count);
  int write_marks (HANDLE mt, int marktype, DWORD count);
  int erase (HANDLE mt, int mode);
  int prepare (HANDLE mt, int action, bool is_auto = false);
  int set_compression (HANDLE mt, int32_t count);
  int set_blocksize (HANDLE mt, DWORD count);
  int get_status (HANDLE mt, struct mtget *get);
  int set_options (HANDLE mt, int32_t options);
  int async_wait (HANDLE mt, DWORD *bytes_written);

public:
  void initialize (int num, bool first_time);
  int get_dp (HANDLE mt);
  int get_mp (HANDLE mt);
  int open (HANDLE mt);
  int close (HANDLE mt, bool rewind);
  int read (HANDLE mt, LPOVERLAPPED pov, void *ptr, size_t &ulen);
  int write (HANDLE mt, LPOVERLAPPED pov, const void *ptr, size_t &len);
  int ioctl (HANDLE mt, unsigned int cmd, void *buf);
  int set_pos (HANDLE mt, int mode, int64_t count, bool sfm_func);

  IMPLEMENT_STATUS_FLAG (bool, buffer_writes)
  IMPLEMENT_STATUS_FLAG (bool, async_writes)
  IMPLEMENT_STATUS_FLAG (bool, two_fm)
  IMPLEMENT_STATUS_FLAG (bool, fast_eom)
  IMPLEMENT_STATUS_FLAG (bool, auto_lock)
  IMPLEMENT_STATUS_FLAG (bool, sysv)
  IMPLEMENT_STATUS_FLAG (bool, nowait)

  PTAPE_GET_DRIVE_PARAMETERS dp () { return &_dp; }
  PTAPE_GET_MEDIA_PARAMETERS mp () { return &_mp; }
  mtinfo_part *part (int num) { return &_part[num]; }
};

class mtinfo
{
  mtinfo_drive _drive[MAX_DRIVE_NUM];

public:
  void initialize ();
  mtinfo_drive *drive (int num) { return &_drive[num]; }
};

#include <machine/syscall.h>
#include "kernel_stat.h"

/* Convert linux's stat64 sturct to newlib's stat.  */
void
_conv_stat (struct stat *st, struct kernel_stat *kst)
{
  st->st_dev = kst->st_dev;
  st->st_ino = kst->st_ino;
  st->st_mode = kst->st_mode;
  st->st_nlink = kst->st_nlink;
  st->st_uid = kst->st_uid;
  st->st_gid = kst->st_gid;
  st->st_rdev = kst->st_rdev;
  st->st_size = kst->st_size;
  st->st_blocks = kst->st_blocks;
  st->st_blksize = kst->st_blksize;
  st->st_atime = kst->st_atim.tv_sec;
  st->st_mtime = kst->st_mtim.tv_sec;
  st->st_ctime = kst->st_ctim.tv_sec;
}

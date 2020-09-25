/*
 * io-lseek.c -- 
 *
 * Copyright (c) 2006 CodeSourcery Inc
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#define IO lseek
#include "io.h"

/*
 * lseek -- reposition a file descriptor
 * input parameters:
 *   0 : file descriptor
 *   1 : offset
 *   2 : seek flag
 * output parameters:
 *   0 : errno
 * result : r0
 */

off_t lseek (int fd, off_t offset, int whence)
{
#if HOSTED
  gdb_parambuf_t parameters;
  int ret;
  parameters[0] = (uint32_t) fd;
  parameters[1] = (uint32_t) offset;
  parameters[2] = __hosted_to_gdb_lseek_flags (whence);
  ret = __hosted (HOSTED_LSEEK, parameters);
  errno = __hosted_from_gdb_errno (parameters[0]);
  return ret;
#else
  errno = ENOSYS;
  return -1;
#endif
}

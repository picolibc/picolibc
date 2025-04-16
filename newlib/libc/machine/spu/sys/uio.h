/* Copyright (c) 2007 Patrick Mansfield <patmans@us.ibm.com> */
#ifndef _SYS_UIO_H
#define _SYS_UIO_H

#include <sys/types.h>

_BEGIN_STD_C

/*
 * Per POSIX
 */

struct iovec {
  void   *iov_base;
  size_t  iov_len;
};

ssize_t readv(int, const struct iovec *, int);
ssize_t writev(int, const struct iovec *, int);

_END_STD_C

#endif

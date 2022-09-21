/*
 * Support file for nvptx in newlib.
 * Copyright (c) 2014-2018 Mentor Graphics.
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

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#undef errno
extern int errno;

int
close(int fd) {
  return -1;
}

int
fstat (int fd, struct stat *buf) {
  return -1;
}

int
gettimeofday (struct timeval *tv, void *tz) {
  return -1;
}

int
getpid (void) {
  return 0;
}

int
isatty (int fd) {
  return fd == 1;
}

int
kill (int pid, int sig) {
  errno = ESRCH;
  return -1;
}

off_t
lseek(int fd, off_t offset, int whence) {
  return 0;
}

int
open (const char *pathname, int flags, ...) {
  return -1;
}

int
read(int fd, void *buf, size_t count) {
  return 0;
}

int
stat (const char *file, struct stat *pstat) {
  errno = EACCES;
  return -1;
}

void
sync (void) {
}

int
unlink (const char *pathname) {
  return -1;
}

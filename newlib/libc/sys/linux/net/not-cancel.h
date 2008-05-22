/* Uncancelable versions of cancelable interfaces.  Linux version.
   Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <unistd.h>
#include <fcntl.h>
#include <machine/syscall.h>

/* Uncancelable open.  */
#define open_not_cancel(name, flags, mode) open((name), (flags), (mode))
#define open_not_cancel_2(name, flags) open((name), (flags))

/* Uncancelable close.  */
#define close_not_cancel(fd) close((fd))
#define close_not_cancel_no_status(fd) (void)close((fd))

/* Uncancelable read.  */
#define read_not_cancel(fd, buf, n) read((fd), (buf), (n))

/* Uncancelable write.  */
#define write_not_cancel(fd, buf, n) write((fd), (buf), (n))

/* Uncancelable writev.  */
#define writev_not_cancel_no_status(fd, iov, n) \
  (void) writev((fd), (iov), (n))

/* Uncancelable fcntl.  */
#define fcntl_not_cancel(fd, cmd, val) fcntl((fd), (cmd), (val))

/* Uncancelable waitpid.  */
#ifdef __NR_waitpid
# define waitpid_not_cancel(pid, stat_loc, options) \
  waitpid((pid), (stat_loc), (options))
#else
# define waitpid_not_cancel(pid, stat_loc, options) \
  wait4((pid), (stat_loc), (options), NULL)
#endif

/* libc/sys/linux/select.c - The select system calls */

/* Written 2000 by Werner Almesberger */


#include <sys/types.h>
#include <sys/time.h>
#include <sys/syscall.h>


_syscall5(int,select,int,n,fd_set *,readfds,fd_set *,writefds,
  fd_set *,exceptfds,struct timeval *,timeout)

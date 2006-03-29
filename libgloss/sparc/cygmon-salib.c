#ifdef TARGET_CPU_SPARC64
#include <sys/types.h>
#endif
#include <sys/time.h>

void
putTtyChar(int c)
{
  /* 2 is fork under solaris; bad juju */
  asm("	mov %i0,%o0
	or %g0,2,%g1
	ta 8
	nop");
}

int
write(int fd,char *ptr,int amt)
{
  if (fd < 0 || fd > 2)
    {
      return -1;
    }
  asm(" or %g0, 4, %g1
	ta 8
	nop");
  return amt;
}

int
read(int fd,char *ptr,int amt)
{
  if (fd < 0 || fd > 2)
    {
      return -1;
    }
  asm(" or %g0, 3, %g1
	ta 8
	nop");
  return amt;
}

void
_exit(int code)
{
  while(1) {
    asm("	or %g0,1,%g1
		ta 8
		nop
		ta 1
		nop");
  }
}

int
setitimer(int which, const struct itimerval *value, struct itimerval *ovalue)
{
  asm(" or %g0, 158, %g1
	ta 8
	nop");
}



long
sbrk (unsigned long amt)
{
  extern char _end;
  static char *ptr = 0;
  char *res;
  if (ptr == 0)
    ptr = &_end;
  if (amt == 0)
    return (long)ptr;

  if (((long)ptr) % 8)
    ptr = ptr + (8 - (((long)(ptr)) % 8));
  res = ptr;
  ptr += amt;
  return (long)res;
}

#ifdef TARGET_CPU_SPARC64
long
_sbrk_r (void *foo, unsigned long amt)
{
  return sbrk(amt);
}

long
_fstat_r (void *foo, void *bar, void *baz)
{
  return -1;
}

long
_brk_r (void *foo)
{
  return sbrk(0);
}

int
_open_r (char *filename, int mode)
{
  return open (filename, mode);
}

int
_close_r (int fd)
{
  return close(fd);
}
#endif

int
close (int fd)
{
  return 0;
}

int
fstat(int des,void *buf)
{
  return -1;
}

int
lseek(int des,unsigned long offset, int whence)
{
  return -1;
}

int
isatty(int fd)
{
  return (fd < 3);
}

int
kill (int pid, int signal)
{
  asm ("or %g0, 37, %g1
	ta 8
	nop");
}

int
getpid ()
{
  return -1;
}

int
getitimer (int which, struct itimerval *value)
{
  asm ("or %g0, 157, %g1
	ta 8
	nop");
}

void
__install_signal_handler (void *func)
{
  asm ("mov %o0, %o1
	mov %g0, %o0
	or %g0, 48, %g1
	ta 8
	nop");
}

int
gettimeofday (struct timeval *tp, struct timezone *tzp)
{
  asm ("or %g0, 156, %g1
	ta 8
	nop");
}

int
stime (long *seconds)
{
  asm ("or %g0, 25, %g1
	ta 8
	nop");
}

int
add_mapping (long vma, long pma, long size)
{
  asm ("or %g0, 115, %g1
	ta 8
	nop");
}

int
remove_mapping (long vma, long vma_end)
{
  asm ("or %g0, 117, %g1
	ta 8
	nop");
}

int
open (char *filename, int mode)
{
  return -1;
}

void *
__getProgramArgs (int *argv)
{
  int *res;

  /* 184 is tsolsys under solaris; bad juju */
  asm ("mov %1, %%o0
	or %%g0, 184, %%g1
	ta 8
	nop
	mov %%o0, %0" : "=r" (res) : "r" (argv): "g1");
  return res;
}

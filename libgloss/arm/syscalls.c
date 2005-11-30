/* Support files for GNU libc.  Files in the system namespace go here.
   Files in the C namespace (ie those that do not start with an
   underscore) go in .c.  */

#include <_ansi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <errno.h>
#include <reent.h>
#include <unistd.h>
#include "swi.h"

/* Forward prototypes.  */
int     _system     _PARAMS ((const char *));
int     _rename     _PARAMS ((const char *, const char *));
int     _isatty		_PARAMS ((int));
clock_t _times		_PARAMS ((struct tms *));
int     _gettimeofday	_PARAMS ((struct timeval *, struct timezone *));
int     _unlink		_PARAMS ((const char *));
int     _link 		_PARAMS ((void));
int     _stat 		_PARAMS ((const char *, struct stat *));
int     _fstat 		_PARAMS ((int, struct stat *));
caddr_t _sbrk		_PARAMS ((int));
int     _getpid		_PARAMS ((int));
int     _kill		_PARAMS ((int, int));
void    _exit		_PARAMS ((int));
int     _close		_PARAMS ((int));
clock_t _clock		_PARAMS ((void));
int     _swiclose	_PARAMS ((int));
int     _open		_PARAMS ((const char *, int, ...));
int     _swiopen	_PARAMS ((const char *, int));
int     _write 		_PARAMS ((int, char *, int));
int     _swiwrite	_PARAMS ((int, char *, int));
int     _lseek		_PARAMS ((int, int, int));
int     _swilseek	_PARAMS ((int, int, int));
int     _read		_PARAMS ((int, char *, int));
int     _swiread	_PARAMS ((int, char *, int));
void    initialise_monitor_handles _PARAMS ((void));

static int	wrap		_PARAMS ((int));
static int	error		_PARAMS ((int));
static int	get_errno	_PARAMS ((void));
static int	remap_handle	_PARAMS ((int));
static int	do_AngelSWI	_PARAMS ((int, void *));
static int 	findslot	_PARAMS ((int));

/* Register name faking - works in collusion with the linker.  */
register char * stack_ptr asm ("sp");


/* following is copied from libc/stdio/local.h to check std streams */
extern void   _EXFUN(__sinit,(struct _reent *));
#define CHECK_INIT(ptr) \
  do						\
    {						\
      if ((ptr) && !(ptr)->__sdidinit)		\
	__sinit (ptr);				\
    }						\
  while (0)

/* Adjust our internal handles to stay away from std* handles.  */
#define FILE_HANDLE_OFFSET (0x20)

static int monitor_stdin;
static int monitor_stdout;
static int monitor_stderr;

/* Struct used to keep track of the file position, just so we
   can implement fseek(fh,x,SEEK_CUR).  */
typedef struct
{
  int handle;
  int pos;
}
poslog;

#define MAX_OPEN_FILES 20
static poslog openfiles [MAX_OPEN_FILES];

static int
findslot (int fh)
{
  int i;
  for (i = 0; i < MAX_OPEN_FILES; i ++)
    if (openfiles[i].handle == fh)
      break;
  return i;
}

#ifdef ARM_RDI_MONITOR

static inline int
do_AngelSWI (int reason, void * arg)
{
  int value;
  asm volatile ("mov r0, %1; mov r1, %2; swi %a3; mov %0, r0"
       : "=r" (value) /* Outputs */
       : "r" (reason), "r" (arg), "i" (AngelSWI) /* Inputs */
       : "r0", "r1", "r2", "r3", "ip", "lr", "memory", "cc"
		/* Clobbers r0 and r1, and lr if in supervisor mode */);
                /* Accordingly to page 13-77 of ARM DUI 0040D other registers
                   can also be clobbered.  Some memory positions may also be
                   changed by a system call, so they should not be kept in
                   registers. Note: we are assuming the manual is right and
                   Angel is respecting the APCS.  */
  return value;
}
#endif /* ARM_RDI_MONITOR */

/* Function to convert std(in|out|err) handles to internal versions.  */
static int
remap_handle (int fh)
{
  CHECK_INIT(_REENT);

  if (fh == STDIN_FILENO)
    return monitor_stdin;
  if (fh == STDOUT_FILENO)
    return monitor_stdout;
  if (fh == STDERR_FILENO)
    return monitor_stderr;

  return fh - FILE_HANDLE_OFFSET;
}

void
initialise_monitor_handles (void)
{
  int i;
  
  /* Open the standard file descriptors by opening the special
   * teletype device, ":tt", read-only to obtain a descritpor for
   * standard input and write-only to obtain a descriptor for standard
   * output. Finally, open ":tt" in append mode to obtain a descriptor
   * for standard error. Since this is a write mode, most kernels will
   * probably return the same value as for standard output, but the
   * kernel can differentiate the two using the mode flag and return a
   * different descriptor for standard error.
   */

#ifdef ARM_RDI_MONITOR
  int volatile block[3];
  
  block[0] = (int) ":tt";
  block[2] = 3;     /* length of filename */
  block[1] = 0;     /* mode "r" */
  monitor_stdin = do_AngelSWI (AngelSWI_Reason_Open, (void *) block);

  block[0] = (int) ":tt";
  block[2] = 3;     /* length of filename */
  block[1] = 4;     /* mode "w" */
  monitor_stdout = do_AngelSWI (AngelSWI_Reason_Open, (void *) block);

  block[0] = (int) ":tt";
  block[2] = 3;     /* length of filename */
  block[1] = 8;     /* mode "a" */
  monitor_stderr = do_AngelSWI (AngelSWI_Reason_Open, (void *) block);
#else
  int fh;
  const char * name;

  name = ":tt";
  asm ("mov r0,%2; mov r1, #0; swi %a1; mov %0, r0"
       : "=r"(fh)
       : "i" (SWI_Open),"r"(name)
       : "r0","r1");
  monitor_stdin = fh;

  name = ":tt";
  asm ("mov r0,%2; mov r1, #4; swi %a1; mov %0, r0"
       : "=r"(fh)
       : "i" (SWI_Open),"r"(name)
       : "r0","r1");
  monitor_stdout = fh;

  name = ":tt";
  asm ("mov r0,%2; mov r1, #8; swi %a1; mov %0, r0"
       : "=r"(fh)
       : "i" (SWI_Open),"r"(name)
       : "r0","r1");
  monitor_stderr = fh;
#endif

  for (i = 0; i < MAX_OPEN_FILES; i ++)
    openfiles[i].handle = -1;

  openfiles[0].handle = monitor_stdin;
  openfiles[0].pos = 0;
  openfiles[1].handle = monitor_stdout;
  openfiles[1].pos = 0;
  openfiles[2].handle = monitor_stderr;
  openfiles[2].pos = 0;
}

static int
get_errno (void)
{
#ifdef ARM_RDI_MONITOR
  return do_AngelSWI (AngelSWI_Reason_Errno, NULL);
#else
  register r0 asm("r0");
  asm ("swi %a1" : "=r"(r0) : "i" (SWI_GetErrno));
  return r0;
#endif
}

static int
error (int result)
{
  errno = get_errno ();
  return result;
}

static int
wrap (int result)
{
  if (result == -1)
    return error (-1);
  return result;
}

/* Returns # chars not! written.  */
int
_swiread (int file,
	  char * ptr,
	  int len)
{
  int fh = remap_handle (file);
#ifdef ARM_RDI_MONITOR
  int block[3];
  
  block[0] = fh;
  block[1] = (int) ptr;
  block[2] = len;
  
  return do_AngelSWI (AngelSWI_Reason_Read, block);
#else
  asm ("mov r0, %1; mov r1, %2;mov r2, %3; swi %a0"
       : /* No outputs */
       : "i"(SWI_Read), "r"(fh), "r"(ptr), "r"(len)
       : "r0","r1","r2");
#endif
}

int
_read (int file,
       char * ptr,
       int len)
{
  int slot = findslot (remap_handle (file));
  int x = _swiread (file, ptr, len);

  if (x < 0)
    return error (-1);

  if (slot != MAX_OPEN_FILES)
    openfiles [slot].pos += len - x;

  /* x == len is not an error, at least if we want feof() to work.  */
  return len - x;
}

int
_swilseek (int file,
	   int ptr,
	   int dir)
{
  int res;
  int fh = remap_handle (file);
  int slot = findslot (fh);
#ifdef ARM_RDI_MONITOR
  int block[2];
#endif

  if (dir == SEEK_CUR)
    {
      if (slot == MAX_OPEN_FILES)
	return -1;
      ptr = openfiles[slot].pos + ptr;
      dir = SEEK_SET;
    }
  
#ifdef ARM_RDI_MONITOR
  if (dir == SEEK_END)
    {
      block[0] = fh;
      ptr += do_AngelSWI (AngelSWI_Reason_FLen, block);
    }
  
  /* This code only does absolute seeks.  */
  block[0] = remap_handle (file);
  block[1] = ptr;
  res = do_AngelSWI (AngelSWI_Reason_Seek, block);
#else
  if (dir == SEEK_END)
    {
      asm ("mov r0, %2; swi %a1; mov %0, r0"
	   : "=r" (res)
	   : "i" (SWI_Flen), "r" (fh)
	   : "r0");
      ptr += res;
    }

  /* This code only does absolute seeks.  */
  asm ("mov r0, %2; mov r1, %3; swi %a1; mov %0, r0"
       : "=r" (res)
       : "i" (SWI_Seek), "r" (fh), "r" (ptr)
       : "r0", "r1");
#endif

  if (slot != MAX_OPEN_FILES && res == 0)
    openfiles[slot].pos = ptr;

  /* This is expected to return the position in the file.  */
  return res == 0 ? ptr : -1;
}

int
_lseek (int file,
	int ptr,
	int dir)
{
  return wrap (_swilseek (file, ptr, dir));
}

/* Returns #chars not! written.  */
int
_swiwrite (
	   int    file,
	   char * ptr,
	   int    len)
{
  int fh = remap_handle (file);
#ifdef ARM_RDI_MONITOR
  int block[3];
  
  block[0] = fh;
  block[1] = (int) ptr;
  block[2] = len;
  
  return do_AngelSWI (AngelSWI_Reason_Write, block);
#else
  asm ("mov r0, %1; mov r1, %2;mov r2, %3; swi %a0"
       : /* No outputs */
       : "i"(SWI_Write), "r"(fh), "r"(ptr), "r"(len)
       : "r0","r1","r2");
#endif
}

int
_write (int    file,
	char * ptr,
	int    len)
{
  int slot = findslot (remap_handle (file));
  int x = _swiwrite (file, ptr,len);

  if (x == -1 || x == len)
    return error (-1);
  
  if (slot != MAX_OPEN_FILES)
    openfiles[slot].pos += len - x;
  
  return len - x;
}

int
_swiopen (const char * path,
	  int          flags)
{
  int aflags = 0, fh;
#ifdef ARM_RDI_MONITOR
  int block[3];
#endif
  
  int i = findslot (-1);
  
  if (i == MAX_OPEN_FILES)
    return -1;

  /* The flags are Unix-style, so we need to convert them.  */
#ifdef O_BINARY
  if (flags & O_BINARY)
    aflags |= 1;
#endif

  if (flags & O_RDWR)
    aflags |= 2;

  if (flags & O_CREAT)
    aflags |= 4;

  if (flags & O_TRUNC)
    aflags |= 4;

  if (flags & O_APPEND)
    {
      aflags &= ~4;     /* Can't ask for w AND a; means just 'a'.  */
      aflags |= 8;
    }
  
#ifdef ARM_RDI_MONITOR
  block[0] = (int) path;
  block[2] = strlen (path);
  block[1] = aflags;
  
  fh = do_AngelSWI (AngelSWI_Reason_Open, block);
  
#else
  asm ("mov r0,%2; mov r1, %3; swi %a1; mov %0, r0"
       : "=r"(fh)
       : "i" (SWI_Open),"r"(path),"r"(aflags)
       : "r0","r1");
#endif
  
  if (fh >= 0)
    {
      openfiles[i].handle = fh;
      openfiles[i].pos = 0;
    }

  return fh >= 0 ? fh + FILE_HANDLE_OFFSET : error (fh);
}

int
_open (const char * path,
       int          flags,
       ...)
{
  return wrap (_swiopen (path, flags));
}

int
_swiclose (int file)
{
  int myhan = remap_handle (file);
  int slot = findslot (myhan);
  
  if (slot != MAX_OPEN_FILES)
    openfiles[slot].handle = -1;

#ifdef ARM_RDI_MONITOR
  return do_AngelSWI (AngelSWI_Reason_Close, & myhan);
#else
  asm ("mov r0, %1; swi %a0" :: "i" (SWI_Close),"r"(myhan):"r0");
#endif
}

int
_close (int file)
{
  return wrap (_swiclose (file));
}

int
_kill (int pid, int sig)
{
  (void)pid; (void)sig;
#ifdef ARM_RDI_MONITOR
  /* Note: Both arguments are thrown away.  */
  return do_AngelSWI (AngelSWI_Reason_ReportException,
		      (void *) ADP_Stopped_ApplicationExit);
#else
  asm ("swi %a0" :: "i" (SWI_Exit));
#endif
}

void
_exit (int status)
{
  /* There is only one SWI for both _exit and _kill. For _exit, call
     the SWI with the second argument set to -1, an invalid value for
     signum, so that the SWI handler can distinguish the two calls.
     Note: The RDI implementation of _kill throws away both its
     arguments.  */
  _kill(status, -1);
}

int __attribute__((weak))
_getpid (int n)
{
  return 1;
  n = n;
}

caddr_t
_sbrk (int incr)
{
  extern char   end asm ("end");	/* Defined by the linker.  */
  static char * heap_end;
  char *        prev_heap_end;

  if (heap_end == NULL)
    heap_end = & end;
  
  prev_heap_end = heap_end;
  
  if (heap_end + incr > stack_ptr)
    {
      /* Some of the libstdc++-v3 tests rely upon detecting
	 out of memory errors, so do not abort here.  */
#if 0
      extern void abort (void);

      _write (1, "_sbrk: Heap and stack collision\n", 32);
      
      abort ();
#else
      errno = ENOMEM;
      return (caddr_t) -1;
#endif
    }
  
  heap_end += incr;

  return (caddr_t) prev_heap_end;
}

int __attribute__((weak))
_fstat (int file, struct stat * st)
{
  memset (st, 0, sizeof (* st));
  st->st_mode = S_IFCHR;
  st->st_blksize = 1024;
  return 0;
  file = file;
}

int __attribute__((weak))
_stat (const char *fname, struct stat *st)
{
  int file;

  /* The best we can do is try to open the file readonly.  If it exists,
     then we can guess a few things about it.  */
  if ((file = _open (fname, O_RDONLY)) < 0)
    return -1;

  memset (st, 0, sizeof (* st));
  st->st_mode = S_IFREG | S_IREAD;
  st->st_blksize = 1024;
  _swiclose (file); /* Not interested in the error.  */
  return 0;
}

int __attribute__((weak))
_link (void)
{
  errno = ENOSYS;
  return -1;
}

int
_unlink (const char *path)
{
#ifdef ARM_RDI_MONITOR
  return do_AngelSWI (AngelSWI_Reason_Remove, &path);
#else
  (void)path;
  asm ("swi %a0" :: "i" (SWI_Remove));
#endif
}

int
_gettimeofday (struct timeval * tp, struct timezone * tzp)
{

  if (tp)
    {
    /* Ask the host for the seconds since the Unix epoch.  */
#ifdef ARM_RDI_MONITOR
      tp->tv_sec = do_AngelSWI (AngelSWI_Reason_Time,NULL);
#else
      {
        int value;
        asm ("swi %a1; mov %0, r0" : "=r" (value): "i" (SWI_Time) : "r0");
        tp->tv_sec = value;
      }
#endif
      tp->tv_usec = 0;
    }

  /* Return fixed data for the timezone.  */
  if (tzp)
    {
      tzp->tz_minuteswest = 0;
      tzp->tz_dsttime = 0;
    }

  return 0;
}

/* Return a clock that ticks at 100Hz.  */
clock_t 
_clock (void)
{
  clock_t timeval;

#ifdef ARM_RDI_MONITOR
  timeval = do_AngelSWI (AngelSWI_Reason_Clock,NULL);
#else
  asm ("swi %a1; mov %0, r0" : "=r" (timeval): "i" (SWI_Clock) : "r0");
#endif
  return timeval;
}

/* Return a clock that ticks at 100Hz.  */
clock_t
_times (struct tms * tp)
{
  clock_t timeval = _clock();

  if (tp)
    {
      tp->tms_utime  = timeval;	/* user time */
      tp->tms_stime  = 0;	/* system time */
      tp->tms_cutime = 0;	/* user time, children */
      tp->tms_cstime = 0;	/* system time, children */
    }
  
  return timeval;
};


int
_isatty (int fd)
{
#ifdef ARM_RDI_MONITOR
  return do_AngelSWI (AngelSWI_Reason_IsTTY, &fd);
#else
  (void)fd;
  asm ("swi %a0" :: "i" (SWI_IsTTY));
#endif
}

int
_system (const char *s)
{
#ifdef ARM_RDI_MONITOR
  return do_AngelSWI (AngelSWI_Reason_System, &s);
#else
  (void)s;
  asm ("swi %a0" :: "i" (SWI_CLI));
#endif
}

int
_rename (const char * oldpath, const char * newpath)
{
#ifdef ARM_RDI_MONITOR
  const char *block[2] = {oldpath, newpath};
  return do_AngelSWI (AngelSWI_Reason_Rename, block);
#else
  (void)oldpath; (void)newpath;
  asm ("swi %a0" :: "i" (SWI_Rename));
#endif
}

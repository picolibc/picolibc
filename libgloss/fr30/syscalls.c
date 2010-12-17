/* FR30 system call emulation code
   Copyright (C) 1998, 2010 Free Software Foundation, Inc.
   Contributed by Cygnus Solutions.  */

#include <sys/stat.h>
#include "../syscall.h"

int
_read (file, ptr, len)
     int    file;
     char * ptr;
     int    len;
{
  asm ("ldi:8 %0, r0" :: "i" (SYS_read) : "r0");
  asm ("int   #10");
  
  return;
}

int
_lseek (file, ptr, dir)
     int file;
     int ptr;
     int dir;
{
  asm ("ldi:8 %0, r0" :: "i" (SYS_lseek) : "r0");
  asm ("int   #10");
  
  return;
}

int
_write (file, ptr, len)
     int    file;
     char * ptr;
     int    len;
{
  asm ("ldi:8 %0, r0" :: "i" (SYS_write) : "r0");
  asm ("int   #10");
  
  return;
}

int
_open (path, flags)
     const char * path;
     int flags;
{
  asm ("ldi:8  %0, r0" :: "i" (SYS_open) : "r0");
  asm ("int    #10");
  
  return;
}

int
_close (file)
     int file;
{
  asm ("ldi:8  %0, r0" :: "i" (SYS_close) : "r0");
  asm ("int    #10");
  
  return 0;
}

void
_exit (n)
     int n;
{
  asm ("ldi:8  %0, r0" :: "i" (SYS_exit) : "r0");
  asm ("int    #10");
}


caddr_t
_sbrk (incr)
     int incr;
{
  extern char   end asm ("_end");	/* Defined by the linker */
  extern int    __stack;                /* Defined by linker script.  */
  static char * heap_end;
  char *        prev_heap_end;

  if (heap_end == NULL)
    heap_end = & end;
  
  prev_heap_end = heap_end;
#if 0  
  if (heap_end + incr > __stack)
    {
      _write ( 1, "_sbrk: Heap and stack collision\n", 32);
      abort ();
    }
#endif
  heap_end += incr;

  return (caddr_t) prev_heap_end;
}

int
_fstat (file, st)
     int file;
     struct stat * st;
{
  st->st_mode = S_IFCHR;
  return 0;
}

int
_unlink ()
{
  return -1;
}

int
_isatty (fd)
     int fd;
{
  return 0;
}

int
_raise ()
{
  return 0;
}

int
_times ()
{
  return 0;
}

int
_kill (pid, sig)
     int pid;
     int sig;
{
  return 0;
}

int
_getpid (void)
{
  return 0;
}

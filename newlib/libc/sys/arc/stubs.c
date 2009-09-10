#include <sys/types.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <stdio.h>

extern char __start_heap;
extern char __end_heap;

void
_init()
{
}


extern void _exit_halt();
void
_exit(int i)
{
  _exit_halt();
}

int
_open(const char* path, int flag, int mode)
{
  return 0;
}


caddr_t 
_sbrk (size_t nbytes)
{
  static char* heap_ptr = NULL;
  char* prev_heap_ptr;

  if (heap_ptr == NULL) 
  {
    heap_ptr = &__start_heap;
  }

  /* Align the 'heap_ptr' so that memory will always be allocated at word
     boundaries. */
  heap_ptr = (char *) ((((unsigned long) heap_ptr) + 7) & ~7);
  prev_heap_ptr = heap_ptr;

  if ((heap_ptr + nbytes) < &__end_heap) 
  {
    heap_ptr += nbytes;
    return (caddr_t) prev_heap_ptr;
  } 

  errno = ENOMEM;
  return (caddr_t) -1;
}


int
_read(int handle, char *buffer, unsigned int length)
{
  return 0;
}

int
_write(int handle, const char *buffer, unsigned int length)
{
    return length;
}

int
_lseek(int file, int ptr, int dir)
{
  return 0;
}

int
_close(int handle)
{
  return -1; 
}

int
_fstat(int file, struct stat *st)
{
  st->st_mode = S_IFCHR;
  return 0;
}

int
isatty(int file)
{
  return 1; 
}

void
_kill(int pid, int val)
{
  _exit(val);
}

_getpid()
{
  return 1;
}

time_t
time(time_t *t)
{
  return -1;
}

int
_fork()
{
  return -1;
}

unsigned
sleep(unsigned n)
{
  return -1;
}

#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

sigset_t unblock_sigsegv;
jmp_buf r;
size_t pg;

int fd;

/* Checks behaviour of anonymous mmap.

   test_1: If we map a 2-page region and unmap its second page, the first page
   must remain.

   test_2: If we map a 2-page region and unmap its first page, the second
   page must remain.

   test_3: If we map two consecutive 1-page regions and unmap them both with
   one munmap, both must go away.
*/

void
perror_exit (const char *str, int code)
{
  printf ("%s: %s\n", str, strerror (errno));
  exit (code);
}

void
anonmap_init ()
{
  sigemptyset (&unblock_sigsegv);
  sigaddset (&unblock_sigsegv, SIGSEGV);
  pg = getpagesize ();
  fd = open ("/dev/zero", O_RDWR);
}

char *
anonmap (size_t size)
{
  return (char *) mmap (0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
}

void
anonfree (char *loc, size_t size)
{
  munmap (loc, size);
}
     
void
sigsegv (int unused)
{
  sigprocmask (SIG_UNBLOCK, &unblock_sigsegv, 0);
  longjmp (r, 1);
}

int
compare_pointers (const void *a, const void *b)
{
  const char *x = *(const char *const *)a;
  const char *y = *(const char *const *)b;

  if (x > y)
    return 1;
  if (x < y)
    return -1;
  return 0;
}

void
test_1 ()
{
  char *x = anonmap (pg * 2);
  if (x == (char *)MAP_FAILED)
    perror_exit ("test 1 mmap", 1);

  signal (SIGSEGV, sigsegv);
  if (setjmp (r))
    perror_exit ("test 1 fault", 2);

  x[0] = 1;
  x[pg] = 1;

  anonfree (x + pg, pg);
  x[0] = 2;

  if (setjmp (r) == 0)
    {
      x[pg] = 1;
      perror_exit ("test 1 no fault", 3);
    }
}

void
test_2 ()
{
  char *x = anonmap (pg * 2);
  if (x == (char *)MAP_FAILED)
    perror_exit ("test 2 mmap", 4);

  signal (SIGSEGV, sigsegv);
  if (setjmp (r))
    perror_exit ("test 2 fault", 5);

  x[0] = 1;
  x[pg] = 1;

  anonfree (x, pg);
  x[pg] = 2;

  if (setjmp (r) == 0)
    {
      x[0] = 1;
      perror_exit ("test 2 no fault", 6);
    }
}

void
test_3 ()
{
  char *x[10];
  char *y;
  int i;

  /* There's no way to guarantee we get consecutive pages from the OS.  The
     approach taken here is to allocate ten of them, sort the list, and
     look for consecutive pages.  */
  for (i = 0; i < 10; i++)
    {
      x[i] = anonmap (pg);
      if (x[i] == (char *)MAP_FAILED)
	perror_exit ("test 3 mmap 1", 7);
    }
  qsort (x, 10, sizeof (char *), compare_pointers);

  y = 0;
  for (i = 0; i < 9; i++)
    if (x[i] + pg == x[i+1])
      {
	y = x[i];
	break;
      }
  if (y == 0)
    {
      fputs ("test 3: couldn't get two consecutive pages, giving up\n", stdout);
      exit (65);
    }
  
  signal (SIGSEGV, sigsegv);
  if (setjmp (r))
    perror_exit ("test 3 fault", 8);

  y[0] = 1;
  y[pg] = 1;

  anonfree (y, pg * 2);

  if (setjmp (r) == 0)
    {
      y[0] = 1;
      perror_exit ("test 3 no fault 1", 9);
    }
  
  signal (SIGSEGV, sigsegv);
  if (setjmp (r) == 0)
    {
      y[pg] = 1;
      perror_exit ("test 3 no fault 2", 10);
    }
}

int
main ()
{
  anonmap_init();

  test_1();
  test_2();
  test_3();

  exit(0);
}

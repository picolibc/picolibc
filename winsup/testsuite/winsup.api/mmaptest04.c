#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif

int
main ()
{
  char *data, *data2 = NULL, *data3;
  int i, pagesize;
  int fd;

  pagesize = 65536;		//getpagesize();

  /*
   * First, make a file with some known garbage in it.
   */
  data = (char *) malloc (pagesize);
  if (!data)
    exit (1);
  for (i = 0; i < pagesize; ++i)
    *(data + i) = rand ();
  umask (0);
  fd = open ("conftestmmap", O_CREAT | O_TRUNC | O_WRONLY | O_BINARY, 0600);
  if (fd < 0)
    {
      printf ("creat: %d\n", errno);
      exit (1);
    }
  if (write (fd, data, pagesize) != pagesize)
    {
      printf ("write: %d\n", errno);
      exit (1);
    }
  close (fd);

  /*
   * Next, try to mmap the file.
   */
  fd = open ("conftestmmap", O_RDWR | O_BINARY);
  if (fd < 0)
    {
      printf ("write: %d\n", errno);
      exit (1);
    }
  if ((data2 = mmap (data2, pagesize, PROT_READ | PROT_WRITE,
		     MAP_PRIVATE, fd, 0L)) == MAP_FAILED)
    {
      printf ("mmap: %d\n", errno);
      exit (1);
    }
  for (i = 0; i < pagesize; ++i)
    if (*(data + i) != *(data2 + i))
      {
	printf ("check-if: %d\n", errno);
	exit (1);
      }

  /*
   * Finally, make sure that changes to the mapped area
   * do not percolate back to the file as seen by read().
   * (This is a bug on some variants of i386 svr4.0.)
   */
  for (i = 0; i < pagesize; ++i)
    *(data2 + i) = *(data2 + i) + 1;
  data3 = (char *) malloc (pagesize);
  if (!data3)
    {
      printf ("malloc2: %d\n", errno);
      exit (1);
    }
  if (read (fd, data3, pagesize) != pagesize)
    {
      printf ("read: %d\n", errno);
      exit (1);
    }
  for (i = 0; i < pagesize; ++i)
    if (*(data + i) != *(data3 + i))
      {
	printf ("check-if2: %d\n", errno);
	exit (1);
      }
  if (msync (data2, pagesize, MS_SYNC))
    {
      printf ("msync: %d\n", errno);
      exit (1);
    }
  if (munmap (data2, pagesize))
    {
      printf ("munmap: %d\n", errno);
      exit (1);
    }
  close (fd);
  unlink ("conftestmmap");
  exit (0);
}

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>

int
main(int argc, char **argv)
{
  int fd, r, l;
  char buf[1024];
  char *v;

  fd = open("/dev/zero", O_RDONLY);
  if (fd < 0)
    {
      fprintf(stderr, "Unable to open /dev/zero for reading\n");
      perror("The error was");
      exit(1);
    }

  l = read(fd, buf, 1024);
  if (l != 1024)
    {
      fprintf(stderr, "Asked to read 1024 bytes, got %d\n", l);
      exit(1);
    }

  for (r=0; r<1024; r++)
    if (buf[r] != 0)
      {
	fprintf(stderr, "/dev/zero returned a byte of %02x at offset %d\n",
		buf[r], r);
	exit(1);
      }

  l = lseek(fd, 4096, 0);
  if (l != 0)
    {
      fprintf(stderr, "l == %d\n", l);
      exit(1);
    }

  l = close(fd);
  if (l != 0)
    {
      fprintf(stderr, "close: returned %d\n", l);
      perror("The error was");
      exit(1);
    }

  fd = open("/dev/zero", O_WRONLY);
  if (fd < 0)
    {
      fprintf(stderr, "Unable to open /dev/zero for writing\n");
      perror("The error was");
      exit(1);
    }

  l = write(fd, buf, 1024);
  if (l != 1024)
    {
      fprintf(stderr, "Asked to write 1024 bytes, got %d\n", l);
      exit(1);
    }

  l = close(fd);
  if (l != 0)
    {
      fprintf(stderr, "close: returned %d\n", l);
      perror("The error was");
      exit(1);
    }

  fd = open("/dev/zero", O_RDWR);
  v = (char *)mmap(0, 65536, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (v == (char *)-1)
    {
      fprintf(stderr, "mmap r/w /dev/zero failed\n");
      perror("The error was");
      exit(1);
    }

  for (r=0; r<65536; r++)
    if (v[r] != 0)
      {
	fprintf(stderr, "mmap'd r/w /dev/zero has byte %d at offset %d\n",
		v[r], r);
	exit(1);
      }
  munmap(v, 65536);
  close(fd);

  fd = open("/dev/zero", O_RDONLY);
  v = (char *)mmap(0, 65536, PROT_READ, MAP_SHARED, fd, 0);
  if (v == (char *)-1)
    {
      fprintf(stderr, "mmap /dev/zero r/o failed\n");
      perror("The error was");
      exit(1);
    }

  for (r=0; r<65536; r++)
    if (v[r] != 0)
      {
	fprintf(stderr, "mmap'd r/o /dev/zero has byte %d at offset %d\n",
		v[r], r);
	exit(1);
      }
  munmap(v, 65536);
  close(fd);

  exit(0);
}

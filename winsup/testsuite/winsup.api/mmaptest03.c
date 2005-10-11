#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

/* - Checks if mapping of already closed file survives fork()
   - Checks if mapping the same region of the same file twice
     is done correctly.
*/

sigset_t unblock_sigsegv;
jmp_buf r;

static const char *msg;
static const char *what;
/* filler for file */
char const line[] = "y1 y1 y1 y1 y1 y1 y1 y1 y1 y1 y1 y1 y1 y1 y1 y1 y1";

void
perror_exit (const char *str)
{    
  printf ("%s: %s\n", str, strerror (errno));
  fflush (stdout);
  exit (1);
}

void
sigsegv (int unused)
{ 
  sigprocmask (SIG_UNBLOCK, &unblock_sigsegv, 0);
  if (msg)
    {
      char buf[132];
      sprintf (buf, "%s %s\n", what, msg);
      write (1, buf, strlen (buf));
      msg = NULL;
    }
  longjmp (r, 1);
}

int
main(int argc, char **argv)
{
  int fd, status;
  struct stat statbuf;
  volatile char c, *buf1, *buf2;
  pid_t pid;

  /* Create data file */
  if ((fd = open("y.txt", O_RDWR | O_CREAT | O_TRUNC, 0644)) == -1)
    perror_exit ("Can't create data file");
  write (fd, line, sizeof(line) - 1);
  close (fd);

  /* Open data file */
  if ((fd = open("y.txt", O_RDONLY)) == -1)
    perror_exit ("Can't open data file");

  if (fstat(fd, &statbuf) < 0)
    perror_exit ("fstat failed");

  if (!statbuf.st_size)
    perror_exit ("filesize is 0");

  if ((buf1 = mmap(NULL, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0))
      == MAP_FAILED)
    perror_exit ("mmap 1 failed");

  close(fd);

  /* Open data file a second time */
  if ((fd = open("y.txt", O_RDONLY)) == -1)
    perror_exit ("Can't open data file in second run");

  if ((buf2 = mmap(NULL, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0))
      == MAP_FAILED)
    perror_exit ("mmap 2 failed");

  close(fd);

  sigemptyset (&unblock_sigsegv);
  sigaddset (&unblock_sigsegv, SIGSEGV);
  signal (SIGSEGV, sigsegv);

  if (setjmp (r))
    perror_exit ("SEGV in fork");

  if ((pid = fork()))
    {
      // write (1, "continuing in parent\n", strlen ("continuing in parent\n"));
      what = "parent";
    }
  else
    {
      // write (1, "continuing in child\n", strlen ("continuing in child\n"));
      what = "child";
    }

  if (pid == -1)
    perror_exit ("fork failed");

  if (setjmp (r))
    perror_exit (pid ? "SEGV in parent" : "SEGV in child");

  msg = "testing buf1";
  c = buf1[0];
  msg = "testing buf2";
  c = buf2[0];

  if (setjmp (r))
    perror_exit (pid ? "SEGV in parent's munmap" : "SEGV in child's munmap");

  if (munmap((void *) buf1, statbuf.st_size))
    perror_exit (pid ? "munmap failed in parent" : "munmap failed in child");

  if (setjmp (r) == 0)
    {
      msg = "testing buf1 after unmap";
      c = buf1[0];
      perror_exit (pid ? "no SEGV in parent after munmap" : "no SEGV in child after munmap");
    }

  if (setjmp (r))
    perror_exit (pid ? "SEGV in parent after munmap" : "SEGV in child after munmap");

  msg = "testing buf2 again";
  c = buf2[0];

  if (setjmp (r))
    perror_exit (pid ? "SEGV in parent's munmap" : "SEGV in child's munmap");

  if (munmap((void *) buf2, statbuf.st_size))
    perror_exit (pid ? "munmap failed in parent" : "munmap failed in child");

  if (pid)
    {
      waitpid (pid, &status, 0);
      unlink ("y.txt");
      if (!WIFEXITED (status) || WEXITSTATUS (status))
	{
	  printf ("forked process exited with status %p\n", (char *) status);
	  return 1;
	}
    }

  return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

int
main (int argc, char **argv)
{
  int fd, pid, n;
  int fds[2];
  static char buf[4096];

  close (0);
  if ((fd = open ("/dev/null", O_WRONLY)) != 0)
    {
      fprintf (stderr, "couldn't redirect stdin to /dev/null, fd %d - %s\n", fd, strerror (errno));
      exit (1);
    }

  close (1);
  if ((fd = open ("/dev/null", O_WRONLY)) != 1)
    {
      fprintf (stderr, "couldn't redirect stdout to /dev/null, fd %d - %s\n", fd, strerror (errno));
      exit (1);
    }
  if (pipe (fds))
    {
      fprintf (stderr, "pipe call failed - %s\n", strerror (errno));
      exit (1);
    }
  if ((pid = fork ()) == 0)
    {
      close (fds[0]);
      if (dup2 (fds[1], 2) != 2)
	{
	  fprintf (stderr, "couldn't redirect stderr to pipe - %s\n", strerror (errno));
	  exit (1);
	}
      exit (system ("ls"));
    }
  else if (pid < 0)
    {
      perror ("couldn't fork");
      exit (1);
    }

  close (fds[1]);
  if (read (fds[0], buf, 4096) != 0)
    {
      fprintf (stderr, "system() call failed?\n%s\n", buf);
      exit (1);
    }

  if (waitpid (pid, &n, 0) < 0)
    {
      perror ("waitpid failed");
      exit (1);
    }
  if (n != 0)
    {
      fprintf (stderr, "system() call returned %p\n", (void *) n);
      exit (1);
    }
  exit (0);
}

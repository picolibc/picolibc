#include <stdio.h>
#include <sys/wait.h>
#include <errno.h>

int
main (int argc, char **argv)
{
  int pid, n;
  if ((pid = fork ()) == 0)
    exit (0);
  sleep (2);
  if ((n = waitpid (pid, NULL, 0)) != pid)
    {
      printf ("wait pid failed, pid %d, n %d, errno %d\n", pid, n, errno);
      exit(1);
    }
  else
    {
      printf ("wait pid succeeded, pid %d, n %d, errno %d\n", pid, n, errno);
      exit (0);
    }
}

#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int doit = 0;
void
ouch (int sig)
{
  fprintf (stderr, "ouch %d\n", sig);
  if (doit++ == 0)
    kill (getpid (), SIGTERM);
}

int
main (int argc, char **argv)
{
  static struct sigaction act;
  if (argc == 1)
    act.sa_flags = SA_RESETHAND;
  act.sa_handler = ouch;
  sigaction (SIGTERM, &act, NULL);
  int pid = fork ();
  int status;
  if (pid > 0)
    waitpid (pid, &status, 0);
  else
    {
      kill (getpid (), SIGTERM);
      exit (0x27);
    }
  fprintf (stderr, "pid %d exited with status %p\n", pid, status);
  exit (argc == 1 ? !(status == SIGTERM) : !(status == 0x2700));
}

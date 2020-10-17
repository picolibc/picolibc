#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int doit = 0;
void
ouch (int sig)
{
  if (doit++ > 0)
    kill (getpid (), SIGTERM);
}

int
main (int argc, char **argv)
{
  static struct rlimit nocore = { 0,0 };
  setrlimit(RLIMIT_CORE, &nocore);

  static struct sigaction act;
  if (argc == 1)
    act.sa_flags = SA_RESETHAND;
  act.sa_handler = ouch;
  sigaction (SIGSEGV, &act, NULL);
  int pid = fork ();
  int status;
  if (pid > 0)
    waitpid (pid, &status, 0);
  else
    {
      int *i = 0;
      *i = 9;
      exit (0x42);
    }
  status &= ~0x80;	// remove core dump flag
  printf ("pid %d exited with status %x\n", pid, status);
  exit (argc == 1 ? !(status == SIGSEGV) : !(status == SIGTERM));
}

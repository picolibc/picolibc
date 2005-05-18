/*
 * Test if signal is delivered to the application which is
 * currently inside of native syscall 
 */

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <windows.h>

int saw_sigchld = 0;
int sleep_stage = -1;

void
handle_child (int signo)
{
  printf ( "saw SIGCHLD, %d", sleep_stage);
  saw_sigchld = 1; 
}

int
main (int argc, char** argv)
{
  pid_t pid;
  if (argc > 1)
    {
      Sleep (200);
      return 0;
    }

  signal (SIGCHLD, handle_child);
  pid = fork ();
  if (pid < 0) 
    {
      perror ( "fork" );
      return 2;
    }
  else if (pid == 0)
    execl ( argv[0], argv[0], "child", NULL );
  else
    {
      sleep_stage = 0;
      Sleep (3000);
      sleep_stage = 1;
      sleep (10);
      sleep_stage = 2;
      if (!saw_sigchld)
        {
          printf ( "oops\n" );
          kill (pid, SIGTERM);
          return 1;
        }
      else
        return 0;
    }
    exit (0);
}

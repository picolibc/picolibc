#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

int no_signal_caught = 1;

void handler ( int signo )
{
  no_signal_caught = 0;
}

int
main()
{
  pid_t pid;
  signal ( SIGCHLD, handler );
  pid = fork();
  if ( pid == 0 ) exit ( 0 );
  sleep ( 2 );
  exit ( no_signal_caught );
}

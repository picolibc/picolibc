#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <stdlib.h>

static void * TestThread ( void * );

int main (void)
{
  pthread_t t;

  pthread_create (&t, NULL, TestThread, NULL);
  pthread_join (t, NULL);

  return 0;
}

static void * TestThread ( void *not_used )
{
  pthread_t iAm = pthread_self();
  int status;
  switch (fork ())
    {
    case -1:
      exit(1);
    case 0:
      if (iAm != pthread_self())
	  exit (1);
      else
	  exit (0);
      break;
    default:
      wait (&status);
      if (status != 0)
	  exit (1);
    }
  exit(0);
}

/*
The forked child will not get the same thread handle as its parent, it
will get the thread handle from the main thread instead. The child will
not terminate because the threadcount is still 2 after the fork (it is
set to 1 in MTinterface::Init and then set back to 2 after the childs
memory gets overwritten by the parent).

concept test by Thomas Pfaff <tpfaff@gmx.net>
scritable test by Robert Collins <rbtcollins@hotmail.com>
*/

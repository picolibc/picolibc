#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

static void * Thread (void *);

static pthread_t main_thread;
static pthread_t secondThread;
static int result = 2;

int main(void)
{
  main_thread = pthread_self ();

  if (pthread_create (&secondThread, NULL, Thread, NULL))
      exit (1);
  sleep (5);
  pthread_exit (&result);
  /* If pthread_exit doesm't (which would be a bug) then we do */
  return 1;
}

static void * Thread (void *not_used)
{
  void *myresult;
  /* We should be able to join this */
  if (pthread_join (main_thread, &myresult))
      exit (1);

  if (*(int *)myresult != 2)
      exit (1);

  exit (0);
}
/*
This valid code doesn't work at all. The mainthread object in MTinterface
is not properly initialized, the cancel_event is NULL and the win32_obj_id
is NULL because myself->hProcess is NULL when MTinterface is initialized
(and i don't think that a process handle can be used as thread handle).
Even if the handles would be valid the pthread_join call would try to
delete a thread object that is created static which would result in a
corrupted heap.

Concept test Contributed by Thomas Pfaff <tpfaff@gmx.net>
Scriptable test by Robert Collins <rbtcollins@hotmail.com>

*/

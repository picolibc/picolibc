/* 
 * mutex1e.c
 *
 * As for mutex1.c but with type set to PTHREAD_MUTEX_ERRORCHECK.
 *
 * Create a simple mutex object, lock it, unlock it, then destroy it.
 * This is the simplest test of the pthread mutex family that we can do.
 *
 * Depends on API functions:
 *	pthread_mutexattr_settype()
 * 	pthread_mutex_init()
 *	pthread_mutex_destroy()
 */

#include "test.h"

pthread_mutex_t mutex = NULL;
pthread_mutexattr_t mxAttr;

int
main()
{
  assert(pthread_mutexattr_init(&mxAttr) == 0);

  assert(pthread_mutexattr_settype(&mxAttr, PTHREAD_MUTEX_ERRORCHECK) == 0);

  assert(mutex == NULL);

  assert(pthread_mutex_init(&mutex, &mxAttr) == 0);

  assert(mutex != NULL);

  assert(pthread_mutex_lock(&mutex) == 0);

  assert(pthread_mutex_unlock(&mutex) == 0);

  assert(pthread_mutex_destroy(&mutex) == 0);

  assert(mutex == NULL);

  return 0;
}

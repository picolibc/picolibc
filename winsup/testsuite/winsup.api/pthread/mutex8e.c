/* 
 * mutex8e.c
 *
 * Tests PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP.
 *
 * Depends on API functions: 
 *	pthread_mutex_lock()
 *	pthread_mutex_unlock()
 */

#include "test.h"
 
pthread_mutex_t mutex = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;

int
main()
{
  assert(mutex == PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP);

  assert(pthread_mutex_lock(&mutex) == 0);

  assert(mutex != PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP);

  assert(mutex != NULL);

  assert(pthread_mutex_lock(&mutex) == EDEADLK);

  assert(pthread_mutex_unlock(&mutex) == 0);

  assert(pthread_mutex_destroy(&mutex) == 0);

  assert(mutex == NULL);

  return 0;
}

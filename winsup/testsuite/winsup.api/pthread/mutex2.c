/* 
 * mutex2.c
 *
 * Declare a static mutex object, lock it, 
 * and then unlock it again.
 *
 * Depends on API functions: 
 *	pthread_mutex_lock()
 *	pthread_mutex_unlock()
 */

#include "test.h"
 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int
main()
{
  assert(mutex == PTHREAD_MUTEX_INITIALIZER);

  assert(pthread_mutex_lock(&mutex) == 0);

  assert(mutex != PTHREAD_MUTEX_INITIALIZER);

  assert(mutex != NULL);

  assert(pthread_mutex_unlock(&mutex) == 0);

  assert(pthread_mutex_destroy(&mutex) == 0);

  assert(mutex == NULL);

  return 0;
}

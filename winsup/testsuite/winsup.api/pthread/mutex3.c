/* 
 * mutex3.c
 *
 * Declare a static mutex object, lock it, trylock it, 
 * and then unlock it again.
 *
 * Depends on API functions: 
 *	pthread_mutex_lock()
 *	pthread_mutex_trylock()
 *	pthread_mutex_unlock()
 */

#include "test.h"
 
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

static int washere = 0;

void * func(void * arg)
{
  assert(pthread_mutex_trylock(&mutex1) == EBUSY);

  washere = 1;

  return 0; 
}
 
int
main()
{
  pthread_t t;

  assert(pthread_mutex_lock(&mutex1) == 0);

  assert(pthread_create(&t, NULL, func, NULL) == 0);
  assert(pthread_join(t, NULL) == 0);

  assert(pthread_mutex_unlock(&mutex1) == 0);

  assert(washere == 1);

  return 0;
}

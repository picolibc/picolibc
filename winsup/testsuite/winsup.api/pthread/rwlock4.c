/* 
 * rwlock4.c
 *
 * Declare a static rwlock object, rdlock it, trywrlock it, 
 * and then unlock it again.
 *
 * Depends on API functions: 
 *	pthread_rwlock_rdlock()
 *	pthread_rwlock_trywrlock()
 *	pthread_rwlock_unlock()
 */

#include "test.h"
 
pthread_rwlock_t rwlock1 = PTHREAD_RWLOCK_INITIALIZER;

static int washere = 0;

void * func(void * arg)
{
  assert(pthread_rwlock_trywrlock(&rwlock1) == EBUSY);

  washere = 1;

  return 0; 
}
 
int
main()
{
  pthread_t t;

  assert(pthread_rwlock_rdlock(&rwlock1) == 0);

  assert(pthread_create(&t, NULL, func, NULL) == 0);

  Sleep(2000);

  assert(pthread_rwlock_unlock(&rwlock1) == 0);

  assert(washere == 1);

  return 0;
}

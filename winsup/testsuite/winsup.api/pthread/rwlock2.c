/* 
 * rwlock2.c
 *
 * Declare a static rwlock object, lock it, 
 * and then unlock it again.
 *
 * Depends on API functions: 
 *	pthread_rwlock_rdlock()
 *	pthread_rwlock_unlock()
 */

#include "test.h"
 
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

int
main()
{
  assert(rwlock == PTHREAD_RWLOCK_INITIALIZER);

  assert(pthread_rwlock_rdlock(&rwlock) == 0);

  assert(rwlock != PTHREAD_RWLOCK_INITIALIZER);

  assert(rwlock != NULL);

  assert(pthread_rwlock_unlock(&rwlock) == 0);

  assert(pthread_rwlock_destroy(&rwlock) == 0);

  assert(rwlock == NULL);

  return 0;
}

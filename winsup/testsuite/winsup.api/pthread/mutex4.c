/* 
 * mutex4.c
 *
 * Thread A locks mutex - thread B tries to unlock.
 *
 * Depends on API functions: 
 *	pthread_mutex_lock()
 *	pthread_mutex_trylock()
 *	pthread_mutex_unlock()
 */

#include "test.h"

static int wasHere = 0;

static pthread_mutex_t mutex1;
 
void * unlocker(void * arg)
{
  int expectedResult = (int) arg;

  wasHere++;
  assert(pthread_mutex_unlock(&mutex1) == expectedResult);
  wasHere++;
  return NULL;
}
 
int
main()
{
  pthread_t t;
  pthread_mutexattr_t ma;

  assert(pthread_mutexattr_init(&ma) == 0);

  wasHere = 0;
  assert(pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_ERRORCHECK) == 0);
  assert(pthread_mutex_init(&mutex1, &ma) == 0);
  assert(pthread_mutex_lock(&mutex1) == 0);
  assert(pthread_create(&t, NULL, unlocker, (void *) EPERM) == 0);
  assert(pthread_join(t, NULL) == 0);
  assert(pthread_mutex_unlock(&mutex1) == 0);
  assert(pthread_mutex_destroy(&mutex1) == 0);
  assert(wasHere == 2);

  wasHere = 0;
  assert(pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_ERRORCHECK) == 0);
  assert(pthread_mutex_init(&mutex1, &ma) == 0);
  assert(pthread_mutex_lock(&mutex1) == 0);
  assert(pthread_create(&t, NULL, unlocker, (void *) EPERM) == 0);
  assert(pthread_join(t, NULL) == 0);
  assert(pthread_mutex_unlock(&mutex1) == 0);
  assert(pthread_mutex_destroy(&mutex1) == 0);
  assert(wasHere == 2);

  wasHere = 0;
  assert(pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE) == 0);
  assert(pthread_mutex_init(&mutex1, &ma) == 0);
  assert(pthread_mutex_lock(&mutex1) == 0);
  assert(pthread_create(&t, NULL, unlocker, (void *) EPERM) == 0);
  assert(pthread_join(t, NULL) == 0);
  assert(pthread_mutex_unlock(&mutex1) == 0);
  assert(pthread_mutex_destroy(&mutex1) == 0);
  assert(wasHere == 2);

  return 0;
}

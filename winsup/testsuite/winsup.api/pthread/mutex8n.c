/* 
 * mutex8n.c
 *
 * Tests PTHREAD_NORMAL_MUTEX_INITIALIZER_NP.
 * Thread locks mutex twice (recursive lock).
 * The thread should deadlock.
 *
 * Depends on API functions: 
 *      pthread_create()
 *      pthread_mutex_init()
 *	pthread_mutex_lock()
 *	pthread_mutex_unlock()
 */

#include "test.h"

static int lockCount = 0;

pthread_mutex_t mutex = PTHREAD_NORMAL_MUTEX_INITIALIZER_NP;

void * locker(void * arg)
{
  assert(pthread_mutex_lock(&mutex) == 0);
  lockCount++;

  /* Should wait here (deadlocked) */
  assert(pthread_mutex_lock(&mutex) == 0);
  lockCount++;
  assert(pthread_mutex_unlock(&mutex) == 0);

  return (void *) 555;
}
 
int
main()
{
  pthread_t t;

  assert(pthread_create(&t, NULL, locker, NULL) == 0);

  Sleep(1000);

  assert(lockCount == 1);

  /*
   * Should succeed even though we don't own the lock
   * because FAST mutexes don't check ownership.
   */
  assert(pthread_mutex_unlock(&mutex) == 0);

  Sleep (1000);

  assert(lockCount == 2);

  exit(0);

  /* Never reached */
  return 0;
}


/*
 * File: condvar3.c
 *
 * Test Synopsis:
 * - Test basic function of a CV
 *
 * Test Method (Validation or Falsification):
 * - Validation
 *
 * Requirements Tested:
 * - 
 *
 * Features Tested:
 * - 
 *
 * Cases Tested:
 * - 
 *
 * Description:
 * - The primary thread takes the lock before creating any threads.
 *   The secondary thread blocks on the lock allowing the primary
 *   thread to enter the cv wait state which releases the lock.
 *   The secondary thread then takes the lock and signals the waiting
 *   primary thread.
 *
 * Environment:
 * - 
 *
 * Input:
 * - None.
 *
 * Output:
 * - File name, Line number, and failed expression on failure.
 * - No output on success.
 *
 * Assumptions:
 * - 
 *
 * Pass Criteria:
 * - pthread_cond_timedwait returns 0.
 * - Process returns zero exit status.
 *
 * Fail Criteria:
 * - pthread_cond_timedwait returns ETIMEDOUT.
 * - Process returns non-zero exit status.
 */

#include "test.h"
#include <sys/timeb.h>

static pthread_cond_t cv;
static pthread_mutex_t mutex;
static int shared = 0;

enum {
  NUMTHREADS = 2         /* Including the primary thread. */
};

void *
mythread(void * arg)
{
  int result = 0;

  assert(pthread_mutex_lock(&mutex) == 0);

  shared++;

  assert(pthread_mutex_unlock(&mutex) == 0);

  if ((result = pthread_cond_signal(&cv)) != 0)
    {
      printf("Error = %s\n", error_string[result]);
    }
  assert(result == 0);

  return (void *) 0;
}

int
main()
{
  pthread_t t[NUMTHREADS];
  struct timespec abstime = { 0, 0 };
  struct timeb currSysTime;
  const DWORD NANOSEC_PER_MILLISEC = 1000000;

  assert((t[0] = pthread_self()) != NULL);

  assert(pthread_cond_init(&cv, NULL) == 0);

  assert(pthread_mutex_init(&mutex, NULL) == 0);

  assert(pthread_mutex_lock(&mutex) == 0);

  /* get current system time */
  ftime(&currSysTime);

  abstime.tv_sec = currSysTime.time;
  abstime.tv_nsec = NANOSEC_PER_MILLISEC * currSysTime.millitm;

  assert(pthread_create(&t[1], NULL, mythread, (void *) 1) == 0);

  abstime.tv_sec += 5;

  while (! (shared > 0))
    assert(pthread_cond_timedwait(&cv, &mutex, &abstime) == 0);

  assert(shared > 0);

  assert(pthread_mutex_unlock(&mutex) == 0);

  assert(pthread_cond_destroy(&cv) == 0);

  return 0;
}

/*
 * File: condvar3_2.c
 *
 * Test Synopsis:
 * - Test timeout of multiple waits on a CV with remainder broadcast awoken.
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
 * - Because some CVs are never signaled, we expect their waits to time out.
 *   Some time out, the rest are broadcast signaled. Pthread_cond_destroy() will fail
 *   unless all are accounted for, either signaled or timedout.
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
 * - pthread_cond_timedwait returns ETIMEDOUT.
 * - Process returns zero exit status.
 *
 * Fail Criteria:
 * - pthread_cond_timedwait does not return ETIMEDOUT.
 * - Process returns non-zero exit status.
 */

#include "test.h"
#include <sys/timeb.h>

static pthread_cond_t cv;
static pthread_mutex_t mutex;
static struct timespec abstime = { 0, 0 };
static struct timespec abstime2 = { 0, 0 };
static int timedout = 0;
static int awoken = 0;

enum {
  NUMTHREADS = 60
};

void *
mythread(void * arg)
{
  int result;

  assert(pthread_mutex_lock(&mutex) == 0);

  abstime2.tv_sec = abstime.tv_sec;

  if ((int) arg % 3 == 0)
    {
      abstime2.tv_sec += 2;
    }

  result = pthread_cond_timedwait(&cv, &mutex, &abstime2);
  if (result == ETIMEDOUT)
    {
      timedout++;
    }
  else
    {
      awoken++;
    }

  assert(pthread_mutex_unlock(&mutex) == 0);

  return arg;
}

int
main()
{
  int i;
  pthread_t t[NUMTHREADS + 1];
  int result = 0;
  struct timeb currSysTime;
  const DWORD NANOSEC_PER_MILLISEC = 1000000;

  assert(pthread_cond_init(&cv, NULL) == 0);

  assert(pthread_mutex_init(&mutex, NULL) == 0);

  /* get current system time */
  ftime(&currSysTime);

  abstime.tv_sec = abstime.tv_sec = currSysTime.time + 5;
  abstime.tv_nsec = abstime2.tv_nsec = NANOSEC_PER_MILLISEC * currSysTime.millitm;

  assert(pthread_mutex_lock(&mutex) == 0);

  for (i = 1; i <= NUMTHREADS; i++)
    {
      assert(pthread_create(&t[i], NULL, mythread, (void *) i) == 0);
    }

  assert(pthread_mutex_unlock(&mutex) == 0);

  for (i = 1; i <= NUMTHREADS; i++)
    {
      assert(pthread_join(t[i], (void **) &result) == 0);
	assert(result == i);
      /*
       * Approximately 2/3rds of the threads are expected to time out.
       * Signal the remainder after some threads have woken up and exited
       * and while some are still waking up after timeout.
       * Also tests that redundant broadcasts don't return errors.
       */
      if (awoken > NUMTHREADS/3)
        {
          assert(pthread_cond_broadcast(&cv) == 0);
        }
    }

  assert(awoken == NUMTHREADS - timedout);

  result = pthread_cond_destroy(&cv);
  assert(result == 0);

  return 0;
}

/*
 * File: condvar3_1.c
 *
 * Test Synopsis:
 * - Test timeout of multiple waits on a CV with some signaled.
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
 *   Some are signaled, the rest time out. Pthread_cond_destroy() will fail
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
static pthread_cond_t cv1;
static pthread_mutex_t mutex;
static struct timespec abstime = { 0, 0 };
static int timedout = 0;
static int signaled = 0;
static int awoken = 0;
static int waiting = 0;

enum {
  NUMTHREADS = 60
};

void *
mythread(void * arg)
{
  int result;

  assert(pthread_mutex_lock(&mutex) == 0);

  if ( ++waiting == NUMTHREADS)
    assert(pthread_cond_signal(&cv1) == 0);

  result = pthread_cond_timedwait(&cv, &mutex, &abstime);
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
  assert(pthread_cond_init(&cv1, NULL) == 0);

  assert(pthread_mutex_init(&mutex, NULL) == 0);

  /* get current system time */
  ftime(&currSysTime);

  abstime.tv_sec = currSysTime.time;
  abstime.tv_nsec = NANOSEC_PER_MILLISEC * currSysTime.millitm;

  abstime.tv_sec += 5;

  assert(pthread_mutex_lock(&mutex) == 0);

  for (i = 1; i <= NUMTHREADS; i++)
    {
      assert(pthread_create(&t[i], NULL, mythread, (void *) i) == 0);
    }

  do {
    assert(pthread_cond_wait(&cv1,&mutex) == 0);
  } while ( NUMTHREADS != waiting );

  assert(pthread_mutex_unlock(&mutex) == 0);

  for (i = NUMTHREADS/3; i <= 2*NUMTHREADS/3; i++)
    {
      assert(pthread_cond_signal(&cv) == 0);
      signaled++;
    }

  for (i = 1; i <= NUMTHREADS; i++)
    {
      assert(pthread_join(t[i], (void **) &result) == 0);
        assert(result == i);
    }

  printf("awk = %d\n", awoken);
  printf("sig = %d\n", signaled);
  printf("tot = %d\n", timedout);

  assert(signaled == awoken);
  assert(timedout == NUMTHREADS - signaled);

  result = pthread_cond_destroy(&cv);
  assert(result == 0);

  return 0;
}

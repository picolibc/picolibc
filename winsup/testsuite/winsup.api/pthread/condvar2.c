/*
 * File: condvar2.c
 *
 * Test Synopsis:
 * - Test timed wait on a CV.
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
 * - Because the CV is never signaled, we expect the wait to time out.
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

pthread_cond_t cv;
pthread_mutex_t mutex;

int
main()
{
  struct timespec abstime = { 0, 0 };
  struct timeb currSysTime;
  const DWORD NANOSEC_PER_MILLISEC = 1000000;
  int result;

  assert(pthread_cond_init(&cv, NULL) == 0);

  assert(pthread_mutex_init(&mutex, NULL) == 0);

  assert(pthread_mutex_lock(&mutex) == 0);

  /* get current system time */
  ftime(&currSysTime);

  abstime.tv_sec = currSysTime.time;
  abstime.tv_nsec = NANOSEC_PER_MILLISEC * currSysTime.millitm;

  abstime.tv_sec += 5;

  assert(pthread_cond_timedwait(&cv, &mutex, &abstime) == ETIMEDOUT);
  
  assert(pthread_mutex_unlock(&mutex) == 0);

  result = pthread_cond_destroy(&cv);
  assert(result == 0);

  return 0;
}

/*
 * File: condvar3_3.c
 *
 * Test Synopsis:
 * - Test timeouts and lost signals on a CV.
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
 * -
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

/* Timur Aydin (taydin@snet.net) */

#include "test.h"

#include <sys/timeb.h>

pthread_cond_t cnd;
pthread_mutex_t mtx;

int main()
{
   int rc;

   struct timespec abstime = { 0, 0 };
   struct timeb currSysTime;
   const DWORD NANOSEC_PER_MILLISEC = 1000000;

   assert(pthread_cond_init(&cnd, 0) == 0);
   assert(pthread_mutex_init(&mtx, 0) == 0);

   /* get current system time */
   ftime(&currSysTime);

   abstime.tv_sec = currSysTime.time;
   abstime.tv_nsec = NANOSEC_PER_MILLISEC * currSysTime.millitm;
   abstime.tv_sec += 1;

   /* Here pthread_cond_timedwait should time out after one second. */

   assert(pthread_mutex_lock(&mtx) == 0);

   assert((rc = pthread_cond_timedwait(&cnd, &mtx, &abstime)) == ETIMEDOUT);

   assert(pthread_mutex_unlock(&mtx) == 0);

   /* Here, the condition variable is signaled, but there are no
      threads waiting on it. The signal should be lost and
      the next pthread_cond_timedwait should time out too. */

   assert(pthread_mutex_lock(&mtx) == 0);

   assert((rc = pthread_cond_signal(&cnd)) == 0);

   assert(pthread_mutex_unlock(&mtx) == 0);

   assert(pthread_mutex_lock(&mtx) == 0);

   abstime.tv_sec = currSysTime.time;
   abstime.tv_nsec = NANOSEC_PER_MILLISEC * currSysTime.millitm;
   abstime.tv_sec += 1;

   assert((rc = pthread_cond_timedwait(&cnd, &mtx, &abstime)) == ETIMEDOUT);

   assert(pthread_mutex_unlock(&mtx) == 0);

   return 0;
}

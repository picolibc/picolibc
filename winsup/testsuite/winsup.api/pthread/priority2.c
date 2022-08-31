/*
 * File: priority2.c
 *
 * Test Synopsis:
 * - Test thread priority setting after creation.
 *
 * Test Method (Validation or Falsification):
 * - 
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
 * - Process returns zero exit status.
 *
 * Fail Criteria:
 * - Process returns non-zero exit status.
 */

#include "test.h"

pthread_mutex_t startMx = PTHREAD_MUTEX_INITIALIZER;

void * func(void * arg)
{
  int policy;
  struct sched_param param;

  assert(pthread_mutex_lock(&startMx) == 0);
  assert(pthread_getschedparam(pthread_self(), &policy, &param) == 0);
  assert(pthread_mutex_unlock(&startMx) == 0);
  assert(policy == SCHED_FIFO);
  return (void *) (size_t)param.sched_priority;
}
 
int
main()
{
  pthread_t t;
  void * result = NULL;
  struct sched_param param;
  int maxPrio = sched_get_priority_max(SCHED_FIFO);
  int minPrio = sched_get_priority_min(SCHED_FIFO);

  for (param.sched_priority = minPrio;
       param.sched_priority <= maxPrio;
       param.sched_priority++)
    {
      assert(pthread_mutex_lock(&startMx) == 0);
      assert(pthread_create(&t, NULL, func, NULL) == 0);
      assert(pthread_setschedparam(t, SCHED_FIFO, &param) == 0);
      assert(pthread_mutex_unlock(&startMx) == 0);
      pthread_join(t, &result);
      assert((int)(size_t)result == param.sched_priority);
    }

  return 0;
}

/*
 * File: priority1.c
 *
 * Test Synopsis:
 * - Test thread priority explicit setting using thread attribute.
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

void * func(void * arg)
{
  int policy;
  struct sched_param param;

  assert(pthread_getschedparam(pthread_self(), &policy, &param) == 0);
  assert(policy == SCHED_OTHER);
  return (void *) param.sched_priority;
}
 
int
main()
{
  pthread_t t;
  pthread_attr_t attr;
  void * result = NULL;
  struct sched_param param;
  int maxPrio = sched_get_priority_max(SCHED_OTHER);
  int minPrio = sched_get_priority_min(SCHED_OTHER);

  assert(pthread_attr_init(&attr) == 0);
  assert(pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) == 0);

  for (param.sched_priority = minPrio;
       param.sched_priority <= maxPrio;
       param.sched_priority++)
    {
      assert(pthread_attr_setschedparam(&attr, &param) == 0);
      assert(pthread_create(&t, &attr, func, NULL) == 0);
      pthread_join(t, &result);
      assert((int) result == param.sched_priority);
    }

  return 0;
}

/*
 * File: inherit1.c
 *
 * Test Synopsis:
 * - Test thread priority inheritance.
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
  return (void *) (size_t)param.sched_priority;
}

// Windows only supports 7 thread priority levels, which we map onto the 32
// required by POSIX.  The exact mapping also depends on the overall process
// priority class. So only a subset of values will be returned exactly by
// pthread_getschedparam() after pthread_setschedparam().
int doable_pri(int pri)
{
  switch (GetPriorityClass(GetCurrentProcess()))
    {
    case BELOW_NORMAL_PRIORITY_CLASS:
      return (pri == 2) || (pri ==  8) || (pri == 10) || (pri == 12) || (pri == 14) || (pri == 16) || (pri == 30);
    case NORMAL_PRIORITY_CLASS:
      return (pri == 2) || (pri == 12) || (pri == 14) || (pri == 16) || (pri == 18) || (pri == 20) || (pri == 30);
    }

  return TRUE;
}

int
main()
{
  pthread_t t;
  pthread_t mainThread = pthread_self();
  pthread_attr_t attr;
  void * result = NULL;
  struct sched_param param;
  struct sched_param mainParam;
  int maxPrio;
  int minPrio;
  int prio;
  int policy;
  int inheritsched = -1;

  assert((maxPrio = sched_get_priority_max(SCHED_FIFO)) != -1);
  assert((minPrio = sched_get_priority_min(SCHED_FIFO)) != -1);

  assert(pthread_attr_init(&attr) == 0);
  assert(pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED) == 0);
  assert(pthread_attr_getinheritsched(&attr, &inheritsched) == 0);
  assert(inheritsched == PTHREAD_INHERIT_SCHED);

  for (prio = minPrio; prio < maxPrio; prio++)
    {
      mainParam.sched_priority = prio;

      /* Change the main thread priority */
      assert(pthread_setschedparam(mainThread, SCHED_FIFO, &mainParam) == 0);
      assert(pthread_getschedparam(mainThread, &policy, &mainParam) == 0);
      assert(policy == SCHED_FIFO);

      if (doable_pri(prio))
        assert(mainParam.sched_priority == prio);

      for (param.sched_priority = prio;
           param.sched_priority <= maxPrio;
           param.sched_priority++)
        {
          /* The new thread create should ignore this new priority */
          assert(pthread_attr_setschedparam(&attr, &param) == 0);
          assert(pthread_create(&t, &attr, func, NULL) == 0);
          pthread_join(t, &result);
          assert((int)(size_t) result == mainParam.sched_priority);
        }
    }

  return 0;
}

/*
 * File: create2.c
 *
 * Test Synopsis:
 * - Test that threads have a Win32 handle when started.
 *
 * Test Method (Validation or Falsification):
 * - Statistical, not absolute (depends on sample size).
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

const int NUMTHREADS = 10000;

static int washere = 0;

void * func(void * arg)
{
  washere = 1;
  return (void *) 0; 
}
 
int
main()
{
  pthread_t t;
  pthread_attr_t attr;
  void * result = NULL;
  int i;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  for (i = 0; i < NUMTHREADS; i++)
    {
      washere = 0;
      assert(pthread_create(&t, &attr, func, NULL) == 0);
      pthread_join(t, &result);
      assert(washere == 1);
    }

  return 0;
}

/*
 * File: cancel7.c
 *
 * Test Synopsis: Test if sleep is a cancellation point.
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
 * - have working pthread_create, pthread_cancel, pthread_setcancelstate
 *   pthread_join
 *
 * Pass Criteria:
 * - Process returns zero exit status.
 *
 * Fail Criteria:
 * - Process returns non-zero exit status.
 */

#include "test.h"

static int has5secsleeped = 0;

static void *Thread(void *punused)
{
  assert (pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, NULL) == 0);
  /* thread should sleep 5 seconds and not get canceled */
  sleep(5);
  has5secsleeped = 1;
  assert (pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL) == 0);
  /* thread should cancel here */
  sleep (5);

  return NULL;
}

int main (void)
{
  void * result;
  pthread_t t;

  assert (pthread_create (&t, NULL, Thread, NULL) == 0);
  assert (pthread_cancel (t) == 0);
  assert (pthread_join (t, &result) == 0);
  assert (has5secsleeped == 1);
  assert (result == PTHREAD_CANCELED);

  return 0;
}

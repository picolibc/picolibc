/*
 * File: cancel6.c
 *
 * Test Synopsis: Test if pause is a cancellation point.
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

static void *Thread(void *punused)
{
  pause ();

  return NULL;
}

int main (void)
{
  void * result;
  pthread_t t;

  assert (pthread_create (&t, NULL, Thread, NULL) == 0);
  assert (pthread_cancel (t) == 0);
  assert (pthread_join (t, &result) == 0);
  assert (result == PTHREAD_CANCELED);

  return 0;
}

/*
 * File: cancel11.c
 *
 * Test Synopsis: Test if system is a cancellation point.
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

static void sig_handler(int sig)
{
}

static void *Thread(void *punused)
{
  system ("sleep 10");

  return NULL;
}

int main (void)
{
  void * result;
  pthread_t t;

  signal (SIGINT, sig_handler);

  assert (pthread_create (&t, NULL, Thread, NULL) == 0);
  sleep (5);
  assert (pthread_cancel (t) == 0);
  assert (pthread_join (t, &result) == 0);
  assert (result == PTHREAD_CANCELED);

  assert ((void *)signal (SIGINT, NULL) == sig_handler);

  /* Wait until child process has terminated */
  sleep (10);

  return 0;
}

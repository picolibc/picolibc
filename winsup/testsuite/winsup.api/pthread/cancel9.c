/*
 * File: cancel9.c
 *
 * Test Synopsis: Test if waitpid is a cancellation point.
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
#include <assert.h>
#include <sys/wait.h>

static pid_t pid;

static void *Thread(void *punused)
{
  int res;

  pid = fork ();
  assert (pid != -1);
  switch (pid)
    {
    case 0:
      sleep (10);
      break;
    default:
      assert (waitpid (pid, &res, 0) != -1);
    }

  return NULL;
}

int main (void)
{
  int res;

  void * result;
  pthread_t t;

  assert (pthread_create (&t, NULL, Thread, NULL) == 0);
  sleep (5);
  assert (pthread_cancel (t) == 0);
  assert (pthread_join (t, &result) == 0);
  assert (result == PTHREAD_CANCELED);
  assert (waitpid (pid, &res, 0) != -1);

  return 0;
}
